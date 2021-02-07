/*
===============================================================================

  FILE:  lasreader_bin.cpp
  
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
#include "lasreader_bin.hpp"

#include "bytestreamin.hpp"
#include "bytestreamin_file.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

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

BOOL LASreaderBIN::open(const char* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  // open file

  file = fopen(file_name, "rb");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 2*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 2*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  // create input stream

  ByteStreamIn* in;
  if (IS_LITTLE_ENDIAN())
    in = new ByteStreamInFileLE(file);
  else
    in = new ByteStreamInFileBE(file);

  // clean header

  header.clean();

  // maybe set creation date

#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA attr;
	SYSTEMTIME creation;
  GetFileAttributesEx(file_name, GetFileExInfoStandard, &attr);
	FileTimeToSystemTime(&attr.ftCreationTime, &creation);
  int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  header.file_creation_day = startday[creation.wMonth] + creation.wDay;
  header.file_creation_year = creation.wYear;
  // leap year handling
  if ((((creation.wYear)%4) == 0) && (creation.wMonth > 2)) header.file_creation_day++;
#else
  header.file_creation_day = 333;
  header.file_creation_year = 2011;
#endif

  return open(in);
}

BOOL LASreaderBIN::open(ByteStreamIn* stream)
{
  int i;

  if (stream == 0)
  {
    fprintf(stderr,"ERROR: ByteStreamIn* pointer is zero\n");
    return FALSE;
  }

  this->stream = stream;

  // read the TS header

  TSheader tsheader;
  try { stream->getBytes((U8*)&tsheader, sizeof(TSheader)); } catch(...)
  {
    fprintf(stderr,"ERROR: reading terrasolid header\n");
    return FALSE;
  }

  // check the TS header

  if (tsheader.size != sizeof(TSheader))
  {
    fprintf(stderr,"ERROR: corrupt terrasolid header. size != 56.\n");
    return FALSE;
  }

  if (tsheader.recog_val != 970401)
  {
    fprintf(stderr,"ERROR: corrupt terrasolid header. recog_val != 979401.\n");
    return FALSE;
  }

  if (strncmp(tsheader.recog_str, "CXYZ", 4) != 0)
  {
    fprintf(stderr,"ERROR: corrupt terrasolid header. recog_str != CXYZ.\n");
    return FALSE;
  }

  version = tsheader.version;

  // populate the header as much as possible

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderBIN (%d)", LAS_TOOLS_VERSION);

  if (tsheader.time)
  {
    if (tsheader.rgb)
    {
      header.point_data_format = 3;
      header.point_data_record_length = 34;
    }
    else
    {
      header.point_data_format = 1;
      header.point_data_record_length = 28;
    }
  }
  else
  {
    if (tsheader.rgb)
    {
      header.point_data_format = 2;
      header.point_data_record_length = 26;
    }
    else
    {
      header.point_data_format = 0;
      header.point_data_record_length = 20;
    }
  }

  header.number_of_point_records = tsheader.npoints;
  npoints = tsheader.npoints;

  F64 scale = 1.0/(F64)tsheader.units;
  header.x_scale_factor = scale;
  header.y_scale_factor = scale;
  header.z_scale_factor = scale;
  header.x_offset = -tsheader.origin_x*scale;
  header.y_offset = -tsheader.origin_y*scale;
  header.z_offset = -tsheader.origin_z*scale;

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length);
  
  // set point count to zero

  p_count = 0;

  // approximate bounding box init

  if (read_point())
  {
    header.min_x = header.max_x = point.get_x();
    header.min_y = header.max_y = point.get_y();
    header.min_z = header.max_z = point.get_z();

    if (tsheader.npoints > 10)
    {
      for (i = tsheader.npoints/10; i < tsheader.npoints; i += (tsheader.npoints/10))
      {
        if (!seek(i)) return FALSE;
        if (!read_point()) return FALSE;
      }
    }

    header.number_of_points_by_return[0] = 0;
    header.number_of_points_by_return[1] = 0;
  }

  return seek(0);
}

BOOL LASreaderBIN::seek(const I64 p_index)
{
  if (p_index < npoints)
  {
    long pos = sizeof(TSheader);
    if (version == 20020715)
      pos += (sizeof(TSpoint)*(U32)p_index);
    else
      pos += (sizeof(TSrow)*(U32)p_index);
    if (point.have_gps_time)
      pos += (sizeof(U32)*(U32)p_index);
    if (point.have_rgb)
      pos += (sizeof(U32)*(U32)p_index);
    p_count = p_index;
    return stream->seek(pos);
  }
  return FALSE;
}

BOOL LASreaderBIN::read_point_default()
{
  if (p_count < npoints)
  {
    int echo;
    if (version == 20020715)
    {
      TSpoint tspoint;
      try { stream->getBytes((U8*)&tspoint, sizeof(TSpoint)); } catch(...)
      {
        fprintf(stderr,"ERROR: reading terrasolid point after %u of %u\n", (U32)p_count, (U32)npoints);
        return FALSE;
      }
      point.set_X(tspoint.x);
      point.set_Y(tspoint.y);
      point.set_Z(tspoint.z);
      point.intensity = tspoint.intensity;
      point.classification = tspoint.code;
      point.point_source_ID = tspoint.line;
      echo = tspoint.echo;
    }
    else
    {
      TSrow tsrow;
      try { stream->getBytes((U8*)&tsrow, sizeof(TSrow)); } catch(...)
      {
        fprintf(stderr,"ERROR: reading terrasolid row after %u of %u\n", (U32)p_count, (U32)npoints);
        return FALSE;
      }
      point.set_X(tsrow.x);
      point.set_Y(tsrow.y);
      point.set_Z(tsrow.z);
      point.intensity = tsrow.echo_intensity & 0x3FFF;
      point.classification = tsrow.code;
      point.point_source_ID = tsrow.line;
      echo = tsrow.echo_intensity >> 14;
    }

    if (header.min_x > point.get_x()) header.min_x = point.get_x();
    else if (header.max_x < point.get_x()) header.max_x = point.get_x();
    if (header.min_y > point.get_y()) header.min_y = point.get_y();
    else if (header.max_y < point.get_y()) header.max_y = point.get_y();
    if (header.min_z > point.get_z()) header.min_z = point.get_z();
    else if (header.max_z < point.get_z()) header.max_z = point.get_z();

    if (echo == 0) // only echo
    {
      point.return_number = 1;
      point.number_of_returns = 1;
      header.number_of_points_by_return[0]++;
    }
    else if (echo == 1) // first (of many)
    {
      point.return_number = 1;
      point.number_of_returns = 2;
      header.number_of_points_by_return[0]++;
    }
    else if (echo == 3) // last (of many)
    {
      point.return_number = 2;
      point.number_of_returns = 2;
      header.number_of_points_by_return[1]++;
    }
    else // intermediate
    {
      point.return_number = 2;
      point.number_of_returns = 3;
      header.number_of_points_by_return[1]++;
    }

    if (point.have_gps_time)
    {
      U32 time;
      try { stream->getBytes((U8*)&time, sizeof(U32)); } catch(...)
      {
        fprintf(stderr,"ERROR: reading terrasolid time\n");
        return FALSE;
      }
      point.gps_time = 0.0002*time;
    }

    if (point.have_rgb)
    {
      U8 rgba[4];
      try { stream->getBytes((U8*)rgba, sizeof(U8)*4); } catch(...)
      {
        fprintf(stderr,"ERROR: reading terrasolid color\n");
        return FALSE;
      }
      point.rgb[0] = 256*rgba[0];
      point.rgb[1] = 256*rgba[1];
      point.rgb[2] = 256*rgba[2];
    }
    p_count++;
    return TRUE;
  }
  return FALSE;
}

ByteStreamIn* LASreaderBIN::get_stream() const
{
  return stream;
}

void LASreaderBIN::close(BOOL close_stream)
{
  if (close_stream)
  {
    if (stream)
    {
      delete stream;
      stream = 0;
    }
    if (file)
    {
      fclose(file);
      file = 0;
    }
  }
}

LASreaderBIN::LASreaderBIN()
{
  file = 0;
  stream = 0;
}

LASreaderBIN::~LASreaderBIN()
{
  if (stream) close();
}

LASreaderBINrescale::LASreaderBINrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderBIN()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderBINrescale::open(ByteStreamIn* stream)
{
  if (!LASreaderBIN::open(stream)) return FALSE;
  // do we need to change anything
  if (scale_factor[0] && (header.x_scale_factor != scale_factor[0]))
  {
    header.x_scale_factor = scale_factor[0];
  }
  if (scale_factor[1] && (header.y_scale_factor != scale_factor[1]))
  {
    header.y_scale_factor = scale_factor[1];
  }
  if (scale_factor[2] && (header.z_scale_factor != scale_factor[2]))
  {
    header.z_scale_factor = scale_factor[2];
  }
  return TRUE;
}

LASreaderBINreoffset::LASreaderBINreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderBIN()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderBINreoffset::open(ByteStreamIn* stream)
{
  if (!LASreaderBIN::open(stream)) return FALSE;
  // do we need to change anything
  if (header.x_offset != offset[0])
  {
    header.x_offset = offset[0];
  }
  if (header.y_offset != offset[1])
  {
    header.y_offset = offset[1];
  }
  if (header.z_offset != offset[2])
  {
    header.z_offset = offset[2];
  }
  return TRUE;
}

LASreaderBINrescalereoffset::LASreaderBINrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderBINrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderBINreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderBINrescalereoffset::open(ByteStreamIn* stream)
{
  if (!LASreaderBIN::open(stream)) return FALSE;
  // do we need to change anything
  if (scale_factor[0] && (header.x_scale_factor != scale_factor[0]))
  {
    header.x_scale_factor = scale_factor[0];
  }
  if (scale_factor[1] && (header.y_scale_factor != scale_factor[1]))
  {
    header.y_scale_factor = scale_factor[1];
  }
  if (scale_factor[2] && (header.z_scale_factor != scale_factor[2]))
  {
    header.z_scale_factor = scale_factor[2];
  }
  if (header.x_offset != offset[0])
  {
    header.x_offset = offset[0];
  }
  if (header.y_offset != offset[1])
  {
    header.y_offset = offset[1];
  }
  if (header.z_offset != offset[2])
  {
    header.z_offset = offset[2];
  }
  return TRUE;
}
