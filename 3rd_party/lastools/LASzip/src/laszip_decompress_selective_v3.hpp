/*
===============================================================================

  FILE:  laszip_decompress_selective_v3.hpp

  CONTENTS:

    Contains bit mask definitions for selective decompression.

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
  
   14 April 2017 -- created at Lo Que Hay where Gui was having birthday dinner

===============================================================================
*/
#ifndef LASZIP_DECOMPRESS_SELECTIVE_V3_HPP
#define LASZIP_DECOMPRESS_SELECTIVE_V3_HPP

#define LASZIP_DECOMPRESS_SELECTIVE_ALL                0xFFFFFFFF

#define LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY 0x00000000
#define LASZIP_DECOMPRESS_SELECTIVE_Z                  0x00000001
#define LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION     0x00000002
#define LASZIP_DECOMPRESS_SELECTIVE_FLAGS              0x00000004
#define LASZIP_DECOMPRESS_SELECTIVE_INTENSITY          0x00000008
#define LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE         0x00000010
#define LASZIP_DECOMPRESS_SELECTIVE_USER_DATA          0x00000020
#define LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE       0x00000040
#define LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME           0x00000080
#define LASZIP_DECOMPRESS_SELECTIVE_RGB                0x00000100
#define LASZIP_DECOMPRESS_SELECTIVE_NIR                0x00000200
#define LASZIP_DECOMPRESS_SELECTIVE_WAVEPACKET         0x00000400
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE0              0x00010000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE1              0x00020000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE2              0x00040000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE3              0x00080000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE4              0x00100000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE5              0x00200000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE6              0x00400000
#define LASZIP_DECOMPRESS_SELECTIVE_BYTE7              0x00800000
#define LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES        0xFFFF0000

#endif // LASZIP_DECOMPRESS_SELECTIVE_V3_HPP
