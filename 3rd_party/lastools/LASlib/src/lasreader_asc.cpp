/*
===============================================================================

  FILE:  lasreader_asc.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreader_asc.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" FILE* fopen_compressed(const char* filename, const char* mode, bool* piped);

BOOL LASreaderASC::open(const CHAR* file_name, BOOL comma_not_point)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  clean();
  this->comma_not_point = comma_not_point;

  file = fopen_compressed(file_name, "r", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 10*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  // clean the header

  header.clean();

  // populate the header as much as it makes sense

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderASC (%d)", LAS_TOOLS_VERSION);

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
  header.file_creation_year = 2012;
#endif

  header.point_data_format = 0;
  header.point_data_record_length = 20;

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length, &header);

  // read header of ASC file

  if (line == 0)
  {
    line_size = 1024;
    line = (CHAR*)malloc(sizeof(CHAR)*line_size);
  }

  CHAR dummy[32];
  BOOL complete = FALSE;
  ncols = 0;
  nrows = 0;
  F64 xllcorner = F64_MAX;
  F64 yllcorner = F64_MAX;
  xllcenter = F64_MAX;
  xllcenter = F64_MAX;
  cellsize = 0;
  nodata = -9999;
  header_lines = 0;

  while (!complete)
  {
    if (!fgets(line, line_size, file)) break;

    // special handling for European numbers

    if (comma_not_point)
    {
      I32 i, len = (I32)strlen(line);
      for (i = 0; i < len; i++)
      {
        if (line[i] == ',') line[i] = '.';
      }
    }

    if (strstr(line, "ncols") || strstr(line, "NCOLS"))
    {
      sscanf(line, "%s %d", dummy, &ncols);
      free(line);
      line_size = 1024+50*ncols;
      line = (CHAR*)malloc(sizeof(CHAR)*line_size);
    }
    else if (strstr(line, "nrows") || strstr(line, "NROWS"))
    {
      sscanf(line, "%s %d", dummy, &nrows);
    }
    else if (strstr(line, "xllcorner") || strstr(line, "XLLCORNER"))
    {
      sscanf(line, "%s %lf", dummy, &xllcorner);
    }
    else if (strstr(line, "yllcorner") || strstr(line, "YLLCORNER"))
    {
      sscanf(line, "%s %lf", dummy, &yllcorner);
    }
    else if (strstr(line, "xllcenter") || strstr(line, "XLLCENTER"))
    {
      sscanf(line, "%s %lf", dummy, &xllcenter);
    }
    else if (strstr(line, "yllcenter") || strstr(line, "YLLCENTER"))
    {
      sscanf(line, "%s %lf", dummy, &yllcenter);
    }
    else if (strstr(line, "cellsize") || strstr(line, "CELLSIZE"))
    {
      sscanf(line, "%s %f", dummy, &cellsize);
    }
    else if (strstr(line, "nodata_value") || strstr(line, "NODATA_VALUE") || strstr(line, "nodata_VALUE") || strstr(line, "NODATA_value"))
    {
      sscanf(line, "%s %f", dummy, &nodata);
    }
    else if ((ncols != 0) && (nrows != 0) && (((xllcorner != F64_MAX) && (yllcorner != F64_MAX)) || ((xllcenter != F64_MAX) && (yllcenter != F64_MAX))) && (cellsize > 0))
    {
      if (ncols == 1)
      {
        F32 e0, e1;
        if ( sscanf(line, "%f %f", &e0, &e1) == 1)
        {
          complete = TRUE;
        }
      }
      else if (ncols == 2)
      {
        F32 e0, e1, e2;
        if ( sscanf(line, "%f %f %f", &e0, &e1, &e2) == 2)
        {
          complete = TRUE;
        }
      }
      else if (ncols == 3)
      {
        F32 e0, e1, e2, e3;
        if ( sscanf(line, "%f %f %f %f", &e0, &e1, &e2, &e3) == 3)
        {
          complete = TRUE;
        }
      }
      else if (ncols == 4)
      {
        F32 e0, e1, e2, e3, e4;
        if ( sscanf(line, "%f %f %f %f %f", &e0, &e1, &e2, &e3, &e4) == 4)
        {
          complete = TRUE;
        }
      }
      else
      {
        F32 e0, e1, e2, e3, e4;
        if ( sscanf(line, "%f %f %f %f %f", &e0, &e1, &e2, &e3, &e4) == 5)
        {
          complete = TRUE;
        }
      }
    }
    header_lines++;
  }

  if (!complete)
  {
    fprintf(stderr,"ERROR: was not able to find header\n");
    return FALSE;
  }

  // shift the llcorner to the pixel center

  if ((xllcorner != F64_MAX) && (yllcorner != F64_MAX))
  {
    xllcenter = xllcorner + 0.5*cellsize;
    yllcenter = yllcorner + 0.5*cellsize;
  }

  // init the bounding box x y

  header.min_x = xllcenter;
  header.min_y = yllcenter;
  header.max_x = xllcenter + (ncols-1)*cellsize;
  header.max_y = yllcenter + (nrows-1)*cellsize;

  // init the bounding box z and count the rasters

  F32 elevation = 0;
  npoints = 0;
  header.min_z = F64_MAX;
  header.max_z = F64_MIN;

  // skip leading spaces
  line_curr = 0;
  while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;

  for (row = 0; row < nrows; row++)
  {
    for (col = 0; col < ncols; col++)
    {
      if (line[line_curr] == '\0')
      {
        if (!fgets(line, line_size, file))
        {
#ifdef _WIN32
          fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
          fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        }

        // special handling for European numbers

        if (comma_not_point)
        {
          I32 i, len = (I32)strlen(line);
          for (i = 0; i < len; i++)
          {
            if (line[i] == ',') line[i] = '.';
          }
        }

        // skip leading spaces
        line_curr = 0;
        while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;
      }
      // get elevation value
      sscanf(&(line[line_curr]), "%f", &elevation);
      // skip parsed number
      while ((line[line_curr] != '\0') && (line[line_curr] > ' ')) line_curr++;
      // skip following spaces
      while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;
      // should we use the raster
      if (elevation != nodata)
      {
        npoints++;
        if (header.max_z < elevation) header.max_z = elevation;
        if (header.min_z > elevation) header.min_z = elevation;
      }
    }
  }

  // close the ASC file

  close();

  // check the header values

  header.number_of_point_records = (U32)npoints;

  if (npoints)
  {
    // populate scale and offset

    populate_scale_and_offset();

    // check bounding box for this scale and offset

    populate_bounding_box();
  }
  else
  {
    fprintf(stderr,"WARNING: ASC raster contains only no data values\n");
    header.min_z = 0;
    header.max_z = 0;
  }

  // reopen

  return reopen(file_name);
}

void LASreaderASC::set_scale_factor(const F64* scale_factor)
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

void LASreaderASC::set_offset(const F64* offset)
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

BOOL LASreaderASC::seek(const I64 p_index)
{
  return FALSE;
}

BOOL LASreaderASC::read_point_default()
{
  F32 elevation;
  while (p_count < npoints)
  {
    if (line[line_curr] == '\0')
    {
      if (!fgets(line, line_size, file))
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        npoints = p_count;
        return FALSE;
      }

      // special handling for European numbers

      if (comma_not_point)
      {
        I32 i, len = (I32)strlen(line);
        for (i = 0; i < len; i++)
        {
          if (line[i] == ',') line[i] = '.';
        }
      }
      line_curr = 0;
      // skip leading spaces
      while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;
    }
    if (col == ncols)
    {
      col = 0;
      row++;
    }
    // get elevation value
    sscanf(&(line[line_curr]), "%f", &elevation);
    // skip parsed number
    while ((line[line_curr] != '\0') && (line[line_curr] > ' ')) line_curr++;
    // skip following spaces
    while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;
    // should we use the raster
    if (elevation != nodata)
    {
      // compute the quantized x, y, and z values
      point.set_x(xllcenter + col*cellsize);
      point.set_y(yllcenter + (nrows - row - 1) * cellsize);
      point.set_z(elevation);
      p_count++;
      col++;
      return TRUE;
    }
    else
    {
      col++;
    }
  }
  return FALSE;
}

ByteStreamIn* LASreaderASC::get_stream() const
{
  return 0;
}

void LASreaderASC::close(BOOL close_stream)
{
  if (file)
  {
    if (piped) while(fgets(line, line_size, file));
    fclose(file);
    file = 0;
  }
}

BOOL LASreaderASC::reopen(const CHAR* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  file = fopen_compressed(file_name, "r", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot reopen file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 10*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  // read the header lines

  I32 i;
  for (i = 0; i < header_lines; i++)
  {
    fgets(line, line_size, file);
  }

  // special handling for European numbers

  if (comma_not_point)
  {
    I32 i, len = (I32)strlen(line);
    for (i = 0; i < len; i++)
    {
      if (line[i] == ',') line[i] = '.';
    }
  }

  col = 0;
  row = 0;
  p_count = 0;

  // skip leading spaces
  line_curr = 0;
  while ((line[line_curr] != '\0') && (line[line_curr] <= ' ')) line_curr++;

  return TRUE;
}

void LASreaderASC::clean()
{
  if (file)
  {
    fclose(file);
    file = 0;
  }
  if (line)
  {
    free(line);
    line = 0;
  }
  header_lines = 0;
  line_size = 0;
  line_curr = 0;
  piped = false;
  comma_not_point = FALSE;
  col = 0;
  ncols = 0;
  nrows = 0;
  xllcenter = F64_MAX;
  yllcenter = F64_MAX;
  cellsize = 0;
  nodata = -9999;
}

LASreaderASC::LASreaderASC()
{
  file = 0;
  line = 0;
  scale_factor = 0;
  offset = 0;
  clean();
}

LASreaderASC::~LASreaderASC()
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
}

void LASreaderASC::populate_scale_and_offset()
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

void LASreaderASC::populate_bounding_box()
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

LASreaderASCrescale::LASreaderASCrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderASC()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderASCrescale::open(const CHAR* file_name, BOOL comma_not_point)
{
  LASreaderASC::set_scale_factor(scale_factor);
  if (!LASreaderASC::open(file_name, comma_not_point)) return FALSE;
  return TRUE;
}

LASreaderASCreoffset::LASreaderASCreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderASC()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderASCreoffset::open(const CHAR* file_name, BOOL comma_not_point)
{
  LASreaderASC::set_offset(offset);
  if (!LASreaderASC::open(file_name, comma_not_point)) return FALSE;
  return TRUE;
}

LASreaderASCrescalereoffset::LASreaderASCrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderASCrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderASCreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderASCrescalereoffset::open(const CHAR* file_name, BOOL comma_not_point)
{
  LASreaderASC::set_scale_factor(scale_factor);
  LASreaderASC::set_offset(offset);
  if (!LASreaderASC::open(file_name, comma_not_point)) return FALSE;
  return TRUE;
}
