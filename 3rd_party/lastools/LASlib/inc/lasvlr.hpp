/*
===============================================================================

  FILE:  lasvlr.hpp
  
  CONTENTS:
  
    This class defines those VLRs and EVLRs that are defined either in the LAS 
    specification or are specific to and used by LAStools.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 November 2019 -- created after Tobago paddle week in flight POS -> PTY
  
===============================================================================
*/
#ifndef LAS_VLR_HPP
#define LAS_VLR_HPP

#include "mydefs.hpp"

class LASvlr_lastiling
{
public:
  U32 level;
  U32 level_index;
  U32 implicit_levels : 30;
  U32 buffer : 1;
  U32 reversible : 1;
  F32 min_x;
  F32 max_x;
  F32 min_y;
  F32 max_y;

  LASvlr_lastiling()
  {
    memset((void*)this, 0, sizeof(LASvlr_lastiling));
  };
};

class LASvlr_lasoriginal
{
public:
  I64 number_of_point_records;
  I64 number_of_points_by_return[15];
  F64 max_x;
  F64 min_x;
  F64 max_y;
  F64 min_y;
  F64 max_z;
  F64 min_z;
  I64 position;

  LASvlr_lasoriginal()
  {
    memset((void*)this, 0, sizeof(LASvlr_lasoriginal));
  };
};


#endif // LAS_VLR_HPP
