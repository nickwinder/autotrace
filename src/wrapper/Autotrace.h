//
// Created by Nick Winder on 6/10/18.
//

#pragma once

#include "Options.h"
#include <src/bitmap.h>

#include "Expected.h"

#include <string>

class Autotrace {
public:
  Autotrace(std::string inputFile, std::string outputFile, Options options);

  nonstd::expected<void, std::string> produceOutput();

private:
  const std::string inputFile;
  const std::string outputFilePath;

  const Options options;
};

