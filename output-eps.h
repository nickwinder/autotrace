/* output-eps.h: utility routines for PostScript output
   Copyright (C) 2000 Martin Weber */

#ifndef OUTPUT_EPS_H
#define OUTPUT_EPS_H

#include <stdio.h>
#include "ptypes.h"
#include "spline.h"

int output_eps_writer (FILE* file, at_string name,
		       int llx, int lly, int urx, int ury, int dpi,
		       spline_list_array_type shape);


#endif /* not OUTPUT_EPS_H */

