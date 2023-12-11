/*
===============================================================================

  FILE:  laszip_common_v1.hpp
  
  CONTENTS:
  
    Common defines and functionalities for version 1 of LASitemReadCompressed
    and LASitemwriteCompressed.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de
    uday.karan@gmail.com - http://github.com/verma

  COPYRIGHT:

    (c) 2007-2022, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    10 April 2014 - refactor LASwavepacket13 and add other functions to it
  
===============================================================================
*/

#ifndef LASZIP_COMMON_V1_HPP
#define LASZIP_COMMON_V1_HPP

#include "mydefs.hpp"

struct LASwavepacket13
{
  U64 offset;
  U32 packet_size;
  U32I32F32 return_point;
  U32I32F32 x;
  U32I32F32 y;
  U32I32F32 z;

  static inline LASwavepacket13 unpack(const U8* item)
  {
    // unpack a LAS wavepacket out of raw memory
    LASwavepacket13 r;

    r.offset = makeU64(item);
    r.packet_size = makeU32(item + 8);
    r.return_point.u32 = makeU32(item + 12);

    r.x.u32 = makeU32(item + 16);
    r.y.u32 = makeU32(item + 20);
    r.z.u32 = makeU32(item + 24);

    return r;
  }
  
  void inline pack(U8 *item)
  {
    // pack a LAS wavepacket into raw memory
    packU32((U32)(offset & 0xFFFFFFFF), item);
    packU32((U32)(offset >> 32), item+4);

    packU32(packet_size, item + 8);
    packU32(return_point.u32, item + 12);
    packU32(x.u32, item + 16);
    packU32(y.u32, item + 20);
    packU32(z.u32, item + 24);
  }

private:

  static inline U64 makeU64(const U8* item)
  {
    U64 dw0 = (U64)makeU32(item);
    U64 dw1 = (U64)makeU32(item+4);

    return dw0 | (dw1 << 32);
  }

  static inline U32 makeU32(const U8 *item)
  {
    U32 b0 = (U32)item[0];
    U32 b1 = (U32)item[1];
    U32 b2 = (U32)item[2];
    U32 b3 = (U32)item[3];

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
  }

  static inline void packU32(U32 v, U8 *item)
  {
	  item[0] = v & 0xFF;
	  item[1] = (v >> 8) & 0xFF;
	  item[2] = (v >> 16) & 0xFF;
	  item[3] = (v >> 24) & 0xFF;
  }
};

#endif // LASZIP_COMMON_V1_HPP
