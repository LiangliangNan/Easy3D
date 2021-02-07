/*
===============================================================================

  FILE:  laswriter_qfit.cpp
  
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
#include "laswriter_qfit.hpp"

#include "bytestreamout_file.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>

BOOL LASwriterQFIT::refile(FILE* file)
{
  if (stream == 0) return FALSE;
  if (this->file) this->file = file;
  return ((ByteStreamOutFile*)stream)->refile(file);
}

BOOL LASwriterQFIT::open(const char* file_name, const LASheader* header, I32 version, U32 io_buffer_size)
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

  return open(file, header, version);
}

BOOL LASwriterQFIT::open(FILE* file, const LASheader* header, I32 version)
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
  {
    out = new ByteStreamOutFileLE(file);
    endian_swap = FALSE;
  }
  else
  {
    out = new ByteStreamOutFileBE(file);
    endian_swap = TRUE;
  }

  return open(out, header, version);
}

BOOL LASwriterQFIT::open(ByteStreamOut* stream, const LASheader* header, I32 version)
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

  // check whether input is in longitude / latitude

  if (((-361 < header->min_x) && (-361 < header->min_y) && (header->max_x < 361) && (header->max_y < 361)) == FALSE)
  {
    fprintf(stderr,"ERROR: bounding box (%g %g / %g %g) exceeds longitude / latitude\n", header->min_x, header->min_y, header->max_x, header->max_y);
    return FALSE;
  }

  // check whether we need to rescale or reoffset

  rescale_reoffset = FALSE;

  if (header->x_scale_factor != 0.000001 || header->y_scale_factor != 0.000001 || header->z_scale_factor != 0.001)
  {
    rescale_reoffset = TRUE;
  }
  
  if (header->x_offset != 0.0 || header->y_offset != 0.0 || header->z_offset != 0.0)
  {
    rescale_reoffset = TRUE;
  }

  // check whether these attributes are available

  scan_azimuth_array_offset = header->get_attribute_start("scan azimuth");
  pitch_array_offset = header->get_attribute_start("pitch");
  roll_array_offset = header->get_attribute_start("roll");
  pulse_width_array_offset = header->get_attribute_start("pulse width");

  if (version == 0)
  {
    if (pulse_width_array_offset != -1)
    {
      version = 48;
    }
    else
    {
      version = 40;
    }
  }

  if (version == 48 || version == 40 || version == 56)
  {
    this->version = version;
  }
  else
  {
    fprintf(stderr,"WARNING: version %d of QFIT unknown ... using 48\n", version);
    this->version = 48;
  }

  // write version into first header record

  if (!stream->put32bitsLE((U8*)&version))
  {
    fprintf(stderr,"ERROR: while writing version of QFIT header\n");
    return FALSE;
  }

  // populate some ASCII into the remainder of first header record

  memset(buffer, 0, 48);
  sprintf((char*)buffer, "via LASwriterQFIT (version %d)", LAS_TOOLS_VERSION);

  if (!stream->putBytes((U8*)buffer, version-4))
  {
    fprintf(stderr,"ERROR: writing first header record of QFIT header\n");
    return FALSE;
  }

  // write continuation and offset into second header record

  buffer[0] = -9000000;
  buffer[1] = 2*version;

  if (!stream->put32bitsLE((U8*)&buffer[0]))
  {
    fprintf(stderr,"ERROR: while writing -9000000 into QFIT header\n");
    return FALSE;
  }
  if (!stream->put32bitsLE((U8*)&buffer[1]))
  {
    fprintf(stderr,"ERROR: while writing offset into QFIT header\n");
    return FALSE;
  }

  // populate some ASCII into the remainder of second header record

  memset(buffer, 0, 48);
  sprintf((char*)buffer, "LAStools by Martin Isenburg");
  if (!stream->putBytes((U8*)buffer, version-8))
  {
    fprintf(stderr,"ERROR: writing second header record of QFIT header\n");
    return FALSE;
  }

  memset(buffer, 0, 48);

  return TRUE;
}

BOOL LASwriterQFIT::write_point(const LASpoint* point)
{
  buffer[0] = I32_QUANTIZE(point->gps_time / 0.001);
  if (buffer[0] < 0) buffer[0] *= -1;
  if (rescale_reoffset)
  {
    buffer[2] = I32_QUANTIZE(point->get_x()/0.000001);
    buffer[1] = I32_QUANTIZE(point->get_y()/0.000001);
    buffer[3] = I32_QUANTIZE(point->get_z()/0.001);
  }
  else
  {
    buffer[2] = point->get_X();
    buffer[1] = point->get_Y();
    buffer[3] = point->get_Z();
  }
  if (buffer[2] < 0) buffer[2] += 360000000; //  convert negative longitude to LARGE east
  buffer[5] = point->intensity;
  buffer[6] = I32_QUANTIZE(point->scan_angle_rank/0.001);

  if (scan_azimuth_array_offset > -1) point->get_attribute(scan_azimuth_array_offset, buffer[6]);
  if (pitch_array_offset > -1) point->get_attribute(pitch_array_offset, buffer[7]);
  if (roll_array_offset > -1) point->get_attribute(roll_array_offset, buffer[8]);
  if (pulse_width_array_offset > -1) { U8 pulse_width; point->get_attribute(pulse_width_array_offset, pulse_width); buffer[10] = pulse_width; };

  if (endian_swap)
  {
    ENDIAN_SWAP_32((U8*)&buffer[0]);
    ENDIAN_SWAP_32((U8*)&buffer[1]);
    ENDIAN_SWAP_32((U8*)&buffer[2]);
    ENDIAN_SWAP_32((U8*)&buffer[3]);
    ENDIAN_SWAP_32((U8*)&buffer[5]);
    ENDIAN_SWAP_32((U8*)&buffer[6]);
    ENDIAN_SWAP_32((U8*)&buffer[7]);
    ENDIAN_SWAP_32((U8*)&buffer[8]);
    ENDIAN_SWAP_32((U8*)&buffer[10]);
  }

  if (!stream->putBytes((U8*)buffer, version)) return FALSE;

  p_count++;
  return TRUE;
}

BOOL LASwriterQFIT::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  return TRUE;
}

I64 LASwriterQFIT::close(BOOL update_header)
{
  I64 bytes = 0;
  
  if (stream)
  {
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

LASwriterQFIT::LASwriterQFIT()
{
  stream = 0;
  file = 0;
  version = 0;
  endian_swap = FALSE;
  rescale_reoffset = FALSE;
  scan_azimuth_array_offset = -1;
  pitch_array_offset = -1;
  roll_array_offset = -1;
  pulse_width_array_offset = -1;
}

LASwriterQFIT::~LASwriterQFIT()
{
  if (file) close();
}
