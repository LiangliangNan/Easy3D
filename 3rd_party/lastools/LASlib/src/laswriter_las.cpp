/*
===============================================================================

  FILE:  laswriter_las.cpp
  
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
#include "laswriter_las.hpp"

#include "bytestreamout_nil.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamout_ostream.hpp"
#include "laswritepoint.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>

BOOL LASwriterLAS::refile(FILE* file)
{
  if (stream == 0) return FALSE;
  if (this->file) this->file = file;
  return ((ByteStreamOutFile*)stream)->refile(file);
}

BOOL LASwriterLAS::open(const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  ByteStreamOut* out = new ByteStreamOutNil();
  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(const char* file_name, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size, I32 io_buffer_size)
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
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", io_buffer_size);
  }

  ByteStreamOut* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutFileLE(file);
  else
    out = new ByteStreamOutFileBE(file);

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(FILE* file, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
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

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(ostream& stream, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  ByteStreamOut* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutOstreamLE(stream);
  else
    out = new ByteStreamOutOstreamBE(stream);

  return open(out, header, compressor, requested_version, chunk_size);
}

BOOL LASwriterLAS::open(ByteStreamOut* stream, const LASheader* header, U32 compressor, I32 requested_version, I32 chunk_size)
{
  U32 i, j;

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

  // check header contents

  if (!header->check()) return FALSE;

  // copy scale_and_offset
  quantizer.x_scale_factor = header->x_scale_factor;
  quantizer.y_scale_factor = header->y_scale_factor;
  quantizer.z_scale_factor = header->z_scale_factor;
  quantizer.x_offset = header->x_offset;
  quantizer.y_offset = header->y_offset;
  quantizer.z_offset = header->z_offset;

  // check if the requested point type is supported

  LASpoint point;
  U8 point_data_format;
  U16 point_data_record_length;
  BOOL point_is_standard = TRUE;

  if (header->laszip)
  {
    if (!point.init(&quantizer, header->laszip->num_items, header->laszip->items, header)) return FALSE;
    point_is_standard = header->laszip->is_standard(&point_data_format, &point_data_record_length);
  }
  else
  {
    if (!point.init(&quantizer, header->point_data_format, header->point_data_record_length, header)) return FALSE;
    point_data_format = header->point_data_format;
    point_data_record_length = header->point_data_record_length;
  }

  // fail if we don't use the layered compressor for the new LAS 1.4 point types
  
  if (compressor && (point_data_format > 5) && (compressor != LASZIP_COMPRESSOR_LAYERED_CHUNKED))
  {
    fprintf(stderr,"ERROR: point type %d requires using \"native LAS 1.4 extension\" of LASzip\n", point_data_format);
    return FALSE;
  }

  // do we need a LASzip VLR (because we compress or use non-standard points?) 

  LASzip* laszip = 0;
  U32 laszip_vlr_data_size = 0;
  if (compressor || point_is_standard == FALSE)
  {
    laszip = new LASzip();
    laszip->setup(point.num_items, point.items, compressor);
    if (chunk_size > -1) laszip->set_chunk_size((U32)chunk_size);
    if (compressor == LASZIP_COMPRESSOR_NONE) laszip->request_version(0);
    else if (chunk_size == 0 && (point_data_format <= 5)) { fprintf(stderr,"ERROR: adaptive chunking is depricated for point type %d.\n       only available for new LAS 1.4 point types 6 or higher.\n", point_data_format); return FALSE; }
    else if (requested_version) laszip->request_version(requested_version);
    else laszip->request_version(2);
    laszip_vlr_data_size = 34 + 6*laszip->num_items;
  }

  // create and setup the point writer

  writer = new LASwritePoint();
  if (laszip)
  {
    if (!writer->setup(laszip->num_items, laszip->items, laszip))
    {
      fprintf(stderr,"ERROR: point type %d of size %d not supported (with LASzip)\n", header->point_data_format, header->point_data_record_length);
      return FALSE;
    }
  }
  else
  {
    if (!writer->setup(point.num_items, point.items))
    {
      fprintf(stderr,"ERROR: point type %d of size %d not supported\n", header->point_data_format, header->point_data_record_length);
      return FALSE;
    }
  }

  // save the position where we start writing the header

  header_start_position = stream->tell();

  // write header variable after variable (to avoid alignment issues)

  if (!stream->putBytes((U8*)&(header->file_signature), 4))
  {
    fprintf(stderr,"ERROR: writing header->file_signature\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->file_source_ID)))
  {
    fprintf(stderr,"ERROR: writing header->file_source_ID\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->global_encoding)))
  {
    fprintf(stderr,"ERROR: writing header->global_encoding\n");
    return FALSE;
  }
  if (!stream->put32bitsLE((U8*)&(header->project_ID_GUID_data_1)))
  {
    fprintf(stderr,"ERROR: writing header->project_ID_GUID_data_1\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->project_ID_GUID_data_2)))
  {
    fprintf(stderr,"ERROR: writing header->project_ID_GUID_data_2\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->project_ID_GUID_data_3)))
  {
    fprintf(stderr,"ERROR: writing header->project_ID_GUID_data_3\n");
    return FALSE;
  }
  if (!stream->putBytes((U8*)header->project_ID_GUID_data_4, 8))
  {
    fprintf(stderr,"ERROR: writing header->project_ID_GUID_data_4\n");
    return FALSE;
  }
  // check version major
  U8 version_major = header->version_major;
  if (header->version_major != 1)
  {
    fprintf(stderr,"WARNING: header->version_major is %d. writing 1 instead.\n", header->version_major);
    version_major = 1;
  }
  if (!stream->putByte(version_major))
  {
    fprintf(stderr,"ERROR: writing header->version_major\n");
    return FALSE;
  }
  // check version minor
  U8 version_minor = header->version_minor;
  if (version_minor > 4)
  {
    fprintf(stderr,"WARNING: header->version_minor is %d. writing 4 instead.\n", version_minor);
    version_minor = 4;
  }
  if (!stream->putByte(version_minor))
  {
    fprintf(stderr,"ERROR: writing header->version_minor\n");
    return FALSE;
  }
  if (!stream->putBytes((U8*)header->system_identifier, 32))
  {
    fprintf(stderr,"ERROR: writing header->system_identifier\n");
    return FALSE;
  }
  if (!stream->putBytes((U8*)header->generating_software, 32))
  {
    fprintf(stderr,"ERROR: writing header->generating_software\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->file_creation_day)))
  {
    fprintf(stderr,"ERROR: writing header->file_creation_day\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->file_creation_year)))
  {
    fprintf(stderr,"ERROR: writing header->file_creation_year\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->header_size)))
  {
    fprintf(stderr,"ERROR: writing header->header_size\n");
    return FALSE;
  }
  U32 offset_to_point_data = header->offset_to_point_data;
  if (laszip) offset_to_point_data += (54 + laszip_vlr_data_size);
  if (header->vlr_lastiling) offset_to_point_data += (54 + 28);
  if (header->vlr_lasoriginal) offset_to_point_data += (54 + 176);
  if (!stream->put32bitsLE((U8*)&offset_to_point_data))
  {
    fprintf(stderr,"ERROR: writing header->offset_to_point_data\n");
    return FALSE;
  }
  U32 number_of_variable_length_records = header->number_of_variable_length_records;
  if (laszip) number_of_variable_length_records++;
  if (header->vlr_lastiling) number_of_variable_length_records++;
  if (header->vlr_lasoriginal) number_of_variable_length_records++;
  if (!stream->put32bitsLE((U8*)&(number_of_variable_length_records)))
  {
    fprintf(stderr,"ERROR: writing header->number_of_variable_length_records\n");
    return FALSE;
  }
  if (compressor) point_data_format |= 128;
  if (!stream->putByte(point_data_format))
  {
    fprintf(stderr,"ERROR: writing header->point_data_format\n");
    return FALSE;
  }
  if (!stream->put16bitsLE((U8*)&(header->point_data_record_length)))
  {
    fprintf(stderr,"ERROR: writing header->point_data_record_length\n");
    return FALSE;
  }
  if (!stream->put32bitsLE((U8*)&(header->number_of_point_records)))
  {
    fprintf(stderr,"ERROR: writing header->number_of_point_records\n");
    return FALSE;
  }
  for (i = 0; i < 5; i++)
  {
    if (!stream->put32bitsLE((U8*)&(header->number_of_points_by_return[i])))
    {
      fprintf(stderr,"ERROR: writing header->number_of_points_by_return[%d]\n", i);
      return FALSE;
    }
  }
  if (!stream->put64bitsLE((U8*)&(header->x_scale_factor)))
  {
    fprintf(stderr,"ERROR: writing header->x_scale_factor\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->y_scale_factor)))
  {
    fprintf(stderr,"ERROR: writing header->y_scale_factor\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->z_scale_factor)))
  {
    fprintf(stderr,"ERROR: writing header->z_scale_factor\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->x_offset)))
  {
    fprintf(stderr,"ERROR: writing header->x_offset\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->y_offset)))
  {
    fprintf(stderr,"ERROR: writing header->y_offset\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->z_offset)))
  {
    fprintf(stderr,"ERROR: writing header->z_offset\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->max_x)))
  {
    fprintf(stderr,"ERROR: writing header->max_x\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->min_x)))
  {
    fprintf(stderr,"ERROR: writing header->min_x\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->max_y)))
  {
    fprintf(stderr,"ERROR: writing header->max_y\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->min_y)))
  {
    fprintf(stderr,"ERROR: writing header->min_y\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->max_z)))
  {
    fprintf(stderr,"ERROR: writing header->max_z\n");
    return FALSE;
  }
  if (!stream->put64bitsLE((U8*)&(header->min_z)))
  {
    fprintf(stderr,"ERROR: writing header->min_z\n");
    return FALSE;
  }

  // special handling for LAS 1.3 or higher.
  if (version_minor >= 3)
  {
    U64 start_of_waveform_data_packet_record = header->start_of_waveform_data_packet_record;
    if (start_of_waveform_data_packet_record != 0)
    {
#ifdef _WIN32
      fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %I64d. writing 0 instead.\n", start_of_waveform_data_packet_record);
#else
      fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %lld. writing 0 instead.\n", start_of_waveform_data_packet_record);
#endif
      start_of_waveform_data_packet_record = 0;
    }
    if (!stream->put64bitsLE((U8*)&start_of_waveform_data_packet_record))
    {
      fprintf(stderr,"ERROR: writing start_of_waveform_data_packet_record\n");
      return FALSE;
    }
  }

  // special handling for LAS 1.4 or higher.
  if (version_minor >= 4)
  {
    writing_las_1_4 = TRUE;
    if (header->point_data_format >= 6)
    {
      writing_new_point_type = TRUE;
    }
    else
    {
      writing_new_point_type = FALSE;
    }
    start_of_first_extended_variable_length_record = header->start_of_first_extended_variable_length_record;
    if (!stream->put64bitsLE((U8*)&(start_of_first_extended_variable_length_record)))
    {
      fprintf(stderr,"ERROR: writing header->start_of_first_extended_variable_length_record\n");
      return FALSE;
    }
    number_of_extended_variable_length_records = header->number_of_extended_variable_length_records;
    if (!stream->put32bitsLE((U8*)&(number_of_extended_variable_length_records)))
    {
      fprintf(stderr,"ERROR: writing header->number_of_extended_variable_length_records\n");
      return FALSE;
    }
    evlrs = header->evlrs;
    U64 extended_number_of_point_records;
    if (header->number_of_point_records)
      extended_number_of_point_records = header->number_of_point_records;
    else
      extended_number_of_point_records = header->extended_number_of_point_records;
    if (!stream->put64bitsLE((U8*)&extended_number_of_point_records))
    {
      fprintf(stderr,"ERROR: writing header->extended_number_of_point_records\n");
      return FALSE;
    }
    U64 extended_number_of_points_by_return;
    for (i = 0; i < 15; i++)
    {
      if ((i < 5) && header->number_of_points_by_return[i])
        extended_number_of_points_by_return = header->number_of_points_by_return[i];
      else
        extended_number_of_points_by_return = header->extended_number_of_points_by_return[i];
      if (!stream->put64bitsLE((U8*)&extended_number_of_points_by_return))
      {
        fprintf(stderr,"ERROR: writing header->extended_number_of_points_by_return[%d]\n", i);
        return FALSE;
      }
    }
  }
  else
  {
    writing_las_1_4 = FALSE;
    writing_new_point_type = FALSE;
  }

  // write any number of user-defined bytes that might have been added into the header

  if (header->user_data_in_header_size)
  {
    if (header->user_data_in_header)
    {
      if (!stream->putBytes((U8*)header->user_data_in_header, header->user_data_in_header_size))
      {
        fprintf(stderr,"ERROR: writing %d bytes of data from header->user_data_in_header\n", header->user_data_in_header_size);
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr,"ERROR: there should be %d bytes of data in header->user_data_in_header\n", header->user_data_in_header_size);
      return FALSE;
    }
  }

  // write variable length records variable after variable (to avoid alignment issues)

  for (i = 0; i < header->number_of_variable_length_records; i++)
  {
    // check variable length records contents

    if (header->vlrs[i].reserved != 0xAABB)
    {
//      fprintf(stderr,"WARNING: wrong header->vlrs[%d].reserved: %d != 0xAABB\n", i, header->vlrs[i].reserved);
    }

    // write variable length records variable after variable (to avoid alignment issues)

    if (!stream->put16bitsLE((U8*)&(header->vlrs[i].reserved)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].reserved\n", i);
      return FALSE;
    }
    if (!stream->putBytes((U8*)header->vlrs[i].user_id, 16))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].user_id\n", i);
      return FALSE;
    }
    if (!stream->put16bitsLE((U8*)&(header->vlrs[i].record_id)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].record_id\n", i);
      return FALSE;
    }
    if (!stream->put16bitsLE((U8*)&(header->vlrs[i].record_length_after_header)))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].record_length_after_header\n", i);
      return FALSE;
    }
    if (!stream->putBytes((U8*)header->vlrs[i].description, 32))
    {
      fprintf(stderr,"ERROR: writing header->vlrs[%d].description\n", i);
      return FALSE;
    }

    // write the data following the header of the variable length record

    if (header->vlrs[i].record_length_after_header)
    {
      if (header->vlrs[i].data)
      {
        if (!stream->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          fprintf(stderr,"ERROR: writing %d bytes of data from header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }
      else
      {
        fprintf(stderr,"ERROR: there should be %d bytes of data in header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
        return FALSE;
      }
    }
  }

  // write laszip VLR with compression parameters

  if (laszip)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0xAABB;
    if (!stream->put16bitsLE((U8*)&(reserved)))
    {
      fprintf(stderr,"ERROR: writing reserved %d\n", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "laszip encoded\0";
    if (!stream->putBytes((U8*)user_id, 16))
    {
      fprintf(stderr,"ERROR: writing user_id %s\n", user_id);
      return FALSE;
    }
    U16 record_id = 22204;
    if (!stream->put16bitsLE((U8*)&(record_id)))
    {
      fprintf(stderr,"ERROR: writing record_id %d\n", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = laszip_vlr_data_size;
    if (!stream->put16bitsLE((U8*)&(record_length_after_header)))
    {
      fprintf(stderr,"ERROR: writing record_length_after_header %d\n", (I32)record_length_after_header);
      return FALSE;
    }
    char description[32];
    memset(description, 0, 32);
    sprintf(description, "by laszip of LAStools (%d)", LAS_TOOLS_VERSION);  
    if (!stream->putBytes((U8*)description, 32))
    {
      fprintf(stderr,"ERROR: writing description %s\n", description);
      return FALSE;
    }
    // write the data following the header of the variable length record
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

    if (!stream->put16bitsLE((U8*)&(laszip->compressor)))
    {
      fprintf(stderr,"ERROR: writing compressor %d\n", (I32)compressor);
      return FALSE;
    }
    if (!stream->put16bitsLE((U8*)&(laszip->coder)))
    {
      fprintf(stderr,"ERROR: writing coder %d\n", (I32)laszip->coder);
      return FALSE;
    }
    if (!stream->putByte(laszip->version_major))
    {
      fprintf(stderr,"ERROR: writing version_major %d\n", laszip->version_major);
      return FALSE;
    }
    if (!stream->putByte(laszip->version_minor))
    {
      fprintf(stderr,"ERROR: writing version_minor %d\n", laszip->version_minor);
      return FALSE;
    }
    if (!stream->put16bitsLE((U8*)&(laszip->version_revision)))
    {
      fprintf(stderr,"ERROR: writing version_revision %d\n", laszip->version_revision);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(laszip->options)))
    {
      fprintf(stderr,"ERROR: writing options %d\n", (I32)laszip->options);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(laszip->chunk_size)))
    {
      fprintf(stderr,"ERROR: writing chunk_size %d\n", laszip->chunk_size);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(laszip->number_of_special_evlrs)))
    {
      fprintf(stderr,"ERROR: writing number_of_special_evlrs %d\n", (I32)laszip->number_of_special_evlrs);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(laszip->offset_to_special_evlrs)))
    {
      fprintf(stderr,"ERROR: writing offset_to_special_evlrs %d\n", (I32)laszip->offset_to_special_evlrs);
      return FALSE;
    }
    if (!stream->put16bitsLE((U8*)&(laszip->num_items)))
    {
      fprintf(stderr,"ERROR: writing num_items %d\n", laszip->num_items);
      return FALSE;
    }
    for (i = 0; i < laszip->num_items; i++)
    {
      if (!stream->put16bitsLE((U8*)&(laszip->items[i].type)))
      {
        fprintf(stderr,"ERROR: writing type %d of item %d\n", laszip->items[i].type, i);
        return FALSE;
      }
      if (!stream->put16bitsLE((U8*)&(laszip->items[i].size)))
      {
        fprintf(stderr,"ERROR: writing size %d of item %d\n", laszip->items[i].size, i);
        return FALSE;
      }
      if (!stream->put16bitsLE((U8*)&(laszip->items[i].version)))
      {
        fprintf(stderr,"ERROR: writing version %d of item %d\n", laszip->items[i].version, i);
        return FALSE;
      }
    }

    delete laszip;
    laszip = 0;
  }

  // write lastiling VLR with the tile parameters

  if (header->vlr_lastiling)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0xAABB;
    if (!stream->put16bitsLE((U8*)&(reserved)))
    {
      fprintf(stderr,"ERROR: writing reserved %d\n", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "LAStools\0\0\0\0\0\0\0";
    if (!stream->putBytes((U8*)user_id, 16))
    {
      fprintf(stderr,"ERROR: writing user_id %s\n", user_id);
      return FALSE;
    }
    U16 record_id = 10;
    if (!stream->put16bitsLE((U8*)&(record_id)))
    {
      fprintf(stderr,"ERROR: writing record_id %d\n", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = 28;
    if (!stream->put16bitsLE((U8*)&(record_length_after_header)))
    {
      fprintf(stderr,"ERROR: writing record_length_after_header %d\n", (I32)record_length_after_header);
      return FALSE;
    }
    CHAR description[33] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    sprintf(description, "tile %s buffer %s", (header->vlr_lastiling->buffer ? "with" : "without"), (header->vlr_lastiling->reversible ? ", reversible" : ""));
    if (!stream->putBytes((U8*)description, 32))
    {
      fprintf(stderr,"ERROR: writing description %s\n", description);
      return FALSE;
    }

    // write the payload of this VLR which contains 28 bytes
    //   U32  level                                          4 bytes 
    //   U32  level_index                                    4 bytes 
    //   U32  implicit_levels + buffer bit + reversible bit  4 bytes 
    //   F32  min_x                                          4 bytes 
    //   F32  max_x                                          4 bytes 
    //   F32  min_y                                          4 bytes 
    //   F32  max_y                                          4 bytes 

    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->level)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->level %u\n", header->vlr_lastiling->level);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->level_index)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->level_index %u\n", header->vlr_lastiling->level_index);
      return FALSE;
    }
    if (!stream->put32bitsLE(((U8*)header->vlr_lastiling)+8))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->implicit_levels %u\n", header->vlr_lastiling->implicit_levels);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->min_x)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->min_x %g\n", header->vlr_lastiling->min_x);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->max_x)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->max_x %g\n", header->vlr_lastiling->max_x);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->min_y)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->min_y %g\n", header->vlr_lastiling->min_y);
      return FALSE;
    }
    if (!stream->put32bitsLE((U8*)&(header->vlr_lastiling->max_y)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lastiling->max_y %g\n", header->vlr_lastiling->max_y);
      return FALSE;
    }
  }

  // write lasoriginal VLR with the original (unbuffered) counts and bounding box extent

  if (header->vlr_lasoriginal)
  {
    // write variable length records variable after variable (to avoid alignment issues)

    U16 reserved = 0xAABB;
    if (!stream->put16bitsLE((U8*)&(reserved)))
    {
      fprintf(stderr,"ERROR: writing reserved %d\n", (I32)reserved);
      return FALSE;
    }
    U8 user_id[16] = "LAStools\0\0\0\0\0\0\0";
    if (!stream->putBytes((U8*)user_id, 16))
    {
      fprintf(stderr,"ERROR: writing user_id %s\n", user_id);
      return FALSE;
    }
    U16 record_id = 20;
    if (!stream->put16bitsLE((U8*)&(record_id)))
    {
      fprintf(stderr,"ERROR: writing record_id %d\n", (I32)record_id);
      return FALSE;
    }
    U16 record_length_after_header = 176;
    if (!stream->put16bitsLE((U8*)&(record_length_after_header)))
    {
      fprintf(stderr,"ERROR: writing record_length_after_header %d\n", (I32)record_length_after_header);
      return FALSE;
    }
    U8 description[32] = "counters and bbox of original\0\0";
    if (!stream->putBytes((U8*)description, 32))
    {
      fprintf(stderr,"ERROR: writing description %s\n", description);
      return FALSE;
    }

    // write the payload of this VLR which contains 176 bytes

    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->number_of_point_records)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->number_of_point_records %u\n", (U32)header->vlr_lasoriginal->number_of_point_records);
      return FALSE;
    }
    for (j = 0; j < 15; j++)
    {
      if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->number_of_points_by_return[j])))
      {
        fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->number_of_points_by_return[%u] %u\n", j, (U32)header->vlr_lasoriginal->number_of_points_by_return[j]);
        return FALSE;
      }
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->min_x)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->min_x %g\n", header->vlr_lasoriginal->min_x);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->max_x)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->max_x %g\n", header->vlr_lasoriginal->max_x);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->min_y)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->min_y %g\n", header->vlr_lasoriginal->min_y);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->max_y)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->max_y %g\n", header->vlr_lasoriginal->max_y);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->min_z)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->min_z %g\n", header->vlr_lasoriginal->min_z);
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->vlr_lasoriginal->max_z)))
    {
      fprintf(stderr,"ERROR: writing header->vlr_lasoriginal->max_z %g\n", header->vlr_lasoriginal->max_z);
      return FALSE;
    }
  }

  // write any number of user-defined bytes that might have been added after the header

  if (header->user_data_after_header_size)
  {
    if (header->user_data_after_header)
    {
      if (!stream->putBytes((U8*)header->user_data_after_header, header->user_data_after_header_size))
      {
        fprintf(stderr,"ERROR: writing %d bytes of data from header->user_data_after_header\n", header->user_data_after_header_size);
        return FALSE;
      }
    }
    else
    {
      fprintf(stderr,"ERROR: there should be %d bytes of data in header->user_data_after_header\n", header->user_data_after_header_size);
      return FALSE;
    }
  }

  // initialize the point writer

  if (!writer->init(stream)) return FALSE;

  npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
  p_count = 0;

  return TRUE;
}

BOOL LASwriterLAS::write_point(const LASpoint* point)
{
  p_count++;
  return writer->write(point->point);
}

BOOL LASwriterLAS::chunk()
{
  return writer->chunk();
}

BOOL LASwriterLAS::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  I32 i;
  if (header == 0)
  {
    fprintf(stderr,"ERROR: header pointer is zero\n");
    return FALSE;
  }
  if (stream == 0)
  {
    fprintf(stderr,"ERROR: stream pointer is zero\n");
    return FALSE;
  }
  if (!stream->isSeekable())
  {
    fprintf(stderr,"WARNING: stream not seekable. cannot update header.\n");
    return FALSE;
  }
  if (use_inventory)
  {
    U32 number;
    stream->seek(header_start_position+107);
    if (header->point_data_format >= 6)
    {
      number = 0; // legacy counters are zero for new point types
    }
    else if (inventory.extended_number_of_point_records > U32_MAX)
    {
      if (header->version_minor >= 4)
      {
        number = 0;
      }
      else
      {
        fprintf(stderr,"WARNING: too many points in LAS %d.%d file. limit is %u.\n", header->version_major, header->version_minor, U32_MAX);
        number = U32_MAX;
      }
    }
    else
    {
      number = (U32)inventory.extended_number_of_point_records;
    }
    if (!stream->put32bitsLE((U8*)&number))
    {
      fprintf(stderr,"ERROR: updating inventory.number_of_point_records\n");
      return FALSE;
    }
    npoints = inventory.extended_number_of_point_records;
    for (i = 0; i < 5; i++)
    {
      if (header->point_data_format >= 6)
      {
        number = 0; // legacy counters are zero for new point types
      }
      else if (inventory.extended_number_of_points_by_return[i+1] > U32_MAX)
      {
        if (header->version_minor >= 4)
        {
          number = 0;
        }
        else
        {
          number = U32_MAX;
        }
      }
      else
      {
        number = (U32)inventory.extended_number_of_points_by_return[i+1];
      }
      if (!stream->put32bitsLE((U8*)&number))
      {
        fprintf(stderr,"ERROR: updating inventory.number_of_points_by_return[%d]\n", i);
        return FALSE;
      }
    }
    stream->seek(header_start_position+179);
    F64 value;
    value = quantizer.get_x(inventory.max_X);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.max_X\n");
      return FALSE;
    }
    value = quantizer.get_x(inventory.min_X);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.min_X\n");
      return FALSE;
    }
    value = quantizer.get_y(inventory.max_Y);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.max_Y\n");
      return FALSE;
    }
    value = quantizer.get_y(inventory.min_Y);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.min_Y\n");
      return FALSE;
    }
    value = quantizer.get_z(inventory.max_Z);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.max_Z\n");
      return FALSE;
    }
    value = quantizer.get_z(inventory.min_Z);
    if (!stream->put64bitsLE((U8*)&value))
    {
      fprintf(stderr,"ERROR: updating inventory.min_Z\n");
      return FALSE;
    }
    // special handling for LAS 1.4 or higher.
    if (header->version_minor >= 4)
    {
      stream->seek(header_start_position+247);
      if (!stream->put64bitsLE((U8*)&(inventory.extended_number_of_point_records)))
      {
        fprintf(stderr,"ERROR: updating header->extended_number_of_point_records\n");
        return FALSE;
      }
      for (i = 0; i < 15; i++)
      {
        if (!stream->put64bitsLE((U8*)&(inventory.extended_number_of_points_by_return[i+1])))
        {
          fprintf(stderr,"ERROR: updating header->extended_number_of_points_by_return[%d]\n", i);
          return FALSE;
        }
      }
    }
  }
  else
  {
    U32 number;
    stream->seek(header_start_position+107);
    if (header->point_data_format >= 6)
    {
      number = 0; // legacy counters are zero for new point types
    }
    else
    {
      number = header->number_of_point_records;
    }
    if (!stream->put32bitsLE((U8*)&number))
    {
      fprintf(stderr,"ERROR: updating header->number_of_point_records\n");
      return FALSE;
    }
    npoints = header->number_of_point_records;
    for (i = 0; i < 5; i++)
    {
      if (header->point_data_format >= 6)
      {
        number = 0; // legacy counters are zero for new point types
      }
      else
      {
        number = header->number_of_points_by_return[i];
      }
      if (!stream->put32bitsLE((U8*)&number))
      {
        fprintf(stderr,"ERROR: updating header->number_of_points_by_return[%d]\n", i);
        return FALSE;
      }
    }
    stream->seek(header_start_position+179);
    if (!stream->put64bitsLE((U8*)&(header->max_x)))
    {
      fprintf(stderr,"ERROR: updating header->max_x\n");
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->min_x)))
    {
      fprintf(stderr,"ERROR: updating header->min_x\n");
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->max_y)))
    {
      fprintf(stderr,"ERROR: updating header->max_y\n");
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->min_y)))
    {
      fprintf(stderr,"ERROR: updating header->min_y\n");
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->max_z)))
    {
      fprintf(stderr,"ERROR: updating header->max_z\n");
      return FALSE;
    }
    if (!stream->put64bitsLE((U8*)&(header->min_z)))
    {
      fprintf(stderr,"ERROR: updating header->min_z\n");
      return FALSE;
    }
    // special handling for LAS 1.3 or higher.
    if (header->version_minor >= 3)
    {
      // nobody currently includes waveform. we set the field always to zero
      if (header->start_of_waveform_data_packet_record != 0)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %I64d. writing 0 instead.\n", header->start_of_waveform_data_packet_record);
#else
        fprintf(stderr,"WARNING: header->start_of_waveform_data_packet_record is %lld. writing 0 instead.\n", header->start_of_waveform_data_packet_record);
#endif
        U64 start_of_waveform_data_packet_record = 0;
        if (!stream->put64bitsLE((U8*)&start_of_waveform_data_packet_record))
        {
          fprintf(stderr,"ERROR: updating start_of_waveform_data_packet_record\n");
          return FALSE;
        }
      }
      else
      {
        if (!stream->put64bitsLE((U8*)&(header->start_of_waveform_data_packet_record)))
        {
          fprintf(stderr,"ERROR: updating header->start_of_waveform_data_packet_record\n");
          return FALSE;
        }
      }
    }
    // special handling for LAS 1.4 or higher.
    if (header->version_minor >= 4)
    {
      stream->seek(header_start_position+235);
      if (!stream->put64bitsLE((U8*)&(header->start_of_first_extended_variable_length_record)))
      {
        fprintf(stderr,"ERROR: updating header->start_of_first_extended_variable_length_record\n");
        return FALSE;
      }
      if (!stream->put32bitsLE((U8*)&(header->number_of_extended_variable_length_records)))
      {
        fprintf(stderr,"ERROR: updating header->number_of_extended_variable_length_records\n");
        return FALSE;
      }
      U64 value;
      if (header->number_of_point_records)
        value = header->number_of_point_records;
      else
        value = header->extended_number_of_point_records;
      if (!stream->put64bitsLE((U8*)&value))
      {
        fprintf(stderr,"ERROR: updating header->extended_number_of_point_records\n");
        return FALSE;
      }
      for (i = 0; i < 15; i++)
      {
        if ((i < 5) && header->number_of_points_by_return[i])
          value = header->number_of_points_by_return[i];
        else
          value = header->extended_number_of_points_by_return[i];
        if (!stream->put64bitsLE((U8*)&value))
        {
          fprintf(stderr,"ERROR: updating header->extended_number_of_points_by_return[%d]\n", i);
          return FALSE;
        }
      }
    }
  }
  stream->seekEnd();
  if (update_extra_bytes)
  {
    if (header == 0)
    {
      fprintf(stderr,"ERROR: header pointer is zero\n");
      return FALSE;
    }
    if (header->number_attributes)
    {
      I64 start = header_start_position + header->header_size;
      for (i = 0; i < (I32)header->number_of_variable_length_records; i++)
      {
        start += 54;
        if ((header->vlrs[i].record_id == 4) && (strcmp(header->vlrs[i].user_id, "LASF_Spec") == 0))
        {
          break;
        }
        else
        {
          start += header->vlrs[i].record_length_after_header;
        }
      }
      if (i == (I32)header->number_of_variable_length_records)
      {
        fprintf(stderr,"WARNING: could not find extra bytes VLR for update\n");
      }
      else
      {
        stream->seek(start);
        if (!stream->putBytes((U8*)header->vlrs[i].data, header->vlrs[i].record_length_after_header))
        {
          fprintf(stderr,"ERROR: writing %d bytes of data from header->vlrs[%d].data\n", header->vlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }
    }
    stream->seekEnd();
  }
  return TRUE;
}

I64 LASwriterLAS::close(BOOL update_npoints)
{
  I64 bytes = 0;

  if (p_count != npoints)
  {
    if (npoints || !update_npoints)
    {
#ifdef _WIN32
      fprintf(stderr,"WARNING: written %I64d points but expected %I64d points\n", p_count, npoints);
#else
      fprintf(stderr,"WARNING: written %lld points but expected %lld points\n", p_count, npoints);
#endif
    }
  }

  if (writer) 
  {
    writer->done();
    delete writer;
    writer = 0;
  }

  if (writing_las_1_4 && number_of_extended_variable_length_records)
  {
    I64 real_start_of_first_extended_variable_length_record = stream->tell();

    // write extended variable length records variable after variable (to avoid alignment issues)

    for (U32 i = 0; i < number_of_extended_variable_length_records; i++)
    {
      // check variable length records contents

      if (evlrs[i].reserved != 0xAABB)
      {
  //      fprintf(stderr,"WARNING: wrong evlrs[%d].reserved: %d != 0xAABB\n", i, evlrs[i].reserved);
      }

      // write variable length records variable after variable (to avoid alignment issues)

      if (!stream->put16bitsLE((U8*)&(evlrs[i].reserved)))
      {
        fprintf(stderr,"ERROR: writing evlrs[%d].reserved\n", i);
        return FALSE;
      }
      if (!stream->putBytes((U8*)evlrs[i].user_id, 16))
      {
        fprintf(stderr,"ERROR: writing evlrs[%d].user_id\n", i);
        return FALSE;
      }
      if (!stream->put16bitsLE((U8*)&(evlrs[i].record_id)))
      {
        fprintf(stderr,"ERROR: writing evlrs[%d].record_id\n", i);
        return FALSE;
      }
      if (!stream->put64bitsLE((U8*)&(evlrs[i].record_length_after_header)))
      {
        fprintf(stderr,"ERROR: writing evlrs[%d].record_length_after_header\n", i);
        return FALSE;
      }
      if (!stream->putBytes((U8*)evlrs[i].description, 32))
      {
        fprintf(stderr,"ERROR: writing evlrs[%d].description\n", i);
        return FALSE;
      }

      // write the data following the header of the variable length record

      if (evlrs[i].record_length_after_header)
      {
        if (evlrs[i].data)
        {
          if (!stream->putBytes((U8*)evlrs[i].data, (U32)evlrs[i].record_length_after_header))
          {
            fprintf(stderr,"ERROR: writing %u bytes of data from evlrs[%d].data\n", (U32)evlrs[i].record_length_after_header, i);
            return FALSE;
          }
        }
        else
        {
          fprintf(stderr,"ERROR: there should be %u bytes of data in evlrs[%d].data\n", (U32)evlrs[i].record_length_after_header, i);
          return FALSE;
        }
      }
    }

    if (real_start_of_first_extended_variable_length_record != start_of_first_extended_variable_length_record)
    {
  	  stream->seek(header_start_position+235);
  	  stream->put64bitsLE((U8*)&real_start_of_first_extended_variable_length_record);
      stream->seekEnd();
    }
  }

  if (stream)
  {
    if (update_npoints && p_count != npoints)
    {
      if (!stream->isSeekable())
      {
#ifdef _WIN32
        fprintf(stderr, "WARNING: stream not seekable. cannot update header from %I64d to %I64d points.\n", npoints, p_count);
#else
        fprintf(stderr, "WARNING: stream not seekable. cannot update header from %lld to %lld points.\n", npoints, p_count);
#endif
      }
      else
      {
        U32 number;
        if (writing_new_point_type)
        {
          number = 0;
        }
        else if (p_count > U32_MAX)
        {
          if (writing_las_1_4)
          {
            number = 0;
          }
          else
          {
            number = U32_MAX;
          }
        }
        else
        {
          number = (U32)p_count;
        }
	      stream->seek(header_start_position+107);
	      stream->put32bitsLE((U8*)&number);
        if (writing_las_1_4)
        {
  	      stream->seek(header_start_position+235+12);
  	      stream->put64bitsLE((U8*)&p_count);
        }
        stream->seekEnd();
      }
    }
    bytes = stream->tell() - header_start_position;
    if (delete_stream)
    {
      delete stream;
    }
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

LASwriterLAS::LASwriterLAS()
{
  file = 0;
  stream = 0;
  delete_stream = TRUE;
  writer = 0;
  writing_las_1_4 = FALSE;
  writing_new_point_type = FALSE;
  // for delayed write of EVLRs
  start_of_first_extended_variable_length_record = 0;
  number_of_extended_variable_length_records = 0;
  evlrs = 0;
}

LASwriterLAS::~LASwriterLAS()
{
  if (writer || stream) close();
}
