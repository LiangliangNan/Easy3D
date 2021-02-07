/*
===============================================================================

  FILE:  lasdefinitions.hpp
  
  CONTENTS:
  
    Contains the Header and Point classes for reading and writing LiDAR points
    in the LAS format

      Version 1.4,   Nov 14, 2011.
      Version 1.3,   Oct 24, 2010.
      Version 1.2, April 29, 2008.
      Version 1.1, March 07, 2005.
      Version 1.0,   May 09, 2003

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2018, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    19 April 2017 -- support for selective decompression for new LAS 1.4 points 
    1 February 2017 -- better support for OGC WKT strings in VLRs or EVLRs
    22 June 2016 -- set default of VLR header "reserved" to 0 instead of 0xAABB
    1 August 2015 -- moving LASpoint, LASquantizer, and LASattributer to LASzip
    9 December 2013 -- bug fix and improved writing of new LAS 1.4 point types
    21 December 2011 -- (limited) support for LAS 1.4 and attributed extra bytes 
    10 January 2011 -- licensing change for LGPL release and liblas integration
    16 December 2010 -- updated to support generic LASitem point formats
    3 December 2010 -- updated to (somewhat) support LAS format 1.3
    7 September 2008 -- updated to support LAS format 1.2 
    11 June 2007 -- number of return / scan direction bitfield order was wrong
    18 February 2007 -- created after repairing 2 vacuum cleaners in the garden
  
===============================================================================
*/
#ifndef LAS_DEFINITIONS_HPP
#define LAS_DEFINITIONS_HPP

#define LAS_TOOLS_VERSION 181108

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mydefs.hpp"
#include "laszip.hpp"
#include "laspoint.hpp"

#define LAS_TOOLS_FORMAT_DEFAULT 0
#define LAS_TOOLS_FORMAT_LAS     1
#define LAS_TOOLS_FORMAT_LAZ     2
#define LAS_TOOLS_FORMAT_BIN     3
#define LAS_TOOLS_FORMAT_QFIT    4
#define LAS_TOOLS_FORMAT_VRML    5
#define LAS_TOOLS_FORMAT_TXT     6
#define LAS_TOOLS_FORMAT_SHP     7
#define LAS_TOOLS_FORMAT_PLY     8
#define LAS_TOOLS_FORMAT_ASC     9
#define LAS_TOOLS_FORMAT_BIL    10
#define LAS_TOOLS_FORMAT_FLT    11
#define LAS_TOOLS_FORMAT_DTM    12

#define LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE 0
#define LAS_TOOLS_GLOBAL_ENCODING_BIT_WDP_INTERNAL  1
#define LAS_TOOLS_GLOBAL_ENCODING_BIT_WDP_EXTERNAL  2
#define LAS_TOOLS_GLOBAL_ENCODING_BIT_SYNTHETIC     3
#define LAS_TOOLS_GLOBAL_ENCODING_BIT_OGC_WKT_CRS   4

#define LAS_TOOLS_IO_IBUFFER_SIZE   262144
#define LAS_TOOLS_IO_OBUFFER_SIZE   262144

class LASvlr
{
public:
  U16 reserved;
  CHAR user_id[16]; 
  U16 record_id;
  U16 record_length_after_header;
  CHAR description[32];
  U8* data;
  LASvlr() { memset(this, 0, sizeof(LASvlr)); };
};

class LASevlr
{
public:
  U16 reserved;
  CHAR user_id[16]; 
  U16 record_id;
  I64 record_length_after_header;
  CHAR description[32];
  U8* data;
  LASevlr() { memset(this, 0, sizeof(LASevlr)); };
};

class LASvlr_geo_keys
{
public:
  U16 key_directory_version;
  U16 key_revision;
  U16 minor_revision;
  U16 number_of_keys;
};

class LASvlr_key_entry
{
public:
  U16 key_id;
  U16 tiff_tag_location;
  U16 count;
  U16 value_offset;
};

class LASvlr_classification
{
public:
  U8 class_number;
  CHAR description[15];
};

class LASvlr_wave_packet_descr
{
public:
  LASvlr_wave_packet_descr() {clean();};
  void clean() {memset(data, 0, 26);};
  inline U8 getBitsPerSample() const {return data[0];};
  inline U8 getCompressionType() const {return data[1];};
  inline U32 getNumberOfSamples() const {return ((U32*)&(data[2]))[0];};
  inline U32 getTemporalSpacing() const {return ((U32*)&(data[6]))[0];};
  inline F64 getDigitizerGain() const {return ((F64*)&(data[10]))[0];};
  inline F64 getDigitizerOffset() const {return ((F64*)&(data[18]))[0];};
  inline void setBitsPerSample(U8 bps) {data[0] = bps;};
  inline void setCompressionType(U8 compression) {data[1] = compression;};
  inline void setNumberOfSamples(U32 samples) {((U32*)&(data[2]))[0] = samples;};
  inline void setTemporalSpacing(U32 spacing) {((U32*)&(data[6]))[0] = spacing;};
  inline void setDigitizerGain(F64 gain) {((F64*)&(data[10]))[0] = gain;};
  inline void setDigitizerOffset(F64 offset) {((F64*)&(data[18]))[0] = offset;};
private:
  U8 data[26];
};

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
};

class LASheader : public LASquantizer, public LASattributer
{
public:
  CHAR file_signature[4];                  // starts at byte   0
  U16 file_source_ID;                      // starts at byte   4
  U16 global_encoding;                     // starts at byte   6
  U32 project_ID_GUID_data_1;              // starts at byte   8
  U16 project_ID_GUID_data_2;              // starts at byte  12
  U16 project_ID_GUID_data_3;              // starts at byte  14
  U8 project_ID_GUID_data_4[8];            // starts at byte  16
  U8 version_major;                        // starts at byte  24
  U8 version_minor;                        // starts at byte  25
  CHAR system_identifier[32];              // starts at byte  26
  CHAR generating_software[32];            // starts at byte  58
  U16 file_creation_day;                   // starts at byte  90
  U16 file_creation_year;                  // starts at byte  92
  U16 header_size;                         // starts at byte  94
  U32 offset_to_point_data;                // starts at byte  96
  U32 number_of_variable_length_records;   // starts at byte 100
  U8 point_data_format;                    // starts at byte 104
  U16 point_data_record_length;            // starts at byte 105
  U32 number_of_point_records;             // starts at byte 107
  U32 number_of_points_by_return[5];       // starts at byte 111
  F64 max_x;
  F64 min_x;
  F64 max_y;
  F64 min_y;
  F64 max_z;
  F64 min_z;

  // LAS 1.3 only
  U64 start_of_waveform_data_packet_record;

  // LAS 1.4 only
  U64 start_of_first_extended_variable_length_record;
  U32 number_of_extended_variable_length_records;
  U64 extended_number_of_point_records;
  U64 extended_number_of_points_by_return[15];

  U32 user_data_in_header_size;
  U8* user_data_in_header;

  LASvlr* vlrs;
  LASevlr* evlrs;
  LASvlr_geo_keys* vlr_geo_keys;
  LASvlr_key_entry* vlr_geo_key_entries;
  F64* vlr_geo_double_params;
  CHAR* vlr_geo_ascii_params;
  CHAR* vlr_geo_ogc_wkt_math;
  CHAR* vlr_geo_ogc_wkt;
  LASvlr_classification* vlr_classification;
  LASvlr_wave_packet_descr** vlr_wave_packet_descr;

  LASzip* laszip;
  LASvlr_lastiling* vlr_lastiling;
  LASvlr_lasoriginal* vlr_lasoriginal;

  U32 user_data_after_header_size;
  U8* user_data_after_header;

  LASheader()
  {
    clean_las_header();
  };

  // set functions

  void set_bounding_box(F64 min_x, F64 min_y, F64 min_z, F64 max_x, F64 max_y, F64 max_z, BOOL auto_scale=TRUE, BOOL auto_offset=TRUE)
  {
    if (auto_scale)
    {
      if (-360 < min_x  && -360 < min_y && max_x < 360 && max_y < 360)
      {
        x_scale_factor = 0.0000001;
        y_scale_factor = 0.0000001;
      }
      else
      {
        x_scale_factor = 0.01;
        y_scale_factor = 0.01;
      }
      z_scale_factor = 0.01;
    }
    if (auto_offset)
    {
      if (-360 < min_x  && -360 < min_y && max_x < 360 && max_y < 360)
      {
        x_offset = 0;
        y_offset = 0;
        z_offset = 0;
      }
      else
      {
        x_offset = ((I32)((min_x + max_x)/200000))*100000;
        y_offset = ((I32)((min_y + max_y)/200000))*100000;
        z_offset = ((I32)((min_z + max_z)/200000))*100000;
      }
    }
    this->min_x = x_offset + x_scale_factor*I32_QUANTIZE((min_x-x_offset)/x_scale_factor);
    this->min_y = y_offset + y_scale_factor*I32_QUANTIZE((min_y-y_offset)/y_scale_factor);
    this->min_z = z_offset + z_scale_factor*I32_QUANTIZE((min_z-z_offset)/z_scale_factor);
    this->max_x = x_offset + x_scale_factor*I32_QUANTIZE((max_x-x_offset)/x_scale_factor);
    this->max_y = y_offset + y_scale_factor*I32_QUANTIZE((max_y-y_offset)/y_scale_factor);
    this->max_z = z_offset + z_scale_factor*I32_QUANTIZE((max_z-z_offset)/z_scale_factor);
  };

  void set_global_encoding_bit(I32 bit)
  {
    global_encoding |= (1 << bit);
  }

  void unset_global_encoding_bit(I32 bit)
  {
    global_encoding &= ~(1 << bit);
  }

  BOOL get_global_encoding_bit(I32 bit) const
  {
    return (BOOL)(global_encoding & (1 << bit));
  }

  // clean functions

  void clean_las_header()
  {
    memset((void*)this, 0, sizeof(LASheader));
    file_signature[0] = 'L'; file_signature[1] = 'A'; file_signature[2] = 'S'; file_signature[3] = 'F';
    version_major = 1;
    version_minor = 2;
    header_size = 227;
    offset_to_point_data = 227;
    point_data_record_length = 20;
    x_scale_factor = 0.01;
    y_scale_factor = 0.01;
    z_scale_factor = 0.01;
  };

  void clean_user_data_in_header()
  {
    if (user_data_in_header)
    {
      header_size -= user_data_in_header_size;
      offset_to_point_data -= user_data_in_header_size;
      delete [] user_data_in_header;
      user_data_in_header = 0;
      user_data_in_header_size = 0;
    }
  };

  void clean_vlrs()
  {
    if (vlrs)
    {
      U32 i;
      for (i = 0; i < number_of_variable_length_records; i++)
      {
        offset_to_point_data -= (54 + vlrs[i].record_length_after_header);
        if (vlrs[i].data && (vlrs[i].data != (U8*)attributes))
        {
          delete [] vlrs[i].data;
        }
      }
      free(vlrs);
      vlrs = 0;
      vlr_geo_keys = 0;
      vlr_geo_key_entries = 0;
      vlr_geo_double_params = 0;
      vlr_geo_ascii_params = 0;
      vlr_geo_ogc_wkt_math = 0;
      vlr_geo_ogc_wkt = 0;
      vlr_classification = 0;
      if (vlr_wave_packet_descr) delete [] vlr_wave_packet_descr;
      vlr_wave_packet_descr = 0;
      number_of_variable_length_records = 0;
    }
  };

  void clean_evlrs()
  {
    if (evlrs)
    {
      U32 i;
      for (i = 0; i < number_of_extended_variable_length_records; i++)
      {
        if (evlrs[i].data && (evlrs[i].data != (U8*)attributes))
        {
          delete [] evlrs[i].data;
        }
      }
      free(evlrs);
      evlrs = 0;
    }
    start_of_first_extended_variable_length_record = 0;
    number_of_extended_variable_length_records = 0;
  };

  void clean_laszip()
  {
    if (laszip)
    {
      delete laszip;
    }
    laszip = 0;
  };

  void clean_lastiling()
  {
    if (vlr_lastiling)
    {
      delete vlr_lastiling;
    }
    vlr_lastiling = 0;
  };

  void clean_lasoriginal()
  {
    if (vlr_lasoriginal)
    {
      delete vlr_lasoriginal;
    }
    vlr_lasoriginal = 0;
  };

  void clean_user_data_after_header()
  {
    if (user_data_after_header)
    {
      offset_to_point_data -= user_data_after_header_size;
      delete [] user_data_after_header;
      user_data_after_header = 0;
      user_data_after_header_size = 0;
    }
  };

  void clean()
  {
    clean_user_data_in_header();
    clean_vlrs();
    clean_evlrs();
    clean_laszip();
    clean_lastiling();
    clean_lasoriginal();
    clean_user_data_after_header();
    clean_attributes();
    clean_las_header();
  };

  void unlink()
  {
    user_data_in_header_size = 0;
    user_data_in_header = 0;
    vlrs = 0;
    number_of_variable_length_records = 0;
    evlrs = 0;
    start_of_first_extended_variable_length_record = 0;
    number_of_extended_variable_length_records = 0;
    laszip = 0;
    vlr_lastiling = 0;
    vlr_lasoriginal = 0;
    user_data_after_header_size = 0;
    user_data_after_header = 0;
    attributes_linked = FALSE;
    offset_to_point_data = header_size;
  }

  LASheader & operator=(const LASquantizer & quantizer)
  {
    this->x_scale_factor = quantizer.x_scale_factor;
    this->y_scale_factor = quantizer.y_scale_factor;
    this->z_scale_factor = quantizer.z_scale_factor;
    this->x_offset = quantizer.x_offset;
    this->y_offset = quantizer.y_offset;
    this->z_offset = quantizer.z_offset;
    return *this;
  };

  BOOL check() const
  {
    if (strncmp(file_signature, "LASF", 4) != 0)
    {
      fprintf(stderr,"ERROR: wrong file signature '%4s'\n", file_signature);
      return FALSE;
    }
    if ((version_major != 1) || (version_minor > 4))
    {
      fprintf(stderr,"WARNING: unknown version %d.%d (should be 1.0 or 1.1 or 1.2 or 1.3 or 1.4)\n", version_major, version_minor);
    }
    if (header_size < 227)
    {
      fprintf(stderr,"ERROR: header size is %d but should be at least 227\n", header_size);
      return FALSE;
    }
    if (offset_to_point_data < header_size)
    {
      fprintf(stderr,"ERROR: offset to point data %d is smaller than header size %d\n", offset_to_point_data, header_size);
      return FALSE;
    }
    if (x_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: x scale factor is zero.\n");
    }
    if (y_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: y scale factor is zero.\n");
    }
    if (z_scale_factor == 0)
    {
      fprintf(stderr,"WARNING: z scale factor is zero.\n");
    }
    if (max_x < min_x || max_y < min_y || max_z < min_z)
    {
      fprintf(stderr,"WARNING: invalid bounding box [ %g %g %g / %g %g %g ]\n", min_x, min_y, min_z, max_x, max_y, max_z);
    }
    return TRUE;
  };

  BOOL is_compressed() const
  {
    if (laszip)
    {
      if (laszip->compressor)
      {
        return TRUE;
      }
    }
    return FALSE;
  };

  BOOL is_lonlat() const
  {
    if ((-360.0 <= min_x) && (-90.0 <= min_y) && (max_x <= 360.0) && (max_y <= 90.0))
    {
      return TRUE;
    }
    return FALSE;
  };

  // note that data needs to be allocated with new [] and not malloc and that LASheader
  // will become the owner over this and manage its deallocation 
  BOOL add_vlr(const CHAR* user_id, const U16 record_id, const U16 record_length_after_header, U8* data, const BOOL keep_description=FALSE, const CHAR* description=0, const BOOL keep_existing=FALSE)
  {
    U32 i = 0;
    BOOL found_description = FALSE;
    if (vlrs)
    {
      if (keep_existing)
      {
        i = number_of_variable_length_records;
      }
      else
      {
        for (i = 0; i < number_of_variable_length_records; i++)
        {
          if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
          {
            if (vlrs[i].record_length_after_header)
            {
              offset_to_point_data -= vlrs[i].record_length_after_header;
              delete [] vlrs[i].data;
              vlrs[i].data = 0;
            }
            found_description = TRUE;
            break;
          }
        }
      }
      if (i == number_of_variable_length_records)
      {
        number_of_variable_length_records++;
        offset_to_point_data += 54;
        vlrs = (LASvlr*)realloc(vlrs, sizeof(LASvlr)*number_of_variable_length_records);
      }
    }
    else
    {
      number_of_variable_length_records = 1;
      offset_to_point_data += 54;
      vlrs = (LASvlr*)malloc(sizeof(LASvlr));
    }
    memset(&(vlrs[i]), 0, sizeof(LASvlr));
    vlrs[i].reserved = 0; // used to be 0xAABB
    strncpy(vlrs[i].user_id, user_id, 16);
    vlrs[i].record_id = record_id;
    vlrs[i].record_length_after_header = record_length_after_header;
    if (keep_description && found_description)
    {
      // do nothing
    }
    else if (description)
    {
      sprintf(vlrs[i].description, "%.31s", description);
    }
    else
    {
      sprintf(vlrs[i].description, "by LAStools of rapidlasso GmbH");
    }
    if (record_length_after_header)
    {
      offset_to_point_data += record_length_after_header;
      vlrs[i].data = data;
    }
    else
    {
      vlrs[i].data = 0;
    }
		return TRUE;
  };

  const LASvlr* get_vlr(const CHAR* user_id, U16 record_id) const
  {
    U32 i = 0;
    for (i = 0; i < number_of_variable_length_records; i++)
    {
      if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
      {
        return &(vlrs[i]);
      }
    }
    return 0;
  };

  BOOL remove_vlr(U32 i, BOOL delete_data=TRUE)
  {
    if (vlrs)
    {
      if (i < number_of_variable_length_records)
      {
        offset_to_point_data -= (54 + vlrs[i].record_length_after_header);
        if (vlrs[i].record_length_after_header)
        {
          if (delete_data)
          {
            delete [] vlrs[i].data;
          }
        }
        number_of_variable_length_records--;
        if (number_of_variable_length_records)
        {
          vlrs[i] = vlrs[number_of_variable_length_records];
          vlrs = (LASvlr*)realloc(vlrs, sizeof(LASvlr)*number_of_variable_length_records);
        }
        else
        {
          free(vlrs);
          vlrs = 0;
        }
        return TRUE;
      }
      return FALSE;
    }
    return FALSE;
  };

  BOOL remove_vlr(const CHAR* user_id, U16 record_id)
  {
    U32 i;
    for (i = 0; i < number_of_variable_length_records; i++)
    {
      if ((strcmp(vlrs[i].user_id, user_id) == 0) && (vlrs[i].record_id == record_id))
      {
        return remove_vlr(i);
      }
    }
    return FALSE;
  };

  // note that data needs to be allocated with new [] and not malloc and that LASheader
  // will become the owner over this and manage its deallocation 
  void add_evlr(const CHAR* user_id, const U16 record_id, const I64 record_length_after_header, U8* data, const BOOL keep_description=FALSE, const CHAR* description=0, const BOOL keep_existing=FALSE)
  {
    U32 i = 0;
    BOOL found_description = FALSE;
    if (evlrs)
    {
      if (keep_existing)
      {
        i = number_of_variable_length_records;
      }
      else
      {
        for (i = 0; i < number_of_variable_length_records; i++)
        {
          if ((strcmp(evlrs[i].user_id, user_id) == 0) && (evlrs[i].record_id == record_id))
          {
            if (evlrs[i].record_length_after_header)
            {
              delete [] evlrs[i].data;
              evlrs[i].data = 0;
            }
            found_description = TRUE;
            break;
          }
        }
      }
      if (i == number_of_extended_variable_length_records)
      {
        number_of_extended_variable_length_records++;
        evlrs = (LASevlr*)realloc(evlrs, sizeof(LASevlr)*number_of_extended_variable_length_records);
      }
    }
    else
    {
      number_of_extended_variable_length_records = 1;
      evlrs = (LASevlr*)malloc(sizeof(LASevlr)*number_of_extended_variable_length_records);
    }
    evlrs[i].reserved = 0; // used to be 0xAABB
    strncpy(evlrs[i].user_id, user_id, 16);
    evlrs[i].record_id = record_id;
    evlrs[i].record_length_after_header = record_length_after_header;
    if (keep_description && found_description)
    {
      // do nothing
    }
    else if (description)
    {
      sprintf(evlrs[i].description, "%.31s", description);
    }
    else
    {
      sprintf(evlrs[i].description, "by LAStools of rapidlasso GmbH");
    }
    if (record_length_after_header)
    {
      evlrs[i].data = data;
    }
    else
    {
      evlrs[i].data = 0;
    }
  };

  BOOL remove_evlr(U32 i, BOOL delete_data=TRUE)
  {
    if (evlrs)
    {
      if (i < number_of_extended_variable_length_records)
      {
        if (evlrs[i].record_length_after_header)
        {
          if (delete_data)
          {
            delete [] evlrs[i].data;
          }
        }
        number_of_extended_variable_length_records--;
        if (number_of_extended_variable_length_records)
        {
          evlrs[i] = evlrs[number_of_extended_variable_length_records];
          evlrs = (LASevlr*)realloc(evlrs, sizeof(LASvlr)*number_of_extended_variable_length_records);
        }
        else
        {
          free(evlrs);
          evlrs = 0;
          start_of_first_extended_variable_length_record = 0;
        }
        return TRUE;
      }
      return FALSE;
    }
    return FALSE;
  };

  BOOL remove_evlr(const CHAR* user_id, U16 record_id)
  {
    U32 i;
    for (i = 0; i < number_of_extended_variable_length_records; i++)
    {
      if ((strcmp(evlrs[i].user_id, user_id) == 0) && (evlrs[i].record_id == record_id))
      {
        return remove_evlr(i);
      }
    }
    return FALSE;
  };

  void set_lastiling(U32 level, U32 level_index, U32 implicit_levels, BOOL buffer, BOOL reversible, F32 min_x, F32 max_x, F32 min_y, F32 max_y)
  {
    clean_lastiling();
    vlr_lastiling = new LASvlr_lastiling();
    vlr_lastiling->level = level;
    vlr_lastiling->level_index = level_index;
    vlr_lastiling->implicit_levels = implicit_levels;
    vlr_lastiling->buffer = buffer;
    vlr_lastiling->reversible = reversible;
    vlr_lastiling->min_x = min_x;
    vlr_lastiling->max_x = max_x;
    vlr_lastiling->min_y = min_y;
    vlr_lastiling->max_y = max_y;
  };

  void set_lasoriginal()
  {
    clean_lasoriginal();
    vlr_lasoriginal = new LASvlr_lasoriginal();
    if (version_minor >= 4)
    {
      vlr_lasoriginal->number_of_point_records = extended_number_of_point_records;
      vlr_lasoriginal->number_of_points_by_return[0] = extended_number_of_points_by_return[0];
      vlr_lasoriginal->number_of_points_by_return[1] = extended_number_of_points_by_return[1];
      vlr_lasoriginal->number_of_points_by_return[2] = extended_number_of_points_by_return[2];
      vlr_lasoriginal->number_of_points_by_return[3] = extended_number_of_points_by_return[3];
      vlr_lasoriginal->number_of_points_by_return[4] = extended_number_of_points_by_return[4];
      vlr_lasoriginal->number_of_points_by_return[5] = extended_number_of_points_by_return[5];
      vlr_lasoriginal->number_of_points_by_return[6] = extended_number_of_points_by_return[6];
      vlr_lasoriginal->number_of_points_by_return[7] = extended_number_of_points_by_return[7];
      vlr_lasoriginal->number_of_points_by_return[8] = extended_number_of_points_by_return[8];
      vlr_lasoriginal->number_of_points_by_return[9] = extended_number_of_points_by_return[9];
      vlr_lasoriginal->number_of_points_by_return[10] = extended_number_of_points_by_return[10];
      vlr_lasoriginal->number_of_points_by_return[11] = extended_number_of_points_by_return[11];
      vlr_lasoriginal->number_of_points_by_return[12] = extended_number_of_points_by_return[12];
      vlr_lasoriginal->number_of_points_by_return[13] = extended_number_of_points_by_return[13];
      vlr_lasoriginal->number_of_points_by_return[14] = extended_number_of_points_by_return[14];
    }
    else
    {
      vlr_lasoriginal->number_of_point_records = number_of_point_records;
      vlr_lasoriginal->number_of_points_by_return[0] = number_of_points_by_return[0];
      vlr_lasoriginal->number_of_points_by_return[1] = number_of_points_by_return[1];
      vlr_lasoriginal->number_of_points_by_return[2] = number_of_points_by_return[2];
      vlr_lasoriginal->number_of_points_by_return[3] = number_of_points_by_return[3];
      vlr_lasoriginal->number_of_points_by_return[4] = number_of_points_by_return[4];
    }
    vlr_lasoriginal->max_x = max_x;
    vlr_lasoriginal->min_x = min_x;
    vlr_lasoriginal->max_y = max_y;
    vlr_lasoriginal->min_y = min_y;
    vlr_lasoriginal->max_z = max_z;
    vlr_lasoriginal->min_z = min_z;
  }

  BOOL restore_lasoriginal()
  {
    if (vlr_lasoriginal)
    {
      if (version_minor >= 4)
      {
        extended_number_of_point_records = vlr_lasoriginal->number_of_point_records;
        extended_number_of_points_by_return[0] = vlr_lasoriginal->number_of_points_by_return[0];
        extended_number_of_points_by_return[1] = vlr_lasoriginal->number_of_points_by_return[1];
        extended_number_of_points_by_return[2] = vlr_lasoriginal->number_of_points_by_return[2];
        extended_number_of_points_by_return[3] = vlr_lasoriginal->number_of_points_by_return[3];
        extended_number_of_points_by_return[4] = vlr_lasoriginal->number_of_points_by_return[4];
        extended_number_of_points_by_return[5] = vlr_lasoriginal->number_of_points_by_return[5];
        extended_number_of_points_by_return[6] = vlr_lasoriginal->number_of_points_by_return[6];
        extended_number_of_points_by_return[7] = vlr_lasoriginal->number_of_points_by_return[7];
        extended_number_of_points_by_return[8] = vlr_lasoriginal->number_of_points_by_return[8];
        extended_number_of_points_by_return[9] = vlr_lasoriginal->number_of_points_by_return[9];
        extended_number_of_points_by_return[10] = vlr_lasoriginal->number_of_points_by_return[10];
        extended_number_of_points_by_return[11] = vlr_lasoriginal->number_of_points_by_return[11];
        extended_number_of_points_by_return[12] = vlr_lasoriginal->number_of_points_by_return[12];
        extended_number_of_points_by_return[13] = vlr_lasoriginal->number_of_points_by_return[13];
        extended_number_of_points_by_return[14] = vlr_lasoriginal->number_of_points_by_return[14];
      }
      else
      {
        number_of_point_records = (U32)vlr_lasoriginal->number_of_point_records;
        number_of_points_by_return[0] = (U32)vlr_lasoriginal->number_of_points_by_return[0];
        number_of_points_by_return[1] = (U32)vlr_lasoriginal->number_of_points_by_return[1];
        number_of_points_by_return[2] = (U32)vlr_lasoriginal->number_of_points_by_return[2];
        number_of_points_by_return[3] = (U32)vlr_lasoriginal->number_of_points_by_return[3];
        number_of_points_by_return[4] = (U32)vlr_lasoriginal->number_of_points_by_return[4];
      }
      max_x = vlr_lasoriginal->max_x;
      min_x = vlr_lasoriginal->min_x;
      max_y = vlr_lasoriginal->max_y;
      min_y = vlr_lasoriginal->min_y;
      max_z = vlr_lasoriginal->max_z;
      min_z = vlr_lasoriginal->min_z;
      delete vlr_lasoriginal;
      vlr_lasoriginal = 0;
      return TRUE;
    }
    return FALSE;
  }

  BOOL set_geo_keys(const I32 number_of_keys, const LASvlr_key_entry* geo_keys)
  {
    if ((sizeof(LASvlr_geo_keys)*(number_of_keys+1)) > U16_MAX)
		{
			return FALSE;
		}
    vlr_geo_keys = new LASvlr_geo_keys[number_of_keys+1];
    vlr_geo_keys->key_directory_version = 1;
    vlr_geo_keys->key_revision = 1;
    vlr_geo_keys->minor_revision = 0;
    vlr_geo_keys->number_of_keys = number_of_keys;
    vlr_geo_key_entries = (LASvlr_key_entry*)&vlr_geo_keys[1];
    memcpy(vlr_geo_key_entries, geo_keys, sizeof(LASvlr_key_entry)*number_of_keys);
    return add_vlr("LASF_Projection", 34735, (U16)(sizeof(LASvlr_geo_keys)*(number_of_keys+1)), (U8*)vlr_geo_keys);
  }

  BOOL set_geo_double_params(const I32 num_geo_double_params, const F64* geo_double_params)
  {
    if ((sizeof(F64)*num_geo_double_params) > U16_MAX)
		{
			return FALSE;
		}
    vlr_geo_double_params = new F64[num_geo_double_params];
    memcpy(vlr_geo_double_params, geo_double_params, sizeof(F64)*num_geo_double_params);
    return add_vlr("LASF_Projection", 34736, (U16)(sizeof(F64)*num_geo_double_params), (U8*)vlr_geo_double_params);
  }

  void del_geo_double_params()
  {
    if (vlr_geo_double_params)
    {
      remove_vlr("LASF_Projection", 34736);
      vlr_geo_double_params = 0;
    }
  }

  BOOL set_geo_ascii_params(const I32 num_geo_ascii_params, const CHAR* geo_ascii_params)
  {
    if ((sizeof(CHAR)*num_geo_ascii_params) > U16_MAX)
		{
			return FALSE;
		}
    vlr_geo_ascii_params = new CHAR[num_geo_ascii_params];
    memcpy(vlr_geo_ascii_params, geo_ascii_params, sizeof(CHAR)*num_geo_ascii_params);
    return add_vlr("LASF_Projection", 34737, (U16)(sizeof(CHAR)*num_geo_ascii_params), (U8*)vlr_geo_ascii_params);
  }

  void del_geo_ascii_params()
  {
    if (vlr_geo_ascii_params)
    {
      remove_vlr("LASF_Projection", 34737);
      vlr_geo_ascii_params = 0;
    }
  }

  void set_geo_wkt_ogc_math(const I32 num_geo_wkt_ogc_math, const CHAR* geo_wkt_ogc_math)
  {
    I32 null_terminator = 0;
    if (geo_wkt_ogc_math[num_geo_wkt_ogc_math-1] == '\0')
    {
      vlr_geo_ogc_wkt_math = new CHAR[num_geo_wkt_ogc_math];
    }
    else
    {
      null_terminator = 1;
      vlr_geo_ogc_wkt_math = new CHAR[num_geo_wkt_ogc_math+1];
      vlr_geo_ogc_wkt_math[num_geo_wkt_ogc_math] = '\0';
    }
    memcpy(vlr_geo_ogc_wkt_math, geo_wkt_ogc_math, sizeof(CHAR)*num_geo_wkt_ogc_math);
    add_vlr("LASF_Projection", 2111, (U16)(sizeof(CHAR)*(num_geo_wkt_ogc_math+null_terminator)), (U8*)vlr_geo_ogc_wkt_math);
  }

  void del_geo_wkt_ogc_math()
  {
    if (vlr_geo_ogc_wkt_math)
    {
      remove_vlr("LASF_Projection", 2111);
      vlr_geo_ogc_wkt_math = 0;
    }
  }

  void set_geo_ogc_wkt(const I32 num_geo_ogc_wkt, const CHAR* geo_ogc_wkt, BOOL in_evlr=FALSE)
  {
    I32 null_terminator = 0;
    if (geo_ogc_wkt[num_geo_ogc_wkt-1] == '\0')
    {
      vlr_geo_ogc_wkt = new CHAR[num_geo_ogc_wkt];
    }
    else
    {
      null_terminator = 1;
      vlr_geo_ogc_wkt = new CHAR[num_geo_ogc_wkt+1];
      vlr_geo_ogc_wkt[num_geo_ogc_wkt] = '\0';
    }
    memcpy(vlr_geo_ogc_wkt, geo_ogc_wkt, sizeof(CHAR)*num_geo_ogc_wkt);
    if (in_evlr)
    {
      add_evlr("LASF_Projection", 2112, sizeof(CHAR)*(num_geo_ogc_wkt+null_terminator), (U8*)vlr_geo_ogc_wkt);
    }
    else
    {
      add_vlr("LASF_Projection", 2112, (U16)(sizeof(CHAR)*(num_geo_ogc_wkt+null_terminator)), (U8*)vlr_geo_ogc_wkt);
    }
  }

  void del_geo_ogc_wkt()
  {
    if (vlr_geo_ogc_wkt)
    {
      remove_vlr("LASF_Projection", 2112);
      remove_evlr("LASF_Projection", 2112);
      vlr_geo_ogc_wkt = 0;
    }
  }

  BOOL update_extra_bytes_vlr(const BOOL keep_description=FALSE)
  {
    if (number_attributes)
    {
      if ((sizeof(LASattribute)*number_attributes) > U16_MAX)
      {
        return FALSE;
      }
      U16 record_length_after_header = (U16)(sizeof(LASattribute)*number_attributes);
      U8* data = new U8[record_length_after_header];
      memcpy(data, attributes, record_length_after_header);
      return add_vlr("LASF_Spec", 4, record_length_after_header, data, keep_description);
    }
    else
    {
      return remove_vlr("LASF_Spec", 4);
    }
  }

  ~LASheader()
  {
    clean();
  };
};

#endif
