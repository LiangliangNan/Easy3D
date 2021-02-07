/*
===============================================================================

  FILE:  lasitemreadraw.hpp
  
  CONTENTS:
  
    Implementation of LASitemReadRaw for *all* items that compose a point.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 August 2017 -- moving 'context' from global development hack to interface  
    10 January 2011 -- licensing change for LGPL release and liblas integration
    7 December 2010 -- refactored after getting invited to KAUST in Saudi Arabia
  
===============================================================================
*/
#ifndef LAS_READ_ITEM_RAW_HPP
#define LAS_READ_ITEM_RAW_HPP

#include "lasreaditem.hpp"

#include <assert.h>

class LASreadItemRaw_POINT10_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_POINT10_LE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, 20);
  }
};

class LASreadItemRaw_POINT10_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_POINT10_BE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 20);
    ENDIAN_SWAP_32(&swapped[ 0], &item[ 0]);    // X
    ENDIAN_SWAP_32(&swapped[ 4], &item[ 4]);    // Y
    ENDIAN_SWAP_32(&swapped[ 8], &item[ 8]);    // Z
    ENDIAN_SWAP_16(&swapped[12], &item[12]);    // intensity
    *((U32*)&item[14]) = *((U32*)&swapped[14]); // bitfield, classification, scan_angle_rank, user_data
    ENDIAN_SWAP_16(&swapped[18], &item[18]);    // point_source_ID
  };
private:
  U8 swapped[20];
};

class LASreadItemRaw_GPSTIME11_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_GPSTIME11_LE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, 8);
  };
};

class LASreadItemRaw_GPSTIME11_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_GPSTIME11_BE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 8);
    ENDIAN_SWAP_64(swapped, item);
  };
private:
  U8 swapped[8];
};

class LASreadItemRaw_RGB12_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_RGB12_LE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, 6);
  };
};

class LASreadItemRaw_RGB12_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_RGB12_BE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 6);
    ENDIAN_SWAP_32(&swapped[ 0], &item[ 0]); // R
    ENDIAN_SWAP_32(&swapped[ 2], &item[ 2]); // G
    ENDIAN_SWAP_32(&swapped[ 4], &item[ 4]); // B
  };
private:
  U8 swapped[6];
};

class LASreadItemRaw_WAVEPACKET13_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_WAVEPACKET13_LE(){}
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, 29);
  };
};

class LASreadItemRaw_WAVEPACKET13_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_WAVEPACKET13_BE(){}
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 29);
    item[0] = swapped[0];                    // wavepacket descriptor index
    ENDIAN_SWAP_64(&swapped[ 1], &item[ 1]); // byte offset to waveform data
    ENDIAN_SWAP_32(&swapped[ 9], &item[ 9]); // waveform packet size in bytes
    ENDIAN_SWAP_32(&swapped[13], &item[13]); // return point waveform location
    ENDIAN_SWAP_32(&swapped[17], &item[17]); // X(t)
    ENDIAN_SWAP_32(&swapped[21], &item[21]); // Y(t)
    ENDIAN_SWAP_32(&swapped[25], &item[25]); // Z(t)
  };
private:
  U8 swapped[29];
};

class LASreadItemRaw_BYTE : public LASreadItemRaw
{
public:
  LASreadItemRaw_BYTE(U32 number)
  {
    this->number = number;
  }
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, number);
  };
private:
  U32 number;
};

class LAStempReadPoint10
{
public:
  I32 X;
  I32 Y;
  I32 Z;
  U16 intensity;
  U8 return_number : 3;
  U8 number_of_returns : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification;
  I8 scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;
  // LAS 1.4 only
  I16 extended_scan_angle;
  U8 extended_point_type : 2;
  U8 extended_scanner_channel : 2;
  U8 extended_classification_flags : 4;
  U8 extended_classification;
  U8 extended_return_number : 4;
  U8 extended_number_of_returns : 4;
  // for 8 byte alignment of the GPS time
  U8 dummy[3];
  // LASlib only
  U32 deleted_flag;
  F64 gps_time;
};

class LAStempReadPoint14
{
public:
  I32 X;
  I32 Y;
  I32 Z;
  U16 intensity;
  U8 return_number : 4;
  U8 number_of_returns : 4;
  U8 classification_flags : 4;
  U8 scanner_channel : 2;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification;
  U8 user_data;
  I16 scan_angle;
  U16 point_source_ID;
};

class LASreadItemRaw_POINT14_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_POINT14_LE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(buffer, 30);
    ((LAStempReadPoint10*)item)->X = ((LAStempReadPoint14*)buffer)->X;
    ((LAStempReadPoint10*)item)->Y = ((LAStempReadPoint14*)buffer)->Y;
    ((LAStempReadPoint10*)item)->Z = ((LAStempReadPoint14*)buffer)->Z;
    ((LAStempReadPoint10*)item)->intensity = ((LAStempReadPoint14*)buffer)->intensity;
    if (((LAStempReadPoint14*)buffer)->number_of_returns > 7)
    {
      if (((LAStempReadPoint14*)buffer)->return_number > 6)
      {
        if (((LAStempReadPoint14*)buffer)->return_number >= ((LAStempReadPoint14*)buffer)->number_of_returns)
        {
          ((LAStempReadPoint10*)item)->return_number = 7;
        }
        else
        {
          ((LAStempReadPoint10*)item)->return_number = 6;
        }
      }
      else
      {
        ((LAStempReadPoint10*)item)->return_number = ((LAStempReadPoint14*)buffer)->return_number;
      }
      ((LAStempReadPoint10*)item)->number_of_returns = 7;
    }
    else
    {
      ((LAStempReadPoint10*)item)->return_number = ((LAStempReadPoint14*)buffer)->return_number;
      ((LAStempReadPoint10*)item)->number_of_returns = ((LAStempReadPoint14*)buffer)->number_of_returns;
    }
    ((LAStempReadPoint10*)item)->scan_direction_flag = ((LAStempReadPoint14*)buffer)->scan_direction_flag;
    ((LAStempReadPoint10*)item)->edge_of_flight_line = ((LAStempReadPoint14*)buffer)->edge_of_flight_line;
    ((LAStempReadPoint10*)item)->classification = (U8)(((((LAStempReadPoint14*)buffer)->classification_flags) << 5) & 0xE0);
    if (((LAStempReadPoint14*)buffer)->classification < 32) ((LAStempReadPoint10*)item)->classification |= ((LAStempReadPoint14*)buffer)->classification;
    ((LAStempReadPoint10*)item)->scan_angle_rank = I8_CLAMP(I16_QUANTIZE(0.006f*((LAStempReadPoint14*)buffer)->scan_angle));
    ((LAStempReadPoint10*)item)->user_data = ((LAStempReadPoint14*)buffer)->user_data;
    ((LAStempReadPoint10*)item)->point_source_ID = ((LAStempReadPoint14*)buffer)->point_source_ID;
    ((LAStempReadPoint10*)item)->extended_scanner_channel = ((LAStempReadPoint14*)buffer)->scanner_channel;
    ((LAStempReadPoint10*)item)->extended_classification_flags = ((LAStempReadPoint14*)buffer)->classification_flags;
    ((LAStempReadPoint10*)item)->extended_classification = ((LAStempReadPoint14*)buffer)->classification;
    ((LAStempReadPoint10*)item)->extended_return_number = ((LAStempReadPoint14*)buffer)->return_number;
    ((LAStempReadPoint10*)item)->extended_number_of_returns = ((LAStempReadPoint14*)buffer)->number_of_returns;
    ((LAStempReadPoint10*)item)->extended_scan_angle = ((LAStempReadPoint14*)buffer)->scan_angle;
    ((LAStempReadPoint10*)item)->gps_time = *((F64*)&buffer[22]);
  }
private:
  U8 buffer[30];
};

class LASreadItemRaw_POINT14_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_POINT14_BE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 30);
    ENDIAN_SWAP_32(&swapped[ 0], &item[ 0]);    // X
    ENDIAN_SWAP_32(&swapped[ 4], &item[ 4]);    // Y
    ENDIAN_SWAP_32(&swapped[ 8], &item[ 8]);    // Z
    ENDIAN_SWAP_16(&swapped[12], &item[12]);    // intensity
    if (((LAStempReadPoint14*)swapped)->number_of_returns > 7)
    {
      if (((LAStempReadPoint14*)swapped)->return_number > 6)
      {
        if (((LAStempReadPoint14*)swapped)->return_number >= ((LAStempReadPoint14*)swapped)->number_of_returns)
        {
          ((LAStempReadPoint10*)item)->return_number = 7;
        }
        else
        {
          ((LAStempReadPoint10*)item)->return_number = 6;
        }
      }
      else
      {
        ((LAStempReadPoint10*)item)->return_number = ((LAStempReadPoint14*)swapped)->return_number;
      }
      ((LAStempReadPoint10*)item)->number_of_returns = 7;
    }
    else
    {
      ((LAStempReadPoint10*)item)->return_number = ((LAStempReadPoint14*)swapped)->return_number;
      ((LAStempReadPoint10*)item)->number_of_returns = ((LAStempReadPoint14*)swapped)->number_of_returns;
    }
    ((LAStempReadPoint10*)item)->scan_direction_flag = ((LAStempReadPoint14*)swapped)->scan_direction_flag;
    ((LAStempReadPoint10*)item)->edge_of_flight_line = ((LAStempReadPoint14*)swapped)->edge_of_flight_line;
    ((LAStempReadPoint10*)item)->classification = (((LAStempReadPoint14*)swapped)->classification_flags << 5);
    if (((LAStempReadPoint14*)swapped)->classification < 32) ((LAStempReadPoint10*)item)->classification |= ((LAStempReadPoint14*)swapped)->classification;
    ((LAStempReadPoint10*)item)->user_data = ((LAStempReadPoint14*)swapped)->user_data;
    ENDIAN_SWAP_16(&swapped[20], &item[18]); // point_source_ID
    ((LAStempReadPoint10*)item)->extended_scanner_channel = ((LAStempReadPoint14*)swapped)->scanner_channel;
    ((LAStempReadPoint10*)item)->extended_classification_flags = ((LAStempReadPoint14*)swapped)->classification_flags;
    ((LAStempReadPoint10*)item)->extended_classification = ((LAStempReadPoint14*)swapped)->classification;
    ((LAStempReadPoint10*)item)->extended_return_number = ((LAStempReadPoint14*)swapped)->return_number;
    ((LAStempReadPoint10*)item)->extended_number_of_returns = ((LAStempReadPoint14*)swapped)->number_of_returns;
    ENDIAN_SWAP_16(&swapped[18], (U8*)&(((LAStempReadPoint10*)item)->extended_scan_angle));
    ((LAStempReadPoint10*)item)->scan_angle_rank = I8_CLAMP(I16_QUANTIZE(0.006f*((LAStempReadPoint10*)item)->extended_scan_angle));
    ENDIAN_SWAP_64(&swapped[22], (U8*)&(((LAStempReadPoint10*)item)->gps_time));
  }
private:
  U8 swapped[30];
};

class LASreadItemRaw_RGBNIR14_LE : public LASreadItemRaw
{
public:
  LASreadItemRaw_RGBNIR14_LE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(item, 8);
  };
};

class LASreadItemRaw_RGBNIR14_BE : public LASreadItemRaw
{
public:
  LASreadItemRaw_RGBNIR14_BE(){};
  inline void read(U8* item, U32& context)
  {
    instream->getBytes(swapped, 8);
    ENDIAN_SWAP_32(&swapped[ 0], &item[ 0]); // R
    ENDIAN_SWAP_32(&swapped[ 2], &item[ 2]); // G
    ENDIAN_SWAP_32(&swapped[ 4], &item[ 4]); // B
    ENDIAN_SWAP_32(&swapped[ 6], &item[ 6]); // NIR
  };
private:
  U8 swapped[8];
};

#endif
