/*
===============================================================================

  FILE:  lasignore.hpp
  
  CONTENTS:
  
    Implements the various rules for ignoring points when processing LiDAR.

  PROGRAMMERS:

    info@rapidlasso.de - https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    21 July 2021 -- fixed '-ignore_extended_class 128'. please memoy, call me!
    18 August 2019 -- created after Memoy's cousine stole USD 300 and EUR 100
  
===============================================================================
*/
#ifndef LAS_IGNORE_HPP
#define LAS_IGNORE_HPP

#include "lasdefinitions.hpp"

#define LASIGNORE_CLASSIFICATIONS 0x000000FF
#define LASIGNORE_RETURNS         0x00000F00
#define LASIGNORE_FLAGS           0x0000F000

#define LASIGNORE_FIRST_OF_MANY   0x00000100
#define LASIGNORE_LAST_OF_MANY    0x00000200
#define LASIGNORE_INTERMEDIATE    0x00000400
#define LASIGNORE_SINGLE          0x00000800

#define LASIGNORE_SYNTHETIC       0x00001000
#define LASIGNORE_KEYPOINT        0x00002000
#define LASIGNORE_WITHHELD        0x00004000
#define LASIGNORE_OVERLAP         0x00008000

class LASignore
{
public:
  void usage() const;

  void ignore_class(U8 classification);
  void dont_ignore_class(U8 classification);

  BOOL parse(int& i, int argc, char *argv[]);
  I32 unparse(CHAR* string) const;

  BOOL parse(U32 curr_parameter, const U32 num_parameters, const F64* parameters);
  void unparse(U32& num_parameters, F64* parameters) const;

  U32 get_decompress_selective() const;

  BOOL ignore(const LASpoint* point) const;

  LASignore();
  ~LASignore();
protected:
  U32 ignore_mask;
  U32 ignore_classification_mask[8];
};

#endif
