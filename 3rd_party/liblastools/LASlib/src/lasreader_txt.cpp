/*
===============================================================================

  FILE:  lasreader_txt.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "lasreader_txt.hpp"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern "C" FILE* fopen_compressed(const char* filename, const char* mode, bool* piped);

BOOL LASreaderTXT::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  FILE* file = fopen_compressed(file_name, "r", &piped);
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 10*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  return open(file, file_name, point_type, parse_string, skip_lines, populate_header);
}

BOOL LASreaderTXT::open(FILE* file, const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  int i;

  if (file == 0)
  {
    fprintf(stderr,"ERROR: file pointer is zero\n");
    return FALSE;
  }

  // clean the reader

  clean();

  // clean the header

  header.clean();

  // set the file pointer

  this->file = file;

  // add attributes in extra bytes

  if (number_attributes)
  {
    for (i = 0; i < number_attributes; i++)
    {
      I32 type = (attributes_data_types[i]-1)%10;
      try {
        LASattribute attribute(type, attribute_names[i], attribute_descriptions[i]);
        if (attribute_scales[i] != 1.0 || attribute_offsets[i] != 0.0)
        {
          attribute.set_scale(attribute_scales[i]);
        }
        if (attribute_offsets[i] != 0.0)
        {
          attribute.set_offset(attribute_offsets[i]);
        }
        if (attribute_no_datas[i] != F64_MAX)
        {
          attribute.set_no_data(attribute_no_datas[i]);
        }
        header.add_attribute(attribute);
      }
      catch(...) {
        fprintf(stderr,"ERROR: initializing attribute %s\n", attribute_descriptions[i]);
        return FALSE;
      }
    }
  }

  if (parse_string && !check_parse_string(parse_string))
  {
    return FALSE;
  }

  // populate the header as much as it makes sense

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderTXT (%d)", LAS_TOOLS_VERSION);

  // maybe set creation date

#ifdef _WIN32
  if (file_name)
  {
    WIN32_FILE_ATTRIBUTE_DATA attr;
	  SYSTEMTIME creation;
    GetFileAttributesEx(file_name, GetFileExInfoStandard, &attr);
	  FileTimeToSystemTime(&attr.ftCreationTime, &creation);
    int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    header.file_creation_day = startday[creation.wMonth] + creation.wDay;
    header.file_creation_year = creation.wYear;
    // leap year handling
    if ((((creation.wYear)%4) == 0) && (creation.wMonth > 2)) header.file_creation_day++;
  }
  else
  {
    header.file_creation_day = 1;
    header.file_creation_year = 2016;
  }
#else
  header.file_creation_day = 1;
  header.file_creation_year = 2017;
#endif
  if (point_type)
  {
    switch (point_type)
    {
    case 1:
      header.point_data_record_length = 28;
      break;
    case 2:
      header.point_data_record_length = 26;
      break;
    case 3:
      header.point_data_record_length = 34;
      break;
    case 6:
      header.point_data_record_length = 30;
      break;
    case 7:
      header.point_data_record_length = 36;
      break;
    case 8:
      header.point_data_record_length = 38;
      break;
    default:
      return FALSE;
    }
    header.point_data_format = point_type;
  }
  else if (parse_string)
  {
    if (strstr(parse_string,"o") || strstr(parse_string,"l") || strstr(parse_string,"I"))
    {
      // new point types
      if (strstr(parse_string,"I"))
      {
        header.point_data_format = 8;
        header.point_data_record_length = 38;
      }
      else if (strstr(parse_string,"R") || strstr(parse_string,"G") || strstr(parse_string,"B") || strstr(parse_string,"H"))
      {
        header.point_data_format = 7;
        header.point_data_record_length = 36;
      }
      else
      {
        header.point_data_format = 6;
        header.point_data_record_length = 30;
      }
    }
    else if (strstr(parse_string,"t"))
    {
      if (strstr(parse_string,"R") || strstr(parse_string,"G") || strstr(parse_string,"B") || strstr(parse_string,"H"))
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
      if (strstr(parse_string,"R") || strstr(parse_string,"G") || strstr(parse_string,"B") || strstr(parse_string,"H"))
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
  }
  else
  {
    header.point_data_format = 0;
    header.point_data_record_length = 20;
  }

  if (header.point_data_format > 5)
  {
    header.version_minor = 4;
    header.header_size = 375;
    header.offset_to_point_data = 375;
  }

  this->point_type = header.point_data_format;

  // maybe attributes in extra bytes

  if (header.number_attributes)
  {
    header.update_extra_bytes_vlr();
    header.point_data_record_length += header.get_attributes_size();
  }

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length, &header);

  // we do not know yet how many points to expect

  npoints = 0;

  // should we perform an extra pass to fully populate the header

  if (populate_header && file_name)
  {
    // create a cheaper parse string that only looks for 'x' 'y' 'z' 'r' and attributes in extra bytes

    char* parse_less;
    if (parse_string == 0)
    {
      parse_less = LASCopyString("xyz");
    }
    else
    {
      parse_less = LASCopyString(parse_string);
      for (i = 0; i < (int)strlen(parse_string); i++)
      {
        if (parse_less[i] != 'x' && parse_less[i] != 'y' && parse_less[i] != 'z' && parse_less[i] != 'r' && (parse_less[i] < '0' || parse_less[i] > '0'))
        {
          parse_less[i] = 's';
        }
      }
      do
      {
        parse_less[i] = '\0';
        i--;
      } while (parse_less[i] == 's');
    }

    // skip lines if we have to

    for (i = 0; i < skip_lines; i++) fgets(line, 512, file);

    // read the first line

    while (fgets(line, 512, file))
    {
      if (parse(parse_less))
      {
        // mark that we found the first point
        npoints++;
        // we can stop this loop
        break;
      }
      else
      {
        line[strlen(line)-1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_less);
      }
    }

    // did we manage to parse a line

    if (npoints == 0)
    {
      fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", parse_less);
      fclose(file);
      file = 0;
      free(parse_less);
      return FALSE;
    }

    // init the bounding box

    header.min_x = header.max_x = point.coordinates[0];
    header.min_y = header.max_y = point.coordinates[1];
    header.min_z = header.max_z = point.coordinates[2];

    // create return histogram

    if (point.extended_point_type)
    {
      if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number-1]++;
    }
    else
    {
      if (point.return_number >= 1 && point.return_number <= 5) header.number_of_points_by_return[point.return_number-1]++;
    }

    // init the min and max of attributes in extra bytes

    if (number_attributes)
    {
      for (i = 0; i < number_attributes; i++)
      {
        header.attributes[i].set_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].set_max(point.extra_bytes + attribute_starts[i]);
      }
    }

    // loop over the remaining lines

    while (fgets(line, 512, file))
    {
      if (parse(parse_less))
      {
        // count points
        npoints++;
        // create return histogram
        if (point.extended_point_type)
        {
          if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number-1]++;
        }
        else
        {
          if (point.return_number >= 1 && point.return_number <= 5) header.number_of_points_by_return[point.return_number-1]++;
        }
        // update bounding box
        if (point.coordinates[0] < header.min_x) header.min_x = point.coordinates[0];
        else if (point.coordinates[0] > header.max_x) header.max_x = point.coordinates[0];
        if (point.coordinates[1] < header.min_y) header.min_y = point.coordinates[1];
        else if (point.coordinates[1] > header.max_y) header.max_y = point.coordinates[1];
        if (point.coordinates[2] < header.min_z) header.min_z = point.coordinates[2];
        else if (point.coordinates[2] > header.max_z) header.max_z = point.coordinates[2];
        // update the min and max of attributes in extra bytes
        if (number_attributes)
        {
          for (i = 0; i < number_attributes; i++)
          {
            header.attributes[i].update_min(point.extra_bytes + attribute_starts[i]);
            header.attributes[i].update_max(point.extra_bytes + attribute_starts[i]);
          }
        }
      }
      else
      {
        line[strlen(line)-1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_less);
      }
    }
    if (point.extended_point_type || (npoints > U32_MAX))
    {
      header.version_minor = 4;
      header.number_of_point_records = 0;
      header.number_of_points_by_return[0] = 0;
      header.number_of_points_by_return[1] = 0;
      header.number_of_points_by_return[2] = 0;
      header.number_of_points_by_return[3] = 0;
      header.number_of_points_by_return[4] = 0;
      header.extended_number_of_point_records = npoints;
    }
    else
    {
      header.number_of_point_records = (U32)npoints;
    }
    free(parse_less);

    // close the input file

    fclose(file);

    // populate scale and offset

    populate_scale_and_offset();

    // populate bounding box

    populate_bounding_box();

    // mark that header is already populated

    populated_header = TRUE;

    // reopen input file for the second pass

    file = fopen_compressed(file_name, "r", &piped);
    if (file == 0)
    {
      fprintf(stderr, "ERROR: could not open '%s' for second pass\n", file_name);
      return FALSE;
    }

    if (setvbuf(file, NULL, _IOFBF, 10*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
    {
      fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 10*LAS_TOOLS_IO_IBUFFER_SIZE);
    }
  }

  if (parse_string == 0)
  {
    this->parse_string = LASCopyString("xyz");
  }
  else
  {
    this->parse_string = LASCopyString(parse_string);
  }

  // skip lines if we have to

  this->skip_lines = skip_lines;
  if (skip_lines)
  {
    for (i = 0; i < skip_lines; i++) fgets(line, 512, file);
  }
  else if (ipts)
  {
    if (fgets(line, 512, file))
    {
#ifdef _WIN32
      sscanf(line, "%I64d", &npoints);
#else
      sscanf(line, "%lld", &npoints);
#endif
      if (npoints > U32_MAX)
      {
        header.version_minor = 4;
        header.number_of_point_records = 0;
        header.extended_number_of_point_records = npoints;
      }
      else
      {
        header.number_of_point_records = (U32)npoints;
      }
    }

    // add payload that informs about PTS

    U8* payload = new U8[32];
    memset(payload, 0, 32);
    ((F32*)payload)[0] = translate_intensity;
    ((F32*)payload)[1] = scale_intensity;
    strcpy((char*)(payload + 16), (parse_string ? parse_string : "xyz"));
    header.add_vlr("LAStools", 2000, 32, payload);
  }
  else if (iptx)
  {
    I32 ncols;
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%d", &ncols) != 1)
      {
        fprintf(stderr, "ERROR: parsing number of cols\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with number of cols\n");
      return FALSE;
    }
    I32 nrows;
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%d", &nrows) != 1)
      {
        fprintf(stderr, "ERROR: parsing number of rows\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with number of rows\n");
      return FALSE;
    }
    npoints = (I64)ncols*(I64)nrows;
    if (npoints > U32_MAX)
    {
      header.version_minor = 4;
      header.number_of_point_records = 0;
      header.extended_number_of_point_records = npoints;
    }
    else
    {
      header.number_of_point_records = (U32)npoints;
    }
    F64 translation[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(translation[0]), &(translation[1]), &(translation[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing translation\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with translation\n");
      return FALSE;
    }
    F64 rotation_row_0[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(rotation_row_0[0]), &(rotation_row_0[1]), &(rotation_row_0[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 0\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 0\n");
      return FALSE;
    }
    F64 rotation_row_1[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(rotation_row_1[0]), &(rotation_row_1[1]), &(rotation_row_1[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 1\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 1\n");
      return FALSE;
    }
    F64 rotation_row_2[3];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf", &(rotation_row_2[0]), &(rotation_row_2[1]), &(rotation_row_2[2])) != 3)
      {
        fprintf(stderr, "ERROR: parsing rotation row 2\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with rotation row 2\n");
      return FALSE;
    }
    F64 transformation_row_0[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(transformation_row_0[0]), &(transformation_row_0[1]), &(transformation_row_0[2]), &(transformation_row_0[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 0\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 0\n");
      return FALSE;
    }
    F64 transformation_row_1[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(transformation_row_1[0]), &(transformation_row_1[1]), &(transformation_row_1[2]), &(transformation_row_1[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 1\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 1\n");
      return FALSE;
    }
    F64 transformation_row_2[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(transformation_row_2[0]), &(transformation_row_2[1]), &(transformation_row_2[2]), &(transformation_row_2[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 2\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 2\n");
      return FALSE;
    }
    F64 transformation_row_3[4];
    if (fgets(line, 512, file))
    {
      if (sscanf(line, "%lf %lf %lf %lf", &(transformation_row_3[0]), &(transformation_row_3[1]), &(transformation_row_3[2]), &(transformation_row_3[3])) != 4)
      {
        fprintf(stderr, "ERROR: parsing transformation row 3\n");
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr, "ERROR: reading line with transformation row 3\n");
      return FALSE;
    }

    // add payload that informs about PTX

    U8* payload = new U8[32+240];
    memset(payload, 0, 32+240);
    ((F32*)payload)[0] = translate_intensity;
    ((F32*)payload)[1] = scale_intensity;
    strcpy((char*)(payload + 16), (parse_string ? parse_string : "xyz"));
    ((I64*)payload)[4] = (I64)ncols;
    ((I64*)payload)[5] = (I64)nrows;
    ((F64*)payload)[6] = translation[0];
    ((F64*)payload)[7] = translation[1];
    ((F64*)payload)[8] = translation[2];
    ((F64*)payload)[ 9] = rotation_row_0[0];
    ((F64*)payload)[10] = rotation_row_0[1];
    ((F64*)payload)[11] = rotation_row_0[2];
    ((F64*)payload)[12] = rotation_row_1[0];
    ((F64*)payload)[13] = rotation_row_1[1];
    ((F64*)payload)[14] = rotation_row_1[2];
    ((F64*)payload)[15] = rotation_row_2[0];
    ((F64*)payload)[16] = rotation_row_2[1];
    ((F64*)payload)[17] = rotation_row_2[2];
    ((F64*)payload)[18] = transformation_row_0[0];
    ((F64*)payload)[19] = transformation_row_0[1];
    ((F64*)payload)[20] = transformation_row_0[2];
    ((F64*)payload)[21] = transformation_row_0[3];
    ((F64*)payload)[22] = transformation_row_1[0];
    ((F64*)payload)[23] = transformation_row_1[1];
    ((F64*)payload)[24] = transformation_row_1[2];
    ((F64*)payload)[25] = transformation_row_1[3];
    ((F64*)payload)[26] = transformation_row_2[0];
    ((F64*)payload)[27] = transformation_row_2[1];
    ((F64*)payload)[28] = transformation_row_2[2];
    ((F64*)payload)[29] = transformation_row_2[3];
    ((F64*)payload)[30] = transformation_row_3[0];
    ((F64*)payload)[31] = transformation_row_3[1];
    ((F64*)payload)[32] = transformation_row_3[2];
    ((F64*)payload)[33] = transformation_row_3[3];
    header.add_vlr("LAStools", 2001, 32+240, payload);
  }

  // read the first line with full parse_string

  i = 0;
  while (fgets(line, 512, file))
  {
    if (parse(this->parse_string))
    {
      // mark that we found the first point
      i = 1;
      break;
    }
    else
    {
      line[strlen(line)-1] = '\0';
      fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, this->parse_string);
    }
  }

  // did we manage to parse a line

  if (i != 1)
  {
    fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", this->parse_string);
    fclose(this->file);
    this->file = 0;
    free(this->parse_string);
    this->parse_string = 0;
    return FALSE;
  }

  if (!populated_header)
  {
    // init the bounding box that we will incrementally compute

    header.min_x = header.max_x = point.coordinates[0];
    header.min_y = header.max_y = point.coordinates[1];
    header.min_z = header.max_z = point.coordinates[2];

    // init the min and max of attributes in extra bytes

    if (number_attributes)
    {
      for (i = 0; i < number_attributes; i++)
      {
        header.attributes[i].set_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].set_max(point.extra_bytes + attribute_starts[i]);
      }
    }

    // set scale and offset

    populate_scale_and_offset();
  }

  p_count = 0;

  return TRUE;
}

void LASreaderTXT::set_pts(BOOL pts)
{
  translate_intensity = 2048.0f;
  scale_intensity = 1.0f;
  this->ipts = pts;
}

void LASreaderTXT::set_ptx(BOOL ptx)
{
  translate_intensity = 0.0f;
  scale_intensity = 4095.0f;
  this->iptx = ptx;
}

void LASreaderTXT::set_translate_intensity(F32 translate_intensity)
{
  this->translate_intensity = translate_intensity;
}

void LASreaderTXT::set_scale_intensity(F32 scale_intensity)
{
  this->scale_intensity = scale_intensity;
}

void LASreaderTXT::set_translate_scan_angle(F32 translate_scan_angle)
{
  this->translate_scan_angle = translate_scan_angle;
}

void LASreaderTXT::set_scale_scan_angle(F32 scale_scan_angle)
{
  this->scale_scan_angle = scale_scan_angle;
}

void LASreaderTXT::set_scale_factor(const F64* scale_factor)
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

void LASreaderTXT::set_offset(const F64* offset)
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

void LASreaderTXT::add_attribute(I32 data_type, const char* name, const char* description, F64 scale, F64 offset, F64 pre_scale, F64 pre_offset, F64 no_data)
{
  attributes_data_types[number_attributes] = data_type;
  if (name)
  {
    attribute_names[number_attributes] = LASCopyString(name);
  }
  else
  {
    char temp[32];
    sprintf(temp, "attribute %d", number_attributes);
    attribute_names[number_attributes] = LASCopyString(temp);
  }
  if (description)
  {
    attribute_descriptions[number_attributes] = LASCopyString(description);
  }
  else
  {
    attribute_descriptions[number_attributes] = 0;
  }
  attribute_scales[number_attributes] = scale;
  attribute_offsets[number_attributes] = offset;
  attribute_pre_scales[number_attributes] = pre_scale;
  attribute_pre_offsets[number_attributes] = pre_offset;
  attribute_no_datas[number_attributes] = no_data;
  number_attributes++;
}

BOOL LASreaderTXT::seek(const I64 p_index)
{
  U32 delta = 0;
  if (p_index > p_count)
  {
    delta = (U32)(p_index - p_count);
  }
  else if (p_index < p_count)
  {
    if (piped) return FALSE;
    fseek(file, 0, SEEK_SET);
    // skip lines if we have to
    int i;
    for (i = 0; i < skip_lines; i++) fgets(line, 512, file);
    // read the first line with full parse_string
    i = 0;
    while (fgets(line, 512, file))
    {
      if (parse(this->parse_string))
      {
        // mark that we found the first point
        i = 1;
        break;
      }
      else
      {
        line[strlen(line)-1] = '\0';
        fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, this->parse_string);
      }
    }
    // did we manage to parse a line
    if (i != 1)
    {
      fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", this->parse_string);
      fclose(file);
      file = 0;
      free(this->parse_string);
      this->parse_string = 0;
      return FALSE;
    }
    delta = (U32)p_index;
  }
  while (delta)
  {
    read_point_default();
    delta--;
  }
  p_count = p_index;
  return TRUE;
}

BOOL LASreaderTXT::read_point_default()
{
  if (p_count)
  {
    while (true)
    {
      if (fgets(line, 512, file))
      {
        if (parse(parse_string))
        {
          break;
        }
        else
        {
          line[strlen(line)-1] = '\0';
          fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, this->parse_string);
        }
      }
      else
      {
        if (populated_header)
        {
          if (p_count != npoints)
          {
#ifdef _WIN32
            fprintf(stderr,"WARNING: end-of-file after %I64d of %I64d points\n", p_count, npoints);
#else
            fprintf(stderr,"WARNING: end-of-file after %lld of %lld points\n", p_count, npoints);
#endif
          }
        }
        else
        {
          if (npoints)
          {
            if (p_count != npoints)
            {
#ifdef _WIN32
              fprintf(stderr,"WARNING: end-of-file after %I64d of %I64d points\n", p_count, npoints);
#else
              fprintf(stderr,"WARNING: end-of-file after %lld of %lld points\n", p_count, npoints);
#endif
            }
          }
          npoints = p_count;
          populate_bounding_box();
        }
        return FALSE;
      }
    }
  }
  // compute the quantized x, y, and z values
  point.set_X(header.get_X(point.coordinates[0]));
  point.set_Y(header.get_Y(point.coordinates[1]));
  point.set_Z(header.get_Z(point.coordinates[2]));
  p_count++;
  if (!populated_header)
  {
    // update number of point records
    // create return histogram
    if (point.extended_point_type)
    {
      if (point.extended_return_number >= 1 && point.extended_return_number <= 15) header.extended_number_of_points_by_return[point.extended_return_number-1]++;
    }
    else
    {
      if (point.return_number >= 1 && point.return_number <= 5) header.number_of_points_by_return[point.return_number-1]++;
    }
    // update bounding box
    if (point.coordinates[0] < header.min_x) header.min_x = point.coordinates[0];
    else if (point.coordinates[0] > header.max_x) header.max_x = point.coordinates[0];
    if (point.coordinates[1] < header.min_y) header.min_y = point.coordinates[1];
    else if (point.coordinates[1] > header.max_y) header.max_y = point.coordinates[1];
    if (point.coordinates[2] < header.min_z) header.min_z = point.coordinates[2];
    else if (point.coordinates[2] > header.max_z) header.max_z = point.coordinates[2];
    // update the min and max of attributes in extra bytes
    if (number_attributes)
    {
      for (I32 i = 0; i < number_attributes; i++)
      {
        header.attributes[i].update_min(point.extra_bytes + attribute_starts[i]);
        header.attributes[i].update_max(point.extra_bytes + attribute_starts[i]);
      }
    }
  }
  return TRUE;
}

ByteStreamIn* LASreaderTXT::get_stream() const
{
  return 0;
}

void LASreaderTXT::close(BOOL close_stream)
{
  if (file)
  {
    if (piped) while(fgets(line, 512, file));
    fclose(file);
    file = 0;
  }
}

BOOL LASreaderTXT::reopen(const char* file_name)
{
  int i;

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

  // skip lines if we have to

  for (i = 0; i < skip_lines; i++) fgets(line, 512, file);

  // read the first line with full parse_string

  i = 0;
  while (fgets(line, 512, file))
  {
    if (parse(parse_string))
    {
      // mark that we found the first point
      i = 1;
      break;
    }
    else
    {
      line[strlen(line)-1] = '\0';
      fprintf(stderr, "WARNING: cannot parse '%s' with '%s'. skipping ...\n", line, parse_string);
    }
  }

  // did we manage to parse a line

  if (i != 1)
  {
    fprintf(stderr, "ERROR: could not parse any lines with '%s'\n", parse_string);
    fclose(file);
    file = 0;
    return FALSE;
  }

  p_count = 0;

  return TRUE;
}

void LASreaderTXT::clean()
{
  if (file)
  {
    fclose(file);
    file = 0;
  }
  if (parse_string)
  {
    free(parse_string);
    parse_string = 0;
  }
  skip_lines = 0;
  populated_header = FALSE;
}

LASreaderTXT::LASreaderTXT()
{
  file = 0;
  piped = false;
  point_type = 0;
  parse_string = 0;
  scale_factor = 0;
  offset = 0;
  ipts = FALSE;
  iptx = FALSE;
  translate_intensity = 0.0f;
  scale_intensity = 1.0f;
  translate_scan_angle = 0.0f;
  scale_scan_angle = 1.0f;
  number_attributes = 0;
  clean();
}

LASreaderTXT::~LASreaderTXT()
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

BOOL LASreaderTXT::parse_attribute(const char* l, I32 index)
{
  if (index >= header.number_attributes)
  {
    return FALSE;
  }
  F64 temp_d;
  if (sscanf(l, "%lf", &temp_d) != 1) return FALSE;
  if (attribute_pre_scales[index] != 1.0)
  {
    temp_d *= attribute_pre_scales[index];
  }
  if (attribute_pre_offsets[index] != 0.0)
  {
    temp_d -= attribute_pre_offsets[index];
  }
  if (header.attributes[index].data_type == 1)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U8_MIN || temp_i > U8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U8 is %d. clamped to [%d %d] range.\n", index, temp_i, U8_MIN, U8_MAX);
      point.set_attribute(attribute_starts[index], U8_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (U8)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 2)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I8_MIN || temp_i > I8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I8 is %d. clamped to [%d %d] range.\n", index, temp_i, I8_MIN, I8_MAX);
      point.set_attribute(attribute_starts[index], I8_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (I8)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 3)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U16_MIN || temp_i > U16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U16 is %d. clamped to [%d %d] range.\n", index, temp_i, U16_MIN, U16_MAX);
      point.set_attribute(attribute_starts[index], U16_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (U16)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 4)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I16_MIN || temp_i > I16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I16 is %d. clamped to [%d %d] range.\n", index, temp_i, I16_MIN, I16_MAX);
      point.set_attribute(attribute_starts[index], I16_CLAMP(temp_i));
    }
    else
    {
      point.set_attribute(attribute_starts[index], (I16)temp_i);
    }
  }
  else if (header.attributes[index].data_type == 5)
  {
    U32 temp_u;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_u = U32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_u = U32_QUANTIZE(temp_d);
    }
    point.set_attribute(attribute_starts[index], temp_u);
  }
  else if (header.attributes[index].data_type == 6)
  {
    I32 temp_i;
    if (header.attributes[index].has_offset())
    {
      temp_d -= header.attributes[index].offset[0];
    }
    if (header.attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header.attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    point.set_attribute(attribute_starts[index], temp_i);
  }
  else if (header.attributes[index].data_type == 9)
  {
    F32 temp_f = (F32)temp_d;
    point.set_attribute(attribute_starts[index], temp_f);
  }
  else if (header.attributes[index].data_type == 10)
  {
    point.set_attribute(attribute_starts[index], temp_d);
  }
  else
  {
    fprintf(stderr, "WARNING: attribute %d not (yet) implemented.\n", index);
    return FALSE;
  }
  return TRUE;
}

BOOL LASreaderTXT::parse(const char* parse_string)
{
  I32 temp_i;
  F32 temp_f;
  const char* p = parse_string;
  const char* l = line;

  while (p[0])
  {
    if (p[0] == 'x') // we expect the x coordinate
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%lf", &(point.coordinates[0])) != 1) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'y') // we expect the y coordinate
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%lf", &(point.coordinates[1])) != 1) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'z') // we expect the x coordinate
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%lf", &(point.coordinates[2])) != 1) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 't') // we expect the gps time
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%lf", &(point.gps_time)) != 1) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'R') // we expect the red channel of the RGB field
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      point.rgb[0] = (short)temp_i;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'G') // we expect the green channel of the RGB field
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      point.rgb[1] = (short)temp_i;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'B') // we expect the blue channel of the RGB field
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      point.rgb[2] = (short)temp_i;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'I') // we expect the NIR channel of LAS 1.4 point type 8
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      point.rgb[3] = (short)temp_i;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 's') // we expect a string or a number that we don't care about
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'i') // we expect the intensity
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%f", &temp_f) != 1) return FALSE;
      if (translate_intensity != 0.0f) temp_f = temp_f+translate_intensity;
      if (scale_intensity != 1.0f) temp_f = temp_f*scale_intensity;
      if (temp_f < 0.0f || temp_f >= 65535.5f) fprintf(stderr, "WARNING: intensity %g is out of range of unsigned short\n", temp_f);
      point.set_intensity(U16_QUANTIZE(temp_f));
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'a') // we expect the scan angle
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%f", &temp_f) != 1) return FALSE;
      if (translate_scan_angle != 0.0f) temp_f = temp_f+translate_scan_angle;
      if (scale_scan_angle != 1.0f) temp_f = temp_f*scale_scan_angle;
      if (temp_f < -128.0f || temp_f > 127.0f) fprintf(stderr, "WARNING: scan angle %g is out of range of char\n", temp_f);
      point.set_scan_angle(temp_f);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'n') // we expect the number of returns of given pulse
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (point_type > 5)
      {
        if (temp_i < 0 || temp_i > 15) fprintf(stderr, "WARNING: number of returns of given pulse %d is out of range of four bits\n", temp_i);
        point.set_extended_number_of_returns(temp_i & 15);
      }
      else
      {
        if (temp_i < 0 || temp_i > 7) fprintf(stderr, "WARNING: number of returns of given pulse %d is out of range of three bits\n", temp_i);
        point.set_number_of_returns(temp_i & 7);
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'r') // we expect the number of the return
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (point_type > 5)
      {
        if (temp_i < 0 || temp_i > 15) fprintf(stderr, "WARNING: return number %d is out of range of four bits\n", temp_i);
        point.set_extended_return_number(temp_i & 15);
      }
      else
      {
        if (temp_i < 0 || temp_i > 7) fprintf(stderr, "WARNING: return number %d is out of range of three bits\n", temp_i);
        point.set_return_number(temp_i & 7);
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'h') // we expect the with<h>eld flag
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: withheld flag %d is out of range of single bit\n", temp_i);
      point.set_withheld_flag(temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'k') // we expect the <k>eypoint flag
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: keypoint flag %d is out of range of single bit\n", temp_i);
      point.set_keypoint_flag(temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'g') // we expect the synthetic fla<g>
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: keypoint flag %d is out of range of single bit\n", temp_i);
      point.set_synthetic_flag(temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'o') // we expect the <o>verlap flag
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: overlap flag %d is out of range of single bit\n", temp_i);
      point.set_extended_overlap_flag(temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'l') // we expect the scanner channe<l>
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 3) fprintf(stderr, "WARNING: scanner channel %d is out of range of two bits\n", temp_i);
      point.extended_scanner_channel = temp_i & 3;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'E') // we expect a terrasolid echo encoding)
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 3) fprintf(stderr, "WARNING: terrasolid echo encoding %d is out of range of 0 to 3\n", temp_i);
      if (temp_i == 0) // only echo
      {
        point.number_of_returns = 1;
        point.return_number = 1;
      }
      else if (temp_i == 1) // first (of many)
      {
        point.number_of_returns = 2;
        point.return_number = 1;
      }
      else if (temp_i == 3) // last (of many)
      {
        point.number_of_returns = 2;
        point.return_number = 2;
      }
      else // intermediate
      {
        point.number_of_returns = 3;
        point.return_number = 2;
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'c') // we expect the classification
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 255)
      {
        fprintf(stderr, "WARNING: classification %d is out of range of unsigned char\n", temp_i);
        point.set_classification(U8_CLAMP(temp_i));
      }
      else
      {
        point.set_classification((U8)temp_i);
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'u') // we expect the user data
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 255)
      {
        fprintf(stderr, "WARNING: user data %d is out of range of unsigned char\n", temp_i);
        point.set_user_data(U8_CLAMP(temp_i));
      }
      else
      {
        point.set_user_data((U8)temp_i);
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'p') // we expect the point source ID
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 65535)
      {
        fprintf(stderr, "WARNING: point source ID %d is out of range of unsigned short\n", temp_i);
        point.set_point_source_ID(U16_CLAMP(temp_i));
      }
      else
      {
        point.set_point_source_ID((U16)temp_i);
      }
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'e') // we expect the edge of flight line flag
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: edge of flight line flag %d is out of range of boolean flag\n", temp_i);
      point.edge_of_flight_line = (temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'd') // we expect the direction of scan flag
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      if (sscanf(l, "%d", &temp_i) != 1) return FALSE;
      if (temp_i < 0 || temp_i > 1) fprintf(stderr, "WARNING: direction of scan flag %d is out of range of boolean flag\n", temp_i);
      point.scan_direction_flag = (temp_i ? 1 : 0);
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if ((p[0] >= '0') && (p[0] <= '9')) // we expect attribute number 0 to 9
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      I32 index = (I32)(p[0] - '0');
      if (!parse_attribute(l, index)) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == '(') // we expect attribute number 10 or higher
    {
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';')) l++; // first skip white spaces
      if (l[0] == 0) return FALSE;
      p++;
      I32 index = 0;
      while (p[0] >= '0' && p[0] <= '9')
      {
        index = 10*index + (I32)(p[0] - '0');
        p++;
      }
      if (!parse_attribute(l, index)) return FALSE;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'H') // we expect a hexadecimal coded RGB color
    {
      I32 hex_value;
      char hex_string[3] = "__";
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';' || l[0] == '\"')) l++; // first skip white spaces and quotes
      if (l[0] == 0) return FALSE;
      hex_string[0] = l[0]; hex_string[1] = l[1];
      sscanf(hex_string,"%x",&hex_value);
      point.rgb[0] = hex_value;
      hex_string[0] = l[2]; hex_string[1] = l[3];
      sscanf(hex_string,"%x",&hex_value);
      point.rgb[1] = hex_value;
      hex_string[0] = l[4]; hex_string[1] = l[5];
      sscanf(hex_string,"%x",&hex_value);
      point.rgb[2] = hex_value;
      l+=6;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else if (p[0] == 'J') // we expect a hexadecimal coded intensity
    {
      I32 hex_value;
      while (l[0] && (l[0] == ' ' || l[0] == ',' || l[0] == '\t' || l[0] == ';' || l[0] == '\"')) l++; // first skip white spaces and quotes
      if (l[0] == 0) return FALSE;
      sscanf(l,"%x",&hex_value);
      point.intensity = U8_CLAMP(((F64)hex_value/(F64)0xFFFFFF)*255);
      l+=6;
      while (l[0] && l[0] != ' ' && l[0] != ',' && l[0] != '\t' && l[0] != ';') l++; // then advance to next white space
    }
    else
    {
      fprintf(stderr, "ERROR: unknown symbol '%c' in parse string\n", p[0]);
    }
    p++;
  }
  return TRUE;
}

BOOL LASreaderTXT::check_parse_string(const char* parse_string)
{
  const char* p = parse_string;
  while (p[0])
  {
    if ((p[0] != 'x') && // we expect the x coordinate
        (p[0] != 'y') && // we expect the y coordinate
        (p[0] != 'z') && // we expect the z coordinate
        (p[0] != 't') && // we expect the gps time
        (p[0] != 'R') && // we expect the red channel of the RGB field
        (p[0] != 'G') && // we expect the green channel of the RGB field
        (p[0] != 'B') && // we expect the blue channel of the RGB field
        (p[0] != 'I') && // we expect the NIR channel
        (p[0] != 's') && // we expect a string or a number that we don't care about
        (p[0] != 'i') && // we expect the intensity
        (p[0] != 'a') && // we expect the scan angle
        (p[0] != 'n') && // we expect the number of returns of given pulse
        (p[0] != 'r') && // we expect the number of the return
        (p[0] != 'h') && // we expect the with<h>eld flag
        (p[0] != 'k') && // we expect the <k>eypoint flag
        (p[0] != 'g') && // we expect the synthetic fla<g>
        (p[0] != 'o') && // we expect the <o>verlap flag
        (p[0] != 'l') && // we expect the scanner channe<l>
        (p[0] != 'E') && // we expect terrasolid echo encoding
        (p[0] != 'c') && // we expect the classification
        (p[0] != 'u') && // we expect the user data
        (p[0] != 'p') && // we expect the point source ID
        (p[0] != 'e') && // we expect the edge of flight line flag
        (p[0] != 'd') && // we expect the direction of scan flag
        (p[0] != 'H') && // we expect hexadecimal coded RGB(I) colors
        (p[0] != 'J'))   // we expect a hexadecimal coded intensity
    {
      if ((p[0] >= '0') && (p[0] <= '9'))
      {
        I32 index = (I32)(p[0] - '0');
        if (index >= header.number_attributes)
        {
          fprintf(stderr, "ERROR: extra bytes attribute '%d' was not described.\n", index);
          return FALSE;
        }
        attribute_starts[index] = header.get_attribute_start(index);
      }
      else if (p[0] == '(')
      {
        p++;
        if ((p[0] >= '0') && (p[0] <= '9'))
        {
          I32 index = 0;
          while ((p[0] >= '0') && (p[0] <= '9'))
          {
            index = 10*index + (I32)(p[0] - '0');
            p++;
          }
          if (index >= header.number_attributes)
          {
            fprintf(stderr, "ERROR: extra bytes attribute '%d' was not described.\n", index);
            return FALSE;
          }
          if (p[0] != ')')
          {
            fprintf(stderr, "ERROR: extra bytes attribute '%d' misses closing bracket.\n", index);
            return FALSE;
          }
          attribute_starts[index] = header.get_attribute_start(index);
        }
        else
        {
          fprintf(stderr, "ERROR: parse string opening bracket '(' misses extra bytes index.\n");
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr, "ERROR: unknown symbol '%c' in parse string. valid are\n", p[0]);
        fprintf(stderr, "       'x' : the <x> coordinate\n");
        fprintf(stderr, "       'y' : the <y> coordinate\n");
        fprintf(stderr, "       'z' : the <z> coordinate\n");
        fprintf(stderr, "       't' : the gps <t>ime\n");
        fprintf(stderr, "       'R' : the <R>ed channel of the RGB field\n");
        fprintf(stderr, "       'G' : the <G>reen channel of the RGB field\n");
        fprintf(stderr, "       'B' : the <B>lue channel of the RGB field\n");
        fprintf(stderr, "       'I' : the N<I>R channel of LAS 1.4 point type 8\n");
        fprintf(stderr, "       's' : <s>kip a string or a number that we don't care about\n");
        fprintf(stderr, "       'i' : the <i>ntensity\n");
        fprintf(stderr, "       'a' : the scan <a>ngle\n");
        fprintf(stderr, "       'n' : the <n>umber of returns of that given pulse\n");
        fprintf(stderr, "       'r' : the number of the <r>eturn\n");
        fprintf(stderr, "       'h' : the with<h>eld flag\n");
        fprintf(stderr, "       'k' : the <k>eypoint flag\n");
        fprintf(stderr, "       'g' : the synthetic fla<g>\n");
        fprintf(stderr, "       'o' : the <o>verlap flag of LAS 1.4 point types 6, 7, 8\n");
        fprintf(stderr, "       'l' : the scanner channe<l> of LAS 1.4 point types 6, 7, 8\n");
        fprintf(stderr, "       'E' : terrasolid <E>hco Encoding\n");
        fprintf(stderr, "       'c' : the <c>lassification\n");
        fprintf(stderr, "       'u' : the <u>ser data\n");
        fprintf(stderr, "       'p' : the <p>oint source ID\n");
        fprintf(stderr, "       'e' : the <e>dge of flight line flag\n");
        fprintf(stderr, "       'd' : the <d>irection of scan flag\n");
        fprintf(stderr, "   '0'-'9' : additional attributes described as extra bytes (0 through 9)\n");
        fprintf(stderr, "    '(13)' : additional attributes described as extra bytes (10 and up)\n");
        fprintf(stderr, "       'H' : a hexadecimal string encoding the RGB color\n");
        fprintf(stderr, "       'J' : a hexadecimal string encoding the intensity\n");
        return FALSE;
      }
    }
    p++;
  }
  return TRUE;
}

void LASreaderTXT::populate_scale_and_offset()
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

void LASreaderTXT::populate_bounding_box()
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

LASreaderTXTrescale::LASreaderTXTrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderTXT()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderTXTrescale::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
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

LASreaderTXTreoffset::LASreaderTXTreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderTXT()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderTXTreoffset::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
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

LASreaderTXTrescalereoffset::LASreaderTXTrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderTXTrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderTXTreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderTXTrescalereoffset::open(const CHAR* file_name, U8 point_type, const CHAR* parse_string, I32 skip_lines, BOOL populate_header)
{
  if (!LASreaderTXT::open(file_name, point_type, parse_string, skip_lines, populate_header)) return FALSE;
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
