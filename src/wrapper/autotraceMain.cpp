//
// Created by Nick Winder on 6/9/18.
//

#include "FittingOptionsBuidler.h"
#include "InputOptionsBuilder.h"
#include "OutputOptionsBuilder.h"

#include <src/wrapper/Autotrace.h>

#include <iostream>
#include <fstream>

using namespace json11;

namespace {
const std::string assetPath = ASSET_PATH;        // NOLINT
const auto inputPath = assetPath + "Test.png";   // NOLINT
const auto outputPath = assetPath + "test.svg";  // NOLINT
}

struct Result {
  bool success;
  std::string error;
};

struct ResultULongLong {
  bool success;
  std::string error;
  unsigned long value;
};

struct ResultVectorUint8 {
  bool success;
  std::string error;
  std::vector<uint8_t> value;
};

Result autotraceRun(const std::vector<uint8_t> &inputBuffer,
                    const std::string &fittingOptionsJson,
                    const std::string &inputOptionsJson,
                    const std::string &outputOptionsJson) {
  std::string jsonError;
  auto jsonResult = Json::parse(fittingOptionsJson, jsonError, STANDARD);
  if (!jsonError.empty()) {
    return {false, "Fitting options json error : " + jsonError};
  }
  FittingOptions fittingOptions{jsonResult};

  jsonResult = Json::parse(inputOptionsJson, jsonError, STANDARD);
  if (!jsonError.empty()) {
    return {false, "Fitting options json error : " + jsonError};
  }
  InputOptions inputOptions{jsonResult};

  jsonResult = Json::parse(outputOptionsJson, jsonError, STANDARD);
  if (!jsonError.empty()) {
    return {false, "Fitting options json error : " + jsonError};
  }
  OutputOptions outputOptions{jsonResult};

  std::ofstream inputFile(inputPath, std::ofstream::binary);
  inputFile.write(reinterpret_cast<const char *>(inputBuffer.data()),
                  inputBuffer.size());
  inputFile.close();

  Options options{fittingOptions, inputOptions, outputOptions};
  Autotrace autotrace{inputPath, outputPath, options};
  const auto outputResult = autotrace.produceOutput();

  return {outputResult.has_value(), outputResult ? "" : outputResult.error()};
}

ResultULongLong outputFileSize() {
  std::ifstream in(outputPath, std::ifstream::ate | std::ifstream::binary);
  return {true, "", static_cast<unsigned long>(in.tellg())};
}

ResultVectorUint8 getOutputFile() {
  std::ifstream in(outputPath, std::ifstream::binary);
  std::vector<uint8_t> outputBuffer((std::istreambuf_iterator<char>(in)),
                                 std::istreambuf_iterator<char>());

  return {true, "", outputBuffer};
}

#ifdef EMSCRIPTEN
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(autotraceCpp) {
  register_vector<uint8_t>("VectorUint8");

  value_object<Result>("Result")
      .field("success", &Result::success)
      .field("error", &Result::error);

  value_object<ResultULongLong>("ResultULongLong")
      .field("success", &ResultULongLong::success)
      .field("error", &ResultULongLong::error)
      .field("value", &ResultULongLong::value);

  value_object<ResultVectorUint8>("ResultVectorUint8")
      .field("success", &ResultVectorUint8::success)
      .field("error", &ResultVectorUint8::error)
      .field("value", &ResultVectorUint8::value);

  function("autotraceRun", &autotraceRun);
  function("getOutputFile", &getOutputFile);
  function("outputFileSize", &outputFileSize);
}
#else

int main() {
  const FittingOptions fittingOptions =
      FittingOptionsBuilder::builder().build();
  const InputOptions inputOptions = InputOptionsBuilder::builder().build();
  const OutputOptions outputOptions = OutputOptionsBuilder::builder().build();

  Options options{fittingOptions, inputOptions, outputOptions};
  Autotrace autotrace{inputPath, outputPath, options};
  const auto outputResult = autotrace.produceOutput();
  if (!outputResult.has_value()) {
    std::cout << outputResult.error();
    return 1;
  }
  return 0;
}

#endif
