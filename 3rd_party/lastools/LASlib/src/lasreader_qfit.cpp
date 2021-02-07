/*
===============================================================================

  FILE:  lasreader_qfit.cpp
  
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
#include "lasreader_qfit.hpp"

#include "bytestreamin.hpp"
#include "bytestreamin_file.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

BOOL LASreaderQFIT::open(const char* file_name)
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

  // create input stream

  ByteStreamIn* in;
  if (IS_LITTLE_ENDIAN())
    in = new ByteStreamInFileLE(file);
  else
    in = new ByteStreamInFileBE(file);

  // clean the header

  header.clean();

  // set projection

  LASvlr_key_entry geo_keys[4];

  // projected coordinates
  geo_keys[0].key_id = 1024; // GTModelTypeGeoKey
  geo_keys[0].tiff_tag_location = 0;
  geo_keys[0].count = 1;
  geo_keys[0].value_offset = 2; // ModelTypeGeographic

  // ellipsoid used with latitude/longitude coordinates
  geo_keys[1].key_id = 2048; // GeographicTypeGeoKey
  geo_keys[1].tiff_tag_location = 0;
  geo_keys[1].count = 1;
  geo_keys[1].value_offset = 4326; // GCS_WGS_84 

  // vertical units
  geo_keys[2].key_id = 4099; // VerticalUnitsGeoKey
  geo_keys[2].tiff_tag_location = 0;
  geo_keys[2].count = 1;
  geo_keys[2].value_offset = 9001; // meter

  // vertical datum
  geo_keys[3].key_id = 4096; // VerticalCSTypeGeoKey
  geo_keys[3].tiff_tag_location = 0;
  geo_keys[3].count = 1;
  geo_keys[3].value_offset = 5030; // VertCS_WGS_84_ellipsoid

  header.set_geo_keys(4, geo_keys);

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

BOOL LASreaderQFIT::open(ByteStreamIn* stream)
{
  U32 i;

  if (stream == 0)
  {
    fprintf(stderr,"ERROR: ByteStreamIn* pointer is zero\n");
    return FALSE;
  }

  this->stream = stream;

  // read the QFIT header

  try { stream->get32bitsLE((U8*)&version); } catch(...)
  {
    fprintf(stderr,"ERROR: reading QFIT header\n");
    return FALSE;
  }

  // is QFIT file little-endian

  if (version == 40 || version == 48 || version == 56)
  {
    little_endian = TRUE;
    endian_swap = (IS_LITTLE_ENDIAN() == FALSE);
  }
  else
  {
    ENDIAN_SWAP_32((U8*)&version);
    if (version == 40 || version == 48 || version == 56)
    {
      little_endian = FALSE;
      endian_swap = (IS_LITTLE_ENDIAN() == TRUE);
    }
    else
    {
      fprintf(stderr,"ERROR: corrupt QFIT header.\n");
      return FALSE;
    }
  }

  // read version bytes until point start offset

  try { stream->getBytes((U8*)buffer, version); } catch(...)
  {
    fprintf(stderr,"ERROR: reading %d bytes until point start offset from QFIT header\n", version);
    return FALSE;
  }

  // read point start offset

  try { if (little_endian) stream->get32bitsLE((U8*)&offset); else stream->get32bitsBE((U8*)&offset); } catch(...)
  {
    fprintf(stderr,"ERROR: reading point start offset from QFIT header\n");
    return FALSE;
  }

  // seek to end of file find out number of points

  stream->seekEnd();
  npoints = (stream->tell() - offset) / version;

  // seek back to start of points

  stream->seek(offset);

  // populate the header as much as possible

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderQFIT (%d)", LAS_TOOLS_VERSION);

  header.number_of_point_records = (U32)npoints;
  header.number_of_points_by_return[0] = header.number_of_point_records;

  header.extended_number_of_point_records = npoints;
  header.extended_number_of_points_by_return[0] = npoints;

  header.x_scale_factor = 0.000001;
  header.y_scale_factor = 0.000001;
  header.z_scale_factor = 0.001;
  header.x_offset = 0;
  header.y_offset = 0;
  header.z_offset = 0;

  try { 
    LASattribute scan_azimuth(LAS_ATTRIBUTE_I32, "scan azimuth", "Scan Azimuth (degrees X 1,000)");
    scan_azimuth.set_scale(0.001);
    scan_azimuth.set_min(0);
    scan_azimuth.set_max(360000);
    header.add_attribute(scan_azimuth);
  }
  catch(...) {
    fprintf(stderr,"ERROR: initializing attribute scan_azimuth\n");
    return FALSE;
  }


  try { 
    LASattribute pitch(LAS_ATTRIBUTE_I32, "pitch", "Pitch (degrees X 1,000)");
    pitch.set_scale(0.001);
    pitch.set_min(-90000);
    pitch.set_max(90000);
    header.add_attribute(pitch);
  }
  catch(...) {
    fprintf(stderr,"ERROR: initializing attribute pitch\n");
    return FALSE;
  }


  try { 
    LASattribute roll(LAS_ATTRIBUTE_I32, "roll", "Roll (degrees X 1,000)");
    roll.set_scale(0.001);
    roll.set_min(-90000);
    roll.set_max(90000);
    header.add_attribute(roll);
  }
  catch(...) {
    fprintf(stderr,"ERROR: initializing attribute roll\n");
    return FALSE;
  }

  if (version == 48)
  {
    try { 
      LASattribute pulse_width(LAS_ATTRIBUTE_U8, "pulse width", "Pulse Width (digitizer samples)");
      header.add_attribute(pulse_width);
    }
    catch(...) {
      fprintf(stderr,"ERROR: initializing attribute pulse width\n");
      return FALSE;
    }
  }

  header.update_extra_bytes_vlr();

  // set point type

  header.point_data_format = 1;
  header.point_data_record_length = 28 + header.get_attributes_size();

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length, &header);

  // initialize starts for attributes stored as extra bytes

  scan_azimuth_start = point.attributer->get_attribute_start("scan azimuth");
  pitch_start = point.attributer->get_attribute_start("pitch");
  roll_start = point.attributer->get_attribute_start("roll");
  if (version == 48)
  {
    pulse_width_start = point.attributer->get_attribute_start("pulse width");
  }

  // set point count to zero

  p_count = 0;

  // approximate bounding box init

  populated_header = FALSE;

  if (!read_point()) return FALSE;

  header.min_x = header.max_x = point.get_x();
  header.min_y = header.max_y = point.get_y();
  header.min_z = header.max_z = point.get_z();

  for (i = header.number_of_point_records/50; i < header.number_of_point_records; i += header.number_of_point_records/50)
  {
    if (!seek(i)) return FALSE;
    if (!read_point()) return FALSE;
  }

  return seek(0);
}

BOOL LASreaderQFIT::seek(const I64 p_index)
{
  if (p_index < npoints)
  {
    p_count = p_index;
    return stream->seek(p_index*version+offset);
  }
  return FALSE;
}

BOOL LASreaderQFIT::read_point_default()
{
  if (p_count < npoints)
  {
    try { stream->getBytes((U8*)buffer, version); } catch(...)
    {
      fprintf(stderr,"ERROR: reading QFIT point after %u of %u\n", (U32)p_count, (U32)npoints);
      return FALSE;
    }

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
      ENDIAN_SWAP_32((U8*)&buffer[9]);
      if (version >= 48)
      {
        ENDIAN_SWAP_32((U8*)&buffer[10]);
        ENDIAN_SWAP_32((U8*)&buffer[11]);
      }
      if (version >= 56)
      {
        ENDIAN_SWAP_32((U8*)&buffer[12]);
        ENDIAN_SWAP_32((U8*)&buffer[13]);
      }
    }

    point.gps_time = 0.001*buffer[0];
    point.set_X(buffer[2]);
    if (point.get_X() > 180000000) point.set_X(point.get_X() - 360000000); //  convert LARGE positive east longitude to negative
    point.set_Y(buffer[1]);
    point.set_Z(buffer[3]);
    point.intensity = buffer[5];
    point.scan_angle_rank = I8_CLAMP(I16_QUANTIZE((0.001*buffer[6])-180.0));

    point.set_attribute(scan_azimuth_start, (I32)buffer[6]);
    point.set_attribute(pitch_start, (I32)buffer[7]);
    point.set_attribute(roll_start, (I32)buffer[8]);
    if (version == 48)
    {
      point.set_attribute(pulse_width_start, (U8)buffer[10]);
    }

    if (!populated_header)
    {
      point.compute_coordinates();
      // update bounding box
      if (point.coordinates[0] < header.min_x) header.min_x = point.coordinates[0];
      else if (point.coordinates[0] > header.max_x) header.max_x = point.coordinates[0];
      if (point.coordinates[1] < header.min_y) header.min_y = point.coordinates[1];
      else if (point.coordinates[1] > header.max_y) header.max_y = point.coordinates[1];
      if (point.coordinates[2] < header.min_z) header.min_z = point.coordinates[2];
      else if (point.coordinates[2] > header.max_z) header.max_z = point.coordinates[2];
    }
    
    p_count++;
    return TRUE;
  }
  populated_header = TRUE;
  return FALSE;
}

ByteStreamIn* LASreaderQFIT::get_stream() const
{
  return stream;
}

void LASreaderQFIT::close(BOOL close_stream)
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

BOOL LASreaderQFIT::reopen(const char* file_name)
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

  // create input stream

  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamInFileLE(file);
  else
    stream = new ByteStreamInFileBE(file);

  p_count = 0;

  return stream->seek(offset);
}

LASreaderQFIT::LASreaderQFIT()
{
  file = 0;
  stream = 0;
  version = 0;
  little_endian = TRUE;
  endian_swap = FALSE;
  offset = 0;
  populated_header = FALSE;
  scan_azimuth_start = -1;
  pitch_start = -1;
  roll_start = -1;
  pulse_width_start = -1;
}

LASreaderQFIT::~LASreaderQFIT()
{
  if (stream) close();
}

LASreaderQFITrescale::LASreaderQFITrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderQFIT()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderQFITrescale::open(ByteStreamIn* stream)
{
  if (!LASreaderQFIT::open(stream)) return FALSE;
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

LASreaderQFITreoffset::LASreaderQFITreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderQFIT()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderQFITreoffset::open(ByteStreamIn* stream)
{
  if (!LASreaderQFIT::open(stream)) return FALSE;
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

LASreaderQFITrescalereoffset::LASreaderQFITrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderQFITrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderQFITreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderQFITrescalereoffset::open(ByteStreamIn* stream)
{
  if (!LASreaderQFIT::open(stream)) return FALSE;
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
