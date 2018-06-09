/* output-p2e.c: utility routines for pstoedit intermediate output

   Copyright (C) 2000-2001 Wolfgang Glunz, Martin Weber

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA. */

/* Initial author: Wolfgang Glunz - wglunz@pstoedit.net
   use the generated file to run "pstoedit -bo -f format input output"
   Adapted to pstoedit 3.30 by Wolfgang Glunz. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* Def: HAVE_CONFIG_H */

#include "types.h"
#include "spline.h"
#include "color.h"
#include "output-p2e.h"
#include <math.h>

/* Output macros.  */

/* This should be used for outputting a string S on a line by itself.  */
#define OUT_LINE(l)							\
  fprintf (ps_file, "%s\n", l)

/* These output their arguments, preceded by the indentation.  */
#define OUT(...)							\
  fprintf (ps_file, __VA_ARGS__)

/* These macros just output their arguments.  */
#define OUT_REAL(r)	fprintf (ps_file,				\
                                 r == lround (r) ? "%.0f " : "%.3f ", r)

/* For a PostScript command with two real arguments, e.g., lineto.  OP
   should be a constant string.  */
#define OUT_COMMAND2(first, second, op)					\
  do									\
	{								\
      OUT (" ");							\
      OUT_REAL (first);							\
      OUT_REAL (second);						\
      OUT (op "\n");						\
    }									\
  while (0)

/* For a PostScript command with six real arguments, e.g., curveto.
   Again, OP should be a constant string.  */
#define OUT_COMMAND6(first, second, third, fourth, fifth, sixth, op)	\
  do									\
    {									\
      OUT (" ");							\
      OUT_REAL (first);							\
      OUT_REAL (second);						\
      OUT (" ");							\
      OUT_REAL (third);							\
      OUT_REAL (fourth);						\
      OUT (" ");							\
      OUT_REAL (fifth);							\
      OUT_REAL (sixth);							\
      OUT (" " op "\n");						\
    }									\
   while (0)

/* This should be called before the others in this file.  It opens the
   output file `OUTPUT_NAME.ps', and writes some preliminary boilerplate. */

static int
output_p2e_header(FILE *ps_file, char *name, int llx, int lly, int urx, int ury, spline_list_array_type shape)
{
  unsigned this_list, with_curves = 1;

  for (this_list = 0; this_list < SPLINE_LIST_ARRAY_LENGTH(shape); this_list++) {
    unsigned this_spline;

    spline_list_type list = SPLINE_LIST_ARRAY_ELT(shape, this_list);

    for (this_spline = 0; this_spline < SPLINE_LIST_LENGTH(list); this_spline++) {
      spline_type s = SPLINE_LIST_ELT(list, this_spline);

      if (SPLINE_DEGREE(s) != LINEARTYPE) {
        with_curves = 0;
        this_list = SPLINE_LIST_ARRAY_LENGTH(shape);
        break;
      }
    }

  }

  OUT_LINE("%!PS-Adobe-3.0");
  OUT("%%%%Title: flattened PostScript generated by autotrace: %s\n", name);
  OUT_LINE("%%Creator: pstoedit");
  OUT_LINE("%%BoundingBox: (atend)");
  OUT_LINE("%%Pages: (atend)");
  OUT_LINE("%%EndComments");
  OUT_LINE("%%BeginProlog");
  OUT_LINE("/setPageSize { pop pop } def");
  OUT_LINE("/ntranslate { neg exch neg exch translate } def");
  OUT_LINE("/setshowparams { pop pop pop} def");
  OUT_LINE("/awidthshowhex { dup /ASCIIHexDecode filter exch length 2 div cvi string readstring pop awidthshow } def");
  OUT_LINE("/backendconstraints { pop pop } def");
  OUT_LINE("/pstoedit.newfont { 80 string cvs  findfont  dup length dict begin {1 index /FID ne {def} {pop pop} ifelse} forall  /Encoding ISOLatin1Encoding def   dup 80 string cvs /FontName exch def  currentdict end  definefont pop } def");
  OUT_LINE("/imagestring 1 string def");
  OUT_LINE("%%EndProlog");
  OUT_LINE("%%BeginSetup");
  OUT_LINE("% textastext doflatten backendconstraints  ");
  OUT("%d 0 backendconstraints\n", with_curves);
  OUT_LINE("%%EndSetup");

  return 0;
}

/* This outputs the PostScript code which produces the shape in
   SHAPE.  */

static void out_splines(FILE * ps_file, spline_list_array_type shape)
{
  unsigned this_list;
  unsigned int pathnr = 1;
  spline_list_type list;
  at_color last_color = { 0, 0, 0 };

  OUT_LINE(" 612 792 setPageSize");
  OUT_LINE(" 0 setlinecap");
  OUT_LINE(" 10.0 setmiterlimit");
  OUT_LINE(" 0 setlinejoin");
  OUT_LINE(" [ ] 0.0 setdash");
  OUT_LINE(" 1.0 setlinewidth");

  for (this_list = 0; this_list < SPLINE_LIST_ARRAY_LENGTH(shape); this_list++) {
    unsigned this_spline;
    spline_type first;

    list = SPLINE_LIST_ARRAY_ELT(shape, this_list);
    first = SPLINE_LIST_ELT(list, 0);

    if (this_list == 0 || !at_color_equal(&list.color, &last_color)) {
      int c, m, y, k;

      OUT_LINE((shape.centerline || list.open) ? "stroke" : "fill");
      OUT("\n\n%% %d pathnumber\n", pathnr);
      OUT_LINE((shape.centerline || list.open) ? "% strokedpath" : "% filledpath");
      pathnr++;
#ifdef withrgbcolor
      /* in the long term this can be removed. */
      OUT("%.3f %.3f %.3f setrgbcolor\n", (real) list.color.r / 255.0, (real) list.color.g / 255.0, (real) list.color.b / 255.0);
#else
      c = k = 255 - list.color.r;
      m = 255 - list.color.g;
      if (m < k)
        k = m;
      y = 255 - list.color.b;
      if (y < k)
        k = y;
      c -= k;
      m -= k;
      y -= k;
      OUT("%.3f %.3f %.3f %.3f %s\n", (double)c / 255.0, (double)m / 255.0, (double)y / 255.0, (double)k / 255.0, "setcmykcolor");
#endif
      last_color = list.color;
    }

    OUT_LINE("newpath");
    OUT_COMMAND2(START_POINT(first).x, START_POINT(first).y, "moveto");

    for (this_spline = 0; this_spline < SPLINE_LIST_LENGTH(list); this_spline++) {
      spline_type s = SPLINE_LIST_ELT(list, this_spline);

      if (SPLINE_DEGREE(s) == LINEARTYPE)
        OUT_COMMAND2(END_POINT(s).x, END_POINT(s).y, "lineto");
      else
        OUT_COMMAND6(CONTROL1(s).x, CONTROL1(s).y, CONTROL2(s).x, CONTROL2(s).y, END_POINT(s).x, END_POINT(s).y, "curveto");
    }
    if (!list.open)
      OUT_LINE("closepath");
  }
  if (SPLINE_LIST_ARRAY_LENGTH(shape) > 0)
    OUT_LINE((shape.centerline || list.open) ? "stroke" : "fill");
}

int output_p2e_writer(FILE *ps_file, char *name, int llx, int lly, int urx, int ury, at_output_opts_type *opts,
                      spline_list_array_type shape, at_msg_func msg_func, void *msg_data, void *user_data)
{
  int result;

  result = output_p2e_header(ps_file, name, llx, lly, urx, ury, shape);
  if (result != 0)
    return result;

  out_splines(ps_file, shape);

  OUT_LINE("showpage");
  OUT("%%%%BoundingBox: %d %d %d %d\n", llx, lly, urx, ury);
  OUT_LINE("%%Page: 1 1");
  OUT_LINE("% normal end reached by pstoedit.pro");
  OUT_LINE("%%Trailer");
  OUT_LINE("%%Pages: 1");
  OUT_LINE("%%EOF");

  return 0;
}
