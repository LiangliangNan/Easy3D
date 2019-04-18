/*
===============================================================================

  FILE:  laswritercompatible.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "laswritercompatible.hpp"

#include "bytestreamout_array.hpp"
#include "bytestreamin_array.hpp"

BOOL LASwriterCompatibleDown::open(LASheader* header, LASwriteOpener* laswriteopener, BOOL moveCRSfromEVLRtoVLR, BOOL moveEVLRtoVLR)
{
  U32 i;

  if (header == 0)
  {
    return FALSE;
  }
  if (laswriteopener == 0)
  {
    return FALSE;
  }
  if (header->version_minor < 4) // makes only sense for LAS 1.4 input
  {
    return FALSE;
  }
  if (header->point_data_format <= 5) // but not for old point types 0, 1, 2, 3, 4, and 5
  {
    return FALSE;
  }
  else if (header->point_data_format > 10) // only the new point types 6, 7, 8, 9, and 10 are supported
  {
    return FALSE;
  }
  if (header->extended_number_of_point_records > U32_MAX) // only less than 2^32-1 points are supported
  {
    return FALSE;
  }
  this->header = header;

  // downgrade it to LAS 1.2 or LAS 1.3
  if (header->point_data_format <= 8)
  {
    header->version_minor = 2;
    // LAS 1.2 header is 148 bytes less than LAS 1.4+ header
    header->header_size -= 148;
    header->offset_to_point_data -= 148;
  }
  else
  {
    header->version_minor = 3;
    // LAS 1.3 header is 140 bytes less than LAS 1.4+ header
    header->header_size -= 140;
    header->offset_to_point_data -= 140;
  }

  // turn off the bit indicating the presence of the OGC WKT
  header->global_encoding &= ~(1<<4);

  // old point type is two bytes shorter
  header->point_data_record_length -= 2;
  // but we add 5 bytes of attributes
  header->point_data_record_length += 5;

  // create 2+2+4+148 bytes payload for compatibility VLR
  ByteStreamOutArray* out;
  if (IS_LITTLE_ENDIAN())
    out = new ByteStreamOutArrayLE();
  else
    out = new ByteStreamOutArrayBE();
  // write control info
  U16 laszip_version = (U16)LASZIP_VERSION_BUILD_DATE;
  out->put16bitsLE((U8*)&laszip_version);
  U16 compatible_version = 3;
  out->put16bitsLE((U8*)&compatible_version);
  U32 unused = 0;
  out->put32bitsLE((U8*)&unused);
  // write the 148 bytes of the extended LAS 1.4 header
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
  out->put64bitsLE((U8*)&start_of_waveform_data_packet_record);
  U64 start_of_first_extended_variable_length_record = header->start_of_first_extended_variable_length_record;
  if (start_of_first_extended_variable_length_record != 0)
  {
#ifdef _WIN32
    fprintf(stderr,"WARNING: EVLRs not supported. header->start_of_first_extended_variable_length_record is %I64d. writing 0 instead.\n", start_of_first_extended_variable_length_record);
#else
    fprintf(stderr,"WARNING: EVLRs not supported. header->start_of_first_extended_variable_length_record is %lld. writing 0 instead.\n", start_of_first_extended_variable_length_record);
#endif
    start_of_first_extended_variable_length_record = 0;
  }
  out->put64bitsLE((U8*)&start_of_first_extended_variable_length_record);
  U32 number_of_extended_variable_length_records = header->number_of_extended_variable_length_records;
  if (number_of_extended_variable_length_records != 0)
  {
    fprintf(stderr,"WARNING: EVLRs not supported. header->number_of_extended_variable_length_records is %u. writing 0 instead.\n", number_of_extended_variable_length_records);
    number_of_extended_variable_length_records = 0;
  }
  out->put32bitsLE((U8*)&number_of_extended_variable_length_records);
  U64 extended_number_of_point_records;
  if (header->number_of_point_records)
  {
    extended_number_of_point_records = header->number_of_point_records;
    fprintf(stderr,"WARNING: legacy number_of_point_records in header of LAS 1.4 file should be zero.\n");
  }
  else
  {
    extended_number_of_point_records = header->extended_number_of_point_records;
    header->number_of_point_records = (U32)header->extended_number_of_point_records;
  }
  out->put64bitsLE((U8*)&extended_number_of_point_records);
  U64 extended_number_of_points_by_return;
  for (i = 0; i < 15; i++)
  {
    if (i < 5)
    {
      if (header->number_of_points_by_return[i])
      {
        extended_number_of_points_by_return = header->number_of_points_by_return[i];
        fprintf(stderr,"WARNING: legacy number_of_points_by_return[%d] in header of LAS 1.4 file should be zero.\n", i);
      }
      else
      {
        extended_number_of_points_by_return = header->extended_number_of_points_by_return[i];
        header->number_of_points_by_return[i] = (U32)header->extended_number_of_points_by_return[i];
      }
    }
    else
    {
      extended_number_of_points_by_return = header->extended_number_of_points_by_return[i];
    }
    out->put64bitsLE((U8*)&extended_number_of_points_by_return);
  }
  // add the compatibility VLR
  header->add_vlr("lascompatible\0\0", 22204, 2+2+4+148, out->takeData());
  delete out;

  // scan_angle (difference or remainder) is stored as a I16
  LASattribute lasattribute_scan_angle(3, "LAS 1.4 scan angle", "additional attributes");
  lasattribute_scan_angle.set_scale(0.006);
  I32 index_scan_angle = header->add_attribute(lasattribute_scan_angle);
  start_scan_angle = header->get_attribute_start(index_scan_angle);
  // extended returns stored as a U8
  LASattribute lasattribute_extended_returns(0, "LAS 1.4 extended returns", "additional attributes");
  I32 index_extended_returns = header->add_attribute(lasattribute_extended_returns);
  start_extended_returns = header->get_attribute_start(index_extended_returns);
  // classification stored as a U8
  LASattribute lasattribute_classification(0, "LAS 1.4 classification", "additional attributes");
  I32 index_classification = header->add_attribute(lasattribute_classification);
  start_classification = header->get_attribute_start(index_classification);
  // flags and channel stored as a U8
  LASattribute lasattribute_flags_and_channel(0, "LAS 1.4 flags and channel", "additional attributes");
  I32 index_flags_and_channel = header->add_attribute(lasattribute_flags_and_channel);
  start_flags_and_channel = header->get_attribute_start(index_flags_and_channel);
  // maybe store the NIR band as a U16
  if (header->point_data_format == 8 || header->point_data_format == 10)
  {
    // the NIR band is stored as a U16
    LASattribute lasattribute_NIR_band(2, "LAS 1.4 NIR band", "additional attributes");
    I32 index_NIR_band = header->add_attribute(lasattribute_NIR_band);
    start_NIR_band = header->get_attribute_start(index_NIR_band);
  }
  // update VLR
  header->update_extra_bytes_vlr(TRUE);

  // update point type
  if (header->point_data_format == 6)
  {
    header->point_data_format = 1;
  }
  else if (header->point_data_format <= 8)
  {
    header->point_data_format = 3;
  }
  else // 9->4 and 10->5 
  {
    header->point_data_format -= 5;
  }

  // look for CRS in EVLRs and move them to VLRs

  if (moveEVLRtoVLR || moveCRSfromEVLRtoVLR)
  {
    if (header->evlrs)
    {
      for (int i = 0; i < (int)header->number_of_extended_variable_length_records; i++)
      {
        if (moveEVLRtoVLR)
        {
          if (header->evlrs[i].record_length_after_header <= U16_MAX)
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else
          {
#ifdef _WIN32
            fprintf(stderr,"large EVLR with user ID '%s' and record ID %d with payload size %I64d not moved to VLRs.\n", header->evlrs[i].user_id, header->evlrs[i].record_id, header->evlrs[i].record_length_after_header);
#else
            fprintf(stderr,"large EVLR with user ID '%s' and record ID %d with payload size %lld not moved to VLRs.\n", header->evlrs[i].user_id, header->evlrs[i].record_id, header->evlrs[i].record_length_after_header);
#endif
          }
        }
        else if (strcmp(header->evlrs[i].user_id, "LASF_Projection") == 0)
        {
          if (header->evlrs[i].record_id == 34735) // GeoKeyDirectoryTag
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else if (header->evlrs[i].record_id == 34736) // GeoDoubleParamsTag
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else if (header->evlrs[i].record_id == 34737) // GeoAsciiParamsTag
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else if (header->evlrs[i].record_id == 2111) // OGC MATH TRANSFORM WKT
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else if (header->evlrs[i].record_id == 2112) // OGC COORDINATE SYSTEM WKT
          {
            header->add_vlr("LASF_Projection", header->evlrs[i].record_id, (U16)header->evlrs[i].record_length_after_header, header->evlrs[i].data);
            header->evlrs[i].record_length_after_header = 0;
            header->evlrs[i].data = 0;
          }
          else
          {
            fprintf(stderr,"unknown LASF_Projection EVLR with record ID %d not moved to VLRs.\n", header->evlrs[i].record_id);
          }
        }
      }
    }
  }

  writer = laswriteopener->open(header);

  if (writer == 0)
  {
    return FALSE;
  }

  pointCompatibleDown.init(header, header->point_data_format, header->point_data_record_length, header);

  return TRUE;
}

BOOL LASwriterCompatibleDown::write_point(const LASpoint* point)
{
  I32 scan_angle_remainder;
  I32 number_of_returns_increment;
  I32 return_number_increment;
  I32 return_count_difference;
  I32 overlap_bit;
  I32 scanner_channel;

  pointCompatibleDown = *point;

  // distill extended attributes

  scan_angle_remainder = pointCompatibleDown.extended_scan_angle - I16_QUANTIZE(((F32)pointCompatibleDown.scan_angle_rank)/0.006f);
  if (pointCompatibleDown.extended_number_of_returns <= 7)
  {
    pointCompatibleDown.number_of_returns = pointCompatibleDown.extended_number_of_returns;
    if (pointCompatibleDown.extended_return_number <= 7)
    {
      pointCompatibleDown.return_number = pointCompatibleDown.extended_return_number;
    }
    else
    {
      pointCompatibleDown.return_number = 7;
    }
  }
  else
  {
    pointCompatibleDown.number_of_returns = 7;
    if (pointCompatibleDown.extended_return_number <= 4)
    {
      pointCompatibleDown.return_number = pointCompatibleDown.extended_return_number;
    }
    else
    {
      return_count_difference = pointCompatibleDown.extended_number_of_returns - pointCompatibleDown.extended_return_number;
      if (return_count_difference <= 0)
      {
        pointCompatibleDown.return_number = 7;
      }
      else if (return_count_difference >= 3)
      {
        pointCompatibleDown.return_number = 4;
      }
      else
      {
        pointCompatibleDown.return_number = 7 - return_count_difference;
      }
    }
  }
  return_number_increment = pointCompatibleDown.extended_return_number - pointCompatibleDown.return_number;
  assert(return_number_increment >= 0);
  number_of_returns_increment = pointCompatibleDown.extended_number_of_returns - pointCompatibleDown.number_of_returns;
  assert(number_of_returns_increment >= 0);
  if (pointCompatibleDown.extended_classification > 31)
  {
    pointCompatibleDown.set_classification(0);
  }
  else
  {
    pointCompatibleDown.extended_classification = 0;
  }
  scanner_channel = pointCompatibleDown.extended_scanner_channel;
  overlap_bit = (pointCompatibleDown.extended_classification_flags >> 3);

  // write distilled extended attributes into extra bytes 

  pointCompatibleDown.set_attribute(start_scan_angle, ((I16)scan_angle_remainder));
  pointCompatibleDown.set_attribute(start_extended_returns, (U8)((return_number_increment << 4) | number_of_returns_increment));
  pointCompatibleDown.set_attribute(start_classification, (U8)(pointCompatibleDown.extended_classification));
  pointCompatibleDown.set_attribute(start_flags_and_channel, (U8)((scanner_channel << 1) | overlap_bit));
  if (start_NIR_band != -1)
  {
    pointCompatibleDown.set_attribute(start_NIR_band, pointCompatibleDown.rgb[3]);
  }

  writer->write_point(&pointCompatibleDown);
  p_count++;
  return TRUE;
}

BOOL LASwriterCompatibleDown::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  return writer->update_header(header, use_inventory, update_extra_bytes);
}

I64 LASwriterCompatibleDown::close(BOOL update_header)
{
  I64 bytes = writer->close(update_header);

  npoints = p_count;
  p_count = 0;

  return bytes;
}

LASwriterCompatibleDown::LASwriterCompatibleDown()
{
  header = 0;
  writer = 0;
  start_scan_angle = -1;
  start_extended_returns = -1;
  start_classification = -1;
  start_flags_and_channel = -1;
  start_NIR_band = -1;
}

LASwriterCompatibleDown::~LASwriterCompatibleDown()
{
}

BOOL LASwriterCompatibleUp::open(LASheader* header, LASwriteOpener* laswriteopener)
{
  U32 i;

  if (header == 0)
  {
    return FALSE;
  }
  if (laswriteopener == 0)
  {
    return FALSE;
  }
  if (header->version_minor > 3) // makes only sense for LAS 1.0, 1.1, 1.2, or 1.3 input
  {
    return FALSE;
  }
  else if (header->point_data_format == 0) // and not for the old point type 0
  {
    return FALSE;
  }
  else if (header->point_data_format == 2) // and not for the old point type 2
  {
    return FALSE;
  }
  else if (header->point_data_format > 5) // and certainly not fow newer point types
  {
    return FALSE;
  }
  // the compatibility VLR must be in the header
  const LASvlr* compatibility_vlr = header->get_vlr("lascompatible", 22204);
  if (compatibility_vlr == 0)
  {
    fprintf(stderr, "ERROR: no compatibility VLR in header\n"); 
    return FALSE;
  }
  // the compatibility VLR must have the right length
  if (compatibility_vlr->record_length_after_header != (2+2+4+148))
  {
    fprintf(stderr, "ERROR: compatibility VLR has %u instead of %u bytes in payload\n", compatibility_vlr->record_length_after_header, 2+2+4+148);
    return FALSE;
  }
  I32 index_scan_angle = header->get_attribute_index("LAS 1.4 scan angle");
  if (index_scan_angle == -1)
  {
    fprintf(stderr, "ERROR: attribute \"LAS 1.4 scan angle\" is not in EXTRA_BYTES\n");
    return FALSE;
  }
  start_scan_angle = header->get_attribute_start(index_scan_angle);
  I32 index_extended_returns = header->get_attribute_index("LAS 1.4 extended returns");
  if (index_extended_returns == -1)
  {
    fprintf(stderr, "ERROR: attribute \"LAS 1.4 extended returns\" is not in EXTRA_BYTES\n");
    return FALSE;
  }
  start_extended_returns = header->get_attribute_start(index_extended_returns);
  I32 index_classification = header->get_attribute_index("LAS 1.4 classification");
  if (index_classification == -1)
  {
    fprintf(stderr, "ERROR: attribute \"LAS 1.4 classification\" is not in EXTRA_BYTES\n");
    return FALSE;
  }
  start_classification = header->get_attribute_start(index_classification);
  I32 index_flags_and_channel = header->get_attribute_index("LAS 1.4 flags and channel");
  if (index_scan_angle == -1)
  {
    fprintf(stderr, "ERROR: attribute \"LAS 1.4 flags and channel\" is not in EXTRA_BYTES\n");
    return FALSE;
  }
  start_flags_and_channel = header->get_attribute_start(index_flags_and_channel); 

  this->header = header;

  // upgrade it to LAS 1.4

  if (header->version_minor < 3)
  {
    // add the 148 byte difference between LAS 1.4 and LAS 1.2 header sizes
    header->header_size += 148;
    header->offset_to_point_data += 148;
  }
  else if (header->version_minor == 3)
  {
    // add the 140 byte difference between LAS 1.4 and LAS 1.3 header sizes
    header->header_size += 140;
    header->offset_to_point_data += 140;
  }
  header->version_minor = 4;

  // maybe turn on the bit indicating the presence of the OGC WKT
  for (i = 0; i < header->number_of_variable_length_records; i++)
  {
    if ((strncmp(header->vlrs[i].user_id, "LASF_Projection", 16) == 0) && (header->vlrs[i].record_id == 2112))
    {
      header->global_encoding |= (1<<4);
      break;
    }
  }

  // read the 2+2+4+148 bytes payload from the compatibility VLR
  ByteStreamInArray* in;
  if (IS_LITTLE_ENDIAN())
    in = new ByteStreamInArrayLE(compatibility_vlr->data, compatibility_vlr->record_length_after_header);
  else
    in = new ByteStreamInArrayBE(compatibility_vlr->data, compatibility_vlr->record_length_after_header);
  // read the 2+2+4+148 bytes of the extended LAS 1.4 header
  U16 lastools_version;
  in->get16bitsLE((U8*)&lastools_version);
  U16 compatible_version;
  in->get16bitsLE((U8*)&compatible_version);
  if (compatible_version != 3)
  {
    fprintf(stderr, "ERROR: compatibility mode version %u not implemented\n", compatible_version);
    return FALSE;
  }
  U32 unused;
  in->get32bitsLE((U8*)&unused);
  if (unused != 0)
  {
    fprintf(stderr, "WARNING: unused is %u instead of 0\n", unused);
  }
  in->get64bitsLE((U8*)&(header->start_of_waveform_data_packet_record));
  in->get64bitsLE((U8*)&(header->start_of_first_extended_variable_length_record));
  in->get32bitsLE((U8*)&(header->number_of_extended_variable_length_records));
  in->get64bitsLE((U8*)&(header->extended_number_of_point_records));
  for (i = 0; i < 15; i++)
  {
    in->get64bitsLE((U8*)&(header->extended_number_of_points_by_return[i]));
  }
  // delete the compatibility VLR
  header->remove_vlr("lascompatible", 22204);
  delete in;
  // zero the 32-bit legary counters as we have new point types
  header->number_of_point_records = 0;
  for (i = 0; i < 5; i++)
  {
    header->number_of_points_by_return[i] = 0;
  }
  // new point type is two bytes longer
  header->point_data_record_length += 2;
  // but we subtract 5 bytes of attributes
  header->point_data_record_length -= 5;
  // maybe we also have a NIR band?
  if ((header->point_data_format == 3) || (header->point_data_format == 5))
  {
    I32 index_NIR_band = header->get_attribute_index("LAS 1.4 NIR band");
    if (index_NIR_band != -1)
    {
      start_NIR_band = header->get_attribute_start(index_NIR_band);
      header->remove_attribute(index_NIR_band);
    }
  }
  // remove attributes from Extra Bytes VLR
  header->remove_attribute(index_flags_and_channel);
  header->remove_attribute(index_classification);
  header->remove_attribute(index_extended_returns);
  header->remove_attribute(index_scan_angle);
  // update VLR
  header->update_extra_bytes_vlr(TRUE);
  // update point type
  if (header->point_data_format == 1)
  {
    header->point_data_format = 6;
  }
  else if (header->point_data_format == 3)
  {
    if (start_NIR_band != -1)
    {
      header->point_data_format = 8;
    }
    else
    {
      header->point_data_format = 7;
    }
  }
  else
  {
    header->point_data_format += 5;
  }
  // remove old LASzip
  header->clean_laszip();

  writer = laswriteopener->open(header);

  if (writer == 0)
  {
    return FALSE;
  }

  pointCompatibleUp.init(header, header->point_data_format, header->point_data_record_length, header);

  return TRUE;
}

BOOL LASwriterCompatibleUp::write_point(const LASpoint* point)
{
  I16 scan_angle;
  U8 extended_returns;
  U8 classification;
  U8 flags_and_channel;
  I32 return_number_increment;
  I32 number_of_returns_increment;
  I32 overlap_bit;
  I32 scanner_channel;
  // copy point
  pointCompatibleUp = *point;
  // get extra_attributes
  point->get_attribute(start_scan_angle, scan_angle);
  point->get_attribute(start_extended_returns, extended_returns);
  point->get_attribute(start_classification, classification);
  point->get_attribute(start_flags_and_channel, flags_and_channel);
  if (start_NIR_band != -1)
  {
    point->get_attribute(start_NIR_band, pointCompatibleUp.rgb[3]);
  }
  // decompose into individual attributes
  return_number_increment = (extended_returns >> 4) & 0x0F;
  number_of_returns_increment = extended_returns & 0x0F;
  scanner_channel = (flags_and_channel >> 1) & 0x03;
  overlap_bit = flags_and_channel & 0x01;
  // instill into point
  pointCompatibleUp.extended_scan_angle = scan_angle + I16_QUANTIZE(((F32)pointCompatibleUp.scan_angle_rank) / 0.006f);
  pointCompatibleUp.extended_return_number = return_number_increment + pointCompatibleUp.return_number;
  pointCompatibleUp.extended_number_of_returns = number_of_returns_increment + pointCompatibleUp.number_of_returns;
  pointCompatibleUp.extended_classification = classification + pointCompatibleUp.get_classification();
  pointCompatibleUp.extended_scanner_channel = scanner_channel;
  pointCompatibleUp.extended_classification_flags = (overlap_bit << 3) | ((pointCompatibleUp.withheld_flag) << 2) | ((pointCompatibleUp.keypoint_flag) << 1) | (pointCompatibleUp.synthetic_flag);

  writer->write_point(&pointCompatibleUp);
  p_count++;
  return TRUE;
}

BOOL LASwriterCompatibleUp::update_header(const LASheader* header, BOOL use_inventory, BOOL update_extra_bytes)
{
  return writer->update_header(header, use_inventory, update_extra_bytes);
}

I64 LASwriterCompatibleUp::close(BOOL update_header)
{
  I64 bytes = writer->close(update_header);

  npoints = p_count;
  p_count = 0;

  return bytes;
}

LASwriterCompatibleUp::LASwriterCompatibleUp()
{
  header = 0;
  writer = 0;
  start_scan_angle = -1;
  start_extended_returns = -1;
  start_classification = -1;
  start_flags_and_channel = -1;
  start_NIR_band = -1;
}

LASwriterCompatibleUp::~LASwriterCompatibleUp()
{
}
