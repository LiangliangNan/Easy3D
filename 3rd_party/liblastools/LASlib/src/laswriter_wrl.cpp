/*
===============================================================================

  FILE:  laswriter_wrl.cpp
  
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
#include "laswriter_wrl.hpp"

#include <stdlib.h>
#include <string.h>


BOOL LASwriterWRL::open(const CHAR* file_name, const LASheader* header, const CHAR* parse_string)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  file = fopen(file_name, "w");

  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  close_file = TRUE;

  return open(file, header, parse_string);
}

BOOL LASwriterWRL::open(FILE* file, const LASheader* header, const CHAR* parse_string)
{
  if (file == 0)
  {
    fprintf(stderr,"ERROR: file pointer is zero\n");
    return FALSE;
  }

  this->file = file;
  this->header = header;

  if (parse_string)
  {
    if (strstr(parse_string, "RGB"))
    {
      if (header->point_data_format == 2 || header->point_data_format == 3 || header->point_data_format == 5)
      {
        if (rgb == 0)
        {
          rgb_alloc = 1024;
          rgb = (U8*)malloc(3*sizeof(U8)*rgb_alloc);
        }
      }
      else
      {
        fprintf(stderr,"WARNING: points do not have RGB colors\n");
        if (rgb)
        {
          rgb_alloc = 0;
          free(rgb);
          rgb = 0;
        }
      }
    }
  }
  else
  {
    if (header->point_data_format == 2 || header->point_data_format == 3 || header->point_data_format == 5)
    {
      if (rgb == 0)
      {
        rgb_alloc = 1024;
        rgb = (U8*)malloc(3*sizeof(U8)*rgb_alloc);
      }
    }
    else
    {
      if (rgb)
      {
        rgb_alloc = 0;
        free(rgb);
        rgb = 0;
      }
    }
  }
  // write header
  fprintf(file, "Shape {\012");
  if (rgb == 0)
  {
    fprintf(file, "\tappearance Appearance {\012");
    fprintf(file, "\t\tmaterial Material {\012");
    fprintf(file, "\t\t\temmissiveColor .5 .2 .7\012");
    fprintf(file, "\t\t}\012");
    fprintf(file, "\t}\012");
  }
  fprintf(file, "\tgeometry PointSet {\012");
  fprintf(file, "\t\tcoord Coordinate {\012");
  fprintf(file, "\t\t\tpoint [\012");
  return TRUE;
}

static void lidardouble2string(CHAR* string, double value)
{
  int len;
  len = sprintf(string, "%.15f", value) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
}

static void lidardouble2string(CHAR* string, double value, double precision)
{
  if (precision == 0.1)
    sprintf(string, "%.1f", value);
  else if (precision == 0.01)
    sprintf(string, "%.2f", value);
  else if (precision == 0.001)
    sprintf(string, "%.3f", value);
  else if (precision == 0.0001)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00001)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000001)
    sprintf(string, "%.6f", value);
  else if (precision == 0.0000001)
    sprintf(string, "%.7f", value);
  else if (precision == 0.00000001)
    sprintf(string, "%.8f", value);
  else if (precision == 0.000000001)
    sprintf(string, "%.9f", value);
  else
    lidardouble2string(string, value);
}

BOOL LASwriterWRL::write_point(const LASpoint* point)
{
  lidardouble2string(printstring, header->get_x(point->get_X()), header->x_scale_factor); fprintf(file, "%s ", printstring);
  lidardouble2string(printstring, header->get_y(point->get_Y()), header->y_scale_factor); fprintf(file, "%s ", printstring);
  lidardouble2string(printstring, header->get_z(point->get_Z()), header->z_scale_factor); fprintf(file, "%s\012", printstring);
  if (rgb)
  {
    if (p_count == rgb_alloc)
    {
      rgb_alloc *= 2;
      rgb = (U8*)realloc(rgb, 3*sizeof(U8)*rgb_alloc);
    }
    if (point->rgb[0] > 255)
      rgb[3*p_count+0] = U8_CLAMP(point->rgb[0]/256);
    else
      rgb[3*p_count+0] = U8_CLAMP(point->rgb[0]);
    if (point->rgb[1] > 255)
      rgb[3*p_count+1] = U8_CLAMP(point->rgb[1]/256);
    else
      rgb[3*p_count+1] = U8_CLAMP(point->rgb[1]);
    if (point->rgb[2] > 255)
      rgb[3*p_count+2] = U8_CLAMP(point->rgb[2]/256);
    else
      rgb[3*p_count+2] = U8_CLAMP(point->rgb[2]);
  }
  p_count++;
  return TRUE;
}

BOOL LASwriterWRL::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  return TRUE;
}

I64 LASwriterWRL::close(BOOL update_header)
{
  if (file == 0)
  {
    return 0;
  }
  fprintf(file, "\t\t\t]\012");
  fprintf(file, "\t\t}\012");
  if (rgb)
  {
    fprintf(file, "\t\tcolor Color {\012");
    fprintf(file, "\t\t\tcolor [\012");
    for (I32 i = 0; i < p_count; i++)
    {
      fprintf(file, "%.2f %.2f %.2f\012",((1.0f/255.0f)*rgb[3*i]),((1.0f/255.0f)*rgb[3*i+1]),((1.0f/255.0f)*rgb[3*i+2]));
    }
    fprintf(file, "\t\t\t]\012");
    fprintf(file, "\t\t}\012");
  }
  fprintf(file, "\t}\012");
  fprintf(file, "}\012");

  I64 bytes = ftell(file);

  if (file)
  {
    if (close_file)
    {
      fclose(file);
      close_file = FALSE;
    }
    file = 0;
  }

  npoints = p_count;
  p_count = 0;

  return bytes;
}

LASwriterWRL::LASwriterWRL()
{
  close_file = FALSE;
  file = 0;
  rgb = 0;
  rgb_alloc = 0;
}

LASwriterWRL::~LASwriterWRL()
{
  if (file) close();
}
