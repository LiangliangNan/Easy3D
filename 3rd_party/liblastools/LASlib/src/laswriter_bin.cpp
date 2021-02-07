/*
===============================================================================

  FILE:  laswriter_bin.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "laswriter_bin.hpp"

#include "bytestreamout_file.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>

struct TSrow
{
  U8 code;
  U8 line;
  U16 echo_intensity;
  I32 x;
  I32 y;
  I32 z;
};

struct TSpoint
{
  I32 x;
  I32 y;
  I32 z;
  U8 code;
  U8 echo;
  U8 flag;
  U8 mark;
  U16 line;
  U16 intensity;
};

struct TSheader
{
  I32 size;
  I32 version;
  I32 recog_val;
  I8 recog_str[4];
  I32 npoints;
  I32 units;
  F64 origin_x;
  F64 origin_y;
  F64 origin_z;
  I32 time;
  I32 rgb;
};

BOOL LASwriterBIN::refile(FILE* file)
{
  if (stream == 0) return FALSE;
  if (this->file) this->file = file;
  return ((ByteStreamOutFile*)stream)->refile(file);
}

BOOL LASwriterBIN::open(const char* file_name, const LASheader* header, const char* version, U32 io_buffer_size)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  file = fopen(file_name, "wb");

  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, io_buffer_size) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %u\n", io_buffer_size);
  }

  ByteStreamOut* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  return open(out, header, version);
}

BOOL LASwriterBIN::open(FILE* file, const LASheader* header, const char* version)
{
  if (file == 0)
  {
    fprintf(stderr,"ERROR: file pointer is zero\n");
    return FALSE;
  }

#ifdef _WIN32
  if (file == stdout)
  {
    if(_setmode( _fileno( stdout ), _O_BINARY ) == -1 )
    {
      fprintf(stderr, "ERROR: cannot set stdout to binary (untranslated) mode\n");
    }
  }
#endif

  ByteStreamOut* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  return open(out, header, version);
}

BOOL LASwriterBIN::open(ByteStreamOut* stream, const LASheader* header, const char* version)
{
  if (stream == 0)
  {
    fprintf(stderr,"ERROR: ByteStreamOut pointer is zero\n");
    return FALSE;
  }
  this->stream = stream;

  if (header == 0)
  {
    fprintf(stderr,"ERROR: LASheader pointer is zero\n");
    return FALSE;
  }

  if (strstr(version, "ts16"))
  {
    this->version = 20020715;
  }
  else
  {
    this->version = 20010712;
  }

  TSheader tsheader;
  tsheader.size = sizeof(TSheader);
  tsheader.version = this->version;
  tsheader.recog_val = 970401;
  strncpy(tsheader.recog_str, "CXYZ", 4);
  tsheader.npoints = (header->number_of_point_records ? header->number_of_point_records : (U32)header->extended_number_of_point_records);
  double scale = header->x_scale_factor;
  if (header->y_scale_factor < scale) scale = header->y_scale_factor; 
  if (header->z_scale_factor < scale) scale = header->z_scale_factor; 
  units = tsheader.units = (I32)(1.0 / scale);
  origin_x = tsheader.origin_x = -header->x_offset/scale;
  origin_y = tsheader.origin_y = -header->y_offset/scale;
  origin_z = tsheader.origin_z = -header->z_offset/scale;
  tsheader.time = (header->point_data_format == 1) || (header->point_data_format == 3) || (header->point_data_format == 4) || (header->point_data_format == 5);
  tsheader.rgb = (header->point_data_format == 2) || (header->point_data_format == 3) || (header->point_data_format == 5);

  return stream->putBytes((U8*)&tsheader, sizeof(TSheader));
}

BOOL LASwriterBIN::write_point(const LASpoint* point)
{
  U16 echo;

  if (point->number_of_returns <= 1)
    echo = 0;
  else if (point->return_number == 1)
    echo = 1;
  else if (point->return_number >= point->number_of_returns)
    echo = 3;
  else
    echo = 2;

  if (version == 20020715)
  {
    TSpoint tspoint;
    tspoint.x = I32_QUANTIZE(point->get_x()*units+origin_x);
    tspoint.y = I32_QUANTIZE(point->get_y()*units+origin_y);
    tspoint.z = I32_QUANTIZE(point->get_z()*units+origin_z);
    tspoint.code = point->classification;
    tspoint.echo = (U8)echo;
    tspoint.flag  = 0;
    tspoint.mark  = 0;
    tspoint.line = point->point_source_ID;
    tspoint.intensity = point->intensity;
    if (!stream->putBytes((U8*)&tspoint, sizeof(TSpoint))) return FALSE;
  }
  else
  {
    TSrow tsrow;
    tsrow.code = point->classification;
    tsrow.line = (U8)(point->point_source_ID);
    tsrow.echo_intensity = (echo << 14) | (point->intensity & 0x3FFF);
    tsrow.x = I32_QUANTIZE(point->get_x()*units+origin_x);
    tsrow.y = I32_QUANTIZE(point->get_y()*units+origin_y);
    tsrow.z = I32_QUANTIZE(point->get_z()*units+origin_z);
    if (!stream->putBytes((U8*)&tsrow, sizeof(TSrow))) return FALSE;
  }

  if (point->have_gps_time)
  {
    U32 time = (U32)(point->gps_time/0.0002+0.5);
    if (!stream->putBytes((U8*)&time, sizeof(U32))) return FALSE;
  }
  if (point->have_rgb)
  {
    U8 rgba[4];
    rgba[0] = point->rgb[0]/256;
    rgba[1] = point->rgb[1]/256;
    rgba[2] = point->rgb[2]/256;
    rgba[3] = 0;
    if (!stream->putBytes((U8*)&rgba, sizeof(U32))) return FALSE;
  }
  p_count++;
  return TRUE;
}

BOOL LASwriterBIN::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  return TRUE;
}

I64 LASwriterBIN::close(BOOL update_npoints)
{
  I64 bytes = 0;
  
  if (stream)
  {
    if (update_npoints && p_count != npoints)
    {
      if (!stream->isSeekable())
      {
#ifdef _WIN32
        fprintf(stderr, "ERROR: stream not seekable. cannot update header from %I64d to %I64d points.\n", npoints, p_count);
#else
        fprintf(stderr, "ERROR: stream not seekable. cannot update header from %lld to %lld points.\n", npoints, p_count);
#endif
      }
      else
      {
	      stream->seek(16);
	      stream->put32bitsLE((U8*)&p_count);
        stream->seekEnd();
      }
    }
    bytes = stream->tell();
    delete stream;
    stream = 0;
  }

  if (file)
  {
    fclose(file);
    file = 0;
  }

  npoints = p_count;
  p_count = 0;

  return bytes;
}

LASwriterBIN::LASwriterBIN()
{
  stream = 0;
  file = 0;
}

LASwriterBIN::~LASwriterBIN()
{
  if (file) close();
}
