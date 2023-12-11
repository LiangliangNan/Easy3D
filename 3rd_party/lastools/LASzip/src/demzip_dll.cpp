/*
===============================================================================

  FILE:  demzip_dll.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    31 August 2019 -- created during codesprint after FOSS4G 2019 in Bucharest 

===============================================================================
*/

#define DEMZIP_DYN_LINK
#define DEMZIP_SOURCE

#include "demzip_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "laszip.hpp"
#include "lasattributer.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamin_file.hpp"
#include "bytestreamout_array.hpp"
#include "bytestreamin_array.hpp"
#include "bytestreamin_istream.hpp"
#include "bytestreamout_ostream.hpp"
#include "laswritepoint.hpp"
#include "lasreadpoint.hpp"
#include "lasquadtree.hpp"
#include "lasindex.hpp"

class demzip_dll_inventory
{
public:
  BOOL active() const { return (first == FALSE); };
  U32 number_of_point_records;
  U32 number_of_points_by_return[16];
  I32 max_X;
  I32 min_X;
  I32 max_Y;
  I32 min_Y;
  I32 max_Z;
  I32 min_Z;
  void add(const demzip_point_struct* point)
  {
    number_of_point_records++;
    if (point->extended_point_type)
    {
      number_of_points_by_return[point->extended_return_number]++;
    }
    else
    {
      number_of_points_by_return[point->return_number]++;
    }
    if (first)
    {
      min_X = max_X = point->X;
      min_Y = max_Y = point->Y;
      min_Z = max_Z = point->Z;
      first = FALSE;
    }
    else
    {
      if (point->X < min_X) min_X = point->X;
      else if (point->X > max_X) max_X = point->X;
      if (point->Y < min_Y) min_Y = point->Y;
      else if (point->Y > max_Y) max_Y = point->Y;
      if (point->Z < min_Z) min_Z = point->Z;
      else if (point->Z > max_Z) max_Z = point->Z;
    }
  }
  demzip_dll_inventory()
  {
    U32 i;
    number_of_point_records = 0;
    for (i = 0; i < 16; i++) number_of_points_by_return[i] = 0;
    max_X = min_X = 0;
    max_Y = min_Y = 0;
    max_Z = min_Z = 0;
    first = TRUE;
  }
private:
  BOOL first;
};

typedef struct demzip_dll {
  demzip_header_struct header;
  I64 p_count;
  I64 npoints;
  demzip_point_struct point;
  U8** point_items;
  FILE* file;
  ByteStreamIn* streamin;
  LASreadPoint* reader;
  ByteStreamOut* streamout;
  LASwritePoint* writer;
  LASattributer* attributer;
  CHAR error[1024];
  CHAR warning[1024];
  LASindex* lax_index;
  F64 lax_r_min_x;
  F64 lax_r_min_y;
  F64 lax_r_max_x;
  F64 lax_r_max_y;
  CHAR* lax_file_name;
  BOOL lax_create;
  BOOL lax_append;
  BOOL lax_exploit;
  U32 las14_decompress_selective;
  BOOL preserve_generating_software;
  BOOL request_native_extension;
  BOOL request_compatibility_mode;
  BOOL compatibility_mode;
  U32 set_chunk_size;
  I32 start_scan_angle;
  I32 start_extended_returns;
  I32 start_classification;
  I32 start_flags_and_channel;
  I32 start_NIR_band;
  demzip_dll_inventory* inventory;
  std::vector<void *> buffers;

  void zero()
  {
    memset(&header, 0, sizeof(demzip_header_struct));
    p_count = 0;
    npoints = 0;
    memset(&point, 0, sizeof(demzip_point_struct));
    point_items = NULL;
    file = NULL;
    streamin = NULL;
    reader = NULL;
    streamout = NULL;
    writer = NULL;
    attributer = NULL;
    memset(error, 0, 1024);
    memset(warning, 0, 1024);
    lax_index = NULL;
    lax_r_min_x = 0.0;
    lax_r_min_y = 0.0;
    lax_r_max_x = 0.0;
    lax_r_max_y = 0.0;
    lax_file_name = NULL;
    lax_create = FALSE;
    lax_append = FALSE;
    lax_exploit = FALSE;
    las14_decompress_selective = 0;
    preserve_generating_software = FALSE;
    request_native_extension = FALSE;
    request_compatibility_mode = FALSE;
    compatibility_mode = FALSE;
    set_chunk_size = 0;
    start_scan_angle = 0;
    start_extended_returns = 0;
    start_classification = 0;
    start_flags_and_channel = 0;
    start_NIR_band = 0;
    inventory = NULL;
  };
} demzip_dll_struct;

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_version(
    demzip_U8*                         version_major
    , demzip_U8*                       version_minor
    , demzip_U16*                      version_revision
    , demzip_U32*                      version_build
)
{
  try
  {
    *version_major = LASZIP_VERSION_MAJOR;
    *version_minor = LASZIP_VERSION_MINOR;
    *version_revision = LASZIP_VERSION_REVISION;
    *version_build = LASZIP_VERSION_BUILD_DATE;
  }
  catch (...)
  {
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_error(
    demzip_POINTER                     pointer
    , demzip_CHAR**                    error
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    *error = demzip_dll->error;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_error");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_warning(
    demzip_POINTER                     pointer
    , demzip_CHAR**                    warning
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    *warning = demzip_dll->warning;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_warning");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_create(
    demzip_POINTER*                    pointer
)
{
  if (pointer == 0) return 1;

  try
  {
    demzip_dll_struct* demzip_dll = new demzip_dll_struct;
    if (demzip_dll == 0)
    {
      return 1;
    }

    // zero every field of the demzip_dll struct

    demzip_dll->zero();

    // create the default

    demzip_clean(demzip_dll);

    *pointer = demzip_dll;
  }
  catch (...)
  {
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_clean(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot clean while reader is open.");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot clean while writer is open.");
      return 1;
    }

    // dealloc everything alloc in the header

    if (demzip_dll->header.user_data_in_header)
    {
      delete [] demzip_dll->header.user_data_in_header;
      demzip_dll->header.user_data_in_header = 0;
    }

    if (demzip_dll->header.vlrs)
    {
      U32 i;
      for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
      {
        if (demzip_dll->header.vlrs[i].data)
        {
          delete [] demzip_dll->header.vlrs[i].data;
        }
      }
      free(demzip_dll->header.vlrs);
      demzip_dll->header.vlrs = 0;
    }

    if (demzip_dll->header.user_data_after_header)
    {
      delete [] demzip_dll->header.user_data_after_header;
      demzip_dll->header.user_data_after_header = 0;
    }

    // dealloc everything alloc in the point

    if (demzip_dll->point.extra_bytes)
    {
      delete [] demzip_dll->point.extra_bytes;
      demzip_dll->point.extra_bytes = 0;
    }

    // dealloc point items although close_reader() / close_writer() call should have done this already

    if (demzip_dll->point_items)
    {
      delete [] demzip_dll->point_items;
      demzip_dll->point_items = 0;
    }

    // close file although close_reader() / close_writer() call should have done this already

    if (demzip_dll->file)
    {
      fclose(demzip_dll->file);
      demzip_dll->file = 0;
    }

    // dealloc streamin although close_reader() call should have done this already

    if (demzip_dll->streamin)
    {
      delete demzip_dll->streamin;
      demzip_dll->streamin = 0;
    }

    // dealloc streamout although close_writer() call should have done this already

    if (demzip_dll->streamout)
    {
      delete demzip_dll->streamout;
      demzip_dll->streamout = 0;
    }

    // dealloc the attributer

    if (demzip_dll->attributer)
    {
      delete demzip_dll->attributer;
      demzip_dll->attributer = 0;
    }

    // dealloc lax_index although close_reader() / close_writer() call should have done this already

    if (demzip_dll->lax_index)
    {
      delete demzip_dll->lax_index;
      demzip_dll->lax_index = 0;
    }

    // dealloc lax_file_name although close_writer() call should have done this already

    if (demzip_dll->lax_file_name)
    {
      free(demzip_dll->lax_file_name);
      demzip_dll->lax_file_name = 0;
    }

    // dealloc the inventory although close_writer() call should have done this already

    if (demzip_dll->inventory == 0)
    {
      delete demzip_dll->inventory;
      demzip_dll->inventory = 0;
    }

    // dealloc any data fields that were kept around in memory for others

    if (demzip_dll->buffers.size())
    {
      for (size_t i = 0; i < demzip_dll->buffers.size(); i++)
      {
        free(demzip_dll->buffers[i]);
      }
      demzip_dll->buffers.clear();
    }

    // zero every field of the demzip_dll struct

    demzip_dll->zero();

    // create default header

    sprintf(demzip_dll->header.generating_software, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    demzip_dll->header.version_major = 1;
    demzip_dll->header.version_minor = 2;
    demzip_dll->header.header_size = 227;
    demzip_dll->header.offset_to_point_data = 227;
    demzip_dll->header.point_data_format = 1;
    demzip_dll->header.point_data_record_length = 28;
    demzip_dll->header.x_scale_factor = 0.01;
    demzip_dll->header.y_scale_factor = 0.01;
    demzip_dll->header.z_scale_factor = 0.01;
    demzip_dll->set_chunk_size = LASZIP_CHUNK_SIZE_DEFAULT;
    demzip_dll->request_native_extension = TRUE;
    demzip_dll->las14_decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_ALL;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_clean");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_destroy(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  int err = 0;

  try
  {
    err = demzip_clean(demzip_dll);
    delete demzip_dll;
  }
  catch (...)
  {
    return 1;
  }

  return err;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_header_pointer(
    demzip_POINTER                     pointer
    , demzip_header_struct**           header_pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (header_pointer == 0)
    {
      sprintf(demzip_dll->error, "demzip_header_struct pointer 'header_pointer' is zero");
      return 1;
    }

    *header_pointer = &demzip_dll->header;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_header_pointer");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_point_pointer(
    demzip_POINTER                     pointer
    , demzip_point_struct**            point_pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (point_pointer == 0)
    {
      sprintf(demzip_dll->error, "demzip_point_struct pointer 'point_pointer' is zero");
      return 1;
    }

    *point_pointer = &demzip_dll->point;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_point_pointer");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_point_count(
    demzip_POINTER                     pointer
    , demzip_I64*                      count
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (count == 0)
    {
      sprintf(demzip_dll->error, "demzip_I64 pointer 'count' is zero");
      return 1;
    }

    if ((demzip_dll->reader == 0) && (demzip_dll->writer == 0))
    {
      sprintf(demzip_dll->error, "getting count before reader or writer was opened");
      return 1;
    }

    *count = demzip_dll->p_count;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_point_count");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_header(
    demzip_POINTER                     pointer
    , const demzip_header_struct*      header
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (header == 0)
    {
      sprintf(demzip_dll->error, "demzip_header_struct pointer 'header' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set header after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot set header after writer was opened");
      return 1;
    }

    // dealloc the attributer (if needed)

    if (demzip_dll->attributer)
    {
      delete demzip_dll->attributer;
      demzip_dll->attributer = 0;
    }

    // populate the header

    U32 i;

    demzip_dll->header.file_source_ID = header->file_source_ID;
    demzip_dll->header.global_encoding = header->global_encoding;
    demzip_dll->header.project_ID_GUID_data_1 = header->project_ID_GUID_data_1;
    demzip_dll->header.project_ID_GUID_data_2 = header->project_ID_GUID_data_2;
    demzip_dll->header.project_ID_GUID_data_3 = header->project_ID_GUID_data_3;
    memcpy(demzip_dll->header.project_ID_GUID_data_4, header->project_ID_GUID_data_4, 8);
    demzip_dll->header.version_major = header->version_major;
    demzip_dll->header.version_minor = header->version_minor;
    memcpy(demzip_dll->header.system_identifier, header->system_identifier, 32);
    memcpy(demzip_dll->header.generating_software, header->generating_software, 32);
    demzip_dll->header.file_creation_day = header->file_creation_day;
    demzip_dll->header.file_creation_year = header->file_creation_year;
    demzip_dll->header.header_size = header->header_size;
    demzip_dll->header.offset_to_point_data = header->offset_to_point_data;
    demzip_dll->header.number_of_variable_length_records = header->number_of_variable_length_records;
    demzip_dll->header.point_data_format = header->point_data_format;
    demzip_dll->header.point_data_record_length = header->point_data_record_length;
    demzip_dll->header.number_of_point_records = header->number_of_point_records;
    for (i = 0; i < 5; i++) demzip_dll->header.number_of_points_by_return[i] = header->number_of_points_by_return[i];
    demzip_dll->header.x_scale_factor = header->x_scale_factor;
    demzip_dll->header.y_scale_factor = header->y_scale_factor;
    demzip_dll->header.z_scale_factor = header->z_scale_factor;
    demzip_dll->header.x_offset = header->x_offset;
    demzip_dll->header.y_offset = header->y_offset;
    demzip_dll->header.z_offset = header->z_offset;
    demzip_dll->header.max_x = header->max_x;
    demzip_dll->header.min_x = header->min_x;
    demzip_dll->header.max_y = header->max_y;
    demzip_dll->header.min_y = header->min_y;
    demzip_dll->header.max_z = header->max_z;
    demzip_dll->header.min_z = header->min_z;

    if (demzip_dll->header.version_minor >= 3)
    {
      demzip_dll->header.start_of_waveform_data_packet_record = header->start_of_first_extended_variable_length_record;
    }

    if (demzip_dll->header.version_minor >= 4)
    {
      demzip_dll->header.start_of_first_extended_variable_length_record = header->start_of_first_extended_variable_length_record;
      demzip_dll->header.number_of_extended_variable_length_records = header->number_of_extended_variable_length_records;
      demzip_dll->header.extended_number_of_point_records = header->extended_number_of_point_records;
      for (i = 0; i < 15; i++) demzip_dll->header.extended_number_of_points_by_return[i] = header->extended_number_of_points_by_return[i];
    }

    demzip_dll->header.user_data_in_header_size = header->user_data_in_header_size;
    if (demzip_dll->header.user_data_in_header)
    {
      delete [] demzip_dll->header.user_data_in_header;
      demzip_dll->header.user_data_in_header = 0;
    }
    if (header->user_data_in_header_size)
    {
      if (header->user_data_in_header == 0)
      {
        sprintf(demzip_dll->error, "header->user_data_in_header_size is %d but header->user_data_in_header is NULL", header->user_data_in_header_size);
        return 1;
      }
      demzip_dll->header.user_data_in_header = new U8[header->user_data_in_header_size];
      memcpy(demzip_dll->header.user_data_in_header, header->user_data_in_header, header->user_data_in_header_size);
    }

    if (demzip_dll->header.vlrs)
    {
      for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
      {
        if (demzip_dll->header.vlrs[i].data)
        {
          delete [] demzip_dll->header.vlrs[i].data;
        }
      }
      free(demzip_dll->header.vlrs);
      demzip_dll->header.vlrs = 0;
    }
    if (header->number_of_variable_length_records)
    {
      demzip_dll->header.vlrs = (demzip_vlr*)malloc(sizeof(demzip_vlr)*header->number_of_variable_length_records);
      for (i = 0; i < header->number_of_variable_length_records; i++)
      {
        demzip_dll->header.vlrs[i].reserved = header->vlrs[i].reserved;
        memcpy(demzip_dll->header.vlrs[i].user_id, header->vlrs[i].user_id, 16);
        demzip_dll->header.vlrs[i].record_id = header->vlrs[i].record_id;
        demzip_dll->header.vlrs[i].record_length_after_header = header->vlrs[i].record_length_after_header;
        memcpy(demzip_dll->header.vlrs[i].description, header->vlrs[i].description, 32);
        if (header->vlrs[i].record_length_after_header)
        {
          if (header->vlrs[i].data == 0)
          {
            sprintf(demzip_dll->error, "header->vlrs[%d].record_length_after_header is %d but header->vlrs[%d].data is NULL", i, header->vlrs[i].record_length_after_header, i);
            return 1;
          }
          demzip_dll->header.vlrs[i].data = new U8[header->vlrs[i].record_length_after_header];
          memcpy(demzip_dll->header.vlrs[i].data, header->vlrs[i].data, header->vlrs[i].record_length_after_header);
        }
        else
        {
          demzip_dll->header.vlrs[i].data = 0;
        }

        // populate the attributer if needed

        if ((strcmp(demzip_dll->header.vlrs[i].user_id, "LASF_Spec") == 0) && (demzip_dll->header.vlrs[i].record_id == 4))
        {
          if (demzip_dll->attributer == 0)
          {
            demzip_dll->attributer = new LASattributer;
            if (demzip_dll->attributer == 0)
            {
              sprintf(demzip_dll->error, "cannot allocate LASattributer");
              return 1;
            }
          }
          demzip_dll->attributer->init_attributes(demzip_dll->header.vlrs[i].record_length_after_header/sizeof(LASattribute), (LASattribute*)demzip_dll->header.vlrs[i].data);
        }
      }
    }

    demzip_dll->header.user_data_after_header_size = header->user_data_after_header_size;
    if (demzip_dll->header.user_data_after_header)
    {
      delete [] demzip_dll->header.user_data_after_header;
      demzip_dll->header.user_data_after_header = 0;
    }
    if (header->user_data_after_header_size)
    {
      if (header->user_data_after_header == 0)
      {
        sprintf(demzip_dll->error, "header->user_data_after_header_size is %d but header->user_data_after_header is NULL", header->user_data_after_header_size);
        return 1;
      }
      demzip_dll->header.user_data_after_header = new U8[header->user_data_after_header_size];
      memcpy(demzip_dll->header.user_data_after_header, header->user_data_after_header, header->user_data_after_header_size);
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_header");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_point_type_and_size(
    demzip_POINTER                     pointer
    , demzip_U8                        point_type
    , demzip_U16                       point_size
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set point format and point size after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot set point format and point size after writer was opened");
      return 1;
    }

    // check if point type and type are supported

    if (!LASzip().setup(point_type, point_size, LASZIP_COMPRESSOR_NONE))
    {
      sprintf(demzip_dll->error, "invalid combination of point_type %d and point_size %d", (I32)point_type, (I32)point_size);
      return 1;
    }

    // set point type and point size

    demzip_dll->header.point_data_format = point_type;
    demzip_dll->header.point_data_record_length = point_size;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_point_type_and_size");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_check_for_integer_overflow(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    // get a pointer to the header

    demzip_header_struct* header = &(demzip_dll->header);

    // quantize and dequantize the bounding box with current scale_factor and offset

    I32 quant_min_x = I32_QUANTIZE((header->min_x-header->x_offset)/header->x_scale_factor);
    I32 quant_max_x = I32_QUANTIZE((header->max_x-header->x_offset)/header->x_scale_factor);
    I32 quant_min_y = I32_QUANTIZE((header->min_y-header->y_offset)/header->y_scale_factor);
    I32 quant_max_y = I32_QUANTIZE((header->max_y-header->y_offset)/header->y_scale_factor);
    I32 quant_min_z = I32_QUANTIZE((header->min_z-header->z_offset)/header->z_scale_factor);
    I32 quant_max_z = I32_QUANTIZE((header->max_z-header->z_offset)/header->z_scale_factor);

    F64 dequant_min_x = header->x_scale_factor*quant_min_x+header->x_offset;
    F64 dequant_max_x = header->x_scale_factor*quant_max_x+header->x_offset;
    F64 dequant_min_y = header->y_scale_factor*quant_min_y+header->y_offset;
    F64 dequant_max_y = header->y_scale_factor*quant_max_y+header->y_offset;
    F64 dequant_min_z = header->z_scale_factor*quant_min_z+header->z_offset;
    F64 dequant_max_z = header->z_scale_factor*quant_max_z+header->z_offset;

    // make sure that there is not sign flip (a 32-bit integer overflow) for the bounding box

    if ((header->min_x > 0) != (dequant_min_x > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for min_x from %g to %g. set scale factor for x coarser than %g\n", header->min_x, dequant_min_x, header->x_scale_factor);
      return 1;
    }
    if ((header->max_x > 0) != (dequant_max_x > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for max_x from %g to %g. set scale factor for x coarser than %g\n", header->max_x, dequant_max_x, header->x_scale_factor);
      return 1;
    }
    if ((header->min_y > 0) != (dequant_min_y > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for min_y from %g to %g. set scale factor for y coarser than %g\n", header->min_y, dequant_min_y, header->y_scale_factor);
      return 1;
    }
    if ((header->max_y > 0) != (dequant_max_y > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for max_y from %g to %g. set scale factor for y coarser than %g\n", header->max_y, dequant_max_y, header->y_scale_factor);
      return 1;
    }
    if ((header->min_z > 0) != (dequant_min_z > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for min_z from %g to %g. set scale factor for z coarser than %g\n", header->min_z, dequant_min_z, header->z_scale_factor);
      return 1;
    }
    if ((header->max_z > 0) != (dequant_max_z > 0))
    {
      sprintf(demzip_dll->error, "quantization sign flip for max_z from %g to %g. set scale factor for z coarser than %g\n", header->max_z, dequant_max_z, header->z_scale_factor);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_auto_offset");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_auto_offset(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot auto offset after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot auto offset after writer was opened");
      return 1;
    }

    // get a pointer to the header

    demzip_header_struct* header = &(demzip_dll->header);

    // check scale factor

    F64 x_scale_factor = header->x_scale_factor;
    F64 y_scale_factor = header->y_scale_factor;
    F64 z_scale_factor = header->z_scale_factor;

    if ((x_scale_factor <= 0) || !F64_IS_FINITE(x_scale_factor))
    {
      sprintf(demzip_dll->error, "invalid x scale_factor %g in header", header->x_scale_factor);
      return 1;
    }

    if ((y_scale_factor <= 0) || !F64_IS_FINITE(y_scale_factor))
    {
      sprintf(demzip_dll->error, "invalid y scale_factor %g in header", header->y_scale_factor);
      return 1;
    }

    if ((z_scale_factor <= 0) || !F64_IS_FINITE(z_scale_factor))
    {
      sprintf(demzip_dll->error, "invalid z scale_factor %g in header", header->z_scale_factor);
      return 1;
    }

    F64 center_bb_x = (header->min_x + header->max_x) / 2;
    F64 center_bb_y = (header->min_y + header->max_y) / 2;
    F64 center_bb_z = (header->min_z + header->max_z) / 2;

    if (!F64_IS_FINITE(center_bb_x))
    {
      sprintf(demzip_dll->error, "invalid x coordinate at center of bounding box (min: %g max: %g)", header->min_x, header->max_x);
      return 1;
    }

    if (!F64_IS_FINITE(center_bb_y))
    {
      sprintf(demzip_dll->error, "invalid y coordinate at center of  bounding box (min: %g max: %g)", header->min_y, header->max_y);
      return 1;
    }

    if (!F64_IS_FINITE(center_bb_z))
    {
      sprintf(demzip_dll->error, "invalid z coordinate at center of  bounding box (min: %g max: %g)", header->min_z, header->max_z);
      return 1;
    }

    F64 x_offset = header->x_offset;
    F64 y_offset = header->y_offset;
    F64 z_offset = header->z_offset;

    header->x_offset = (I64_FLOOR(center_bb_x/x_scale_factor/10000000))*10000000*x_scale_factor;
    header->y_offset = (I64_FLOOR(center_bb_y/y_scale_factor/10000000))*10000000*y_scale_factor;
    header->z_offset = (I64_FLOOR(center_bb_z/z_scale_factor/10000000))*10000000*z_scale_factor;

    if (demzip_check_for_integer_overflow(pointer))
    {
      header->x_offset = x_offset;
      header->y_offset = y_offset;
      header->z_offset = z_offset;
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_auto_offset");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_point(
    demzip_POINTER                     pointer
    , const demzip_point_struct*       point
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (point == 0)
    {
      sprintf(demzip_dll->error, "demzip_point_struct pointer 'point' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set point for reader");
      return 1;
    }

    memcpy(&demzip_dll->point, point, ((U8*)&(demzip_dll->point.extra_bytes)) - ((U8*)&(demzip_dll->point.X)));

    if (demzip_dll->point.extra_bytes)
    {
      if (point->extra_bytes)
      {
        if (demzip_dll->point.num_extra_bytes == point->num_extra_bytes)
        {
          memcpy(demzip_dll->point.extra_bytes, point->extra_bytes, demzip_dll->point.num_extra_bytes);
        }
        else
        {
          sprintf(demzip_dll->error, "target point has %d extra bytes but source point has %d", demzip_dll->point.num_extra_bytes, point->num_extra_bytes);
          return 1;
        }
      }
      else if (!demzip_dll->compatibility_mode)
      {
        sprintf(demzip_dll->error, "target point has extra bytes but source point does not");
        return 1;
      }
    }
/*
    else
    {
      if (point->extra_bytes)
      {
        sprintf(demzip_dll->error, "source point has extra bytes but target point does not");
        return 1;
      }
    }
*/
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_coordinates(
    demzip_POINTER                     pointer
    , const demzip_F64*                coordinates
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (coordinates == 0)
    {
      sprintf(demzip_dll->error, "demzip_F64 pointer 'coordinates' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set coordinates for reader");
      return 1;
    }

    // get a pointer to the header

    demzip_header_struct* header = &(demzip_dll->header);

    // get a pointer to the point

    demzip_point_struct* point = &(demzip_dll->point);

    // set the coordinates

    point->X = I32_QUANTIZE((coordinates[0]-header->x_offset)/header->x_scale_factor);
    point->Y = I32_QUANTIZE((coordinates[1]-header->y_offset)/header->y_scale_factor);
    point->Z = I32_QUANTIZE((coordinates[2]-header->z_offset)/header->z_scale_factor);
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_coordinates");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_get_coordinates(
    demzip_POINTER                     pointer
    , demzip_F64*                      coordinates
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (coordinates == 0)
    {
      sprintf(demzip_dll->error, "demzip_F64 pointer 'coordinates' is zero");
      return 1;
    }

    // get a pointer to the header

    demzip_header_struct* header = &(demzip_dll->header);

    // get a pointer to the point

    demzip_point_struct* point = &(demzip_dll->point);

    // get the coordinates

    coordinates[0] = header->x_scale_factor*point->X+header->x_offset;
    coordinates[1] = header->y_scale_factor*point->Y+header->y_offset;
    coordinates[2] = header->z_scale_factor*point->Z+header->z_offset;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_get_coordinates");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geokeys(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_geokey_struct*      key_entries
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(demzip_dll->error, "number of key_entries is zero");
      return 1;
    }

    if (key_entries == 0)
    {
      sprintf(demzip_dll->error, "demzip_geokey_struct pointer 'key_entries' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set geokeys after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot set geokeys after writer was opened");
      return 1;
    }

    // create the geokey directory

    demzip_geokey_struct* key_entries_plus_one = new demzip_geokey_struct[number+1];
    if (key_entries_plus_one == 0)
    {
      sprintf(demzip_dll->error, "allocating demzip_geokey_struct[%u] array", number+1);
      return 1;
    }
    key_entries_plus_one[0].key_id = 1;            // aka key_directory_version
    key_entries_plus_one[0].tiff_tag_location = 1; // aka key_revision
    key_entries_plus_one[0].count = 0;             // aka minor_revision
    key_entries_plus_one[0].value_offset = number; // aka number_of_keys
    memcpy(key_entries_plus_one + 1, key_entries, sizeof(demzip_geokey_struct)*number);

    // add the VLR

    if (demzip_add_vlr(demzip_dll, "LASF_Projection", 34735, (demzip_U16)(8 + number*8), 0, (demzip_U8*)key_entries_plus_one))
    {
      sprintf(demzip_dll->error, "setting %u geodouble_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_geokey_entries");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geodouble_params(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_F64*                geodouble_params
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(demzip_dll->error, "number of geodouble_params is zero");
      return 1;
    }

    if (geodouble_params == 0)
    {
      sprintf(demzip_dll->error, "demzip_F64 pointer 'geodouble_params' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set geodouble_params after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot set geodouble_params after writer was opened");
      return 1;
    }

    // add the VLR

    if (demzip_add_vlr(demzip_dll, "LASF_Projection", 34736, (demzip_U16)(number*8), 0, (demzip_U8*)geodouble_params))
    {
      sprintf(demzip_dll->error, "setting %u geodouble_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_geodouble_params");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_geoascii_params(
    demzip_POINTER                     pointer
    , demzip_U32                       number
    , const demzip_CHAR*               geoascii_params
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (number == 0)
    {
      sprintf(demzip_dll->error, "number of geoascii_params is zero");
      return 1;
    }

    if (geoascii_params == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'geoascii_params' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot set geoascii_params after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot set geoascii_params after writer was opened");
      return 1;
    }

    // add the VLR

    if (demzip_add_vlr(demzip_dll, "LASF_Projection", 34737, (demzip_U16)(number), 0, (demzip_U8*)geoascii_params))
    {
      sprintf(demzip_dll->error, "setting %u geoascii_params", number);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_geoascii_params");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_add_attribute(
    demzip_POINTER                     pointer
    , demzip_U32                       type
    , const demzip_CHAR*               name
    , const demzip_CHAR*               description
    , demzip_F64                       scale
    , demzip_F64                       offset
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (type > LAS_ATTRIBUTE_F64)
    {
      sprintf(demzip_dll->error, "demzip_U32 'type' is %u but needs to be between %d and %d", type, LAS_ATTRIBUTE_U8, LAS_ATTRIBUTE_F64);
      return 1;
    }

    if (name == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'name' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot add attribute after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot add attribute after writer was opened");
      return 1;
    }

    LASattribute lasattribute(type, name, description);
    lasattribute.set_scale(scale);
    lasattribute.set_offset(offset);

    if (demzip_dll->attributer == 0)
    {
      demzip_dll->attributer = new LASattributer;
      if (demzip_dll->attributer == 0)
      {
        sprintf(demzip_dll->error, "cannot allocate LASattributer");
        return 1;
      }
    }

    if (demzip_dll->attributer->add_attribute(lasattribute) == -1)
    {
      sprintf(demzip_dll->error, "cannot add attribute '%s' to attributer", name);
      return 1;
    }

    if (demzip_add_vlr(demzip_dll, "LASF_Spec\0\0\0\0\0\0", 4, (demzip_U16)(demzip_dll->attributer->number_attributes*sizeof(LASattribute)), 0, (demzip_U8*)demzip_dll->attributer->attributes))
    {
      sprintf(demzip_dll->error, "adding the new extra bytes VLR with the additional attribute '%s'", name);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_add_attribute");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_add_vlr(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               user_id
    , demzip_U16                       record_id
    , demzip_U16                       record_length_after_header
    , const demzip_CHAR*               description
    , const demzip_U8*                 data
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (user_id == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'user_id' is zero");
      return 1;
    }

    if ((record_length_after_header > 0) && (data == 0))
    {
      sprintf(demzip_dll->error, "record_length_after_header of VLR is %u but data pointer is zero", (U32)record_length_after_header);
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot add vlr after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot add vlr after writer was opened");
      return 1;
    }

    U32 i = 0;

    if (demzip_dll->header.vlrs)
    {
      // overwrite existing VLR ?

      for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
      {
        if ((strncmp(demzip_dll->header.vlrs[i].user_id, user_id, 16) == 0) && (demzip_dll->header.vlrs[i].record_id == record_id))
        {
          if (demzip_dll->header.vlrs[i].record_length_after_header)
          {
            demzip_dll->header.offset_to_point_data -= demzip_dll->header.vlrs[i].record_length_after_header;
            demzip_dll->header.vlrs[i].record_length_after_header = 0;
            delete [] demzip_dll->header.vlrs[i].data;
            demzip_dll->header.vlrs[i].data = 0;
          }
          break;
        }
      }

      // create new VLR

      if (i == demzip_dll->header.number_of_variable_length_records)
      {
        demzip_dll->header.number_of_variable_length_records++;
        demzip_dll->header.offset_to_point_data += 54;
        demzip_dll->header.vlrs = (demzip_vlr_struct*)realloc(demzip_dll->header.vlrs, sizeof(demzip_vlr_struct)*demzip_dll->header.number_of_variable_length_records);
        if (demzip_dll->header.vlrs == 0)
        {
          sprintf(demzip_dll->error, "reallocating vlrs[%u] array", demzip_dll->header.number_of_variable_length_records);
          return 1;
        }
      }
    }
    else
    {
      demzip_dll->header.number_of_variable_length_records = 1;
      demzip_dll->header.offset_to_point_data += 54;
      demzip_dll->header.vlrs = (demzip_vlr_struct*)malloc(sizeof(demzip_vlr_struct));
      if (demzip_dll->header.vlrs == 0)
      {
        sprintf(demzip_dll->error, "allocating vlrs[1] array");
        return 1;
      }
    }

    // zero the VLR

    memset(&(demzip_dll->header.vlrs[i]), 0, sizeof(demzip_vlr_struct));

    // copy the VLR

    demzip_dll->header.vlrs[i].reserved = 0x0;
    strncpy(demzip_dll->header.vlrs[i].user_id, user_id, 16);
    demzip_dll->header.vlrs[i].record_id = record_id;
    demzip_dll->header.vlrs[i].record_length_after_header = record_length_after_header;
    if (description)
    {
      strncpy(demzip_dll->header.vlrs[i].description, description, 32);
    }
    else
    {
      sprintf(demzip_dll->header.vlrs[i].description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
    }
    if (record_length_after_header)
    {
      demzip_dll->header.offset_to_point_data += record_length_after_header;
      demzip_dll->header.vlrs[i].data = new U8[record_length_after_header];
      memcpy(demzip_dll->header.vlrs[i].data, data, record_length_after_header);
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_add_vlr");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_remove_vlr(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               user_id
    , demzip_U16                       record_id
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (user_id == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'user_id' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "cannot remove vlr after reader was opened");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "cannot remove vlr after writer was opened");
      return 1;
    }

    U32 i = 0;

    if (demzip_dll->header.vlrs)
    {
      for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
      {
        if ((strncmp(demzip_dll->header.vlrs[i].user_id, user_id, 16) == 0) && (demzip_dll->header.vlrs[i].record_id == record_id))
        {
          if (demzip_dll->header.vlrs[i].record_length_after_header)
          {
            demzip_dll->header.offset_to_point_data -= (54 + demzip_dll->header.vlrs[i].record_length_after_header);
            delete [] demzip_dll->header.vlrs[i].data;
            demzip_dll->header.vlrs[i].data = 0;
          }
          demzip_dll->header.number_of_variable_length_records--;
          for (/*i = i*/; i < demzip_dll->header.number_of_variable_length_records; i++)
          {
            demzip_dll->header.vlrs[i] = demzip_dll->header.vlrs[i+1];
          }
          if (demzip_dll->header.number_of_variable_length_records)
          {
            demzip_dll->header.vlrs = (demzip_vlr_struct*)realloc(demzip_dll->header.vlrs, sizeof(demzip_vlr_struct)*demzip_dll->header.number_of_variable_length_records);
            if (demzip_dll->header.vlrs == 0)
            {
              sprintf(demzip_dll->error, "reallocating vlrs[%u] array", demzip_dll->header.number_of_variable_length_records);
              return 1;
            }
          }
          else
          {
            free(demzip_dll->header.vlrs);
            demzip_dll->header.vlrs = 0;
          }
          i = U32_MAX;
          break;
        }
      }
      if (i != U32_MAX)
      {
        sprintf(demzip_dll->error, "cannot find VLR with user_id '%s' and record_id %d among the %u VLRs in the header", user_id, (I32)record_id, demzip_dll->header.number_of_variable_length_records);
        return 1;
      }
    }
    else
    {
      sprintf(demzip_dll->error, "cannot remove VLR with user_id '%s' and record_id %d because header has no VLRs", user_id, (I32)record_id);
      return 1;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_add_vlr");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_preserve_generating_software(
    demzip_POINTER                     pointer
    , const demzip_BOOL                preserve
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->preserve_generating_software = preserve;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_preserve_generating_software");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}


/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_request_native_extension(
    demzip_POINTER                     pointer
    , const demzip_BOOL                request
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->request_native_extension = request;

    if (request) // only one should be on
    {
      demzip_dll->request_compatibility_mode = FALSE;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_request_native_extension");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_request_compatibility_mode(
    demzip_POINTER                     pointer
    , const demzip_BOOL                request
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->request_compatibility_mode = request;

    if (request) // only one should be on
    {
      demzip_dll->request_native_extension = FALSE;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_request_compatibility_mode");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_set_chunk_size(
    demzip_POINTER                     pointer
    , const demzip_U32                 chunk_size
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->set_chunk_size = chunk_size;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_set_chunk_size");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_create_spatial_index(
    demzip_POINTER                     pointer
    , const demzip_BOOL                create
    , const demzip_BOOL                append
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    if (append)
    {
      sprintf(demzip_dll->error, "appending of spatial index not (yet) supported in this version");
      return 1;
    }

    demzip_dll->lax_create = create;
    demzip_dll->lax_append = append;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_create_spatial_index");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
demzip_prepare_header_for_write(
    demzip_dll_struct*                 demzip_dll
)
{
  if ((demzip_dll->header.version_major != 1) || (demzip_dll->header.version_minor > 4))
  {
    sprintf(demzip_dll->error, "unknown LAS version %d.%d", (I32)demzip_dll->header.version_major, (I32)demzip_dll->header.version_minor);
    return 1;
  }

  // check counters
  U32 i;

  if (demzip_dll->header.point_data_format > 5)
  {
    // legacy counters are zero for new point types

    demzip_dll->header.number_of_point_records = 0;
    for (i = 0; i < 5; i++)
    {
      demzip_dll->header.number_of_points_by_return[i] = 0;
    }
  }
  else if (demzip_dll->header.version_minor > 3)
  {
    // legacy counters must be zero or consistent for old point types

    if (demzip_dll->header.number_of_point_records != demzip_dll->header.extended_number_of_point_records)
    {
      if (demzip_dll->header.number_of_point_records != 0)
      {
#ifdef _WIN32
        sprintf(demzip_dll->error, "inconsistent number_of_point_records %u and extended_number_of_point_records %I64d", demzip_dll->header.number_of_point_records, demzip_dll->header.extended_number_of_point_records);
#else
        sprintf(demzip_dll->error, "inconsistent number_of_point_records %u and extended_number_of_point_records %llu", demzip_dll->header.number_of_point_records, demzip_dll->header.extended_number_of_point_records);
#endif
        return 1;
      }
      else if (demzip_dll->header.extended_number_of_point_records <= U32_MAX)
      {
        demzip_dll->header.number_of_point_records = (U32)demzip_dll->header.extended_number_of_point_records;
      }
    }
    for (i = 0; i < 5; i++)
    {
      if (demzip_dll->header.number_of_points_by_return[i] != demzip_dll->header.extended_number_of_points_by_return[i])
      {
        if (demzip_dll->header.number_of_points_by_return[i] != 0)
        {
#ifdef _WIN32
          sprintf(demzip_dll->error, "inconsistent number_of_points_by_return[%u] %u and extended_number_of_points_by_return[%u] %I64d", i, demzip_dll->header.number_of_points_by_return[i], i, demzip_dll->header.extended_number_of_points_by_return[i]);
#else
          sprintf(demzip_dll->error, "inconsistent number_of_points_by_return[%u] %u and extended_number_of_points_by_return[%u] %llu", i, demzip_dll->header.number_of_points_by_return[i], i, demzip_dll->header.extended_number_of_points_by_return[i]);
#endif
          return 1;
        }
        else if (demzip_dll->header.extended_number_of_points_by_return[i] <= U32_MAX)
        {
          demzip_dll->header.number_of_points_by_return[i] = (U32)demzip_dll->header.extended_number_of_points_by_return[i];
        }
      }
    }
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
demzip_prepare_point_for_write(
    demzip_dll_struct*                 demzip_dll
    , const demzip_BOOL                compress
)
{
  U32 i;

  if (demzip_dll->header.point_data_format > 5)
  {
    // must be set for the new point types 6 or higher ...

    demzip_dll->point.extended_point_type = 1;

    if (demzip_dll->request_native_extension)
    {
      // we are *not* operating in compatibility mode

      demzip_dll->compatibility_mode = FALSE;
    }
    else if (demzip_dll->request_compatibility_mode)
    {
      // we are *not* using the native extension

      demzip_dll->request_native_extension = FALSE;

      // make sure there are no more than U32_MAX points

      if (demzip_dll->header.extended_number_of_point_records > U32_MAX)
      {
#ifdef _WIN32
        sprintf(demzip_dll->error, "extended_number_of_point_records of %I64d is too much for 32-bit counters of compatibility mode", demzip_dll->header.extended_number_of_point_records);
#else
        sprintf(demzip_dll->error, "extended_number_of_point_records of %llu is too much for 32-bit counters of compatibility mode", demzip_dll->header.extended_number_of_point_records);
#endif
        return 1;
      }

      // copy 64-bit extended counters back into 32-bit legacy counters

      demzip_dll->header.number_of_point_records = (U32)(demzip_dll->header.extended_number_of_point_records);
      for (i = 0; i < 5; i++)
      {
        demzip_dll->header.number_of_points_by_return[i] = (U32)(demzip_dll->header.extended_number_of_points_by_return[i]);
      }

      // are there any "extra bytes" already ... ?

      I32 number_of_existing_extrabytes = 0;

      switch (demzip_dll->header.point_data_format)
      {
      case 6:
        number_of_existing_extrabytes = demzip_dll->header.point_data_record_length - 30;
        break;
      case 7:
        number_of_existing_extrabytes = demzip_dll->header.point_data_record_length - 36;
        break;
      case 8:
        number_of_existing_extrabytes = demzip_dll->header.point_data_record_length - 38;
        break;
      case 9:
        number_of_existing_extrabytes = demzip_dll->header.point_data_record_length - 59;
        break;
      case 10:
        number_of_existing_extrabytes = demzip_dll->header.point_data_record_length - 67;
        break;
      default:
        sprintf(demzip_dll->error, "unknown point_data_format %d", demzip_dll->header.point_data_format);
        return 1;
      }

      if (number_of_existing_extrabytes < 0)
      {
        sprintf(demzip_dll->error, "bad point_data_format %d point_data_record_length %d combination", demzip_dll->header.point_data_format, demzip_dll->header.point_data_record_length);
        return 1;
      }

      // downgrade to LAS 1.2 or LAS 1.3
      if (demzip_dll->header.point_data_format <= 8)
      {
        demzip_dll->header.version_minor = 2;
        // LAS 1.2 header is 148 bytes less than LAS 1.4+ header
        demzip_dll->header.header_size -= 148;
        demzip_dll->header.offset_to_point_data -= 148;
      }
      else
      {
        demzip_dll->header.version_minor = 3;
        // LAS 1.3 header is 140 bytes less than LAS 1.4+ header
        demzip_dll->header.header_size -= 140;
        demzip_dll->header.offset_to_point_data -= 140;
      }
      // turn off the bit indicating the presence of the OGC WKT
      demzip_dll->header.global_encoding &= ~(1<<4);

      // old point type is two bytes shorter
      demzip_dll->header.point_data_record_length -= 2;
      // but we add 5 bytes of attributes
      demzip_dll->header.point_data_record_length += 5;

      // create 2+2+4+148 bytes payload for compatibility VLR
      ByteStreamOutArray* out;
      if (IS_LITTLE_ENDIAN())
        out = new ByteStreamOutArrayLE();
      else
        out = new ByteStreamOutArrayBE();
      // write control info
      U16 demzip_version = (U16)LASZIP_VERSION_BUILD_DATE;
      out->put16bitsLE((U8*)&demzip_version);
      U16 compatible_version = 3;
      out->put16bitsLE((U8*)&compatible_version);
      U32 unused = 0;
      out->put32bitsLE((U8*)&unused);
      // write the 148 bytes of the extended LAS 1.4 header
      U64 start_of_waveform_data_packet_record = demzip_dll->header.start_of_waveform_data_packet_record;
      if (start_of_waveform_data_packet_record != 0)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %I64d. writing 0 instead.\n", start_of_waveform_data_packet_record);
#else
        fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %llu. writing 0 instead.\n", start_of_waveform_data_packet_record);
#endif
        start_of_waveform_data_packet_record = 0;
      }
      out->put64bitsLE((U8*)&start_of_waveform_data_packet_record);
      U64 start_of_first_extended_variable_length_record = demzip_dll->header.start_of_first_extended_variable_length_record;
      if (start_of_first_extended_variable_length_record != 0)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: EVLRs not supported. header->start_of_first_extended_variable_length_record is %I64d. writing 0 instead.\n", start_of_first_extended_variable_length_record);
#else
        fprintf(stderr,"WARNING: EVLRs not supported. header->start_of_first_extended_variable_length_record is %llu. writing 0 instead.\n", start_of_first_extended_variable_length_record);
#endif
        start_of_first_extended_variable_length_record = 0;
      }
      out->put64bitsLE((U8*)&start_of_first_extended_variable_length_record);
      U32 number_of_extended_variable_length_records = demzip_dll->header.number_of_extended_variable_length_records;
      if (number_of_extended_variable_length_records != 0)
      {
        fprintf(stderr,"WARNING: EVLRs not supported. header->number_of_extended_variable_length_records is %u. writing 0 instead.\n", number_of_extended_variable_length_records);
        number_of_extended_variable_length_records = 0;
      }
      out->put32bitsLE((U8*)&number_of_extended_variable_length_records);
      U64 extended_number_of_point_records;
      if (demzip_dll->header.number_of_point_records)
        extended_number_of_point_records = demzip_dll->header.number_of_point_records;
      else
        extended_number_of_point_records = demzip_dll->header.extended_number_of_point_records;
      out->put64bitsLE((U8*)&extended_number_of_point_records);
      U64 extended_number_of_points_by_return;
      for (i = 0; i < 15; i++)
      {
        if ((i < 5) && demzip_dll->header.number_of_points_by_return[i])
          extended_number_of_points_by_return = demzip_dll->header.number_of_points_by_return[i];
        else
          extended_number_of_points_by_return = demzip_dll->header.extended_number_of_points_by_return[i];
        out->put64bitsLE((U8*)&extended_number_of_points_by_return);
      }

      // add the compatibility VLR

      if (demzip_add_vlr(demzip_dll, "lascompatible\0\0", 22204, (demzip_U16)(2+2+4+148), 0, (demzip_U8*)out->takeData()))
      {
        sprintf(demzip_dll->error, "adding the compatibility VLR");
        return 1;
      }
      delete out;

      // if needed create an attributer to describe the "extra bytes"

      if (demzip_dll->attributer == 0)
      {
        demzip_dll->attributer = new LASattributer;
        if (demzip_dll->attributer == 0)
        {
          sprintf(demzip_dll->error, "cannot allocate LASattributer");
          return 1;
        }
      }

      // were there any pre-existing extra bytes

      if (number_of_existing_extrabytes > 0)
      {
        // make sure the existing "extra bytes" are documented

        if (demzip_dll->attributer->get_attributes_size() > number_of_existing_extrabytes)
        {
          sprintf(demzip_dll->error, "bad \"extra bytes\" VLR describes %d bytes more than points actually have", demzip_dll->attributer->get_attributes_size() - number_of_existing_extrabytes);
          return 1;
        }
        else if (demzip_dll->attributer->get_attributes_size() < number_of_existing_extrabytes)
        {
          // maybe the existing "extra bytes" are documented in a VLR
          if (demzip_dll->header.vlrs)
          {
            for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
            {
              if ((strcmp(demzip_dll->header.vlrs[i].user_id, "LASF_Spec") == 0) && (demzip_dll->header.vlrs[i].record_id == 4))
              {
                demzip_dll->attributer->init_attributes(demzip_dll->header.vlrs[i].record_length_after_header/sizeof(LASattribute), (LASattribute*)demzip_dll->header.vlrs[i].data);
              }
            }
          }

          // describe any undocumented "extra bytes" as "unknown" U8  attributes
          for (I32 i = (I32)(demzip_dll->attributer->get_attributes_size()); i < number_of_existing_extrabytes; i++)
          {
            CHAR unknown_name[16];
            memset(unknown_name, 0, 16);
            sprintf(unknown_name, "unknown %d", i);
            LASattribute lasattribute_unknown(LAS_ATTRIBUTE_U8, unknown_name, unknown_name);
            if (demzip_dll->attributer->add_attribute(lasattribute_unknown) == -1)
            {
              sprintf(demzip_dll->error, "cannot add unknown U8 attribute '%s' of %d to attributer", unknown_name, number_of_existing_extrabytes);
              return 1;
            }
          }
        }
      }

      // create the "extra bytes" that store the newer LAS 1.4 point attributes

      // scan_angle (difference or remainder) is stored as a I16
      LASattribute lasattribute_scan_angle(LAS_ATTRIBUTE_I16, "LAS 1.4 scan angle", "additional attributes");
      lasattribute_scan_angle.set_scale(0.006);
      I32 index_scan_angle = demzip_dll->attributer->add_attribute(lasattribute_scan_angle);
      demzip_dll->start_scan_angle = demzip_dll->attributer->get_attribute_start(index_scan_angle);
      // extended returns stored as a U8
      LASattribute lasattribute_extended_returns(LAS_ATTRIBUTE_U8, "LAS 1.4 extended returns", "additional attributes");
      I32 index_extended_returns = demzip_dll->attributer->add_attribute(lasattribute_extended_returns);
      demzip_dll->start_extended_returns = demzip_dll->attributer->get_attribute_start(index_extended_returns);
      // classification stored as a U8
      LASattribute lasattribute_classification(LAS_ATTRIBUTE_U8, "LAS 1.4 classification", "additional attributes");
      I32 index_classification = demzip_dll->attributer->add_attribute(lasattribute_classification);
      demzip_dll->start_classification = demzip_dll->attributer->get_attribute_start(index_classification);
      // flags and channel stored as a U8
      LASattribute lasattribute_flags_and_channel(LAS_ATTRIBUTE_U8, "LAS 1.4 flags and channel", "additional attributes");
      I32 index_flags_and_channel = demzip_dll->attributer->add_attribute(lasattribute_flags_and_channel);
      demzip_dll->start_flags_and_channel = demzip_dll->attributer->get_attribute_start(index_flags_and_channel);
      // maybe store the NIR band as a U16
      if (demzip_dll->header.point_data_format == 8 || demzip_dll->header.point_data_format == 10)
      {
        // the NIR band is stored as a U16
        LASattribute lasattribute_NIR_band(LAS_ATTRIBUTE_U16, "LAS 1.4 NIR band", "additional attributes");
        I32 index_NIR_band = demzip_dll->attributer->add_attribute(lasattribute_NIR_band);
        demzip_dll->start_NIR_band = demzip_dll->attributer->get_attribute_start(index_NIR_band);
      }
      else
      {
        demzip_dll->start_NIR_band = -1;
      }

      // add the extra bytes VLR with the additional attributes

      if (demzip_add_vlr(demzip_dll, "LASF_Spec\0\0\0\0\0\0", 4, (demzip_U16)(demzip_dll->attributer->number_attributes*sizeof(LASattribute)), 0, (demzip_U8*)demzip_dll->attributer->attributes))
      {
        sprintf(demzip_dll->error, "adding the extra bytes VLR with the additional attributes");
        return 1;
      }

      // update point type

      if (demzip_dll->header.point_data_format == 6)
      {
        demzip_dll->header.point_data_format = 1;
      }
      else if (demzip_dll->header.point_data_format <= 8)
      {
        demzip_dll->header.point_data_format = 3;
      }
      else // 9->4 and 10->5
      {
        demzip_dll->header.point_data_format -= 5;
      }

      // we are operating in compatibility mode
      demzip_dll->compatibility_mode = TRUE;
    }
    else if (compress)
    {
      sprintf(demzip_dll->error, "LASzip DLL %d.%d r%d (%d) cannot compress point data format %d without requesting 'compatibility mode'", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE, (I32)demzip_dll->header.point_data_format);
      return 1;
    }
  }
  else
  {
    // must *not* be set for the old point type 5 or lower

    demzip_dll->point.extended_point_type = 0;
    
    // we are *not* operating in compatibility mode

    demzip_dll->compatibility_mode = FALSE;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
demzip_prepare_vlrs_for_write(
    demzip_dll_struct*                 demzip_dll
)
{
  U32 i, vlrs_size = 0;

  if (demzip_dll->header.number_of_variable_length_records)
  {
    if (demzip_dll->header.vlrs == 0)
    {
      sprintf(demzip_dll->error, "number_of_variable_length_records is %u but vlrs pointer is zero", demzip_dll->header.number_of_variable_length_records);
      return 1;
    }

    for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
    {
      vlrs_size += 54;
      if (demzip_dll->header.vlrs[i].record_length_after_header)
      {
        if (demzip_dll->header.vlrs == 0)
        {
          sprintf(demzip_dll->error, "vlrs[%u].record_length_after_header is %u but vlrs[%u].data pointer is zero", i, demzip_dll->header.vlrs[i].record_length_after_header, i);
          return 1;
        }
        vlrs_size += demzip_dll->header.vlrs[i].record_length_after_header;
      }
    }
  }

  if ((vlrs_size + demzip_dll->header.header_size + demzip_dll->header.user_data_after_header_size) != demzip_dll->header.offset_to_point_data)
  {
    sprintf(demzip_dll->error,"header_size (%u) plus vlrs_size (%u) plus user_data_after_header_size (%u) does not equal offset_to_point_data (%u)", (U32)demzip_dll->header.header_size, vlrs_size, demzip_dll->header.user_data_after_header_size, demzip_dll->header.offset_to_point_data);
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
static U32
demzip_vrl_payload_size(
    const LASzip*                      laszip
)
{
  return 34 + (6 * laszip->num_items);
}

/*---------------------------------------------------------------------------*/
static I32
write_demzip_vlr_header(
    demzip_dll_struct*                 demzip_dll
    , const LASzip*                    laszip
    , ByteStreamOut*                   out
)
{
  // write the LASzip VLR header

  U16 reserved = 0x0;
  try { out->put16bitsLE((U8*)&reserved); } catch(...)
  {
    sprintf(demzip_dll->error, "writing LASzip VLR header.reserved");
    return 1;
  }
  U8 user_id[16] = "laszip encoded\0";
  try { out->putBytes((U8*)user_id, 16); } catch(...)
  {
    sprintf(demzip_dll->error, "writing LASzip VLR header.user_id");
    return 1;
  }
  U16 record_id = 22204;
  try { out->put16bitsLE((U8*)&record_id); } catch(...)
  {
    sprintf(demzip_dll->error, "writing LASzip VLR header.record_id");
    return 1;
  }
  U16 record_length_after_header = (U16)demzip_vrl_payload_size(laszip);
  try { out->put16bitsLE((U8*)&record_length_after_header); } catch(...)
  {
    sprintf(demzip_dll->error, "writing LASzip VLR header.record_length_after_header");
    return 1;
  }
  CHAR description[32];
  memset(description, 0, 32);
  sprintf(description, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
  try { out->putBytes((U8*)description, 32); } catch(...)
  {
    sprintf(demzip_dll->error, "writing LASzip VLR header.description");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
write_demzip_vlr_payload(
    demzip_dll_struct*                 demzip_dll
    , const LASzip*                    laszip
    , ByteStreamOut*                   out
)
{
  // write the LASzip VLR payload

  //     U16  compressor                2 bytes
  //     U32  coder                     2 bytes
  //     U8   version_major             1 byte
  //     U8   version_minor             1 byte
  //     U16  version_revision          2 bytes
  //     U32  options                   4 bytes
  //     I32  chunk_size                4 bytes
  //     I64  number_of_special_evlrs   8 bytes
  //     I64  offset_to_special_evlrs   8 bytes
  //     U16  num_items                 2 bytes
  //        U16 type                2 bytes * num_items
  //        U16 size                2 bytes * num_items
  //        U16 version             2 bytes * num_items
  // which totals 34+6*num_items

  try { out->put16bitsLE((const U8*)&(laszip->compressor)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing compressor %d", (I32)laszip->compressor);
    return 1;
  }
  try { out->put16bitsLE((const U8*)&(laszip->coder)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing coder %d", (I32)laszip->coder);
    return 1;
  }
  try { out->putBytes((const U8*)&(laszip->version_major), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "writing version_major %d", (I32)laszip->version_major);
    return 1;
  }
  try { out->putBytes((const U8*)&(laszip->version_minor), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "writing version_minor %d", (I32)laszip->version_minor);
    return 1;
  }
  try { out->put16bitsLE((const U8*)&(laszip->version_revision)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing version_revision %d", (I32)laszip->version_revision);
    return 1;
  }
  try { out->put32bitsLE((const U8*)&(laszip->options)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing options %u", laszip->options);
    return 1;
  }
  try { out->put32bitsLE((const U8*)&(laszip->chunk_size)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing chunk_size %u", laszip->chunk_size);
    return 1;
  }
  try { out->put64bitsLE((const U8*)&(laszip->number_of_special_evlrs)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing number_of_special_evlrs %d", (I32)laszip->number_of_special_evlrs);
    return 1;
  }
  try { out->put64bitsLE((const U8*)&(laszip->offset_to_special_evlrs)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing offset_to_special_evlrs %d", (I32)laszip->offset_to_special_evlrs);
    return 1;
  }
  try { out->put16bitsLE((const U8*)&(laszip->num_items)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing num_items %d", (I32)laszip->num_items);
    return 1;
  }

  U32 j;
  for (j = 0; j < laszip->num_items; j++)
  {
    U16 type = (U16)(laszip->items[j].type);
    try { out->put16bitsLE((const U8*)&type); } catch(...)
    {
      sprintf(demzip_dll->error, "writing type %d of item %d", (I32)laszip->items[j].type, j);
      return 1;
    }
    try { out->put16bitsLE((const U8*)&(laszip->items[j].size)); } catch(...)
    {
      sprintf(demzip_dll->error, "writing size %d of item %d", (I32)laszip->items[j].size, j);
      return 1;
    }
    try { out->put16bitsLE((const U8*)&(laszip->items[j].version)); } catch(...)
    {
      sprintf(demzip_dll->error, "writing version %d of item %d", (I32)laszip->items[j].version, j);
      return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
demzip_write_header(
    demzip_dll_struct*                 demzip_dll
    , const LASzip*                    laszip
    , const demzip_BOOL                compress
)
{
  U32 i;

  try { demzip_dll->streamout->putBytes((const U8*)"LASF", 4); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.file_signature");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.file_source_ID)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.file_source_ID");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.global_encoding)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.global_encoding");
    return 1;
  }
  try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.project_ID_GUID_data_1)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.project_ID_GUID_data_1");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.project_ID_GUID_data_2)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.project_ID_GUID_data_2");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.project_ID_GUID_data_3)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.project_ID_GUID_data_3");
    return 1;
  }
  try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.project_ID_GUID_data_4, 8); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.project_ID_GUID_data_4");
    return 1;
  }
  try { demzip_dll->streamout->putBytes((const U8*)&(demzip_dll->header.version_major), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.version_major");
    return 1;
  }
  try { demzip_dll->streamout->putBytes((const U8*)&(demzip_dll->header.version_minor), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.version_minor");
    return 1;
  }
  try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.system_identifier, 32); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.system_identifier");
    return 1;
  }
  if (!demzip_dll->preserve_generating_software)
  {
    memset(demzip_dll->header.generating_software, 0, 32);
    sprintf(demzip_dll->header.generating_software, "LASzip DLL %d.%d r%d (%d)", LASZIP_VERSION_MAJOR, LASZIP_VERSION_MINOR, LASZIP_VERSION_REVISION, LASZIP_VERSION_BUILD_DATE);
  }
  try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.generating_software, 32); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.generating_software");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.file_creation_day)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.file_creation_day");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.file_creation_year)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.file_creation_year");
    return 1;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.header_size)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.header_size");
    return 1;
  }
  if (compress)
  {
    demzip_dll->header.offset_to_point_data += (54 + demzip_vrl_payload_size(laszip));
  }
  try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.offset_to_point_data)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.offset_to_point_data");
    return 1;
  }
  if (compress)
  {
    demzip_dll->header.offset_to_point_data -= (54 + demzip_vrl_payload_size(laszip));
    demzip_dll->header.number_of_variable_length_records += 1;
  }
  try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.number_of_variable_length_records)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.number_of_variable_length_records");
    return 1;
  }
  if (compress)
  {
    demzip_dll->header.number_of_variable_length_records -= 1;
    demzip_dll->header.point_data_format |= 128;
  }
  try { demzip_dll->streamout->putBytes((const U8*)&(demzip_dll->header.point_data_format), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.point_data_format");
    return 1;
  }
  if (compress)
  {
    demzip_dll->header.point_data_format &= 127;
  }
  try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.point_data_record_length)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.point_data_record_length");
    return 1;
  }
  try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.number_of_point_records)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.number_of_point_records");
    return 1;
  }
  for (i = 0; i < 5; i++)
  {
    try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.number_of_points_by_return[i])); } catch(...)
    {
      sprintf(demzip_dll->error, "writing header.number_of_points_by_return %d", i);
      return 1;
    }
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.x_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.x_scale_factor");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.y_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.y_scale_factor");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.z_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.z_scale_factor");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.x_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.x_offset");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.y_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.y_offset");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.z_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.z_offset");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.max_x)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.max_x");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.min_x)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.min_x");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.max_y)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.max_y");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.min_y)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.min_y");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.max_z)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.max_z");
    return 1;
  }
  try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.min_z)); } catch(...)
  {
    sprintf(demzip_dll->error, "writing header.min_z");
    return 1;
  }

  // special handling for LAS 1.3
  if ((demzip_dll->header.version_major == 1) && (demzip_dll->header.version_minor >= 3))
  {
    if (demzip_dll->header.header_size < 235)
    {
      sprintf(demzip_dll->error, "for LAS 1.%d header_size should at least be 235 but it is only %d", demzip_dll->header.version_minor, demzip_dll->header.header_size);
      return 1;
    }
    else
    {
      if (demzip_dll->header.start_of_waveform_data_packet_record != 0)
      {
#ifdef _WIN32
        sprintf(demzip_dll->warning, "header.start_of_waveform_data_packet_record is %I64d. writing 0 instead.", demzip_dll->header.start_of_waveform_data_packet_record);
#else
        sprintf(demzip_dll->warning, "header.start_of_waveform_data_packet_record is %llu. writing 0 instead.", demzip_dll->header.start_of_waveform_data_packet_record);
#endif
        demzip_dll->header.start_of_waveform_data_packet_record = 0;
      }
      try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.start_of_waveform_data_packet_record)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.start_of_waveform_data_packet_record");
        return 1;
      }
      demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 235;
    }
  }
  else
  {
    demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 227;
  }

  // special handling for LAS 1.4
  if ((demzip_dll->header.version_major == 1) && (demzip_dll->header.version_minor >= 4))
  {
    if (demzip_dll->header.header_size < 375)
    {
      sprintf(demzip_dll->error, "for LAS 1.%d header_size should at least be 375 but it is only %d", demzip_dll->header.version_minor, demzip_dll->header.header_size);
      return 1;
    }
    else
    {
      try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.start_of_first_extended_variable_length_record)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.start_of_first_extended_variable_length_record");
        return 1;
      }
      try { demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->header.number_of_extended_variable_length_records)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.number_of_extended_variable_length_records");
        return 1;
      }
      try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.extended_number_of_point_records)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.extended_number_of_point_records");
        return 1;
      }
      for (i = 0; i < 15; i++)
      {
        try { demzip_dll->streamout->put64bitsLE((const U8*)&(demzip_dll->header.extended_number_of_points_by_return[i])); } catch(...)
        {
          sprintf(demzip_dll->error, "writing header.extended_number_of_points_by_return[%d]", i);
          return 1;
        }
      }
      demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 375;
    }
  }

  // write any number of user-defined bytes that might have been added to the header
  if (demzip_dll->header.user_data_in_header_size)
  {
    try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.user_data_in_header, demzip_dll->header.user_data_in_header_size); } catch(...)
    {
      sprintf(demzip_dll->error, "writing %d bytes of data into header.user_data_in_header", demzip_dll->header.user_data_in_header_size);
      return 1;
    }
  }

  // write variable length records into the header

  if (demzip_dll->header.number_of_variable_length_records)
  {
    U32 i;

    for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
    {
      // write variable length records variable after variable (to avoid alignment issues)

      try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.vlrs[i].reserved)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.vlrs[%d].reserved", i);
        return 1;
      }

      try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.vlrs[i].user_id, 16); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.vlrs[%d].user_id", i);
        return 1;
      }
      try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.vlrs[i].record_id)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.vlrs[%d].record_id", i);
        return 1;
      }
      try { demzip_dll->streamout->put16bitsLE((const U8*)&(demzip_dll->header.vlrs[i].record_length_after_header)); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.vlrs[%d].record_length_after_header", i);
        return 1;
      }
      try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.vlrs[i].description, 32); } catch(...)
      {
        sprintf(demzip_dll->error, "writing header.vlrs[%d].description", i);
        return 1;
      }

      // write data following the header of the variable length record

      if (demzip_dll->header.vlrs[i].record_length_after_header)
      {
        try { demzip_dll->streamout->putBytes(demzip_dll->header.vlrs[i].data, demzip_dll->header.vlrs[i].record_length_after_header); } catch(...)
        {
          sprintf(demzip_dll->error, "writing %d bytes of data into header.vlrs[%d].data", demzip_dll->header.vlrs[i].record_length_after_header, i);
          return 1;
        }
      }
    }
  }

  if (compress)
  {
    // write the LASzip VLR header

    if (write_demzip_vlr_header(demzip_dll, laszip, demzip_dll->streamout))
    {
      return 1;
    }

    // write the LASzip VLR payload

    if (write_demzip_vlr_payload(demzip_dll, laszip, demzip_dll->streamout))
    {
      return 1;
    }
  }

  // write any number of user-defined bytes that might have been added after the header

  if (demzip_dll->header.user_data_after_header_size)
  {
    try { demzip_dll->streamout->putBytes((const U8*)demzip_dll->header.user_data_after_header, demzip_dll->header.user_data_after_header_size); } catch(...)
    {
      sprintf(demzip_dll->error, "writing %u bytes of data into header.user_data_after_header", demzip_dll->header.user_data_after_header_size);
      return 1;
    }
  }

  return 0;
}

/*----------------------------------------------------------------------------*/
demzip_I32 create_point_writer
(
    demzip_dll_struct *demzip_dll
    , const LASzip *laszip
)
{
  // create the point writer
  demzip_dll->writer = new LASwritePoint();
  if (demzip_dll->writer == 0)
  {
    sprintf(demzip_dll->error, "could not alloc LASwritePoint");
    return 1;
  }

  if (!demzip_dll->writer->setup(laszip->num_items, laszip->items, laszip))
  {
    sprintf(demzip_dll->error, "setup of LASwritePoint failed");
    return 1;
  }

  if (!demzip_dll->writer->init(demzip_dll->streamout))
  {
    sprintf(demzip_dll->error, "init of LASwritePoint failed");
    return 1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
setup_demzip_items(
    demzip_dll_struct*                 demzip_dll
    , LASzip*                          laszip
    , demzip_BOOL                      compress
)
{
  demzip_U8 point_type = demzip_dll->header.point_data_format;
  demzip_U16 point_size = demzip_dll->header.point_data_record_length;

  if ((point_type > 5) && demzip_dll->request_compatibility_mode)
  {
    if (!laszip->request_compatibility_mode(1))
    {
      sprintf(demzip_dll->error, "requesting 'compatibility mode' has failed");
      return 1;
    }
  }

  // create point items in the LASzip structure from point format and size

  if (!laszip->setup(point_type, point_size, LASZIP_COMPRESSOR_NONE))
  {
    sprintf(demzip_dll->error, "invalid combination of point_type %d and point_size %d", (I32)point_type, (I32)point_size);
    return 1;
  }

  // compute offsets (or points item pointers) for data transfer from the point items

  if (demzip_dll->point_items)
  {
    delete [] demzip_dll->point_items;
  }

  demzip_dll->point_items = new U8*[laszip->num_items];

  if (demzip_dll->point_items == 0)
  {
    sprintf(demzip_dll->error, "could not alloc point_items");
    return 1;
  }

  for (size_t i = 0; i < laszip->num_items; i++)
  {
    switch (laszip->items[i].type)
    {
    case LASitem::POINT10:
    case LASitem::POINT14:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.X);
      break;
    case LASitem::GPSTIME11:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.gps_time);
      break;
    case LASitem::RGB12:
    case LASitem::RGB14:
    case LASitem::RGBNIR14:
      demzip_dll->point_items[i] = (U8*)demzip_dll->point.rgb;
      break;
    case LASitem::BYTE:
    case LASitem::BYTE14:
      demzip_dll->point.num_extra_bytes = laszip->items[i].size;
      if (demzip_dll->point.extra_bytes) delete [] demzip_dll->point.extra_bytes;
      demzip_dll->point.extra_bytes = new U8[demzip_dll->point.num_extra_bytes];
      demzip_dll->point_items[i] = demzip_dll->point.extra_bytes;
      break;
    case LASitem::WAVEPACKET13:
    case LASitem::WAVEPACKET14:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.wave_packet);
      break;
    default:
      sprintf(demzip_dll->error, "unknown LASitem type %d", (I32)laszip->items[i].type);
      return 1;
    }
  }

  if (compress)
  {
    if ((point_type > 5) && demzip_dll->request_native_extension)
    {
      if (!laszip->setup(point_type, point_size, LASZIP_COMPRESSOR_LAYERED_CHUNKED))
      {
        sprintf(demzip_dll->error, "cannot compress point_type %d with point_size %d using native", (I32)point_type, (I32)point_size);
        return 1;
      }
    }
    else
    {
      if (!laszip->setup(point_type, point_size, LASZIP_COMPRESSOR_DEFAULT))
      {
        sprintf(demzip_dll->error, "cannot compress point_type %d with point_size %d", (I32)point_type, (I32)point_size);
        return 1;
      }
    }

    // request version (old point types only, new point types always use version 3)

    laszip->request_version(2);

    // maybe we should change the chunk size

    if (demzip_dll->set_chunk_size != LASZIP_CHUNK_SIZE_DEFAULT)
    {
      if (!laszip->set_chunk_size(demzip_dll->set_chunk_size))
      {
        sprintf(demzip_dll->error, "setting chunk size %d has failed", demzip_dll->set_chunk_size);
        return 1;
      }
    }
  }
  else
  {
    laszip->request_version(0);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_writer(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               file_name
    , demzip_BOOL                      compress
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (file_name == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'file_name' is zero");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    // open the file

    demzip_dll->file = fopen(file_name, "wb");

    if (demzip_dll->file == 0)
    {
      sprintf(demzip_dll->error, "cannot open file '%s'", file_name);
      return 1;
    }

    if (setvbuf(demzip_dll->file, NULL, _IOFBF, 262144) != 0)
    {
      sprintf(demzip_dll->warning, "setvbuf() failed with buffer size 262144\n");
    }

    // create the outstream

    if (IS_LITTLE_ENDIAN())
      demzip_dll->streamout = new ByteStreamOutFileLE(demzip_dll->file);
    else
      demzip_dll->streamout = new ByteStreamOutFileBE(demzip_dll->file);

    if (demzip_dll->streamout == 0)
    {
      sprintf(demzip_dll->error, "could not alloc ByteStreamOutFile");
      return 1;
    }

    // setup the items that make up the point

    LASzip laszip;
    if (setup_demzip_items(demzip_dll, &laszip, compress))
    {
      return 1;
    }

    // prepare header

    if (demzip_prepare_header_for_write(demzip_dll))
    {
      return 1;
    }

    // prepare point

    if (demzip_prepare_point_for_write(demzip_dll, compress))
    {
      return 1;
    }

    // prepare VLRs

    if (demzip_prepare_vlrs_for_write(demzip_dll))
    {
      return 1;
    }

    // write header variable after variable

    if (demzip_write_header(demzip_dll, &laszip, compress))
    {
      return 1;
    }

    // create the point writer

    if (create_point_writer(demzip_dll, &laszip))
    {
      return 1;
    }

    if (demzip_dll->lax_create)
    {
	    // create spatial indexing information using cell_size = 100.0f and threshold = 1000

	    LASquadtree* lasquadtree = new LASquadtree;
	    lasquadtree->setup(demzip_dll->header.min_x, demzip_dll->header.max_x, demzip_dll->header.min_y, demzip_dll->header.max_y, 100.0f);

	    demzip_dll->lax_index = new LASindex;
	    demzip_dll->lax_index->prepare(lasquadtree, 1000);

      // copy the file name for later

      demzip_dll->lax_file_name = LASCopyString(file_name);
    }

    // set the point number and point count

    demzip_dll->npoints = (demzip_dll->header.number_of_point_records ? demzip_dll->header.number_of_point_records : demzip_dll->header.extended_number_of_point_records);
    demzip_dll->p_count = 0;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_open_writer '%s'", file_name);
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_write_point(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    // temporary fix to avoid corrupt LAZ files

    if (demzip_dll->point.extended_point_type)
    {
      // make sure legacy flags and extended flags are identical
      if ((demzip_dll->point.extended_classification_flags & 0x7) != ((((U8*)&(demzip_dll->point.intensity))[3]) >> 5))
      {
        sprintf(demzip_dll->error, "legacy flags and extended flags are not identical");
        return 1;
      }

      // make sure legacy classification is zero or identical to extended classification
      if (demzip_dll->point.classification != 0)
      {
        if (demzip_dll->point.classification != demzip_dll->point.extended_classification)
        {
          sprintf(demzip_dll->error, "legacy classification %d and extended classification %d are not consistent", demzip_dll->point.classification, demzip_dll->point.extended_classification);
          return 1;
        }
      }
    }

    // special recoding of points (in compatibility mode only)

    if (demzip_dll->compatibility_mode)
    {
      I32 scan_angle_remainder;
      I32 number_of_returns_increment;
      I32 return_number_increment;
      I32 return_count_difference;
      I32 overlap_bit;
      I32 scanner_channel;

      // distill extended attributes
      struct demzip_point* point = &demzip_dll->point;

      point->scan_angle_rank = I8_CLAMP(I16_QUANTIZE(0.006f*point->extended_scan_angle));
      scan_angle_remainder = point->extended_scan_angle - I16_QUANTIZE(((F32)point->scan_angle_rank)/0.006f);
      if (point->extended_number_of_returns <= 7)
      {
        point->number_of_returns = point->extended_number_of_returns;
        if (point->extended_return_number <= 7)
        {
          point->return_number = point->extended_return_number;
        }
        else
        {
          point->return_number = 7;
        }
      }
      else
      {
        point->number_of_returns = 7;
        if (point->extended_return_number <= 4)
        {
          point->return_number = point->extended_return_number;
        }
        else
        {
          return_count_difference = point->extended_number_of_returns - point->extended_return_number;
          if (return_count_difference <= 0)
          {
            point->return_number = 7;
          }
          else if (return_count_difference >= 3)
          {
            point->return_number = 4;
          }
          else
          {
            point->return_number = 7 - return_count_difference;
          }
        }
      }
      return_number_increment = point->extended_return_number - point->return_number;
      number_of_returns_increment = point->extended_number_of_returns - point->number_of_returns;
      if (point->extended_classification > 31)
      {
        point->classification = 0;
      }
      else
      {
        point->extended_classification = 0;
      }
      scanner_channel = point->extended_scanner_channel;
      overlap_bit = (point->extended_classification_flags >> 3);

      // write distilled extended attributes into extra bytes

      *((I16*)(point->extra_bytes + demzip_dll->start_scan_angle)) = ((I16)scan_angle_remainder);
      point->extra_bytes[demzip_dll->start_extended_returns] = (U8)((return_number_increment << 4) | number_of_returns_increment);
      point->extra_bytes[demzip_dll->start_classification] = (U8)(point->extended_classification);
      point->extra_bytes[demzip_dll->start_flags_and_channel] = (U8)((scanner_channel << 1) | overlap_bit);
      if (demzip_dll->start_NIR_band != -1)
      {
        *((U16*)(point->extra_bytes + demzip_dll->start_NIR_band)) = point->rgb[3];
      }
    }

    // write the point
    if (!demzip_dll->writer->write(demzip_dll->point_items))
    {
#ifdef _WIN32
      sprintf(demzip_dll->error, "writing point %I64d of %I64d total points", demzip_dll->p_count, demzip_dll->npoints);
#else
      sprintf(demzip_dll->error, "writing point %lld of %lld total points", demzip_dll->p_count, demzip_dll->npoints);
#endif
      return 1;
    }

    demzip_dll->p_count++;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_write_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_write_indexed_point(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    // write the point
    if (!demzip_dll->writer->write(demzip_dll->point_items))
    {
#ifdef _WIN32
      sprintf(demzip_dll->error, "writing point %I64d of %I64d total points", demzip_dll->p_count, demzip_dll->npoints);
#else
      sprintf(demzip_dll->error, "writing point %lld of %lld total points", demzip_dll->p_count, demzip_dll->npoints);
#endif
      return 1;
    }
    // index the point
    F64 x = demzip_dll->header.x_scale_factor*demzip_dll->point.X+demzip_dll->header.x_offset;
    F64 y = demzip_dll->header.y_scale_factor*demzip_dll->point.Y+demzip_dll->header.y_offset;
    demzip_dll->lax_index->add(x, y, (U32)demzip_dll->p_count);
    demzip_dll->p_count++;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_write_indexed_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_update_inventory(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->inventory == 0)
    {
      demzip_dll->inventory = new demzip_dll_inventory;
    }

    demzip_dll->inventory->add(&demzip_dll->point);
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_update_inventory");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_close_writer(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->writer == 0)
    {
      sprintf(demzip_dll->error, "closing writer before it was opened");
      return 1;
    }

    if (!demzip_dll->writer->done())
    {
      sprintf(demzip_dll->error, "done of LASwritePoint failed");
      return 1;
    }

    delete demzip_dll->writer;
    demzip_dll->writer = 0;

    delete [] demzip_dll->point_items;
    demzip_dll->point_items = 0;

    // maybe update the header

    if (demzip_dll->inventory)
    {
      if (demzip_dll->header.point_data_format <= 5) // only update legacy counters for old point types
      {
        demzip_dll->streamout->seek(107);
        if (!demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->inventory->number_of_point_records)))
        {
          sprintf(demzip_dll->error, "updating demzip_dll->inventory->number_of_point_records");
          return 1;
        }
        for (I32 i = 0; i < 5; i++)
        {
          if (!demzip_dll->streamout->put32bitsLE((const U8*)&(demzip_dll->inventory->number_of_points_by_return[i+1])))
          {
            sprintf(demzip_dll->error, "updating demzip_dll->inventory->number_of_points_by_return[%d]\n", i);
            return 1;
          }
        }
      }
      demzip_dll->streamout->seek(179);
      F64 value;
      value = demzip_dll->header.x_scale_factor*demzip_dll->inventory->max_X+demzip_dll->header.x_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->max_X");
        return 1;
      }
      value = demzip_dll->header.x_scale_factor*demzip_dll->inventory->min_X+demzip_dll->header.x_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->min_X");
        return 1;
      }
      value = demzip_dll->header.y_scale_factor*demzip_dll->inventory->max_Y+demzip_dll->header.y_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->max_Y");
        return 1;
      }
      value = demzip_dll->header.y_scale_factor*demzip_dll->inventory->min_Y+demzip_dll->header.y_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->min_Y");
        return 1;
      }
      value = demzip_dll->header.z_scale_factor*demzip_dll->inventory->max_Z+demzip_dll->header.z_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->max_Z");
        return 1;
      }
      value = demzip_dll->header.z_scale_factor*demzip_dll->inventory->min_Z+demzip_dll->header.z_offset;
      if (!demzip_dll->streamout->put64bitsLE((const U8*)&value))
      {
        sprintf(demzip_dll->error, "updating demzip_dll->inventory->min_Z");
        return 1;
      }
      if (demzip_dll->header.version_minor >= 4) // only update extended counters for LAS 1.4
      {
        demzip_dll->streamout->seek(247);
        I64 number = demzip_dll->inventory->number_of_point_records;
        if (!demzip_dll->streamout->put64bitsLE((const U8*)&number))
        {
          sprintf(demzip_dll->error, "updating demzip_dll->inventory->extended_number_of_point_records");
          return 1;
        }
        for (I32 i = 0; i < 15; i++)
        {
          number = demzip_dll->inventory->number_of_points_by_return[i+1];
          if (!demzip_dll->streamout->put64bitsLE((const U8*)&number))
          {
            sprintf(demzip_dll->error, "updating demzip_dll->inventory->extended_number_of_points_by_return[%d]\n", i);
            return 1;
          }
        }
      }
      demzip_dll->streamout->seekEnd();

      delete demzip_dll->inventory;
      demzip_dll->inventory = 0;
    }

    if (demzip_dll->lax_index)
    {
      demzip_dll->lax_index->complete(100000, -20, FALSE);

      if (!demzip_dll->lax_index->write(demzip_dll->lax_file_name))
      {
        sprintf(demzip_dll->error, "writing LAX file to '%s'", demzip_dll->lax_file_name);
        return 1;
      }

      free(demzip_dll->lax_file_name);
      demzip_dll->lax_file_name = 0;

      delete demzip_dll->lax_index;
      demzip_dll->lax_index = 0;
    }

    delete demzip_dll->streamout;
    demzip_dll->streamout = 0;

	  if (demzip_dll->file)
    {
      fclose(demzip_dll->file);
      demzip_dll->file = 0;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_writer_close");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_exploit_spatial_index(
    demzip_POINTER                     pointer
    , const demzip_BOOL                exploit
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->lax_exploit = exploit;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_exploit_spatial_index");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_decompress_selective(
    demzip_POINTER                     pointer
    , const demzip_U32                 decompress_selective
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    demzip_dll->las14_decompress_selective = decompress_selective;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_decompress_selective");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
static I32
demzip_read_header(
    demzip_dll_struct*                 demzip_dll
    , demzip_BOOL*                     is_compressed
)
{
  U32 i;

  // read the header variable after variable

  CHAR file_signature[5];
  try { demzip_dll->streamin->getBytes((U8*)file_signature, 4); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.file_signature");
    return 1;
  }
  if (strncmp(file_signature, "LASF", 4) != 0)
  {
    sprintf(demzip_dll->error, "wrong file_signature. not a LAS/LAZ file.");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.file_source_ID)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.file_source_ID");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.global_encoding)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.global_encoding");
    return 1;
  }
  try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.project_ID_GUID_data_1)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.project_ID_GUID_data_1");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.project_ID_GUID_data_2)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.project_ID_GUID_data_2");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.project_ID_GUID_data_3)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.project_ID_GUID_data_3");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.project_ID_GUID_data_4, 8); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.project_ID_GUID_data_4");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)&(demzip_dll->header.version_major), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.version_major");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)&(demzip_dll->header.version_minor), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.version_minor");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.system_identifier, 32); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.system_identifier");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.generating_software, 32); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.generating_software");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.file_creation_day)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.file_creation_day");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.file_creation_year)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.file_creation_year");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.header_size)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.header_size");
    return 1;
  }
  try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.offset_to_point_data)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.offset_to_point_data");
    return 1;
  }
  try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.number_of_variable_length_records)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.number_of_variable_length_records");
    return 1;
  }
  try { demzip_dll->streamin->getBytes((U8*)&(demzip_dll->header.point_data_format), 1); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.point_data_format");
    return 1;
  }
  try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.point_data_record_length)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.point_data_record_length");
    return 1;
  }
  try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.number_of_point_records)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.number_of_point_records");
    return 1;
  }
  for (i = 0; i < 5; i++)
  {
    try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.number_of_points_by_return[i])); } catch(...)
    {
      sprintf(demzip_dll->error, "reading header.number_of_points_by_return %d", i);
      return 1;
    }
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.x_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.x_scale_factor");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.y_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.y_scale_factor");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.z_scale_factor)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.z_scale_factor");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.x_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.x_offset");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.y_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.y_offset");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.z_offset)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.z_offset");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.max_x)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.max_x");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.min_x)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.min_x");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.max_y)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.max_y");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.min_y)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.min_y");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.max_z)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.max_z");
    return 1;
  }
  try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.min_z)); } catch(...)
  {
    sprintf(demzip_dll->error, "reading header.min_z");
    return 1;
  }

  // special handling for LAS 1.3
  if ((demzip_dll->header.version_major == 1) && (demzip_dll->header.version_minor >= 3))
  {
    if (demzip_dll->header.header_size < 235)
    {
      sprintf(demzip_dll->error, "for LAS 1.%d header_size should at least be 235 but it is only %d", demzip_dll->header.version_minor, demzip_dll->header.header_size);
      return 1;
    }
    else
    {
      try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.start_of_waveform_data_packet_record)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.start_of_waveform_data_packet_record");
        return 1;
      }
      demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 235;
    }
  }
  else
  {
    demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 227;
  }

  // special handling for LAS 1.4
  if ((demzip_dll->header.version_major == 1) && (demzip_dll->header.version_minor >= 4))
  {
    if (demzip_dll->header.header_size < 375)
    {
      sprintf(demzip_dll->error, "for LAS 1.%d header_size should at least be 375 but it is only %d", demzip_dll->header.version_minor, demzip_dll->header.header_size);
      return 1;
    }
    else
    {
      try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.start_of_first_extended_variable_length_record)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.start_of_first_extended_variable_length_record");
        return 1;
      }
      try { demzip_dll->streamin->get32bitsLE((U8*)&(demzip_dll->header.number_of_extended_variable_length_records)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.number_of_extended_variable_length_records");
        return 1;
      }
      try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.extended_number_of_point_records)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.extended_number_of_point_records");
        return 1;
      }
      for (i = 0; i < 15; i++)
      {
        try { demzip_dll->streamin->get64bitsLE((U8*)&(demzip_dll->header.extended_number_of_points_by_return[i])); } catch(...)
        {
          sprintf(demzip_dll->error, "reading header.extended_number_of_points_by_return[%d]", i);
          return 1;
        }
      }
      demzip_dll->header.user_data_in_header_size = demzip_dll->header.header_size - 375;
    }
  }

  // load any number of user-defined bytes that might have been added to the header
  if (demzip_dll->header.user_data_in_header_size)
  {
    if (demzip_dll->header.user_data_in_header)
    {
      delete [] demzip_dll->header.user_data_in_header;
    }
    demzip_dll->header.user_data_in_header = new U8[demzip_dll->header.user_data_in_header_size];

    try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.user_data_in_header, demzip_dll->header.user_data_in_header_size); } catch(...)
    {
      sprintf(demzip_dll->error, "reading %u bytes of data into header.user_data_in_header", demzip_dll->header.user_data_in_header_size);
      return 1;
    }
  }

  // read variable length records into the header

  U32 vlrs_size = 0;
  LASzip* laszip = 0;

  if (demzip_dll->header.number_of_variable_length_records)
  {
    U32 i;

    demzip_dll->header.vlrs = (demzip_vlr*)malloc(sizeof(demzip_vlr)*demzip_dll->header.number_of_variable_length_records);

    if (demzip_dll->header.vlrs == 0)
    {
      sprintf(demzip_dll->error, "allocating %u VLRs", demzip_dll->header.number_of_variable_length_records);
      return 1;
    }

    for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
    {
      // make sure there are enough bytes left to read a variable length record before the point block starts

      if (((int)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size) < 54)
      {
        sprintf(demzip_dll->warning, "only %d bytes until point block after reading %d of %d vlrs. skipping remaining vlrs ...", (int)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size, i, demzip_dll->header.number_of_variable_length_records);
        demzip_dll->header.number_of_variable_length_records = i;
        break;
      }

      // read variable length records variable after variable (to avoid alignment issues)

      try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.vlrs[i].reserved)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.vlrs[%u].reserved", i);
        return 1;
      }

      try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.vlrs[i].user_id, 16); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.vlrs[%u].user_id", i);
        return 1;
      }
      try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.vlrs[i].record_id)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.vlrs[%u].record_id", i);
        return 1;
      }
      try { demzip_dll->streamin->get16bitsLE((U8*)&(demzip_dll->header.vlrs[i].record_length_after_header)); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.vlrs[%u].record_length_after_header", i);
        return 1;
      }
      try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.vlrs[i].description, 32); } catch(...)
      {
        sprintf(demzip_dll->error, "reading header.vlrs[%u].description", i);
        return 1;
      }

      // keep track on the number of bytes we have read so far

      vlrs_size += 54;

      // check variable length record contents

      if ((demzip_dll->header.vlrs[i].reserved != 0xAABB) && (demzip_dll->header.vlrs[i].reserved != 0x0))
      {
        sprintf(demzip_dll->warning,"wrong header.vlrs[%d].reserved: %d != 0xAABB and %d != 0x0", i, demzip_dll->header.vlrs[i].reserved, demzip_dll->header.vlrs[i].reserved);
      }

      // make sure there are enough bytes left to read the data of the variable length record before the point block starts

      if (((int)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size) < demzip_dll->header.vlrs[i].record_length_after_header)
      {
        sprintf(demzip_dll->warning, "only %d bytes until point block when trying to read %d bytes into header.vlrs[%d].data", (int)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size, demzip_dll->header.vlrs[i].record_length_after_header, i);
        demzip_dll->header.vlrs[i].record_length_after_header = (int)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size;
      }

      // load data following the header of the variable length record

      if (demzip_dll->header.vlrs[i].record_length_after_header)
      {
        if ((strcmp(demzip_dll->header.vlrs[i].user_id, "laszip encoded") == 0) && (demzip_dll->header.vlrs[i].record_id == 22204))
        {
          if (laszip)
          {
            delete laszip;
          }

          laszip = new LASzip();

          if (laszip == 0)
          {
            sprintf(demzip_dll->error, "could not alloc LASzip");
            return 1;
          }

          // read the LASzip VLR payload

          //     U16  compressor                2 bytes
          //     U32  coder                     2 bytes
          //     U8   version_major             1 byte
          //     U8   version_minor             1 byte
          //     U16  version_revision          2 bytes
          //     U32  options                   4 bytes
          //     I32  chunk_size                4 bytes
          //     I64  number_of_special_evlrs   8 bytes
          //     I64  offset_to_special_evlrs   8 bytes
          //     U16  num_items                 2 bytes
          //        U16 type                2 bytes * num_items
          //        U16 size                2 bytes * num_items
          //        U16 version             2 bytes * num_items
          // which totals 34+6*num_items

          try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->compressor)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading compressor %d", (I32)laszip->compressor);
            return 1;
          }
          try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->coder)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading coder %d", (I32)laszip->coder);
            return 1;
          }
          try { demzip_dll->streamin->getBytes((U8*)&(laszip->version_major), 1); } catch(...)
          {
            sprintf(demzip_dll->error, "reading version_major %d", (I32)laszip->version_major);
            return 1;
          }
          try { demzip_dll->streamin->getBytes((U8*)&(laszip->version_minor), 1); } catch(...)
          {
            sprintf(demzip_dll->error, "reading version_minor %d", (I32)laszip->version_minor);
            return 1;
          }
          try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->version_revision)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading version_revision %d", (I32)laszip->version_revision);
            return 1;
          }
          try { demzip_dll->streamin->get32bitsLE((U8*)&(laszip->options)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading options %u", laszip->options);
            return 1;
          }
          try { demzip_dll->streamin->get32bitsLE((U8*)&(laszip->chunk_size)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading chunk_size %u", laszip->chunk_size);
            return 1;
          }
          try { demzip_dll->streamin->get64bitsLE((U8*)&(laszip->number_of_special_evlrs)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading number_of_special_evlrs %d", (I32)laszip->number_of_special_evlrs);
            return 1;
          }
          try { demzip_dll->streamin->get64bitsLE((U8*)&(laszip->offset_to_special_evlrs)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading offset_to_special_evlrs %d", (I32)laszip->offset_to_special_evlrs);
            return 1;
          }
          try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->num_items)); } catch(...)
          {
            sprintf(demzip_dll->error, "reading num_items %d", (I32)laszip->num_items);
            return 1;
          }
          laszip->items = new LASitem[laszip->num_items];
          U32 j;
          for (j = 0; j < laszip->num_items; j++)
          {
            U16 type;
            try { demzip_dll->streamin->get16bitsLE((U8*)&type); } catch(...)
            {
              sprintf(demzip_dll->error, "reading type of item %u", j);
              return 1;
            }
            laszip->items[j].type = (LASitem::Type)type;
            try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->items[j].size)); } catch(...)
            {
              sprintf(demzip_dll->error, "reading size of item %u", j);
              return 1;
            }
            try { demzip_dll->streamin->get16bitsLE((U8*)&(laszip->items[j].version)); } catch(...)
            {
              sprintf(demzip_dll->error, "reading version of item %u", j);
              return 1;
            }
          }
        }
        else
        {
          demzip_dll->header.vlrs[i].data = new U8[demzip_dll->header.vlrs[i].record_length_after_header];

          try { demzip_dll->streamin->getBytes(demzip_dll->header.vlrs[i].data, demzip_dll->header.vlrs[i].record_length_after_header); } catch(...)
          {
            sprintf(demzip_dll->error, "reading %d bytes of data into header.vlrs[%u].data", (I32)demzip_dll->header.vlrs[i].record_length_after_header, i);
            return 1;
          }
        }
      }
      else
      {
        demzip_dll->header.vlrs[i].data = 0;
      }

      // keep track on the number of bytes we have read so far

      vlrs_size += demzip_dll->header.vlrs[i].record_length_after_header;

      // special handling for LASzip VLR

      if ((strcmp(demzip_dll->header.vlrs[i].user_id, "laszip encoded") == 0) && (demzip_dll->header.vlrs[i].record_id == 22204))
      {
        // we take our the VLR for LASzip away
        demzip_dll->header.offset_to_point_data -= (54+demzip_dll->header.vlrs[i].record_length_after_header);
        vlrs_size -= (54+demzip_dll->header.vlrs[i].record_length_after_header);
        i--;
        demzip_dll->header.number_of_variable_length_records--;
        // free or resize the VLR array
        if (demzip_dll->header.number_of_variable_length_records == 0)
        {
          free(demzip_dll->header.vlrs);
          demzip_dll->header.vlrs = 0;
        }
        else
        {
          demzip_dll->header.vlrs = (demzip_vlr*)realloc(demzip_dll->header.vlrs, sizeof(demzip_vlr)*demzip_dll->header.number_of_variable_length_records);
        }
      }
    }
  }

  // load any number of user-defined bytes that might have been added after the header

  demzip_dll->header.user_data_after_header_size = (I32)demzip_dll->header.offset_to_point_data - vlrs_size - demzip_dll->header.header_size;
  if (demzip_dll->header.user_data_after_header_size)
  {
    if (demzip_dll->header.user_data_after_header)
    {
      delete [] demzip_dll->header.user_data_after_header;
    }
    demzip_dll->header.user_data_after_header = new U8[demzip_dll->header.user_data_after_header_size];

    try { demzip_dll->streamin->getBytes((U8*)demzip_dll->header.user_data_after_header, demzip_dll->header.user_data_after_header_size); } catch(...)
    {
      sprintf(demzip_dll->error, "reading %u bytes of data into header.user_data_after_header", demzip_dll->header.user_data_after_header_size);
      return 1;
    }
  }

  // remove extra bits in point data type

  if ((demzip_dll->header.point_data_format & 128) || (demzip_dll->header.point_data_format & 64))
  {
    if (!laszip)
    {
      sprintf(demzip_dll->error, "this file was compressed with an experimental version of LASzip. contact 'info@rapidlasso.de' for assistance");
      return 1;
    }
    demzip_dll->header.point_data_format &= 127;
  }

  // check if file is compressed

  if (laszip)
  {
    // yes. check the compressor state
    *is_compressed = 1;
    if (!laszip->check(demzip_dll->header.point_data_record_length))
    {
      sprintf(demzip_dll->error, "%s upgrade to the latest release of LASzip or contact 'info@rapidlasso.de' for assistance", laszip->get_error());
      return 1;
    }
  }
  else
  {
    // no. setup an un-compressed read
    *is_compressed = 0;
    laszip = new LASzip;
    if (laszip == 0)
    {
      sprintf(demzip_dll->error, "could not alloc LASzip");
      return 1;
    }
    if (!laszip->setup(demzip_dll->header.point_data_format, demzip_dll->header.point_data_record_length, LASZIP_COMPRESSOR_NONE))
    {
      sprintf(demzip_dll->error, "invalid combination of point_data_format %d and point_data_record_length %d", (I32)demzip_dll->header.point_data_format, (I32)demzip_dll->header.point_data_record_length);
      return 1;
    }
  }

  // create point's item pointers
  
  if (demzip_dll->point_items)
  {
    delete [] demzip_dll->point_items;
  }

  demzip_dll->point_items = new U8*[laszip->num_items];

  if (demzip_dll->point_items == 0)
  {
    sprintf(demzip_dll->error, "could not alloc point_items");
    return 1;
  }

  for (i = 0; i < laszip->num_items; i++)
  {
    switch (laszip->items[i].type)
    {
    case LASitem::POINT10:
    case LASitem::POINT14:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.X);
      break;
    case LASitem::GPSTIME11:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.gps_time);
      break;
    case LASitem::RGB12:
    case LASitem::RGB14:
    case LASitem::RGBNIR14:
      demzip_dll->point_items[i] = (U8*)demzip_dll->point.rgb;
      break;
    case LASitem::BYTE:
    case LASitem::BYTE14:
      demzip_dll->point.num_extra_bytes = laszip->items[i].size;
      if (demzip_dll->point.extra_bytes) delete [] demzip_dll->point.extra_bytes;
      demzip_dll->point.extra_bytes = new U8[demzip_dll->point.num_extra_bytes];
      demzip_dll->point_items[i] = demzip_dll->point.extra_bytes;
      break;
    case LASitem::WAVEPACKET13:
    case LASitem::WAVEPACKET14:
      demzip_dll->point_items[i] = (U8*)&(demzip_dll->point.wave_packet);
      break;
    default:
      sprintf(demzip_dll->error, "unknown LASitem type %d", (I32)laszip->items[i].type);
      return 1;
    }
  }

  // did the user request to recode the compatibility mode points?

  demzip_dll->compatibility_mode = FALSE;

  if (demzip_dll->request_compatibility_mode && (demzip_dll->header.version_minor < 4))
  {
    // does this file contain compatibility mode recoded LAS 1.4 content

    struct demzip_vlr* compatibility_VLR = 0;

    if (demzip_dll->header.point_data_format == 1 || demzip_dll->header.point_data_format == 3 || demzip_dll->header.point_data_format == 4 || demzip_dll->header.point_data_format == 5)
    {
      // if we find the compatibility VLR

      for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
      {
        if ((strncmp(demzip_dll->header.vlrs[i].user_id, "lascompatible\0\0", 16) == 0) && (demzip_dll->header.vlrs[i].record_id == 22204))
        {
          if (demzip_dll->header.vlrs[i].record_length_after_header == 2+2+4+148)
          {
            compatibility_VLR = &(demzip_dll->header.vlrs[i]);
            break;
          }
        }
      }

      if (compatibility_VLR)
      {
        // and we also find the extra bytes VLR with the right attributes

        LASattributer attributer;
        for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
        {
          if ((strncmp(demzip_dll->header.vlrs[i].user_id, "LASF_Spec\0\0\0\0\0\0", 16) == 0) && (demzip_dll->header.vlrs[i].record_id == 4))
          {
            attributer.init_attributes(demzip_dll->header.vlrs[i].record_length_after_header/192, (LASattribute*)demzip_dll->header.vlrs[i].data);
            demzip_dll->start_scan_angle = attributer.get_attribute_start("LAS 1.4 scan angle");
            demzip_dll->start_extended_returns = attributer.get_attribute_start("LAS 1.4 extended returns");
            demzip_dll->start_classification = attributer.get_attribute_start("LAS 1.4 classification");
            demzip_dll->start_flags_and_channel = attributer.get_attribute_start("LAS 1.4 flags and channel");
            demzip_dll->start_NIR_band = attributer.get_attribute_start("LAS 1.4 NIR band");
            break;
          }
        }

        // can we do it ... ?

        if ((demzip_dll->start_scan_angle != -1) && (demzip_dll->start_extended_returns != -1) && (demzip_dll->start_classification != -1) && (demzip_dll->start_flags_and_channel != -1))
        {
          // yes ... so let's fix the header (using the content from the compatibility VLR)

          ByteStreamInArray* in;
          if (IS_LITTLE_ENDIAN())
            in = new ByteStreamInArrayLE(compatibility_VLR->data, compatibility_VLR->record_length_after_header);
          else
            in = new ByteStreamInArrayBE(compatibility_VLR->data, compatibility_VLR->record_length_after_header);
          // read control info
          U16 demzip_version;
          in->get16bitsLE((U8*)&demzip_version);
          U16 compatible_version;
          in->get16bitsLE((U8*)&compatible_version);
          U32 unused;
          in->get32bitsLE((U8*)&unused);
          // read the 148 bytes of the extended LAS 1.4 header
          U64 start_of_waveform_data_packet_record;
          in->get64bitsLE((U8*)&start_of_waveform_data_packet_record);
          if (start_of_waveform_data_packet_record != 0)
          {
#ifdef _WIN32
            fprintf(stderr,"WARNING: start_of_waveform_data_packet_record is %I64d. reading 0 instead.\n", start_of_waveform_data_packet_record);
#else
            fprintf(stderr,"WARNING: start_of_waveform_data_packet_record is %llu. reading 0 instead.\n", start_of_waveform_data_packet_record);
#endif
          }
          demzip_dll->header.start_of_waveform_data_packet_record = 0;
          U64 start_of_first_extended_variable_length_record;
          in->get64bitsLE((U8*)&start_of_first_extended_variable_length_record);
          if (start_of_first_extended_variable_length_record != 0)
          {
#ifdef _WIN32
            fprintf(stderr,"WARNING: EVLRs not supported. start_of_first_extended_variable_length_record is %I64d. reading 0 instead.\n", start_of_first_extended_variable_length_record);
#else
            fprintf(stderr,"WARNING: EVLRs not supported. start_of_first_extended_variable_length_record is %llu. reading 0 instead.\n", start_of_first_extended_variable_length_record);
#endif
          }
          demzip_dll->header.start_of_first_extended_variable_length_record = 0;
          U32 number_of_extended_variable_length_records ;
          in->get32bitsLE((U8*)&number_of_extended_variable_length_records);
          if (number_of_extended_variable_length_records != 0)
          {
            fprintf(stderr,"WARNING: EVLRs not supported. number_of_extended_variable_length_records is %u. reading 0 instead.\n", number_of_extended_variable_length_records);
          }
          demzip_dll->header.number_of_extended_variable_length_records = 0;
          U64 extended_number_of_point_records = 0;
          in->get64bitsLE((U8*)&extended_number_of_point_records);
          if (demzip_dll->header.number_of_point_records != 0 && ((U64)(demzip_dll->header.number_of_point_records)) != extended_number_of_point_records)
          {
#ifdef _WIN32
            fprintf(stderr,"WARNING: number_of_point_records is %u. but extended_number_of_point_records is %I64u.\n", demzip_dll->header.number_of_point_records, extended_number_of_point_records);
#else
            fprintf(stderr,"WARNING: number_of_point_records is %u. but extended_number_of_point_records is %llu.\n", demzip_dll->header.number_of_point_records, extended_number_of_point_records);
#endif
          }
          demzip_dll->header.extended_number_of_point_records = extended_number_of_point_records;
          U64 extended_number_of_points_by_return;
          for (U32 r = 0; r < 15; r++)
          {
            in->get64bitsLE((U8*)&extended_number_of_points_by_return);
            if ((r < 5) && demzip_dll->header.number_of_points_by_return[r] != 0 && ((U64)(demzip_dll->header.number_of_points_by_return[r])) != extended_number_of_points_by_return)
            {
#ifdef _WIN32
              fprintf(stderr,"WARNING: number_of_points_by_return[%u] is %u. but extended_number_of_points_by_return[%u] is %I64u.\n", r, demzip_dll->header.number_of_points_by_return[r], r, extended_number_of_points_by_return);
#else
              fprintf(stderr,"WARNING: number_of_points_by_return[%u] is %u. but extended_number_of_points_by_return[%u] is %llu.\n", r, demzip_dll->header.number_of_points_by_return[r], r, extended_number_of_points_by_return);
#endif
            }
            demzip_dll->header.extended_number_of_points_by_return[r] = extended_number_of_points_by_return;
          }
          delete in;

          // remove the compatibility VLR

          if (demzip_remove_vlr(demzip_dll, "lascompatible\0\0", 22204))
          {
            sprintf(demzip_dll->error, "removing the compatibility VLR");
            return 1;
          }

          // remove the LAS 1.4 attributes from the "extra bytes" description

          if (demzip_dll->start_NIR_band != -1) attributer.remove_attribute("LAS 1.4 NIR band");
          attributer.remove_attribute("LAS 1.4 flags and channel");
          attributer.remove_attribute("LAS 1.4 classification");
          attributer.remove_attribute("LAS 1.4 extended returns");
          attributer.remove_attribute("LAS 1.4 scan angle");

          // either rewrite or remove the "extra bytes" VLR

          if (attributer.number_attributes)
          {
            if (demzip_add_vlr(demzip_dll, "LASF_Spec\0\0\0\0\0\0", 4, (demzip_U16)(attributer.number_attributes*sizeof(LASattribute)), 0, (demzip_U8*)attributer.attributes))
            {
              sprintf(demzip_dll->error, "rewriting the extra bytes VLR without 'LAS 1.4 compatibility mode' attributes");
              return 1;
            }
          }
          else
          {
            if (demzip_remove_vlr(demzip_dll, "LASF_Spec\0\0\0\0\0\0", 4))
            {
              sprintf(demzip_dll->error, "removing the LAS 1.4 attribute VLR");
              return 1;
            }
          }

          // upgrade to LAS 1.4
          if (demzip_dll->header.version_minor < 3)
          {
            // LAS 1.2 header is 148 bytes less than LAS 1.4+ header
            demzip_dll->header.header_size += 148;
            demzip_dll->header.offset_to_point_data += 148;
          }
          else
          {
            // LAS 1.3 header is 140 bytes less than LAS 1.4+ header
            demzip_dll->header.header_size += 140;
            demzip_dll->header.offset_to_point_data += 140;
          }
          demzip_dll->header.version_minor = 4;

          // maybe turn on the bit indicating the presence of the OGC WKT
          for (i = 0; i < demzip_dll->header.number_of_variable_length_records; i++)
          {
            if ((strncmp(demzip_dll->header.vlrs[i].user_id, "LASF_Projection", 16) == 0) && (demzip_dll->header.vlrs[i].record_id == 2112))
            {
              demzip_dll->header.global_encoding |= (1<<4);
              break;
            }
          }

          // update point type and size

          demzip_dll->point.extended_point_type = 1;

          if (demzip_dll->header.point_data_format == 1)
          {
            demzip_dll->header.point_data_format = 6;
            demzip_dll->header.point_data_record_length += (2 - 5); // record is 2 bytes larger but minus 5 extra bytes
          }
          else if (demzip_dll->header.point_data_format == 3)
          {
            if (demzip_dll->start_NIR_band == -1)
            {
              demzip_dll->header.point_data_format = 7;
              demzip_dll->header.point_data_record_length += (2 - 5); // record is 2 bytes larger but minus 5 extra bytes
            }
            else
            {
              demzip_dll->header.point_data_format = 8;
              demzip_dll->header.point_data_record_length += (4 - 7); // record is 4 bytes larger but minus 7 extra bytes
            }
          }
          else
          {
            if (demzip_dll->start_NIR_band == -1)
            {
              demzip_dll->header.point_data_format = 9;
              demzip_dll->header.point_data_record_length += (2 - 5);
            }
            else
            {
              demzip_dll->header.point_data_format = 10;
              demzip_dll->header.point_data_record_length += (4 - 7);
            }
          }

          // we are operating in compatibility mode

          demzip_dll->compatibility_mode = TRUE;
        }
      }
    }
  }
  else if (demzip_dll->header.point_data_format > 5)
  {
    demzip_dll->point.extended_point_type = 1;
  }

  // create the point reader

  demzip_dll->reader = new LASreadPoint(demzip_dll->las14_decompress_selective);
  if (demzip_dll->reader == 0)
  {
    sprintf(demzip_dll->error, "could not alloc LASreadPoint");
    return 1;
  }

  if (!demzip_dll->reader->setup(laszip->num_items, laszip->items, laszip))
  {
    sprintf(demzip_dll->error, "setup of LASreadPoint failed");
    return 1;
  }

  if (!demzip_dll->reader->init(demzip_dll->streamin))
  {
    sprintf(demzip_dll->error, "init of LASreadPoint failed");
    return 1;
  }

  delete laszip;

  // set the point number and point count

  demzip_dll->npoints = (demzip_dll->header.number_of_point_records ? demzip_dll->header.number_of_point_records : demzip_dll->header.extended_number_of_point_records);
  demzip_dll->p_count = 0;

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_reader(
    demzip_POINTER                     pointer
    , const demzip_CHAR*               file_name
    , demzip_BOOL*                     is_compressed
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (file_name == 0)
    {
      sprintf(demzip_dll->error, "demzip_CHAR pointer 'file_name' is zero");
      return 1;
    }

    if (is_compressed == 0)
    {
      sprintf(demzip_dll->error, "demzip_BOOL pointer 'is_compressed' is zero");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    // open the file

    demzip_dll->file = fopen(file_name, "rb");

    if (demzip_dll->file == 0)
    {
      sprintf(demzip_dll->error, "cannot open file '%s'", file_name);
      return 1;
    }

    if (setvbuf(demzip_dll->file, NULL, _IOFBF, 262144) != 0)
    {
      sprintf(demzip_dll->warning, "setvbuf() failed with buffer size 262144\n");
    }

    if (IS_LITTLE_ENDIAN())
      demzip_dll->streamin = new ByteStreamInFileLE(demzip_dll->file);
    else
      demzip_dll->streamin = new ByteStreamInFileBE(demzip_dll->file);

    if (demzip_dll->streamin == 0)
    {
      sprintf(demzip_dll->error, "could not alloc ByteStreamInFile");
      return 1;
    }

    // read the header variable after variable

    if (demzip_read_header(demzip_dll, is_compressed))
    {
      return 1;
    }

    // should we try to exploit existing spatial indexing information

    if (demzip_dll->lax_exploit)
    {
      demzip_dll->lax_index = new LASindex();

      if (!demzip_dll->lax_index->read(file_name))
      {
        delete demzip_dll->lax_index;
        demzip_dll->lax_index = 0;
      }
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_open_reader");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_has_spatial_index(
    demzip_POINTER                     pointer
    , demzip_BOOL*                     is_indexed
    , demzip_BOOL*                     is_appended
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (is_indexed == 0)
    {
      sprintf(demzip_dll->error, "demzip_BOOL pointer 'is_indexed' is zero");
      return 1;
    }

    if (demzip_dll->reader == 0)
    {
      sprintf(demzip_dll->error, "reader is not open");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    if (demzip_dll->lax_exploit == 0)
    {
      sprintf(demzip_dll->error, "exploiting of spatial indexing not enabled before opening reader");
      return 1;
    }

    // check if reader found spatial indexing information when opening file

    if (demzip_dll->lax_index)
    {
      *is_indexed = 1;
    }
    else
    {
      *is_indexed = 0;
    }

    // optional: inform whether spatial index is appended to LAZ file or in separate LAX file

    if (is_appended)
    {
      *is_appended = 0;
    }

  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_have_spatial_index");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_inside_rectangle(
    demzip_POINTER                     pointer
    , const demzip_F64                 r_min_x
    , const demzip_F64                 r_min_y
    , const demzip_F64                 r_max_x
    , const demzip_F64                 r_max_y
    , demzip_BOOL*                     is_empty
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->reader == 0)
    {
      sprintf(demzip_dll->error, "reader is not open");
      return 1;
    }

    if (is_empty == 0)
    {
      sprintf(demzip_dll->error, "demzip_BOOL pointer 'is_empty' is zero");
      return 1;
    }

    if (demzip_dll->lax_exploit == FALSE)
    {
      sprintf(demzip_dll->error, "exploiting of spatial indexing not enabled before opening reader");
      return 1;
    }

    demzip_dll->lax_r_min_x = r_min_x;
    demzip_dll->lax_r_min_y = r_min_y;
    demzip_dll->lax_r_max_x = r_max_x;
    demzip_dll->lax_r_max_y = r_max_y;

    if (demzip_dll->lax_index)
    {
      if (demzip_dll->lax_index->intersect_rectangle(r_min_x, r_min_y, r_max_x, r_max_y))
      {
        *is_empty = 0;
      }
      else
      {
        // no overlap between spatial indexing cells and query reactangle
        *is_empty = 1;
      }
    }
    else
    {
      if ((demzip_dll->header.min_x > r_max_x) || (demzip_dll->header.min_y > r_max_y) || (demzip_dll->header.max_x < r_min_x) || (demzip_dll->header.max_y < r_min_y))
      {
        // no overlap between header bouding box and query reactangle
        *is_empty = 1;
      }
      else
      {
        *is_empty = 0;
      }
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_inside_rectangle");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_seek_point(
    demzip_POINTER                     pointer
    , demzip_I64                       index
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    // seek to the point
    if (!demzip_dll->reader->seek((U32)demzip_dll->p_count, (U32)index))
    {
#ifdef _WIN32
      sprintf(demzip_dll->error, "seeking from index %I64d to index %I64d for file with %I64d points", demzip_dll->p_count, index, demzip_dll->npoints);
#else
      sprintf(demzip_dll->error, "seeking from index %lld to index %lld for file with %lld points", demzip_dll->p_count, index, demzip_dll->npoints);
#endif
      return 1;
    }
    demzip_dll->p_count = index;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_seek_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_read_point(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    // read the point
    if (!demzip_dll->reader->read(demzip_dll->point_items))
    {
#ifdef _WIN32
      sprintf(demzip_dll->error, "reading point %I64d of %I64d total points", demzip_dll->p_count, demzip_dll->npoints);
#else
      sprintf(demzip_dll->error, "reading point %lld of %lld total points", demzip_dll->p_count, demzip_dll->npoints);
#endif
      return 1;
    }

    // special recoding of points (in compatibility mode only)

    if (demzip_dll->compatibility_mode)
    {
      I16 scan_angle_remainder;
      U8 extended_returns;
      U8 classification;
      U8 flags_and_channel;
      I32 return_number_increment;
      I32 number_of_returns_increment;
      I32 overlap_bit;
      I32 scanner_channel;

      // instill extended attributes
      struct demzip_point* point = &demzip_dll->point;

      // get extended attributes from extra bytes
      scan_angle_remainder = *((I16*)(point->extra_bytes + demzip_dll->start_scan_angle));
      extended_returns = point->extra_bytes[demzip_dll->start_extended_returns];
      classification = point->extra_bytes[demzip_dll->start_classification];
      flags_and_channel = point->extra_bytes[demzip_dll->start_flags_and_channel];
      if (demzip_dll->start_NIR_band != -1)
      {
        point->rgb[3] = *((U16*)(point->extra_bytes + demzip_dll->start_NIR_band));
      }

      // decompose into individual attributes
      return_number_increment = (extended_returns >> 4) & 0x0F;
      number_of_returns_increment = extended_returns & 0x0F;
      scanner_channel = (flags_and_channel >> 1) & 0x03;
      overlap_bit = flags_and_channel & 0x01;

      // instill into point
      point->extended_scan_angle = scan_angle_remainder + I16_QUANTIZE(((F32)point->scan_angle_rank) / 0.006f);
      point->extended_return_number = return_number_increment + point->return_number;
      point->extended_number_of_returns = number_of_returns_increment + point->number_of_returns;
      point->extended_classification = classification + point->classification;
      point->extended_scanner_channel = scanner_channel;
      point->extended_classification_flags = (overlap_bit << 3) | ((point->withheld_flag) << 2) | ((point->keypoint_flag) << 1) | (point->synthetic_flag);
    }

    demzip_dll->p_count++;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_read_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_read_inside_point(
    demzip_POINTER                     pointer
    , demzip_BOOL*                     is_done
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    demzip_F64 xy;

    *is_done = 1;

    if (demzip_dll->lax_index)
    {
      while (demzip_dll->lax_index->seek_next(demzip_dll->reader, demzip_dll->p_count))
      {
        if (demzip_dll->reader->read(demzip_dll->point_items))
        {
          demzip_dll->p_count++;
          xy = demzip_dll->header.x_scale_factor*demzip_dll->point.X+demzip_dll->header.x_offset;
          if (xy < demzip_dll->lax_r_min_x || xy >= demzip_dll->lax_r_max_x) continue;
          xy = demzip_dll->header.y_scale_factor*demzip_dll->point.Y+demzip_dll->header.y_offset;
          if (xy < demzip_dll->lax_r_min_y || xy >= demzip_dll->lax_r_max_y) continue;
          *is_done = 0;
          break;
        }
      }
    }
    else
    {
      while (demzip_dll->reader->read(demzip_dll->point_items))
      {
        demzip_dll->p_count++;
        xy = demzip_dll->header.x_scale_factor*demzip_dll->point.X+demzip_dll->header.x_offset;
        if (xy < demzip_dll->lax_r_min_x || xy >= demzip_dll->lax_r_max_x) continue;
        xy = demzip_dll->header.y_scale_factor*demzip_dll->point.Y+demzip_dll->header.y_offset;
        if (xy < demzip_dll->lax_r_min_y || xy >= demzip_dll->lax_r_max_y) continue;
        *is_done = 0;
        break;
      }

      if (*is_done)
      {
        if (demzip_dll->p_count < demzip_dll->npoints)
        {
#ifdef _WIN32
          sprintf(demzip_dll->error, "reading point %I64d of %I64d total points", demzip_dll->p_count, demzip_dll->npoints);
#else
          sprintf(demzip_dll->error, "reading point %lld of %lld total points", demzip_dll->p_count, demzip_dll->npoints);
#endif
          return 1;
        }
      }
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_read_inside_point");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_close_reader(
    demzip_POINTER                     pointer
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;
  try
  {
    if (demzip_dll->reader == 0)
    {
      sprintf(demzip_dll->error, "closing reader before it was opened");
      return 1;
    }

    if (!demzip_dll->reader->done())
    {
      sprintf(demzip_dll->error, "done of LASreadPoint failed");
      return 1;
    }

    delete demzip_dll->reader;
    demzip_dll->reader = 0;

    delete [] demzip_dll->point_items;
    demzip_dll->point_items = 0;

    delete demzip_dll->streamin;
    demzip_dll->streamin = 0;

    if (demzip_dll->lax_index)
    {
      delete demzip_dll->lax_index;
      demzip_dll->lax_index = 0;
    }

  	if (demzip_dll->file)
    {
      fclose(demzip_dll->file);
      demzip_dll->file = 0;
    }
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_close_reader");
    return 1;
  }

  demzip_dll->error[0] = '\0';
  return 0;
}

#ifdef __cplusplus

/*---------------------------------------------------------------------------*/
DEMZIP_API demzip_I32
demzip_open_reader_stream(
    demzip_POINTER                     pointer
    , istream&                         stream
    , demzip_BOOL*                     is_compressed
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (is_compressed == 0)
    {
      sprintf(demzip_dll->error, "demzip_BOOL pointer 'is_compressed' is zero");
      return 1;
    }

    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    // open the file

    if (IS_LITTLE_ENDIAN())
      demzip_dll->streamin = new ByteStreamInIstreamLE(stream);
    else
      demzip_dll->streamin = new ByteStreamInIstreamBE(stream);

    if (demzip_dll->streamin == 0)
    {
      sprintf(demzip_dll->error, "could not alloc ByteStreamInIstream");
      return 1;
    }

    return demzip_read_header(demzip_dll, is_compressed);
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_open_reader");
    return 1;
  }
}

/*---------------------------------------------------------------------------*/
// The stream writer also supports software that writes the LAS header on its
// own simply by setting the BOOL 'do_not_write_header' to TRUE. This function
// should then be called just prior to writing points as data is then written
// to the current stream position
DEMZIP_API demzip_I32
demzip_open_writer_stream(
    demzip_POINTER                     pointer
    , ostream&                         stream
    , demzip_BOOL                      compress
    , demzip_BOOL                      do_not_write_header
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct* demzip_dll = (demzip_dll_struct*)pointer;

  try
  {
    if (demzip_dll->writer)
    {
      sprintf(demzip_dll->error, "writer is already open");
      return 1;
    }

    if (demzip_dll->reader)
    {
      sprintf(demzip_dll->error, "reader is already open");
      return 1;
    }

    // create the outstream

    if (IS_LITTLE_ENDIAN())
      demzip_dll->streamout = new ByteStreamOutOstreamLE(stream);
    else
      demzip_dll->streamout = new ByteStreamOutOstreamBE(stream);

    if (demzip_dll->streamout == 0)
    {
      sprintf(demzip_dll->error, "could not alloc ByteStreamOutOstream");
      return 1;
    }

    // setup the items that make up the point

    LASzip laszip;
    if (setup_demzip_items(demzip_dll, &laszip, compress))
    {
      return 1;
    }

    // this supports software that writes the LAS header on its own

    if (do_not_write_header == FALSE)
    {
      // prepare header

      if (demzip_prepare_header_for_write(demzip_dll))
      {
        return 1;
      }

      // prepare point

      if (demzip_prepare_point_for_write(demzip_dll, compress))
      {
        return 1;
      }

      // prepare VLRs

      if (demzip_prepare_vlrs_for_write(demzip_dll))
      {
        return 1;
      }

      // write header variable after variable

      if (demzip_write_header(demzip_dll, &laszip, compress))
      {
        return 1;
      }
    }

    // create the point writer

    if (create_point_writer(demzip_dll, &laszip))
    {
      return 1;
    }

    // set the point number and point count

    demzip_dll->npoints = (demzip_dll->header.number_of_point_records ? demzip_dll->header.number_of_point_records : demzip_dll->header.extended_number_of_point_records);
    demzip_dll->p_count = 0;
  }
  catch (...)
  {
    sprintf(demzip_dll->error, "internal error in demzip_open_writer_stream.");
    return 1;
  }
  demzip_dll->error[0] = '\0';
  return 0;
}

/*---------------------------------------------------------------------------*/
// creates complete LASzip VLR for currently selected point type and compression
// The VLR data is valid until the demzip_dll pointer is destroyed.
DEMZIP_API demzip_I32
demzip_create_demzip_vlr(
    demzip_POINTER                     pointer
    , demzip_U8**                      vlr
    , demzip_U32*                      vlr_size
)
{
  if (pointer == 0) return 1;
  demzip_dll_struct *demzip_dll = (demzip_dll_struct *)pointer;

  LASzip laszip;
  if (setup_demzip_items(demzip_dll, &laszip, TRUE))
  {
    return 1;
  }

  ByteStreamOutArray* out = 0;

  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutArrayLE();
  else
    out = new ByteStreamOutArrayBE();

  if (out == 0)
  {
    sprintf(demzip_dll->error, "could not alloc ByteStreamOutArray");
    return 1;
  }

  if (write_demzip_vlr_header(demzip_dll, &laszip, out))
  {
    return 1;
  }

  if (write_demzip_vlr_payload(demzip_dll, &laszip, out))
  {
    return 1;
  }

  *vlr = (demzip_U8*)malloc(out->getSize());
  *vlr_size = (U32)out->getSize();
  demzip_dll->buffers.push_back(*vlr);
  memcpy(*vlr, out->getData(), out->getSize());

  delete out;

  demzip_dll->error[0] = '\0';
  return 0;
}

#endif // __cplusplus
