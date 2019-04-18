/*
===============================================================================

  FILE:  lasreadermerged.cpp
  
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
#include "lasreadermerged.hpp"

#include "lasindex.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"

#include <stdlib.h>
#include <string.h>

void LASreaderMerged::set_io_ibuffer_size(I32 io_ibuffer_size)
{
  this->io_ibuffer_size = io_ibuffer_size;
}

BOOL LASreaderMerged::add_file_name(const char* file_name)
{
  // do we have a file name
  if (file_name == 0)
  {
    fprintf(stderr, "ERROR: file name pointer is NULL\n");
    return FALSE;
  }
  // does the file exist
  FILE* file = fopen(file_name, "r");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: file '%s' cannot be opened\n", file_name);
    return FALSE;
  }
  fclose(file);
  // check file extension
  if (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
  {
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with LAS. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderlas == 0)
    {
      lasreader = lasreaderlas = new LASreaderLAS();
    }
  }
  else if (strstr(file_name, ".bin") || strstr(file_name, ".BIN"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with BIN. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin == 0)
    {
      lasreader = lasreaderbin = new LASreaderBIN();
    }
  }
  else if (strstr(file_name, ".shp") || strstr(file_name, ".SHP"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with SHP. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp == 0)
    {
      lasreader = lasreadershp = new LASreaderSHP();
    }
  }
  else if (strstr(file_name, ".asc") || strstr(file_name, ".ASC"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with ASC. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc == 0)
    {
      lasreader = lasreaderasc = new LASreaderASC();
    }
  }
  else if (strstr(file_name, ".bil") || strstr(file_name, ".BIL"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with BIL. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil == 0)
    {
      lasreader = lasreaderbil = new LASreaderBIL();
    }
  }
  else if (strstr(file_name, ".dtm") || strstr(file_name, ".DTM"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with DTM. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm == 0)
    {
      lasreader = lasreaderdtm = new LASreaderDTM();
    }
  }
  else if (strstr(file_name, ".ply") || strstr(file_name, ".PLY"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with PLY. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply == 0)
    {
      lasreader = lasreaderply = new LASreaderPLY();
    }
  }
  else if (strstr(file_name, ".qi") || strstr(file_name, ".QI"))
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt)
    {
      fprintf(stderr, "ERROR: cannot mix TXT with QFIT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit == 0)
    {
      lasreader = lasreaderqfit = new LASreaderQFIT();
    }
  }
  else
  {
    if (lasreaderlas)
    {
      fprintf(stderr, "ERROR: cannot mix LAS with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbin)
    {
      fprintf(stderr, "ERROR: cannot mix BIN with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadershp)
    {
      fprintf(stderr, "ERROR: cannot mix SHP with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderasc)
    {
      fprintf(stderr, "ERROR: cannot mix ASC with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderbil)
    {
      fprintf(stderr, "ERROR: cannot mix BIL with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderdtm)
    {
      fprintf(stderr, "ERROR: cannot mix DTM with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderply)
    {
      fprintf(stderr, "ERROR: cannot mix PLY with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreaderqfit)
    {
      fprintf(stderr, "ERROR: cannot mix QFIT with TXT. skipping '%s' ...\n", file_name);
      return FALSE;
    }
    if (lasreadertxt == 0)
    {
      lasreader = lasreadertxt = new LASreaderTXT();
    }
  }
  // add the file
  if (file_name_number == file_name_allocated)
  {
    file_name_allocated += 1024;
    if (file_names)
    {
      file_names = (char**)realloc(file_names, sizeof(char*)*file_name_allocated);
    }
    else
    {
      file_names = (char**)malloc(sizeof(char*)*file_name_allocated);
    }
    if (file_names == 0)
    {
      fprintf(stderr, "ERROR: alloc for file_names pointer array failed at %d\n", file_name_allocated);
      return FALSE;
    }
  }
  file_names[file_name_number] = LASCopyString(file_name);
  file_name_number++;
  return TRUE;
}

void LASreaderMerged::set_scale_factor(const F64* scale_factor)
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

void LASreaderMerged::set_offset(const F64* offset)
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

void LASreaderMerged::set_files_are_flightlines(const I32 files_are_flightlines)
{
  this->files_are_flightlines = files_are_flightlines;
  // when merging multiple flightlines the merged header must have a file source ID of 0
  if (files_are_flightlines)
  {
    header.file_source_ID = 0;
  }
}

void LASreaderMerged::set_apply_file_source_ID(BOOL apply_file_source_ID)
{
  this->apply_file_source_ID = apply_file_source_ID;
  // when merging multiple flightlines the merged header must have a file source ID of 0
  if (apply_file_source_ID)
  {
    header.file_source_ID = 0;
  }
}

void LASreaderMerged::set_translate_intensity(F32 translate_intensity)
{
  this->translate_intensity = translate_intensity;
}

void LASreaderMerged::set_scale_intensity(F32 scale_intensity)
{
  this->scale_intensity = scale_intensity;
}

void LASreaderMerged::set_translate_scan_angle(F32 translate_scan_angle)
{
  this->translate_scan_angle = translate_scan_angle;
}

void LASreaderMerged::set_scale_scan_angle(F32 scale_scan_angle)
{
  this->scale_scan_angle = scale_scan_angle;
}

void LASreaderMerged::set_parse_string(const char* parse_string)
{
  if (this->parse_string) free(this->parse_string);
  if (parse_string)
  {
    this->parse_string = LASCopyString(parse_string);
  }
  else
  {
    this->parse_string = 0;
  }
}

void LASreaderMerged::set_skip_lines(I32 skip_lines)
{
  this->skip_lines = skip_lines;
}

void LASreaderMerged::set_populate_header(BOOL populate_header)
{
  this->populate_header = populate_header;
}

void LASreaderMerged::set_keep_lastiling(BOOL keep_lastiling)
{
  this->keep_lastiling = keep_lastiling;
}

BOOL LASreaderMerged::open()
{
  if (file_name_number == 0)
  {
    fprintf(stderr, "ERROR: no valid file names\n");
    return FALSE;
  }

  // allocate space for the individual bounding_boxes
  if (bounding_boxes) delete [] bounding_boxes;
  bounding_boxes = new F64[file_name_number*4];

  // clean  header
  header.clean();

  // combine all headers

  U32 i,j;
  BOOL first = TRUE;
  BOOL attributes = FALSE;

  for (i = 0; i < file_name_number; i++)
  {
    // open the lasreader with the next file name
    if (lasreaderlas)
    {
      if (!lasreaderlas->open(file_names[i], 512, (first == FALSE) && (attributes == FALSE))) // starting from second just "peek" into file to get bounding box and count
      {
        fprintf(stderr, "ERROR: could not open lasreaderlas for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderbin)
    {
      if (!lasreaderbin->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderbin for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreadershp)
    {
      if (!lasreadershp->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreadershp for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderasc)
    {
      if (!lasreaderasc->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderasc for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderbil)
    {
      if (!lasreaderbil->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderbil for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderdtm)
    {
      if (!lasreaderdtm->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderdtm for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderply)
    {
      if (!lasreaderply->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderply for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else if (lasreaderqfit)
    {
      if (!lasreaderqfit->open(file_names[i]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderqfit for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    else
    {
      lasreadertxt->set_translate_intensity(translate_intensity);
      lasreadertxt->set_scale_intensity(scale_intensity);
      lasreadertxt->set_translate_scan_angle(translate_scan_angle);
      lasreadertxt->set_scale_scan_angle(scale_scan_angle);
      lasreadertxt->set_scale_factor(scale_factor);
      lasreadertxt->set_offset(offset);
      if (!lasreadertxt->open(file_names[i], 0, parse_string, skip_lines, populate_header))
      {
        fprintf(stderr, "ERROR: could not open lasreadertxt for file '%s'\n", file_names[i]);
        return FALSE;
      }
    }
    // ignore bounding box if the file has no points
    if (lasreader->npoints == 0)
    {
      // record ignoring bounding box info
      bounding_boxes[4*i+0] = F64_MAX;
      bounding_boxes[4*i+1] = F64_MAX;
      bounding_boxes[4*i+2] = F64_MIN;
      bounding_boxes[4*i+3] = F64_MIN;
    }
    else
    {
      // record individual bounding box info
      bounding_boxes[4*i+0] = lasreader->header.min_x;
      bounding_boxes[4*i+1] = lasreader->header.min_y;
      bounding_boxes[4*i+2] = lasreader->header.max_x;
      bounding_boxes[4*i+3] = lasreader->header.max_y;
    }
    // populate the merged header
    if (first)
    {
      first = FALSE;
      // check for lastiling buffer
      if (lasreader->header.vlr_lastiling)
      {
        if (lasreader->header.vlr_lastiling->buffer)
        {
          fprintf(stderr, "WARNING: first file is a buffered tile. maybe remove buffers first?\n");
        }
      }
      // maybe we should keep the tiling
      if (keep_lastiling)
      {
        if (lasreader->header.vlr_lastiling == 0)
        {
          fprintf(stderr, "WARNING: first file has no LAStiling VLR cannot '-keep_lastiling' ...\n");
        }
      }
      else
      {
        // usually we delete the lastiling information as it becomes meaningless
        lasreader->header.clean_lastiling();
      }
      // use the entire header info from the first file
      header = lasreader->header;
      // unlink the pointers for other header so they don't get deallocated twice
      lasreader->header.unlink();
      // for LAS 1.4 (and 32-bit counter overflows)
      header.extended_number_of_point_records = (lasreader->header.number_of_point_records ? lasreader->header.number_of_point_records : lasreader->header.extended_number_of_point_records);
      for (j = 0; j < 5; j++)
      {
        header.extended_number_of_points_by_return[j] = (lasreader->header.number_of_points_by_return[j] ? lasreader->header.number_of_points_by_return[j] : lasreader->header.extended_number_of_points_by_return[j]);
      }
      if (header.version_minor >= 4)
      {
        for (j = 5; j < 15; j++)
        {
          header.extended_number_of_points_by_return[j] = lasreader->header.extended_number_of_points_by_return[j];
        }
      }
      // count the points up to 64 bits
      npoints = lasreader->npoints;
      // when merging multiple flightlines the merged header must have a file source ID of 0
      if (files_are_flightlines || apply_file_source_ID)
      {
        header.file_source_ID = 0;
      }
      if (header.number_attributes)
      {
        attributes = TRUE;
      }
    }
    else
    {
      if (lasreader->npoints)
      {
        // count the points up to 64 bits
        npoints += lasreader->npoints;
        // have there not been any points before
        if (npoints == lasreader->npoints)
        {
          // use the counters 
          header.number_of_point_records = lasreader->header.number_of_point_records;
          for (j = 0; j < 5; j++)
          {
            header.number_of_points_by_return[j] = lasreader->header.number_of_points_by_return[j];
          }
          // and use the bounding box
          header.max_x = lasreader->header.max_x;
          header.max_y = lasreader->header.max_y;
          header.max_z = lasreader->header.max_z;
          header.min_x = lasreader->header.min_x;
          header.min_y = lasreader->header.min_y;
          header.min_z = lasreader->header.min_z;
          // as well as scale factor and offset
          header.x_scale_factor = lasreader->header.x_scale_factor;
          header.y_scale_factor = lasreader->header.y_scale_factor;
          header.z_scale_factor = lasreader->header.z_scale_factor;
          header.x_offset = lasreader->header.x_offset;
          header.y_offset = lasreader->header.y_offset;
          header.z_offset = lasreader->header.z_offset;
          // for LAS 1.4 (and 32-bit counter overflows)
          header.extended_number_of_point_records = (lasreader->header.number_of_point_records ? lasreader->header.number_of_point_records : lasreader->header.extended_number_of_point_records);
          for (j = 0; j < 5; j++)
          {
            header.extended_number_of_points_by_return[j] = (lasreader->header.number_of_points_by_return[j] ? lasreader->header.number_of_points_by_return[j] : lasreader->header.extended_number_of_points_by_return[j]);
          }
          if (header.version_minor >= 4)
          {
            for (j = 5; j < 15; j++)
            {
              header.extended_number_of_points_by_return[j] = lasreader->header.extended_number_of_points_by_return[j];
            }
          }
        }
        else
        {
          // increment point counters 
          header.number_of_point_records += lasreader->header.number_of_point_records;
          for (j = 0; j < 5; j++)
          {
            header.number_of_points_by_return[j] += lasreader->header.number_of_points_by_return[j];
          }
          // widen the bounding box
          if (header.max_x < lasreader->header.max_x) header.max_x = lasreader->header.max_x;
          if (header.max_y < lasreader->header.max_y) header.max_y = lasreader->header.max_y;
          if (header.max_z < lasreader->header.max_z) header.max_z = lasreader->header.max_z;
          if (header.min_x > lasreader->header.min_x) header.min_x = lasreader->header.min_x;
          if (header.min_y > lasreader->header.min_y) header.min_y = lasreader->header.min_y;
          if (header.min_z > lasreader->header.min_z) header.min_z = lasreader->header.min_z;
          // for LAS 1.4 (and 32-bit counter overflows)
          header.extended_number_of_point_records += (lasreader->header.number_of_point_records ? lasreader->header.number_of_point_records : lasreader->header.extended_number_of_point_records);
          for (j = 0; j < 5; j++)
          {
            header.extended_number_of_points_by_return[j] += (lasreader->header.number_of_points_by_return[j] ? lasreader->header.number_of_points_by_return[j] : lasreader->header.extended_number_of_points_by_return[j]);
          }
          if (header.version_minor >= 4)
          {
            for (j = 5; j < 15; j++)
            {
              header.extended_number_of_points_by_return[j] += lasreader->header.extended_number_of_points_by_return[j];
            }
          }
        }

        // and check if we need to resample points because scalefactor of offsets change
        if (header.x_scale_factor != lasreader->header.x_scale_factor ||
            header.y_scale_factor != lasreader->header.y_scale_factor ||
            header.z_scale_factor != lasreader->header.z_scale_factor)
        {
  //        if (!rescale) fprintf(stderr, "WARNING: files have different scale factors: %g %g %g vs %g %g %g\n", header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, lasreader->header.x_scale_factor, lasreader->header.y_scale_factor, lasreader->header.z_scale_factor);
          rescale = TRUE;
        }
        if (header.x_offset != lasreader->header.x_offset ||
            header.y_offset != lasreader->header.y_offset ||
            header.z_offset != lasreader->header.z_offset)
        {
  //        if (!reoffset) fprintf(stderr, "WARNING: files have different offsets: %g %g %g vs %g %g %g\n", header.x_offset, header.y_offset, header.z_offset, lasreader->header.x_offset, lasreader->header.y_offset, lasreader->header.z_offset);
          reoffset = TRUE;
        }
        // a point type change could be problematic
        if (header.point_data_format != lasreader->header.point_data_format)
        {
          if (!point_type_change) fprintf(stderr, "WARNING: files have different point types: %d vs %d\n", header.point_data_format, lasreader->header.point_data_format);
          point_type_change = TRUE;
        }
        // a point size change could be problematic
        if (header.point_data_record_length != lasreader->header.point_data_record_length)
        {
          if (!point_size_change) fprintf(stderr, "WARNING: files have different point sizes: %d vs %d\n", header.point_data_record_length, lasreader->header.point_data_record_length);
          point_size_change = TRUE;
        }
        // a change in additional attributes (in the extra bytes) could be problematic
        if (header.number_attributes != lasreader->header.number_attributes)
        {
          if (!additional_attribute_change) fprintf(stderr, "WARNING: files have different number of attributes: %d vs %d\n", header.number_attributes, lasreader->header.number_attributes);
          additional_attribute_change = TRUE;
        }
        else if (header.number_attributes)
        {
          for (j = 0; j < (U32)header.number_attributes; j++)
          {
            if (memcmp((const void*)&(header.attributes[j]), (const void*)&(lasreader->header.attributes[j]), 192))
            {
              if (!additional_attribute_change) fprintf(stderr, "WARNING: files have different attribute %d\n", j);
              additional_attribute_change = TRUE;
            }
          }
        }
      }
    }
    lasreader->close();
  }

  if ((npoints > U32_MAX) && (header.version_minor < 4))
  {
    if (0) // (auto_upgrade)
    {
#ifdef _WIN32
      fprintf(stderr,"WARNING: on-the-fly merged LAS 1.%d files contain too many points (%I64d). upgrading to LAS 1.4\n", header.version_minor, npoints);
#else
      fprintf(stderr,"WARNING: on-the-fly merged LAS 1.%d files contain too many points (%lld). upgrading to LAS 1.4\n", header.version_minor, npoints);
#endif
      if (header.version_minor == 3)
      {
        header.header_size += 140;
        header.offset_to_point_data += 140;
      }
      else
      {
        header.header_size += 148;
        header.offset_to_point_data += 148;
      }
      header.version_minor = 4;
    }
    else
    {
#ifdef _WIN32
      fprintf(stderr,"WARNING: on-the-fly merged LAS 1.%d files contain too many points (%I64d) for single LAS 1.%d file.\n", header.version_minor, npoints, header.version_minor);
#else
      fprintf(stderr,"WARNING: on-the-fly merged LAS 1.%d files contain too many points (%lld) for single LAS 1.%d file.\n", header.version_minor, npoints, header.version_minor);
#endif
    }
  }

  // was it requested to rescale or reoffset

  if (scale_factor)
  {
    if (scale_factor[0] && (header.x_scale_factor != scale_factor[0]))
    {
      header.x_scale_factor = scale_factor[0];
      rescale = TRUE;
    }
    if (scale_factor[1] && (header.y_scale_factor != scale_factor[1]))
    {
      header.y_scale_factor = scale_factor[1];
      rescale = TRUE;
    }
    if (scale_factor[2] && (header.z_scale_factor != scale_factor[2]))
    {
      header.z_scale_factor = scale_factor[2];
      rescale = TRUE;
    }
  }
  if (offset)
  {
    if (header.x_offset != offset[0])
    {
      header.x_offset = offset[0];
      reoffset = TRUE;
    }
    if (header.y_offset != offset[1])
    {
      header.y_offset = offset[1];
      reoffset = TRUE;
    }
    if (header.z_offset != offset[2])
    {
      header.z_offset = offset[2];
      reoffset = TRUE;
    }
  }

  // initialize the point with the header info

  if (header.laszip)
  {
    if (!point.init(&header, header.laszip->num_items, header.laszip->items)) return FALSE;
  }
  else
  {
    if (!point.init(&header, header.point_data_format, header.point_data_record_length)) return FALSE;
  }

  // check if the header can support the enlarged bounding box

  // check x

  if ((((header.max_x - header.x_offset) / header.x_scale_factor) > I32_MAX) || (((header.min_x - header.x_offset) / header.x_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 x_offset = (F64)I64_QUANTIZE((header.min_x + header.max_x)/2);
    F64 x_scale_factor = header.x_scale_factor;
    while ((((header.max_x - x_offset) / x_scale_factor) > I32_MAX) || (((header.min_x - x_offset) / x_scale_factor) < I32_MIN))
    {
      x_scale_factor *= 10;
    }
    if (header.x_scale_factor != x_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed x_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.x_scale_factor, x_scale_factor);
      header.x_scale_factor = x_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_x - header.x_offset) / x_scale_factor) > I32_MAX) || (((header.min_x - header.x_offset) / x_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed x_offset from %g to %g to accommodate enlarged bounding box\n", header.x_offset, x_offset);
      header.x_offset = x_offset;
      reoffset = TRUE;
    }
  }
    
  // check y

  if ((((header.max_y - header.y_offset) / header.y_scale_factor) > I32_MAX) || (((header.min_y - header.y_offset) / header.y_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 y_offset = (F64)I64_QUANTIZE((header.min_y + header.max_y)/2);
    F64 y_scale_factor = header.y_scale_factor;
    while ((((header.max_y - y_offset) / y_scale_factor) > I32_MAX) || (((header.min_y - y_offset) / y_scale_factor) < I32_MIN))
    {
      y_scale_factor *= 10;
    }
    if (header.y_scale_factor != y_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed y_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.y_scale_factor, y_scale_factor);
      header.y_scale_factor = y_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_y - header.y_offset) / y_scale_factor) > I32_MAX) || (((header.min_y - header.y_offset) / y_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed y_offset from %g to %g to accommodate enlarged bounding box\n", header.y_offset, y_offset);
      header.y_offset = y_offset;
      reoffset = TRUE;
    }
  }
    
  // check z

  if ((((header.max_z - header.z_offset) / header.z_scale_factor) > I32_MAX) || (((header.min_z - header.z_offset) / header.z_scale_factor) < I32_MIN))
  {
    // maybe we can fix it by adjusting the offset (and if needed by lowering the resolution via the scale factor)
    F64 z_offset = (F64)I64_QUANTIZE((header.min_z + header.max_z)/2);
    F64 z_scale_factor = header.z_scale_factor;
    while ((((header.max_z - z_offset) / z_scale_factor) > I32_MAX) || (((header.min_z - z_offset) / z_scale_factor) < I32_MIN))
    {
      z_scale_factor *= 10;
    }
    if (header.z_scale_factor != z_scale_factor)
    {
      fprintf(stderr, "WARNING: i changed  z_scale_factor from %g to %g to accommodate enlarged bounding box\n", header.z_scale_factor, z_scale_factor);
      header.z_scale_factor = z_scale_factor;
      rescale = TRUE;
    }
    // maybe we changed the resolution ... so do we really need to adjuste the offset
    if ((((header.max_z - header.z_offset) / z_scale_factor) > I32_MAX) || (((header.min_z - header.z_offset) / z_scale_factor) < I32_MIN))
    {
      fprintf(stderr, "WARNING: i changed z_offset from %g to %g to accommodate enlarged bounding box\n", header.z_offset, z_offset);
      header.z_offset = z_offset;
      reoffset = TRUE;
    }
  }

  if (rescale || reoffset)
  {
    if (lasreaderlas)
    {
      delete lasreaderlas;
      if (rescale && reoffset)
        lasreaderlas = new LASreaderLASrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderlas = new LASreaderLASrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderlas = new LASreaderLASreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderlas;
    }
    else if (lasreaderbin)
    {
      delete lasreaderbin;
      if (rescale && reoffset)
        lasreaderbin = new LASreaderBINrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderbin = new LASreaderBINrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderbin = new LASreaderBINreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderbin;
    }
    else if (lasreadershp)
    {
      delete lasreadershp;
      if (rescale && reoffset)
        lasreadershp = new LASreaderSHPrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreadershp = new LASreaderSHPrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreadershp = new LASreaderSHPreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreadershp;
    }
    else if (lasreaderasc)
    {
      delete lasreaderasc;
      if (rescale && reoffset)
        lasreaderasc = new LASreaderASCrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderasc = new LASreaderASCrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderasc = new LASreaderASCreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderasc;
    }
    else if (lasreaderbil)
    {
      delete lasreaderbil;
      if (rescale && reoffset)
        lasreaderbil = new LASreaderBILrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderbil = new LASreaderBILrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderbil = new LASreaderBILreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderbil;
    }
    else if (lasreaderdtm)
    {
      delete lasreaderdtm;
      if (rescale && reoffset)
        lasreaderdtm = new LASreaderDTMrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderdtm = new LASreaderDTMrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderdtm = new LASreaderDTMreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderdtm;
    }
    else if (lasreaderply)
    {
      delete lasreaderply;
      if (rescale && reoffset)
        lasreaderply = new LASreaderPLYrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderply = new LASreaderPLYrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderply = new LASreaderPLYreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderply;
    }
    else if (lasreaderqfit)
    {
      delete lasreaderqfit;
      if (rescale && reoffset)
        lasreaderqfit = new LASreaderQFITrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreaderqfit = new LASreaderQFITrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreaderqfit = new LASreaderQFITreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreader = lasreaderqfit;
    }
    else
    {
      delete lasreadertxt;
      if (rescale && reoffset)
        lasreadertxt = new LASreaderTXTrescalereoffset(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor, header.x_offset, header.y_offset, header.z_offset);
      else if (rescale)
        lasreadertxt = new LASreaderTXTrescale(header.x_scale_factor, header.y_scale_factor, header.z_scale_factor);
      else
        lasreadertxt = new LASreaderTXTreoffset(header.x_offset, header.y_offset, header.z_offset);
      lasreadertxt->set_translate_intensity(translate_intensity);
      lasreadertxt->set_scale_intensity(scale_intensity);
      lasreadertxt->set_translate_scan_angle(translate_scan_angle);
      lasreadertxt->set_scale_scan_angle(scale_scan_angle);
      lasreadertxt->set_scale_factor(scale_factor);
      lasreadertxt->set_offset(offset);
      lasreader = lasreadertxt;
    }
  }

  p_count = 0;
  file_name_current = 0;

  return TRUE;
}

void LASreaderMerged::set_filter(LASfilter* filter)
{
  this->filter = filter;
}

void LASreaderMerged::set_transform(LAStransform* transform)
{
  this->transform = transform;
}

BOOL LASreaderMerged::inside_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  inside = 1;
  t_ll_x = ll_x;
  t_ll_y = ll_y;
  t_size = size;
  t_ur_x = ll_x + size;
  t_ur_y = ll_y + size;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = ll_x;
  header.min_y = ll_y;
  header.max_x = ll_x + size - 0.001f * header.x_scale_factor;
  header.max_y = ll_y + size - 0.001f * header.y_scale_factor;
  return TRUE;
}

BOOL LASreaderMerged::inside_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  inside = 2;
  c_center_x = center_x;
  c_center_y = center_y;
  c_radius = radius;
  c_radius_squared = radius*radius;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = center_x - radius;
  header.min_y = center_y - radius;
  header.max_x = center_x + radius;
  header.max_y = center_y + radius;
  return TRUE;
}

BOOL LASreaderMerged::inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y)
{
  inside = 3;
  r_min_x = min_x;
  r_min_y = min_y;
  r_max_x = max_x;
  r_max_y = max_y;
  orig_min_x = header.min_x;
  orig_min_y = header.min_y;
  orig_max_x = header.max_x;
  orig_max_y = header.max_y;
  header.min_x = min_x;
  header.min_y = min_y;
  header.max_x = max_x;
  header.max_y = max_y;
  return TRUE;
}

I32 LASreaderMerged::get_format() const
{
  return lasreader->get_format();
}

BOOL LASreaderMerged::read_point_default()
{
  if (file_name_current == 0)
  {
    if (!open_next_file()) return FALSE;
  }

  while (true)
  {
    if (lasreader->read_point())
    {
      point = lasreader->point;
      p_count++;
      return TRUE;
    }
    if (lasreaderbin)
    {
      header.number_of_points_by_return[0] += lasreader->header.number_of_points_by_return[0];
      header.number_of_points_by_return[1] += lasreader->header.number_of_points_by_return[1];
      header.number_of_points_by_return[2] += lasreader->header.number_of_points_by_return[2];
      header.number_of_points_by_return[3] += lasreader->header.number_of_points_by_return[3];
      header.number_of_points_by_return[4] += lasreader->header.number_of_points_by_return[4];
      if (header.max_x < lasreader->header.max_x) header.max_x = lasreader->header.max_x;
      if (header.max_y < lasreader->header.max_y) header.max_y = lasreader->header.max_y;
      if (header.max_z < lasreader->header.max_z) header.max_z = lasreader->header.max_z;
      if (header.min_x > lasreader->header.min_x) header.min_x = lasreader->header.min_x;
      if (header.min_y > lasreader->header.min_y) header.min_y = lasreader->header.min_y;
      if (header.min_z > lasreader->header.min_z) header.min_z = lasreader->header.min_z;
    }
    else if (lasreadertxt)
    {
      if (!populate_header)
      {
        header.number_of_point_records += lasreader->header.number_of_point_records;
        header.number_of_points_by_return[0] += lasreader->header.number_of_points_by_return[0];
        header.number_of_points_by_return[1] += lasreader->header.number_of_points_by_return[1];
        header.number_of_points_by_return[2] += lasreader->header.number_of_points_by_return[2];
        header.number_of_points_by_return[3] += lasreader->header.number_of_points_by_return[3];
        header.number_of_points_by_return[4] += lasreader->header.number_of_points_by_return[4];
        if (header.max_x < lasreader->header.max_x) header.max_x = lasreader->header.max_x;
        if (header.max_y < lasreader->header.max_y) header.max_y = lasreader->header.max_y;
        if (header.max_z < lasreader->header.max_z) header.max_z = lasreader->header.max_z;
        if (header.min_x > lasreader->header.min_x) header.min_x = lasreader->header.min_x;
        if (header.min_y > lasreader->header.min_y) header.min_y = lasreader->header.min_y;
        if (header.min_z > lasreader->header.min_z) header.min_z = lasreader->header.min_z;
      }
    }
    lasreader->close();
    point.zero();
    if (!open_next_file()) return FALSE;
  }
  return FALSE;
}

void LASreaderMerged::close(BOOL close_stream)
{
  if (lasreader) 
  {
    lasreader->close(close_stream);
  }
}

BOOL LASreaderMerged::reopen()
{
  p_count = 0;
  file_name_current = 0;
  if (inside) inside_none();
  if (filter) filter->reset();
  return TRUE;
}

void LASreaderMerged::clean()
{
  if (lasreader) 
  {
    delete lasreader;
    lasreader = 0;
    lasreaderlas = 0;
    lasreaderbin = 0;
    lasreadershp = 0;
    lasreaderasc = 0;
    lasreaderbil = 0;
    lasreaderdtm = 0;
    lasreaderply = 0;
    lasreaderqfit = 0;
    lasreadertxt = 0;
  }
  point_type_change = FALSE;
  point_size_change = FALSE;
  additional_attribute_change = FALSE;
  rescale = FALSE;
  reoffset = FALSE;
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
  if (parse_string)
  {
    free(parse_string);
    parse_string = 0;
  }
  skip_lines = 0;
  translate_intensity = 0.0f;
  scale_intensity = 1.0f;
  translate_scan_angle = 0.0f;
  scale_scan_angle = 1.0f;
  populate_header = FALSE;
  keep_lastiling = FALSE;

  if (file_names)
  {
    U32 i;
    for (i = 0; i < file_name_number; i++)
    {
      free(file_names[i]);
    }
    free(file_names);
    file_names = 0;
  }
  if (bounding_boxes)
  {
    delete [] bounding_boxes;
    bounding_boxes = 0;
  }
  file_name_current = 0;
  file_name_number = 0;
  file_name_allocated = 0;
  inside = 0;
}

LASreaderMerged::LASreaderMerged()
{
  lasreader = 0;
  lasreaderlas = 0;
  lasreaderbin = 0;
  lasreadershp = 0;
  lasreaderasc = 0;
  lasreaderbil = 0;
  lasreaderdtm = 0;
  lasreaderply = 0;
  lasreaderqfit = 0;
  lasreadertxt = 0;
  scale_factor = 0;
  offset = 0;
  files_are_flightlines = 0;
  apply_file_source_ID = FALSE;
  parse_string = 0;
  io_ibuffer_size = LAS_TOOLS_IO_IBUFFER_SIZE;
  file_names = 0;
  bounding_boxes = 0;
  clean();
}

LASreaderMerged::~LASreaderMerged()
{
  if (lasreader) close();
  clean();
}

BOOL LASreaderMerged::open_next_file()
{
  while (file_name_current < file_name_number)
  {
    if (inside)
    {
      // check if bounding box overlaps requested bounding box
      if (inside < 3) // tile or circle
      {
        if (bounding_boxes[4*file_name_current+0] >= header.max_x)
        {
          file_name_current++;
          continue;
        }
        if (bounding_boxes[4*file_name_current+1] >= header.max_y)
        {
          file_name_current++;
          continue;
        }
      }
      else // rectangle
      {
        if (bounding_boxes[4*file_name_current+0] > header.max_x)
        {
          file_name_current++;
          continue;
        }
        if (bounding_boxes[4*file_name_current+1] > header.max_y)
        {
          file_name_current++;
          continue;
        }
      }
      if (bounding_boxes[4*file_name_current+2] < header.min_x)
      {
        file_name_current++;
        continue;
      }
      if (bounding_boxes[4*file_name_current+3] < header.min_y)
      {
        file_name_current++;
        continue;
      }
    }
    // open the lasreader with the next file name
    if (lasreaderlas)
    {
      if (!lasreaderlas->open(file_names[file_name_current], io_ibuffer_size))
      {
        fprintf(stderr, "ERROR: could not open lasreaderlas for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderlas->set_index(index);
      else
        delete index;
    }
    else if (lasreaderbin)
    {
      if (!lasreaderbin->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderbin for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderbin->set_index(index);
      else
        delete index;
    }
    else if (lasreadershp)
    {
      if (!lasreadershp->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreadershp for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
    }
    else if (lasreaderasc)
    {
      if (!lasreaderasc->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderasc for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
    }
    else if (lasreaderbil)
    {
      if (!lasreaderbil->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderbil for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderbil->set_index(index);
      else
        delete index;
    }
    else if (lasreaderdtm)
    {
      if (!lasreaderdtm->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderdtm for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderdtm->set_index(index);
      else
        delete index;
    }
    else if (lasreaderply)
    {
      if (!lasreaderply->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderply for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderply->set_index(index);
      else
        delete index;
    }
    else if (lasreaderqfit)
    {
      if (!lasreaderqfit->open(file_names[file_name_current]))
      {
        fprintf(stderr, "ERROR: could not open lasreaderqfit for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
      LASindex* index = new LASindex;
      if (index->read(file_names[file_name_current]))
        lasreaderqfit->set_index(index);
      else
        delete index;
    }
    else
    {
      if (!lasreadertxt->open(file_names[file_name_current], 0, parse_string, skip_lines, populate_header))
      {
        fprintf(stderr, "ERROR: could not open lasreadertxt for file '%s'\n", file_names[file_name_current]);
        return FALSE;
      }
    }
    file_name_current++;
    if (files_are_flightlines)
    {
      transform->setPointSource(file_name_current + files_are_flightlines - 1);
    }
    else if (apply_file_source_ID)
    {
      transform->setPointSource(lasreader->header.file_source_ID);
    }
    if (filter) lasreader->set_filter(filter);
    if (transform) lasreader->set_transform(transform);
    if (inside)
    {
      if (inside == 3) lasreader->inside_rectangle(r_min_x, r_min_y, r_max_x, r_max_y);
      else if (inside == 1) lasreader->inside_tile(t_ll_x, t_ll_y, t_size);
      else lasreader->inside_circle(c_center_x, c_center_y, c_radius);
    }
    return TRUE;
  }
  return FALSE;
}
