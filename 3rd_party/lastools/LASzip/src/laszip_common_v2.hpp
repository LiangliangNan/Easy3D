/*
===============================================================================

  FILE:  laszip_common_v2.hpp
  
  CONTENTS:
  
    Common defines and functionalities for version 2 of LASitemReadCompressed
    and LASitemwriteCompressed.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2022, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    16 March 2011 -- created after designing the "streaming median" algorithm

===============================================================================
*/
#ifndef LASZIP_COMMON_V2_HPP
#define LASZIP_COMMON_V2_HPP

class StreamingMedian5
{
public:
  I32 values[5];
  BOOL high;
  void init()
  {
    values[0] = values[1] = values[2] = values[3] = values[4] = 0;
    high = true;
  }
  inline void add(I32 v)
  {
    if (high)
    {
      if (v < values[2])
      {
        values[4] = values[3];
        values[3] = values[2];
        if (v < values[0])
        {
          values[2] = values[1];
          values[1] = values[0];
          values[0] = v;
        }
        else if (v < values[1])
        {
          values[2] = values[1];
          values[1] = v;
        }
        else
        {
          values[2] = v;
        }
      }
      else
      {
        if (v < values[3])
        {
          values[4] = values[3];
          values[3] = v;
        }
        else
        {
          values[4] = v;
        }
        high = false;
      }
    }
    else
    {
      if (values[2] < v)
      {
        values[0] = values[1];
        values[1] = values[2];
        if (values[4] < v)
        {
          values[2] = values[3];
          values[3] = values[4];
          values[4] = v;
        }
        else if (values[3] < v)
        {
          values[2] = values[3];
          values[3] = v;
        }
        else
        {
          values[2] = v;
        }
      }
      else
      {
        if (values[1] < v)
        {
          values[0] = values[1];
          values[1] = v;
        }
        else
        {
          values[0] = v;
        }
        high = true;
      }
    }
  }
  I32 get() const
  {
    return values[2];
  }
  StreamingMedian5()
  {
    init();
  }
};

// for LAS files with the return (r) and the number (n) of
// returns field correctly populated the mapping should really
// be only the following.
//  { 15, 15, 15, 15, 15, 15, 15, 15 },
//  { 15,  0, 15, 15, 15, 15, 15, 15 },
//  { 15,  1,  2, 15, 15, 15, 15, 15 },
//  { 15,  3,  4,  5, 15, 15, 15, 15 },
//  { 15,  6,  7,  8,  9, 15, 15, 15 },
//  { 15, 10, 11, 12, 13, 14, 15, 15 },
//  { 15, 15, 15, 15, 15, 15, 15, 15 },
//  { 15, 15, 15, 15, 15, 15, 15, 15 }
// however, some files start the numbering of r and n with 0,
// only have return counts r, or only have number of return
// counts n, or mix up the position of r and n. we therefore
// "complete" the table to also map those "undesired" r & n
// combinations to different contexts
const U8 number_return_map[8][8] = 
{
  { 15, 14, 13, 12, 11, 10,  9,  8 },
  { 14,  0,  1,  3,  6, 10, 10,  9 },
  { 13,  1,  2,  4,  7, 11, 11, 10 },
  { 12,  3,  4,  5,  8, 12, 12, 11 },
  { 11,  6,  7,  8,  9, 13, 13, 12 },
  { 10, 10, 11, 12, 13, 14, 14, 13 },
  {  9, 10, 11, 12, 13, 14, 15, 14 },
  {  8,  9, 10, 11, 12, 13, 14, 15 }
};

// for LAS files with the return (r) and the number (n) of
// returns field correctly populated the mapping should really
// be only the following.
//  {  0,  7,  7,  7,  7,  7,  7,  7 },
//  {  7,  0,  7,  7,  7,  7,  7,  7 },
//  {  7,  1,  0,  7,  7,  7,  7,  7 },
//  {  7,  2,  1,  0,  7,  7,  7,  7 },
//  {  7,  3,  2,  1,  0,  7,  7,  7 },
//  {  7,  4,  3,  2,  1,  0,  7,  7 },
//  {  7,  5,  4,  3,  2,  1,  0,  7 },
//  {  7,  6,  5,  4,  3,  2,  1,  0 }
// however, some files start the numbering of r and n with 0,
// only have return counts r, or only have number of return
// counts n, or mix up the position of r and n. we therefore
// "complete" the table to also map those "undesired" r & n
// combinations to different contexts
const U8 number_return_level[8][8] = 
{
  {  0,  1,  2,  3,  4,  5,  6,  7 },
  {  1,  0,  1,  2,  3,  4,  5,  6 },
  {  2,  1,  0,  1,  2,  3,  4,  5 },
  {  3,  2,  1,  0,  1,  2,  3,  4 },
  {  4,  3,  2,  1,  0,  1,  2,  3 },
  {  5,  4,  3,  2,  1,  0,  1,  2 },
  {  6,  5,  4,  3,  2,  1,  0,  1 },
  {  7,  6,  5,  4,  3,  2,  1,  0 }
};

#endif
