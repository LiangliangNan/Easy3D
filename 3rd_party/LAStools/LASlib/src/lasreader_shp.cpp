/*
===============================================================================

  FILE:  lasreader_shp.cpp
  
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
#include "lasreader_shp.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" FILE* fopen_compressed(const char* filename, const char* mode, bool* piped);

static bool is_little_endian = true;

static void initialize_endianness()
{
  int i = 1;
  if (*((unsigned char*)&i) == 1)
    is_little_endian = true;
  else
    is_little_endian = false;
}

static void from_big_endian(int* value)
{
  if (is_little_endian)
  {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[3];
    field[3] = help;
    help = field[1];
    field[1] = field[2];
    field[2] = help;
  }
}

static void from_little_endian(int* value)
{
  if (!is_little_endian)
  {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[3];
    field[3] = help;
    help = field[1];
    field[1] = field[2];
    field[2] = help;
  }
}

static void from_little_endian(double* value)
{
  if (!is_little_endian)
  {
    char help;
    char* field = (char*)value;
    help = field[0];
    field[0] = field[7];
    field[7] = help;
    help = field[1];
    field[1] = field[6];
    field[6] = help;
    help = field[2];
    field[2] = field[5];
    field[5] = help;
    help = field[3];
    field[3] = field[4];
    field[4] = help;
  }
}

BOOL LASreaderSHP::open(const char* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  clean();

  file = fopen_compressed(file_name, "rb", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  // clean the header

  header.clean();

  // populate the header as much as it makes sense

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderSHP (%d)", LAS_TOOLS_VERSION);

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
  header.file_creation_day = 111;
  header.file_creation_year = 2011;
#endif

  header.point_data_format = 0;
  header.point_data_record_length = 20;

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length);

  // read SHP header and populate the LAS header with the bounding box

  int int_input;
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // file code (BIG)
  from_big_endian(&int_input); 
  if (int_input != 9994)
  {
    fprintf(stderr, "ERROR: wrong shapefile code %d != 9994\n", int_input);
    return FALSE;
  }
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // file length (BIG)
  from_big_endian(&int_input); 
  int file_length = int_input;
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // version (LITTLE)
  from_little_endian(&int_input); 
  if (int_input != 1000)
  {
    fprintf(stderr, "ERROR: wrong shapefile version %d != 1000\n", int_input);
    return FALSE;
  }
  if (fread(&int_input, sizeof(int), 1, file) != 1) return false; // shape type (LITTLE)
  from_little_endian(&int_input); 
  shape_type = int_input;
  if (shape_type != 1 && shape_type != 11 && shape_type != 21 && shape_type != 8 && shape_type != 18 && shape_type != 28)
  {
    fprintf(stderr, "ERROR: wrong shape type %d != 1,11,21,8,18,28\n", shape_type);
    return FALSE;
  }
  double double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // xmin (LITTLE)
  from_little_endian(&double_input);
  header.min_x = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // ymin (LITTLE)
  from_little_endian(&double_input); 
  header.min_y = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // xmax (LITTLE)
  from_little_endian(&double_input); 
  header.max_x = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // ymax (LITTLE)
  from_little_endian(&double_input); 
  header.max_y = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // zmin (LITTLE)
  from_little_endian(&double_input); 
  header.min_z = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // zmax (LITTLE)
  from_little_endian(&double_input); 
  header.max_z = double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // mmin (LITTLE)
  from_little_endian(&double_input); 
  if (fread(&double_input, sizeof(double), 1, file) != 1) return false; // mmax (LITTLE)
  from_little_endian(&double_input); 

  // maybe calculate npoints

  if (shape_type == 1)
  {
    npoints = (file_length-50)/(14);
  }
  else if (shape_type == 11)
  {
    npoints = (file_length-50)/(22);
  }
  else if (shape_type == 21)
  {
    npoints = (file_length-50)/(18);
  }
  else if (shape_type == 8)
  {
    npoints = (file_length-50-20)/(8); // over-estimate (assumes all in one record)
  }
  else if (shape_type == 18)
  {
    npoints = (file_length-50-36)/(16); // over-estimate (assumes all in one record)
  }
  else if (shape_type == 28)
  {
    npoints = (file_length-50-28)/(12); // over-estimate (assumes all in one record)
  }
  header.number_of_point_records = (U32)npoints;
  header.number_of_points_by_return[0] = (U32)npoints;

  // figure out the right scale factor and offset

  populate_scale_and_offset();

  // populate the quantized bounding box

  populate_bounding_box();
  
  p_count = 0;

  return TRUE;
}

void LASreaderSHP::set_scale_factor(const F64* scale_factor)
{
  if (scale_factor)
  {
    if (this->scale_factor == 0) this->scale_factor = new F64[3];
    this->scale_factor[0] = scale_factor[0];
    this->scale_factor[1] = scale_factor[1];
    this->scale_factor[2] = scale_factor[2];
  }
  else if (this->scale_factor)
  {
    delete [] this->scale_factor;
    this->scale_factor = 0;
  }
}

void LASreaderSHP::set_offset(const F64* offset)
{
  if (offset)
  {
    if (this->offset == 0) this->offset = new F64[3];
    this->offset[0] = offset[0];
    this->offset[1] = offset[1];
    this->offset[2] = offset[2];
  }
  else if (this->offset)
  {
    delete [] this->offset;
    this->offset = 0;
  }
}

BOOL LASreaderSHP::seek(const I64 p_index)
{
  return FALSE;
}

BOOL LASreaderSHP::read_point_default()
{
  if (point_count == number_of_points)
  {
    int i, int_input;
    if (fread(&int_input, sizeof(int), 1, file) != 1) { npoints = p_count; return FALSE; }; // record number (BIG)
    if (fread(&int_input, sizeof(int), 1, file) != 1) { npoints = p_count; return FALSE; }; // content length (BIG)
    if (fread(&int_input, sizeof(int), 1, file) != 1) { npoints = p_count; return FALSE; }; // shape type (LITTLE)
    from_little_endian(&int_input);
    if (int_input != shape_type)
    {
      fprintf(stderr, "WARNING: wrong shape type %d != %d in record\n", int_input, shape_type);
    }
    double double_input;
    if (shape_type == 8 || shape_type == 18 || shape_type == 28) // Multipoint
    {
      if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // xmin (LITTLE)
      if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // ymin (LITTLE)
      if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // xmax (LITTLE)
      if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // ymax (LITTLE)
      if (fread(&int_input, sizeof(int), 1, file) != 1) { npoints = p_count; return FALSE; }; // number of points (LITTLE)
      from_little_endian(&int_input);
      number_of_points = int_input;
    }
    else // or regular point?
    {
      number_of_points = 1;
    }
    if (shape_type == 11 || shape_type == 18)
    {
      if (points_allocated < number_of_points)
      {
        if (points) delete [] points;
        points = new I32[number_of_points*3];
        points_allocated = number_of_points;
      }
      // read points x and y
      for (i = 0; i < number_of_points; i++)
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // x of point (LITTLE)
        from_little_endian(&double_input);
        points[3*i+0] = header.get_X(double_input);
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // y of point (LITTLE)
        from_little_endian(&double_input);
        points[3*i+1] = header.get_Y(double_input);
      }
      // read points z and write LAS points
      if (shape_type == 18) // Multipoint
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // zmin (LITTLE)
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // zmin (LITTLE)
      }
      for (i = 0; i < number_of_points; i++)
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // z of point (LITTLE)
        from_little_endian(&double_input);
        points[3*i+2] = header.get_Z(double_input);
      }
    }
    else
    {
      if (points_allocated < number_of_points)
      {
        if (points) delete [] points;
        points = new I32[number_of_points*2];
        points_allocated = number_of_points;
      }
      // read points x and y and write LAS points
      for (i = 0; i < number_of_points; i++)
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // x of point (LITTLE)
        from_little_endian(&double_input);
        points[2*i+0] = header.get_Z(double_input);
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // y of point (LITTLE)
        from_little_endian(&double_input);
        points[2*i+1] = header.get_Z(double_input);
      }
    }
    // read points m
    if (shape_type == 11 || shape_type == 21 || shape_type == 18 || shape_type == 28)
    {
      if (shape_type == 18 || shape_type == 28) // Multipoint
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // mmin (LITTLE)
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // mmin (LITTLE)
      }
      for (i = 0; i < number_of_points; i++)
      {
        if (fread(&double_input, sizeof(double), 1, file) != 1) { npoints = p_count; return FALSE; }; // m of point (LITTLE)
      }
    }
    point_count = 0;
  }
  if (shape_type == 11 || shape_type == 18)
  {
    point.set_X(points[3*point_count+0]);
    point.set_Y(points[3*point_count+1]);
    point.set_Z(points[3*point_count+2]);
  }
  else
  {
    point.set_X(points[2*point_count+0]);
    point.set_Y(points[2*point_count+1]);
    point.set_Z(0);
  }
  p_count++;
  point_count++;
  return TRUE;
}

ByteStreamIn* LASreaderSHP::get_stream() const
{
  return 0;
}

void LASreaderSHP::close(BOOL close_stream)
{
  if (file)
  {
    if (piped) while(fgetc(file) != EOF);
    fclose(file);
    file = 0;
  }
}

BOOL LASreaderSHP::reopen(const char* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  file = fopen_compressed(file_name, "rb", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot reopen file '%s'\n", file_name);
    return FALSE;
  }

  // read SHP header again to skip to the right point in the file

  int int_input;
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // file code (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // unused (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // file length (BIG)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // version (LITTLE)
  if (fread(&int_input, sizeof(int), 1, file) != 1) return FALSE; // shape type (LITTLE)
  double double_input;
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // xmin (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // ymin (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // xmax (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // ymax (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // zmin (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // zmax (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // mmin (LITTLE)
  if (fread(&double_input, sizeof(double), 1, file) != 1) return FALSE; // mmax (LITTLE)

  p_count = 0;

  return TRUE;
}

void LASreaderSHP::clean()
{
  if (file)
  {
    if (piped) while(fgetc(file) != EOF);
    fclose(file);
    file = 0;
  }
  number_of_points = 0;
  point_count = 0;
}

LASreaderSHP::LASreaderSHP()
{
  initialize_endianness();

  file = 0;
  piped = false;
  scale_factor = 0;
  offset = 0;
  shape_type = 0;
  points = 0;
  points_allocated = 0;
  number_of_points = 0;
  point_count = 0;
}

LASreaderSHP::~LASreaderSHP()
{
  clean();
  if (scale_factor)
  {
    delete [] scale_factor;
    scale_factor = 0;
  }
  if (offset)
  {
    delete [] offset;
    offset = 0;
  }
  if (points)
  {
    delete [] points;
    points = 0;
  }
}

void LASreaderSHP::populate_scale_and_offset()
{
  // if not specified in the command line, set a reasonable scale_factor
  if (scale_factor)
  {
    header.x_scale_factor = scale_factor[0];
    header.y_scale_factor = scale_factor[1];
    header.z_scale_factor = scale_factor[2];
  }
  else
  {
    if (-360 < header.min_x  && -360 < header.min_y && header.max_x < 360 && header.max_y < 360) // do we have longitude / latitude coordinates
    {
      header.x_scale_factor = 1e-7;
      header.y_scale_factor = 1e-7;
    }
    else // then we assume utm or mercator / lambertian projections
    {
      header.x_scale_factor = 0.01;
      header.y_scale_factor = 0.01;
    }
    header.z_scale_factor = 0.01;
  }

  // if not specified in the command line, set a reasonable offset
  if (offset)
  {
    header.x_offset = offset[0];
    header.y_offset = offset[1];
    header.z_offset = offset[2];
  }
  else
  {
    if (F64_IS_FINITE(header.min_x) && F64_IS_FINITE(header.max_x))
      header.x_offset = ((I64)((header.min_x + header.max_x)/header.x_scale_factor/20000000))*10000000*header.x_scale_factor;
    else
      header.x_offset = 0;

    if (F64_IS_FINITE(header.min_y) && F64_IS_FINITE(header.max_y))
      header.y_offset = ((I64)((header.min_y + header.max_y)/header.y_scale_factor/20000000))*10000000*header.y_scale_factor;
    else
      header.y_offset = 0;

    if (F64_IS_FINITE(header.min_z) && F64_IS_FINITE(header.max_z))
      header.z_offset = ((I64)((header.min_z + header.max_z)/header.z_scale_factor/20000000))*10000000*header.z_scale_factor;
    else
      header.z_offset = 0;
  }
}

void LASreaderSHP::populate_bounding_box()
{
  // compute quantized and then unquantized bounding box

  F64 dequant_min_x = header.get_x(header.get_X(header.min_x));
  F64 dequant_max_x = header.get_x(header.get_X(header.max_x));
  F64 dequant_min_y = header.get_y(header.get_Y(header.min_y));
  F64 dequant_max_y = header.get_y(header.get_Y(header.max_y));
  F64 dequant_min_z = header.get_z(header.get_Z(header.min_z));
  F64 dequant_max_z = header.get_z(header.get_Z(header.max_z));

  // make sure there is not sign flip

  if ((header.min_x > 0) != (dequant_min_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_x from %g to %g.\n", header.min_x, dequant_min_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.min_x = dequant_min_x;
  }
  if ((header.max_x > 0) != (dequant_max_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_x from %g to %g.\n", header.max_x, dequant_max_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.max_x = dequant_max_x;
  }
  if ((header.min_y > 0) != (dequant_min_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_y from %g to %g.\n", header.min_y, dequant_min_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.min_y = dequant_min_y;
  }
  if ((header.max_y > 0) != (dequant_max_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_y from %g to %g.\n", header.max_y, dequant_max_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.max_y = dequant_max_y;
  }
  if ((header.min_z > 0) != (dequant_min_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_z from %g to %g.\n", header.min_z, dequant_min_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.min_z = dequant_min_z;
  }
  if ((header.max_z > 0) != (dequant_max_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_z from %g to %g.\n", header.max_z, dequant_max_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.max_z = dequant_max_z;
  }
}

LASreaderSHPrescale::LASreaderSHPrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderSHP()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderSHPrescale::open(const char* file_name)
{
  if (!LASreaderSHP::open(file_name)) return FALSE;
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

LASreaderSHPreoffset::LASreaderSHPreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderSHP()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderSHPreoffset::open(const char* file_name)
{
  if (!LASreaderSHP::open(file_name)) return FALSE;
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

LASreaderSHPrescalereoffset::LASreaderSHPrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderSHPrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderSHPreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderSHPrescalereoffset::open(const char* file_name)
{
  if (!LASreaderSHP::open(file_name)) return FALSE;
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
