/*
===============================================================================

  FILE:  lasutility.cpp
  
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
#include "lasutility.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LASinventory::LASinventory()
{
  U32 i;
  extended_number_of_point_records = 0;
  for (i = 0; i < 16; i++) extended_number_of_points_by_return[i] = 0;
  max_X = min_X = 0;
  max_Y = min_Y = 0;
  max_Z = min_Z = 0;
  first = TRUE;
}

BOOL LASinventory::init(const LASheader* header)
{
  if (header)
  {
    U32 i;
    extended_number_of_point_records = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);
    extended_number_of_points_by_return[0] = 0;
    for (i = 0; i < 5; i++) extended_number_of_points_by_return[i+1] = (header->number_of_points_by_return[i] ? header->number_of_points_by_return[i] : header->extended_number_of_points_by_return[i]);
    for (i = 5; i < 15; i++) extended_number_of_points_by_return[i+1] = header->extended_number_of_points_by_return[i];
    max_X = header->get_X(header->max_x);
    min_X = header->get_X(header->min_x);
    max_Y = header->get_Y(header->max_y);
    min_Y = header->get_Y(header->min_y);
    max_Z = header->get_Z(header->max_z);
    min_Z = header->get_Z(header->min_z);
    first = FALSE;
    return TRUE;
  }
  return FALSE;
}

BOOL LASinventory::add(const LASpoint* point)
{
  extended_number_of_point_records++;
  if (point->extended_point_type)
  {
    extended_number_of_points_by_return[point->extended_return_number]++;
  }
  else
  {
    extended_number_of_points_by_return[point->return_number]++;
  }
  if (first)
  {
    min_X = max_X = point->get_X();
    min_Y = max_Y = point->get_Y();
    min_Z = max_Z = point->get_Z();
    first = FALSE;
  }
  else
  {
    if (point->get_X() < min_X) min_X = point->get_X();
    else if (point->get_X() > max_X) max_X = point->get_X();
    if (point->get_Y() < min_Y) min_Y = point->get_Y();
    else if (point->get_Y() > max_Y) max_Y = point->get_Y();
    if (point->get_Z() < min_Z) min_Z = point->get_Z();
    else if (point->get_Z() > max_Z) max_Z = point->get_Z();
  }
  return TRUE;
}

BOOL LASinventory::update_header(LASheader* header) const
{
  if (header)
  {
    int i;
    if (extended_number_of_point_records > U32_MAX)
    {
      if (header->version_minor >= 4)
      {
        header->number_of_point_records = 0;
      }
      else
      {
        return FALSE;
      }
    }
    else
    {
      header->number_of_point_records = (U32)extended_number_of_point_records;
    }
    for (i = 0; i < 5; i++)
    {
      if (extended_number_of_points_by_return[i+1] > U32_MAX)
      {
        if (header->version_minor >= 4)
        {
          header->number_of_points_by_return[i] = 0;
        }
        else
        {
          return FALSE;
        }
      }
      else
      {
        header->number_of_points_by_return[i] = (U32)extended_number_of_points_by_return[i+1];
      }
    }
    header->max_x = header->get_x(max_X);
    header->min_x = header->get_x(min_X);
    header->max_y = header->get_y(max_Y);
    header->min_y = header->get_y(min_Y);
    header->max_z = header->get_z(max_Z);
    header->min_z = header->get_z(min_Z);
    header->extended_number_of_point_records = extended_number_of_point_records;
    for (i = 0; i < 15; i++)
    {
      header->extended_number_of_points_by_return[i] = extended_number_of_points_by_return[i+1];
    }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

LASsummary::LASsummary()
{
  U32 i;
  number_of_point_records = 0;
  for (i = 0; i < 16; i++) number_of_points_by_return[i] = 0;
  for (i = 0; i < 16; i++) number_of_returns[i] = 0;
  for (i = 0; i < 32; i++) classification[i] = 0;
  for (i = 0; i < 256; i++) extended_classification[i] = 0;
  for (i = 0; i < 3; i++)
  {
    xyz_fluff_10[i] = 0;
    xyz_fluff_100[i] = 0;
    xyz_fluff_1000[i] = 0;
    xyz_fluff_10000[i] = 0;
  }
  classification_synthetic = 0;
  classification_keypoint = 0;
  classification_withheld = 0;
  classification_extended_overlap = 0;
  first = TRUE;
}

BOOL LASsummary::add(const LASpoint* point)
{
  number_of_point_records++;
  if (point->extended_point_type)
  {
    number_of_points_by_return[point->get_extended_return_number()]++;
    number_of_returns[point->get_extended_number_of_returns()]++;
    if (point->get_extended_classification() > 31)
    {
      extended_classification[point->get_extended_classification()]++;
    }
    else
    {
      classification[point->get_classification()]++;
    }
    if (point->get_extended_overlap_flag()) classification_extended_overlap++;
  }
  else
  {
    number_of_points_by_return[point->get_return_number()]++;
    classification[point->get_classification()]++;
    number_of_returns[point->get_number_of_returns()]++;
  }
  if (point->get_synthetic_flag()) classification_synthetic++;
  if (point->get_keypoint_flag()) classification_keypoint++;
  if (point->get_withheld_flag()) classification_withheld++;
  if (first)
  {
    // initialize min and max
    min = *point;
    max = *point;
    // initialize fluff detection
    xyz_low_digits_10[0] = (U16)(point->get_X()%10);
    xyz_low_digits_10[1] = (U16)(point->get_Y()%10);
    xyz_low_digits_10[2] = (U16)(point->get_Z()%10);
    xyz_low_digits_100[0] = (U16)(point->get_X()%100);
    xyz_low_digits_100[1] = (U16)(point->get_Y()%100);
    xyz_low_digits_100[2] = (U16)(point->get_Z()%100);
    xyz_low_digits_1000[0] = (U16)(point->get_X()%1000);
    xyz_low_digits_1000[1] = (U16)(point->get_Y()%1000);
    xyz_low_digits_1000[2] = (U16)(point->get_Z()%1000);
    xyz_low_digits_10000[0] = (U16)(point->get_X()%10000);
    xyz_low_digits_10000[1] = (U16)(point->get_Y()%10000);
    xyz_low_digits_10000[2] = (U16)(point->get_Z()%10000);
    first = FALSE;
  }
  else
  {
    if (point->get_X() < min.get_X()) min.set_X(point->get_X());
    else if (point->get_X() > max.get_X()) max.set_X(point->get_X());
    if (point->get_Y() < min.get_Y()) min.set_Y(point->get_Y());
    else if (point->get_Y() > max.get_Y()) max.set_Y(point->get_Y());
    if (point->get_Z() < min.get_Z()) min.set_Z(point->get_Z());
    else if (point->get_Z() > max.get_Z()) max.set_Z(point->get_Z());
    if (point->intensity < min.intensity) min.intensity = point->intensity;
    else if (point->intensity > max.intensity) max.intensity = point->intensity;
    if (point->edge_of_flight_line < min.edge_of_flight_line) min.edge_of_flight_line = point->edge_of_flight_line;
    else if (point->edge_of_flight_line > max.edge_of_flight_line) max.edge_of_flight_line = point->edge_of_flight_line;
    if (point->scan_direction_flag < min.scan_direction_flag) min.scan_direction_flag = point->scan_direction_flag;
    else if (point->scan_direction_flag > max.scan_direction_flag) max.scan_direction_flag = point->scan_direction_flag;
    if (point->number_of_returns < min.number_of_returns) min.number_of_returns = point->number_of_returns;
    else if (point->number_of_returns > max.number_of_returns) max.number_of_returns = point->number_of_returns;
    if (point->return_number < min.return_number) min.return_number = point->return_number;
    else if (point->return_number > max.return_number) max.return_number = point->return_number;
    if (point->classification < min.classification) min.classification = point->classification;
    else if (point->classification > max.classification) max.classification = point->classification;
    if (point->scan_angle_rank < min.scan_angle_rank) min.scan_angle_rank = point->scan_angle_rank;
    else if (point->scan_angle_rank > max.scan_angle_rank) max.scan_angle_rank = point->scan_angle_rank;
    if (point->user_data < min.user_data) min.user_data = point->user_data;
    else if (point->user_data > max.user_data) max.user_data = point->user_data;
    if (point->point_source_ID < min.point_source_ID) min.point_source_ID = point->point_source_ID;
    else if (point->point_source_ID > max.point_source_ID) max.point_source_ID = point->point_source_ID;
    if (point->have_gps_time)
    {
      if (point->gps_time < min.gps_time) min.gps_time = point->gps_time;
      else if (point->gps_time > max.gps_time) max.gps_time = point->gps_time;
    }
    if (point->have_rgb)
    {
      if (point->rgb[0] < min.rgb[0]) min.rgb[0] = point->rgb[0];
      else if (point->rgb[0] > max.rgb[0]) max.rgb[0] = point->rgb[0];
      if (point->rgb[1] < min.rgb[1]) min.rgb[1] = point->rgb[1];
      else if (point->rgb[1] > max.rgb[1]) max.rgb[1] = point->rgb[1];
      if (point->rgb[2] < min.rgb[2]) min.rgb[2] = point->rgb[2];
      else if (point->rgb[2] > max.rgb[2]) max.rgb[2] = point->rgb[2];
    }
    if (point->extended_point_type)
    {
      if (point->extended_classification < min.extended_classification) min.extended_classification = point->extended_classification;
      else if (point->extended_classification > max.extended_classification) max.extended_classification = point->extended_classification;
      if (point->extended_return_number < min.extended_return_number) min.extended_return_number = point->extended_return_number;
      else if (point->extended_return_number > max.extended_return_number) max.extended_return_number = point->extended_return_number;
      if (point->extended_number_of_returns < min.extended_number_of_returns) min.extended_number_of_returns = point->extended_number_of_returns;
      else if (point->extended_number_of_returns > max.extended_number_of_returns) max.extended_number_of_returns = point->extended_number_of_returns;
      if (point->extended_scan_angle < min.extended_scan_angle) min.extended_scan_angle = point->extended_scan_angle;
      else if (point->extended_scan_angle > max.extended_scan_angle) max.extended_scan_angle = point->extended_scan_angle;
      if (point->extended_scanner_channel < min.extended_scanner_channel) min.extended_scanner_channel = point->extended_scanner_channel;
      else if (point->extended_scanner_channel > max.extended_scanner_channel) max.extended_scanner_channel = point->extended_scanner_channel;
      if (point->have_nir)
      {
        if (point->rgb[3] < min.rgb[3]) min.rgb[3] = point->rgb[3];
        else if (point->rgb[3] > max.rgb[3]) max.rgb[3] = point->rgb[3];
      }
    }
    if (point->have_wavepacket)
    {
      if (point->wavepacket.getIndex() < min.wavepacket.getIndex()) min.wavepacket.setIndex(point->wavepacket.getIndex());
      else if (point->wavepacket.getIndex() > max.wavepacket.getIndex()) max.wavepacket.setIndex(point->wavepacket.getIndex());
      if (point->wavepacket.getOffset() < min.wavepacket.getOffset()) min.wavepacket.setOffset(point->wavepacket.getOffset());
      else if (point->wavepacket.getOffset() > max.wavepacket.getOffset()) max.wavepacket.setOffset(point->wavepacket.getOffset());
      if (point->wavepacket.getSize() < min.wavepacket.getSize()) min.wavepacket.setSize(point->wavepacket.getSize());
      else if (point->wavepacket.getSize() > max.wavepacket.getSize()) max.wavepacket.setSize(point->wavepacket.getSize());
      if (point->wavepacket.getLocation() < min.wavepacket.getLocation()) min.wavepacket.setLocation(point->wavepacket.getLocation());
      else if (point->wavepacket.getLocation() > max.wavepacket.getLocation()) max.wavepacket.setLocation(point->wavepacket.getLocation());
      if (point->wavepacket.getXt() < min.wavepacket.getXt()) min.wavepacket.setXt(point->wavepacket.getXt());
      else if (point->wavepacket.getXt() > max.wavepacket.getXt()) max.wavepacket.setXt(point->wavepacket.getXt());
      if (point->wavepacket.getYt() < min.wavepacket.getYt()) min.wavepacket.setYt(point->wavepacket.getYt());
      else if (point->wavepacket.getYt() > max.wavepacket.getYt()) max.wavepacket.setYt(point->wavepacket.getYt());
      if (point->wavepacket.getZt() < min.wavepacket.getZt()) min.wavepacket.setZt(point->wavepacket.getZt());
      else if (point->wavepacket.getZt() > max.wavepacket.getZt()) max.wavepacket.setZt(point->wavepacket.getZt());
    }
  }
  if (((U16)(point->get_X()%10)) == xyz_low_digits_10[0])
  {
    xyz_fluff_10[0]++;
    if (((U16)(point->get_X()%100)) == xyz_low_digits_100[0])
    {
      xyz_fluff_100[0]++;
      if (((U16)(point->get_X()%1000)) == xyz_low_digits_1000[0])
      {
        xyz_fluff_1000[0]++;
        if (((U16)(point->get_X()%10000)) == xyz_low_digits_10000[0])
        {
          xyz_fluff_10000[0]++;
        }
      }
    }
  }
  if (((U16)(point->get_Y()%10)) == xyz_low_digits_10[1])
  {
    xyz_fluff_10[1]++;
    if (((U16)(point->get_Y()%100)) == xyz_low_digits_100[1])
    {
      xyz_fluff_100[1]++;
      if (((U16)(point->get_Y()%1000)) == xyz_low_digits_1000[1])
      {
        xyz_fluff_1000[1]++;
        if (((U16)(point->get_Y()%10000)) == xyz_low_digits_10000[1])
        {
          xyz_fluff_10000[1]++;
        }
      }
    }
  }
  if (((U16)(point->get_Z()%10)) == xyz_low_digits_10[2])
  {
    xyz_fluff_10[2]++;
    if (((U16)(point->get_Z()%100)) == xyz_low_digits_100[2])
    {
      xyz_fluff_100[2]++;
      if (((U16)(point->get_Z()%1000)) == xyz_low_digits_1000[2])
      {
        xyz_fluff_1000[2]++;
        if (((U16)(point->get_Z()%10000)) == xyz_low_digits_10000[2])
        {
          xyz_fluff_10000[2]++;
        }
      }
    }
  }
  return TRUE;
}

F64 LASbin::get_step() const
{
  return step;
}

LASbin::LASbin(F64 step, F64 clamp_min, F64 clamp_max)
{
  total = 0;
  count = 0;
  this->step = step;
  this->one_over_step = 1.0/step;
  this->clamp_min = clamp_min;
  this->clamp_max = clamp_max;
  first = TRUE;
  size_pos = 0;
  size_neg = 0;
  bins_pos = 0;
  bins_neg = 0;
  values_pos = 0;
  values_neg = 0;
}

LASbin::~LASbin()
{
  if (bins_pos) free(bins_pos);
  if (bins_neg) free(bins_neg);
  if (values_pos) free(values_pos);
  if (values_neg) free(values_neg);
}

void LASbin::add(I32 item)
{
  if (item > clamp_max)
  {
    item = (I32)clamp_max;
  }
  else if (item < clamp_min)
  {
    item = (I32)clamp_min;
  }
  total += item;
  count++;
  I32 bin = I32_FLOOR(one_over_step*item);
  add_to_bin(bin);
}

void LASbin::add(F64 item)
{
  if (item > clamp_max)
  {
    item = clamp_max;
  }
  else if (item < clamp_min)
  {
    item = clamp_min;
  }
  total += item;
  count++;
  I32 bin = I32_FLOOR(one_over_step*item);
  add_to_bin(bin);
}

void LASbin::add(I64 item)
{
  if (item > clamp_max)
  {
    item = (I64)clamp_max;
  }
  else if (item < clamp_min)
  {
    item = (I64)clamp_min;
  }
  total += item;
  count++;
  I32 bin = I32_FLOOR(one_over_step*item);
  add_to_bin(bin);
}

void LASbin::add_to_bin(I32 bin)
{
  if (first)
  {
    anker = bin;
    first = FALSE;
  }
  bin = bin - anker;
  if (bin >= 0)
  {
    if (bin >= size_pos)
    {
      I32 i;
      if (size_pos == 0)
      {
        size_pos = bin + 1024;
        bins_pos = (U32*)malloc(sizeof(U32)*size_pos);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: allocating %u pos bins\012", size_pos);
          exit(1);
        }
        for (i = 0; i < size_pos; i++) bins_pos[i] = 0;
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_pos = (U32*)realloc(bins_pos, sizeof(U32)*new_size);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u pos bins\012", new_size);
          exit(1);
        }
        for (i = size_pos; i < new_size; i++) bins_pos[i] = 0;
        size_pos = new_size;
      }
    }
    bins_pos[bin]++;
  }
  else
  {
    bin = -(bin+1);
    if (bin >= size_neg)
    {
      I32 i;
      if (size_neg == 0)
      {
        size_neg = bin + 1024;
        bins_neg = (U32*)malloc(sizeof(U32)*size_neg);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: allocating %u neg bins\012", size_neg);
          exit(1);
        }
        for (i = 0; i < size_neg; i++) bins_neg[i] = 0;
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_neg = (U32*)realloc(bins_neg, sizeof(U32)*new_size);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u neg bins\012", new_size);
          exit(1);
        }
        for (i = size_neg; i < new_size; i++) bins_neg[i] = 0;
        size_neg = new_size;
      }
    }
    bins_neg[bin]++;
  }
}

void LASbin::add(I32 item, I32 value)
{
  total += item;
  count++;
  I32 bin = I32_FLOOR(one_over_step*item);
  if (first)
  {
    anker = bin;
    first = FALSE;
  }
  bin = bin - anker;
  if (bin >= 0)
  {
    if (bin >= size_pos)
    {
      I32 i;
      if (size_pos == 0)
      {
        size_pos = 1024;
        bins_pos = (U32*)malloc(sizeof(U32)*size_pos);
        values_pos = (F64*)malloc(sizeof(F64)*size_pos);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: allocating %u pos bins\012", size_pos);
          exit(1);
        }
        if (values_pos == 0)
        {
          fprintf(stderr, "ERROR: allocating %u pos values\012", size_pos);
          exit(1);
        }
        for (i = 0; i < size_pos; i++) { bins_pos[i] = 0; values_pos[i] = 0; }
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_pos = (U32*)realloc(bins_pos, sizeof(U32)*new_size);
        values_pos = (F64*)realloc(values_pos, sizeof(F64)*new_size);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u pos bins\012", new_size);
          exit(1);
        }
        if (values_pos == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u pos values\012", new_size);
          exit(1);
        }
        for (i = size_pos; i < new_size; i++) { bins_pos[i] = 0; values_pos[i] = 0; }
        size_pos = new_size;
      }
    }
    bins_pos[bin]++;
    values_pos[bin] += value;
  }
  else
  {
    bin = -(bin+1);
    if (bin >= size_neg)
    {
      I32 i;
      if (size_neg == 0)
      {
        size_neg = 1024;
        bins_neg = (U32*)malloc(sizeof(U32)*size_neg);
        values_neg = (F64*)malloc(sizeof(F64)*size_neg);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: allocating %u neg bins\012", size_neg);
          exit(1);
        }
        if (values_neg == 0)
        {
          fprintf(stderr, "ERROR: allocating %u neg values\012", size_neg);
          exit(1);
        }
        for (i = 0; i < size_neg; i++) { bins_neg[i] = 0; values_neg[i] = 0; }
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_neg = (U32*)realloc(bins_neg, sizeof(U32)*new_size);
        values_neg = (F64*)realloc(values_neg, sizeof(F64)*new_size);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u neg bins\012", new_size);
          exit(1);
        }
        if (values_neg == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u neg values\012", new_size);
          exit(1);
        }
        for (i = size_neg; i < new_size; i++) { bins_neg[i] = 0; values_neg[i] = 0; }
        size_neg = new_size;
      }
    }
    bins_neg[bin]++;
    values_neg[bin] += value;
  }
}

void LASbin::add(F64 item, F64 value)
{
  total += item;
  count++;
  I32 bin = I32_FLOOR(one_over_step*item);
  if (first)
  {
    anker = bin;
    first = FALSE;
  }
  bin = bin - anker;
  if (bin >= 0)
  {
    if (bin >= size_pos)
    {
      I32 i;
      if (size_pos == 0)
      {
        size_pos = 1024;
        bins_pos = (U32*)malloc(sizeof(U32)*size_pos);
        values_pos = (F64*)malloc(sizeof(F64)*size_pos);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: allocating %u pos bins\012", size_pos);
          exit(1);
        }
        if (values_pos == 0)
        {
          fprintf(stderr, "ERROR: allocating %u pos values\012", size_pos);
          exit(1);
        }
        for (i = 0; i < size_pos; i++) { bins_pos[i] = 0; values_pos[i] = 0; }
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_pos = (U32*)realloc(bins_pos, sizeof(U32)*new_size);
        values_pos = (F64*)realloc(values_pos, sizeof(F64)*new_size);
        if (bins_pos == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u pos bins\012", new_size);
          exit(1);
        }
        if (values_pos == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u pos values\012", new_size);
          exit(1);
        }
        for (i = size_pos; i < new_size; i++) { bins_pos[i] = 0; values_pos[i] = 0; }
        size_pos = new_size;
      }
    }
    bins_pos[bin]++;
    values_pos[bin] += value;
  }
  else
  {
    bin = -(bin+1);
    if (bin >= size_neg)
    {
      I32 i;
      if (size_neg == 0)
      {
        size_neg = 1024;
        bins_neg = (U32*)malloc(sizeof(U32)*size_neg);
        values_neg = (F64*)malloc(sizeof(F64)*size_neg);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: allocating %u neg bins\012", size_neg);
          exit(1);
        }
        if (values_neg == 0)
        {
          fprintf(stderr, "ERROR: allocating %u neg values\012", size_neg);
          exit(1);
        }
        for (i = 0; i < size_neg; i++) { bins_neg[i] = 0; values_neg[i] = 0; }
      }
      else
      {
        I32 new_size = bin + 1024;
        bins_neg = (U32*)realloc(bins_neg, sizeof(U32)*new_size);
        values_neg = (F64*)realloc(values_neg, sizeof(F64)*new_size);
        if (bins_neg == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u neg bins\012", new_size);
          exit(1);
        }
        if (values_neg == 0)
        {
          fprintf(stderr, "ERROR: reallocating %u neg values\012", new_size);
          exit(1);
        }
        for (i = size_neg; i < new_size; i++) { bins_neg[i] = 0; values_neg[i] = 0; }
        size_neg = new_size;
      }
    }
    bins_neg[bin]++;
    values_neg[bin] += value;
  }
}

static void lidardouble2string(CHAR* string, F64 value)
{
  int len;
  len = sprintf(string, "%.15f", value) - 1;
  while (string[len] == '0') len--;
  if (string[len] != '.') len++;
  string[len] = '\0';
}

static void lidardouble2string(CHAR* string, F64 value, F64 precision)
{
  if (precision == 0.1 || precision == 0.2 || precision == 0.3 || precision == 0.4 || precision == 0.5)
    sprintf(string, "%.1f", value);
  else if (precision == 0.01 || precision == 0.02 || precision == 0.03 || precision == 0.04 || precision == 0.05 || precision == 0.25)
    sprintf(string, "%.2f", value);
  else if (precision == 0.001 || precision == 0.002 || precision == 0.003 || precision == 0.004 || precision == 0.005 || precision == 0.025 || precision == 0.125)
    sprintf(string, "%.3f", value);
  else if (precision == 0.0001 || precision == 0.0002 || precision == 0.0005 || precision == 0.0025 || precision == 0.0125)
    sprintf(string, "%.4f", value);
  else if (precision == 0.00001 || precision == 0.00002 || precision == 0.00005 || precision == 0.00025 || precision == 0.00125)
    sprintf(string, "%.5f", value);
  else if (precision == 0.000001 || precision == 0.000002 || precision == 0.000005 || precision == 0.000025 || precision == 0.000125)
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

void LASbin::report(FILE* file, const CHAR* name, const CHAR* name_avg) const
{
  I32 i, bin;
  if (name)
  {
    if (values_pos)
    {
      if (name_avg)
        fprintf(file, "%s histogram of %s averages with bin size %lf\012", name, name_avg, step);
      else
        fprintf(file, "%s histogram of averages with bin size %lf\012", name, step);
    }
    else
      fprintf(file, "%s histogram with bin size %lf\012", name, step);
  }
  CHAR temp1[64];
  CHAR temp2[64];
  if (size_neg)
  {
    for (i = size_neg-1; i >= 0; i--)
    {
      if (bins_neg[i])
      {
        bin = -(i+1) + anker;
        if (step == 1)
        {
          if (values_neg)
            fprintf(file, "  bin %d has average %g (of %d)\012", bin, values_neg[i]/bins_neg[i], bins_neg[i]);
          else
            fprintf(file, "  bin %d has %d\012", bin, bins_neg[i]);
        }
        else
        {
          lidardouble2string(temp1, ((F64)bin)*step, step);
          lidardouble2string(temp2, ((F64)bin+1)*step, step);
          if (values_neg)
            fprintf(file, "  bin [%s,%s) has average %g (of %d)\012", temp1, temp2, values_neg[i]/bins_neg[i], bins_neg[i]);
          else
            fprintf(file, "  bin [%s,%s) has %d\012", temp1, temp2, bins_neg[i]);
        }
      }
    }
  }
  if (size_pos)
  {
    for (i = 0; i < size_pos; i++)
    {
      if (bins_pos[i])
      {
        bin = i + anker;
        if (step == 1)
        {
          if (values_pos)
            fprintf(file, "  bin %d has average %g (of %d)\012", bin, values_pos[i]/bins_pos[i], bins_pos[i]);
          else
            fprintf(file, "  bin %d has %d\012", bin, bins_pos[i]);
        }
        else
        {
          lidardouble2string(temp1, ((F64)bin)*step, step);
          lidardouble2string(temp2, ((F64)bin+1)*step, step);
          if (values_pos)
            fprintf(file, "  bin [%s,%s) average has %g (of %d)\012", temp1, temp2, values_pos[i]/bins_pos[i], bins_pos[i]);
          else
            fprintf(file, "  bin [%s,%s) has %d\012", temp1, temp2, bins_pos[i]);
        }
      }
    }
  }
  if (count)
  {
    lidardouble2string(temp1, total/count, step);
#ifdef _WIN32
    if (name)
      fprintf(file, "  average %s %s for %I64d element(s)\012", name, temp1, count);
    else
      fprintf(file, "  average %s for %I64d element(s)\012", temp1, count);
#else
    if (name)
      fprintf(file, "  average %s %s for %lld element(s)\012", name, temp1, count);
    else
      fprintf(file, "  average %s for %lld element(s)\012", temp1, count);
#endif
  }
}

void LASbin::reset()
{
  first = TRUE;
  count = 0;
  total = 0.0;
  if (size_pos)
  {
    memset(bins_pos, 0, sizeof(U32)*size_pos);
    if (values_pos)
    {
      memset(values_pos, 0, sizeof(F64)*size_pos);
    }
  }
  if (size_neg)
  {
    memset(bins_neg, 0, sizeof(U32)*size_neg);
    if (values_neg)
    {
      memset(values_neg, 0, sizeof(F64)*size_neg);
    }
  }
}

LAShistogram::LAShistogram()
{
  is_active = FALSE;
  // counter bins
  x_bin = 0;
  y_bin = 0;
  z_bin = 0;
  X_bin = 0;
  Y_bin = 0;
  Z_bin = 0;
  intensity_bin = 0;
  classification_bin = 0;
  scan_angle_bin = 0;
  extended_scan_angle_bin = 0;
  return_number_bin = 0;
  number_of_returns_bin = 0;
  user_data_bin = 0;
  point_source_id_bin = 0;
  gps_time_bin = 0;
  scanner_channel_bin = 0;
  R_bin = 0;
  G_bin = 0;
  B_bin = 0;
  I_bin = 0;
  attribute0_bin = 0;
  attribute1_bin = 0;
  attribute2_bin = 0;
  attribute3_bin = 0;
  attribute4_bin = 0;
  wavepacket_index_bin = 0;
  wavepacket_offset_bin = 0;
  wavepacket_size_bin = 0;
  wavepacket_location_bin = 0;
  // averages bins
  classification_bin_intensity = 0;
  classification_bin_scan_angle = 0;
  scan_angle_bin_z = 0;
  scan_angle_bin_intensity = 0;
  scan_angle_bin_number_of_returns = 0;
  return_map_bin_intensity = 0;
}

LAShistogram::~LAShistogram()
{
  // counter bins
  if (x_bin) delete x_bin;
  if (y_bin) delete y_bin;
  if (z_bin) delete z_bin;
  if (X_bin) delete X_bin;
  if (Y_bin) delete Y_bin;
  if (Z_bin) delete Z_bin;
  if (intensity_bin) delete intensity_bin;
  if (classification_bin) delete classification_bin;
  if (scan_angle_bin) delete scan_angle_bin;
  if (extended_scan_angle_bin) delete extended_scan_angle_bin;
  if (return_number_bin) delete return_number_bin;
  if (number_of_returns_bin) delete number_of_returns_bin;
  if (user_data_bin) delete user_data_bin;
  if (point_source_id_bin) delete point_source_id_bin;
  if (gps_time_bin) delete gps_time_bin;
  if (scanner_channel_bin) delete scanner_channel_bin;
  if (R_bin) delete R_bin;
  if (G_bin) delete G_bin;
  if (B_bin) delete B_bin;
  if (I_bin) delete I_bin;
  if (attribute0_bin) delete attribute0_bin;
  if (attribute1_bin) delete attribute1_bin;
  if (attribute2_bin) delete attribute2_bin;
  if (attribute3_bin) delete attribute3_bin;
  if (attribute4_bin) delete attribute4_bin;
  if (wavepacket_index_bin) delete wavepacket_index_bin;
  if (wavepacket_offset_bin) delete wavepacket_offset_bin;
  if (wavepacket_size_bin) delete wavepacket_size_bin;
  if (wavepacket_location_bin) delete wavepacket_location_bin;
  // averages bins
  if (classification_bin_intensity) delete classification_bin_intensity;
  if (classification_bin_scan_angle) delete classification_bin_scan_angle;
  if (scan_angle_bin_z) delete scan_angle_bin_z;
  if (scan_angle_bin_intensity) delete scan_angle_bin_intensity;
  if (scan_angle_bin_number_of_returns) delete scan_angle_bin_number_of_returns;
  if (return_map_bin_intensity) delete return_map_bin_intensity;
}

BOOL LAShistogram::parse(int argc, char* argv[])
{
  int i;
  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      return TRUE;
    }
    else if (strcmp(argv[i],"-histo") == 0)
    {
      if ((i+2) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: name step\n", argv[i]);
        return FALSE;
      }
      F64 step = 0.0;
      if (sscanf(argv[i+2], "%lf", &step) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 2 arguments: name step but '%s' is no valid step\n", argv[i], argv[i+2]);
        return FALSE;
      }
      if (!histo(argv[i+1], step)) return FALSE;
      *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
    }
    else if (strcmp(argv[i],"-histo_avg") == 0)
    {
      if ((i+3) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: name step name_avg\n", argv[i]);
        return FALSE;
      }
      F64 step = 0.0;
      if (sscanf(argv[i+2], "%lf", &step) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs 3 arguments: name step name_avg but '%s' is no valid step\n", argv[i], argv[i+2]);
        return FALSE;
      }
      if (!histo_avg(argv[i+1], step, argv[i+3])) return FALSE;
      *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
    }
  }
  return TRUE;
}

I32 LAShistogram::unparse(CHAR* string) const
{
  I32 n = 0;
  if (x_bin) n += sprintf(&string[n], "-histo x %lf ", x_bin->get_step());
  if (y_bin) n += sprintf(&string[n], "-histo y %lf ", y_bin->get_step());
  if (z_bin) n += sprintf(&string[n], "-histo z %lf ", z_bin->get_step());
  if (X_bin) n += sprintf(&string[n], "-histo X %lf ", X_bin->get_step());
  if (Y_bin) n += sprintf(&string[n], "-histo Y %lf ", Y_bin->get_step());
  if (Z_bin) n += sprintf(&string[n], "-histo Z %lf ", Z_bin->get_step());
  if (intensity_bin) n += sprintf(&string[n], "-histo intensity %lf ", intensity_bin->get_step());
  if (classification_bin) n += sprintf(&string[n], "-histo classification %lf ", classification_bin->get_step());
  if (scan_angle_bin) n += sprintf(&string[n], "-histo scan_angle %lf ", scan_angle_bin->get_step());
  if (extended_scan_angle_bin) n += sprintf(&string[n], "-histo extended_scan_angle %lf ", extended_scan_angle_bin->get_step());
  if (return_number_bin) n += sprintf(&string[n], "-histo return_number %lf ", return_number_bin->get_step());
  if (number_of_returns_bin) n += sprintf(&string[n], "-histo number_of_returns %lf ", number_of_returns_bin->get_step());
  if (user_data_bin) n += sprintf(&string[n], "-histo user_data %lf ", user_data_bin->get_step());
  if (point_source_id_bin) n += sprintf(&string[n], "-histo point_source %lf ", point_source_id_bin->get_step());
  if (gps_time_bin) n += sprintf(&string[n], "-histo gps_time %lf ", gps_time_bin->get_step());
  if (scanner_channel_bin) n += sprintf(&string[n], "-histo scanner_channel %lf ", scanner_channel_bin->get_step());
  if (R_bin) n += sprintf(&string[n], "-histo R %lf ", R_bin->get_step());
  if (G_bin) n += sprintf(&string[n], "-histo G %lf ", G_bin->get_step());
  if (B_bin) n += sprintf(&string[n], "-histo B %lf ", B_bin->get_step());
  if (I_bin) n += sprintf(&string[n], "-histo I %lf ", I_bin->get_step());
  if (attribute0_bin) n += sprintf(&string[n], "-histo 0 %lf ", attribute0_bin->get_step());
  if (attribute1_bin) n += sprintf(&string[n], "-histo 1 %lf ", attribute1_bin->get_step());
  if (attribute2_bin) n += sprintf(&string[n], "-histo 2 %lf ", attribute2_bin->get_step());
  if (attribute3_bin) n += sprintf(&string[n], "-histo 3 %lf ", attribute3_bin->get_step());
  if (attribute4_bin) n += sprintf(&string[n], "-histo 4 %lf ", attribute4_bin->get_step());
  if (wavepacket_index_bin) n += sprintf(&string[n], "-histo wavepacket_index %lf ", wavepacket_index_bin->get_step());
  if (wavepacket_offset_bin) n += sprintf(&string[n], "-histo wavepacket_offset %lf ", wavepacket_offset_bin->get_step());
  if (wavepacket_size_bin) n += sprintf(&string[n], "-histo wavepacket_size %lf ", wavepacket_size_bin->get_step());
  if (wavepacket_location_bin) n += sprintf(&string[n], "-histo wavepacket_location %lf ", wavepacket_location_bin->get_step());
  return n;
}

BOOL LAShistogram::histo(const CHAR* name, F64 step)
{
  if (strcmp(name, "x") == 0)
    x_bin = new LASbin(step);
  else if (strcmp(name, "y") == 0)
    y_bin = new LASbin(step);
  else if (strcmp(name, "z") == 0)
    z_bin = new LASbin(step);
  else if (strcmp(name, "X") == 0)
    X_bin = new LASbin(step);
  else if (strcmp(name, "Y") == 0)
    Y_bin = new LASbin(step);
  else if (strcmp(name, "Z") == 0)
    Z_bin = new LASbin(step);
  else if (strcmp(name, "intensity") == 0)
    intensity_bin = new LASbin(step);
  else if (strcmp(name, "classification") == 0)
    classification_bin = new LASbin(step);
  else if (strstr(name, "extended_scan_angle") != 0)
    extended_scan_angle_bin = new LASbin(step);
  else if (strstr(name, "scan_angle") != 0)
    scan_angle_bin = new LASbin(step);
  else if (strstr(name, "return_number") != 0)
    return_number_bin = new LASbin(step);
  else if (strstr(name, "number_of_returns") != 0)
    number_of_returns_bin = new LASbin(step);
  else if (strstr(name, "user_data") != 0)
    user_data_bin = new LASbin(step);
  else if (strstr(name, "point_source") != 0)
    point_source_id_bin = new LASbin(step);
  else if (strstr(name, "gps_time") != 0)
    gps_time_bin = new LASbin(step);
  else if (strstr(name, "scanner_channel") != 0)
    scanner_channel_bin = new LASbin(step);
  else if (strcmp(name, "R") == 0)
    R_bin = new LASbin(step);
  else if (strcmp(name, "G") == 0)
    G_bin = new LASbin(step);
  else if (strcmp(name, "B") == 0)
    B_bin = new LASbin(step);
  else if (strcmp(name, "I") == 0)
    I_bin = new LASbin(step);
  else if (strcmp(name, "0") == 0 || strcmp(name, "attribute0") == 0)
    attribute0_bin = new LASbin(step);
  else if (strcmp(name, "1") == 0 || strcmp(name, "attribute1") == 0)
    attribute1_bin = new LASbin(step);
  else if (strcmp(name, "2") == 0 || strcmp(name, "attribute2") == 0)
    attribute2_bin = new LASbin(step);
  else if (strcmp(name, "3") == 0 || strcmp(name, "attribute3") == 0)
    attribute3_bin = new LASbin(step);
  else if (strcmp(name, "4") == 0 || strcmp(name, "attribute4") == 0)
    attribute4_bin = new LASbin(step);
  else if (strstr(name, "wavepacket_index") != 0)
    wavepacket_index_bin = new LASbin(step);
  else if (strstr(name, "wavepacket_offset") != 0)
    wavepacket_offset_bin = new LASbin(step);
  else if (strstr(name, "wavepacket_size") != 0)
    wavepacket_size_bin = new LASbin(step);
  else if (strstr(name, "wavepacket_location") != 0)
    wavepacket_location_bin = new LASbin(step);
  else
  {
    fprintf(stderr,"ERROR: histogram of '%s' not implemented\n", name);
    return FALSE;
  }
  is_active = TRUE;
  return TRUE;
}

BOOL LAShistogram::histo_avg(const CHAR* name, F64 step, const CHAR* name_avg)
{
  if (strcmp(name, "classification") == 0)
  {
    if (strcmp(name_avg, "intensity") == 0)
      classification_bin_intensity = new LASbin(step);
    else if (strstr(name_avg, "scan_angle") != 0)
      classification_bin_scan_angle = new LASbin(step);
    else
    {
      fprintf(stderr,"ERROR: histogram of '%s' with '%s' averages not implemented\n", name, name_avg);
      return FALSE;
    }
  }
  else if (strcmp(name, "scan_angle") == 0)
  {
    if (strcmp(name_avg, "z") == 0)
      scan_angle_bin_z = new LASbin(step);
    else if (strcmp(name_avg, "number_of_returns") == 0)
      scan_angle_bin_number_of_returns = new LASbin(step);
    else if (strcmp(name_avg, "intensity") == 0)
      scan_angle_bin_intensity = new LASbin(step);
    else
    {
      fprintf(stderr,"ERROR: histogram of '%s' with '%s' averages not implemented\n", name, name_avg);
      return FALSE;
    }
  }
  else if (strcmp(name, "return_map") == 0)
  {
    if (strcmp(name_avg, "intensity") == 0)
      return_map_bin_intensity = new LASbin(1);
    else
    {
      fprintf(stderr,"ERROR: histogram of '%s' with '%s' averages not implemented\n", name, name_avg);
      return FALSE;
    }
  }
  else
  {
    fprintf(stderr,"ERROR: histogram of '%s' not implemented\n", name);
    return FALSE;
  }
  is_active = TRUE;
  return TRUE;
}

void LAShistogram::add(const LASpoint* point)
{
  // counter bins
  if (x_bin) x_bin->add(point->get_x());
  if (y_bin) y_bin->add(point->get_y());
  if (z_bin) z_bin->add(point->get_z());
  if (X_bin) X_bin->add(point->get_X());
  if (Y_bin) Y_bin->add(point->get_Y());
  if (Z_bin) Z_bin->add(point->get_Z());
  if (intensity_bin) intensity_bin->add(point->get_intensity());
  if (classification_bin) classification_bin->add(point->get_classification());
  if (scan_angle_bin)
  {
    scan_angle_bin->add(point->get_scan_angle());
  }
  if (extended_scan_angle_bin)
  {
    extended_scan_angle_bin->add(point->extended_scan_angle);
  }
  if (return_number_bin) return_number_bin->add(point->get_return_number());
  if (number_of_returns_bin) number_of_returns_bin->add(point->get_number_of_returns());
  if (user_data_bin) user_data_bin->add(point->get_user_data());
  if (point_source_id_bin) point_source_id_bin->add(point->get_point_source_ID());
  if (gps_time_bin) gps_time_bin->add(point->get_gps_time());
  if (scanner_channel_bin) scanner_channel_bin->add(point->get_extended_scanner_channel());
  if (R_bin) R_bin->add(point->rgb[0]);
  if (G_bin) G_bin->add(point->rgb[1]);
  if (B_bin) B_bin->add(point->rgb[2]);
  if (I_bin) I_bin->add(point->rgb[3]);
  if (attribute0_bin) attribute0_bin->add(point->get_attribute_as_float(0));
  if (attribute1_bin) attribute1_bin->add(point->get_attribute_as_float(1));
  if (attribute2_bin) attribute2_bin->add(point->get_attribute_as_float(2));
  if (attribute3_bin) attribute3_bin->add(point->get_attribute_as_float(3));
  if (attribute4_bin) attribute4_bin->add(point->get_attribute_as_float(4));
  if (wavepacket_index_bin) wavepacket_index_bin->add(point->wavepacket.getIndex());
  if (wavepacket_offset_bin) wavepacket_offset_bin->add((I64)point->wavepacket.getOffset());
  if (wavepacket_size_bin) wavepacket_size_bin->add((I32)point->wavepacket.getSize());
  if (wavepacket_location_bin) wavepacket_location_bin->add(point->wavepacket.getLocation());
  // averages bins
  if (classification_bin_intensity) classification_bin_intensity->add(point->get_classification(), point->get_intensity());
  if (classification_bin_scan_angle)
  {
    classification_bin_scan_angle->add((F64)point->get_classification(), (F64)point->get_scan_angle());
  }
  if (scan_angle_bin_z)
  {
    scan_angle_bin_z->add((F64)point->get_scan_angle(), (F64)point->get_Z());
  }
  if (scan_angle_bin_number_of_returns)
  {
    scan_angle_bin_number_of_returns->add((F64)point->get_scan_angle(), (F64)point->get_extended_number_of_returns());
  }
  if (scan_angle_bin_intensity)
  {
    scan_angle_bin_intensity->add((F64)point->get_scan_angle(), (F64)point->get_intensity());
  }
  if (return_map_bin_intensity)
  {
    int n = point->number_of_returns;
    int r = point->return_number;
    return_map_bin_intensity->add((n == 1 ? 0 : (n == 2 ? r : (n == 3 ? r+2 : (n == 4 ? r+5 : (n == 5 ? r+9 : 15))))), point->intensity);
  }
}

void LAShistogram::report(FILE* file) const
{
  // counter bins
  if (x_bin) x_bin->report(file, "x coordinate");
  if (y_bin) y_bin->report(file, "y coordinate");
  if (z_bin) z_bin->report(file, "z coordinate");
  if (X_bin) X_bin->report(file, "raw integer X coordinate");
  if (Y_bin) Y_bin->report(file, "raw integer Y coordinate");
  if (Z_bin) Z_bin->report(file, "raw integer Z coordinate");
  if (intensity_bin) intensity_bin->report(file, "intensity");
  if (classification_bin) classification_bin->report(file, "classification");
  if (scan_angle_bin) scan_angle_bin->report(file, "scan angle");
  if (extended_scan_angle_bin) extended_scan_angle_bin->report(file, "extended scan angle");
  if (return_number_bin) return_number_bin->report(file, "return_number");
  if (number_of_returns_bin) number_of_returns_bin->report(file, "number_of_returns");
  if (user_data_bin) user_data_bin->report(file, "user data");
  if (point_source_id_bin) point_source_id_bin->report(file, "point source id");
  if (gps_time_bin) gps_time_bin->report(file, "gps_time");
  if (scanner_channel_bin) scanner_channel_bin->report(file, "scanner channel");
  if (R_bin) R_bin->report(file, "color R channel");
  if (G_bin) G_bin->report(file, "color G channel");
  if (B_bin) B_bin->report(file, "color B channel");
  if (I_bin) I_bin->report(file, "color I channel");
  if (attribute0_bin) attribute0_bin->report(file, "attribute 0");
  if (attribute1_bin) attribute1_bin->report(file, "attribute 1");
  if (attribute2_bin) attribute2_bin->report(file, "attribute 2");
  if (attribute3_bin) attribute3_bin->report(file, "attribute 3");
  if (attribute4_bin) attribute4_bin->report(file, "attribute 4");
  if (wavepacket_index_bin) wavepacket_index_bin->report(file, "wavepacket_index");
  if (wavepacket_offset_bin) wavepacket_offset_bin->report(file, "wavepacket_offset");
  if (wavepacket_size_bin) wavepacket_size_bin->report(file, "wavepacket_size");
  if (wavepacket_location_bin) wavepacket_location_bin->report(file, "wavepacket_location");
  // averages bins
  if (classification_bin_intensity) classification_bin_intensity->report(file, "classification", "intensity");
  if (classification_bin_scan_angle) classification_bin_scan_angle->report(file, "classification", "scan_angle");
  if (scan_angle_bin_z) scan_angle_bin_z->report(file, "scan angle", "z coordinate");
  if (scan_angle_bin_number_of_returns) scan_angle_bin_number_of_returns->report(file, "scan_angle", "number_of_returns");
  if (scan_angle_bin_intensity) scan_angle_bin_intensity->report(file, "scan angle", "intensity");
  if (return_map_bin_intensity) return_map_bin_intensity->report(file, "return map", "intensity");
}

void LAShistogram::reset()
{
  // reset counter bins
  if (x_bin) x_bin->reset();
  if (y_bin) y_bin->reset();
  if (z_bin) z_bin->reset();
  if (X_bin) X_bin->reset();
  if (Y_bin) Y_bin->reset();
  if (Z_bin) Z_bin->reset();
  if (intensity_bin) intensity_bin->reset();
  if (classification_bin) classification_bin->reset();
  if (scan_angle_bin) scan_angle_bin->reset();
  if (extended_scan_angle_bin) extended_scan_angle_bin->reset();
  if (return_number_bin) return_number_bin->reset();
  if (number_of_returns_bin) number_of_returns_bin->reset();
  if (user_data_bin) user_data_bin->reset();
  if (point_source_id_bin) point_source_id_bin->reset();
  if (gps_time_bin) gps_time_bin->reset();
  if (scanner_channel_bin) scanner_channel_bin->reset();
  if (R_bin) R_bin->reset();
  if (G_bin) G_bin->reset();
  if (B_bin) B_bin->reset();
  if (I_bin) I_bin->reset();
  if (attribute0_bin) attribute0_bin->reset();
  if (attribute1_bin) attribute1_bin->reset();
  if (attribute2_bin) attribute2_bin->reset();
  if (attribute3_bin) attribute3_bin->reset();
  if (attribute4_bin) attribute4_bin->reset();
  if (wavepacket_index_bin) wavepacket_index_bin->reset();
  if (wavepacket_offset_bin) wavepacket_offset_bin->reset();
  if (wavepacket_size_bin) wavepacket_size_bin->reset();
  if (wavepacket_location_bin) wavepacket_location_bin->reset();
  // averages bins
  if (classification_bin_intensity) classification_bin_intensity->reset();
  if (classification_bin_scan_angle) classification_bin_scan_angle->reset();
  if (scan_angle_bin_z) scan_angle_bin_z->reset();
  if (scan_angle_bin_intensity) scan_angle_bin_intensity->reset();
  if (scan_angle_bin_number_of_returns) scan_angle_bin_number_of_returns->reset();
  if (return_map_bin_intensity) return_map_bin_intensity->reset();
}

BOOL LASoccupancyGrid::add(const LASpoint* point)
{
  I32 pos_x, pos_y;
  if (grid_spacing < 0)
  {
    grid_spacing = -grid_spacing;
    pos_x = I32_FLOOR(point->get_x() / grid_spacing);
    pos_y = I32_FLOOR(point->get_y() / grid_spacing);
    anker = pos_y;
    min_x = max_x = pos_x;
    min_y = max_y = pos_y;
  }
  else
  {
    pos_x = I32_FLOOR(point->get_x() / grid_spacing);
    pos_y = I32_FLOOR(point->get_y() / grid_spacing);
    if (pos_x < min_x) min_x = pos_x; else if (pos_x > max_x) max_x = pos_x;
    if (pos_y < min_y) min_y = pos_y; else if (pos_y > max_y) max_y = pos_y;
  }
  return add_internal(pos_x, pos_y);
}

BOOL LASoccupancyGrid::add(I32 pos_x, I32 pos_y)
{
  if (grid_spacing < 0)
  {
    grid_spacing = -grid_spacing;
    anker = pos_y;
    min_x = max_x = pos_x;
    min_y = max_y = pos_y;
  }
  else
  {
    if (pos_x < min_x) min_x = pos_x; else if (pos_x > max_x) max_x = pos_x;
    if (pos_y < min_y) min_y = pos_y; else if (pos_y > max_y) max_y = pos_y;
  }
  return add_internal(pos_x, pos_y);
}

BOOL LASoccupancyGrid::add_internal(I32 pos_x, I32 pos_y)
{
  pos_y = pos_y - anker;
  BOOL no_x_anker = FALSE;
  U32* array_size;
  I32** ankers;
  U32*** array;
  U16** array_sizes;
  if (pos_y < 0)
  {
    pos_y = -pos_y - 1;
    ankers = &minus_ankers;
    if ((U32)pos_y < minus_plus_size && minus_plus_sizes[pos_y])
    {
      pos_x -= minus_ankers[pos_y];
      if (pos_x < 0)
      {
        pos_x = -pos_x - 1;
        array_size = &minus_minus_size;
        array = &minus_minus;
        array_sizes = &minus_minus_sizes;
      }
      else
      {
        array_size = &minus_plus_size;
        array = &minus_plus;
        array_sizes = &minus_plus_sizes;
      }
    }
    else
    {
      no_x_anker = TRUE;
      array_size = &minus_plus_size;
      array = &minus_plus;
      array_sizes = &minus_plus_sizes;
    }
  }
  else
  {
    ankers = &plus_ankers;
    if ((U32)pos_y < plus_plus_size && plus_plus_sizes[pos_y])
    {
      pos_x -= plus_ankers[pos_y];
      if (pos_x < 0)
      {
        pos_x = -pos_x - 1;
        array_size = &plus_minus_size;
        array = &plus_minus;
        array_sizes = &plus_minus_sizes;
      }
      else
      {
        array_size = &plus_plus_size;
        array = &plus_plus;
        array_sizes = &plus_plus_sizes;
      }
    }
    else
    {
      no_x_anker = TRUE;
      array_size = &plus_plus_size;
      array = &plus_plus;
      array_sizes = &plus_plus_sizes;
    }
  }
  // maybe grow banded grid in y direction
  if ((U32)pos_y >= *array_size)
  {
    U32 array_size_new = ((pos_y/1024)+1)*1024;
    if (*array_size)
    {
      if (array == &minus_plus || array == &plus_plus) *ankers = (I32*)realloc(*ankers, array_size_new*sizeof(I32));
      *array = (U32**)realloc(*array, array_size_new*sizeof(U32*));
      *array_sizes = (U16*)realloc(*array_sizes, array_size_new*sizeof(U16));
    }
    else
    {
      if (array == &minus_plus || array == &plus_plus) *ankers = (I32*)malloc(array_size_new*sizeof(I32));
      *array = (U32**)malloc(array_size_new*sizeof(U32*));
      *array_sizes = (U16*)malloc(array_size_new*sizeof(U16));
    }
    for (U32 i = *array_size; i < array_size_new; i++)
    {
      (*array)[i] = 0;
      (*array_sizes)[i] = 0;
    }
    *array_size = array_size_new;
  }
  // is this the first x anker for this y pos?
  if (no_x_anker)
  {
    (*ankers)[pos_y] = pos_x;
    pos_x = 0;
  }
  // maybe grow banded grid in x direction
  U32 pos_x_pos = pos_x/32;
  if (pos_x_pos >= (*array_sizes)[pos_y])
  {
    U32 array_sizes_new = ((pos_x_pos/256)+1)*256;
    if ((*array_sizes)[pos_y])
    {
      (*array)[pos_y] = (U32*)realloc((*array)[pos_y], array_sizes_new*sizeof(U32));
    }
    else
    {
      (*array)[pos_y] = (U32*)malloc(array_sizes_new*sizeof(U32));
    }
    for (U16 i = (*array_sizes)[pos_y]; i < array_sizes_new; i++)
    {
      (*array)[pos_y][i] = 0;
    }
    (*array_sizes)[pos_y] = array_sizes_new;
  }
  U32 pos_x_bit = 1 << (pos_x%32);
  if ((*array)[pos_y][pos_x_pos] & pos_x_bit) return FALSE;
  (*array)[pos_y][pos_x_pos] |= pos_x_bit;
  num_occupied++;
  return TRUE;
}

BOOL LASoccupancyGrid::occupied(const LASpoint* point) const
{
  I32 pos_x = I32_FLOOR(point->get_x() / grid_spacing);
  I32 pos_y = I32_FLOOR(point->get_y() / grid_spacing);
  return occupied(pos_x, pos_y);
}

BOOL LASoccupancyGrid::occupied(I32 pos_x, I32 pos_y) const
{
  if (grid_spacing < 0)
  {
    return FALSE;
  }
  pos_y = pos_y - anker;
  U32 array_size;
  const U32* const * array;
  const U16* array_sizes;
  if (pos_y < 0)
  {
    pos_y = -pos_y - 1;
    if ((U32)pos_y < minus_plus_size && minus_plus_sizes[pos_y])
    {
      pos_x -= minus_ankers[pos_y];
      if (pos_x < 0)
      {
        pos_x = -pos_x - 1;
        array_size = minus_minus_size;
        array = minus_minus;
        array_sizes = minus_minus_sizes;
      }
      else
      {
        array_size = minus_plus_size;
        array = minus_plus;
        array_sizes = minus_plus_sizes;
      }
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
    if ((U32)pos_y < plus_plus_size && plus_plus_sizes[pos_y])
    {
      pos_x -= plus_ankers[pos_y];
      if (pos_x < 0)
      {
        pos_x = -pos_x - 1;
        array_size = plus_minus_size;
        array = plus_minus;
        array_sizes = plus_minus_sizes;
      }
      else
      {
        array_size = plus_plus_size;
        array = plus_plus;
        array_sizes = plus_plus_sizes;
      }
    }
    else
    {
      return FALSE;
    }
  }
  // maybe out of bounds in y direction
  if ((U32)pos_y >= array_size)
  {
    return FALSE;
  }
  // maybe out of bounds in x direction
  U32 pos_x_pos = pos_x/32;
  if (pos_x_pos >= array_sizes[pos_y])
  {
    return FALSE;
  }
  U32 pos_x_bit = 1 << (pos_x%32);
  if (array[pos_y][pos_x_pos] & pos_x_bit) return TRUE;
  return FALSE;
}

BOOL LASoccupancyGrid::active() const
{
  if (grid_spacing < 0) return FALSE;
  return TRUE;
}

void LASoccupancyGrid::reset()
{
  min_x = min_y = max_x = max_y = 0;
  if (grid_spacing > 0) grid_spacing = -grid_spacing;
  if (minus_minus_size)
  {
    for (U32 i = 0; i < minus_minus_size; i++) if (minus_minus[i]) free(minus_minus[i]);
    free(minus_minus);
    minus_minus = 0;
    free(minus_minus_sizes);
    minus_minus_sizes = 0;
    minus_minus_size = 0;
  }
  if (minus_plus_size)
  {
    free(minus_ankers);
    minus_ankers = 0;
    for (U32 i = 0; i < minus_plus_size; i++) if (minus_plus[i]) free(minus_plus[i]);
    free(minus_plus);
    minus_plus = 0;
    free(minus_plus_sizes);
    minus_plus_sizes = 0;
    minus_plus_size = 0;
  }
  if (plus_minus_size)
  {
    for (U32 i = 0; i < plus_minus_size; i++) if (plus_minus[i]) free(plus_minus[i]);
    free(plus_minus);
    plus_minus = 0;
    free(plus_minus_sizes);
    plus_minus_sizes = 0;
    plus_minus_size = 0;
  }
  if (plus_plus_size)
  {
    free(plus_ankers);
    plus_ankers = 0;
    for (U32 i = 0; i < plus_plus_size; i++) if (plus_plus[i]) free(plus_plus[i]);
    free(plus_plus);
    plus_plus = 0;
    free(plus_plus_sizes);
    plus_plus_sizes = 0;
    plus_plus_size = 0;
  }
  num_occupied = 0;
}

BOOL LASoccupancyGrid::write_asc_grid(const CHAR* file_name) const
{
  FILE* file = fopen(file_name, "w");
  if (file == 0) return FALSE;
  fprintf(file, "ncols %d\012", max_x-min_x+1);
  fprintf(file, "nrows %d\012", max_y-min_y+1);
  fprintf(file, "xllcorner %f\012", grid_spacing*min_x);
  fprintf(file, "yllcorner %f\012", grid_spacing*min_y);
  fprintf(file, "cellsize %lf\012", grid_spacing);
  fprintf(file, "NODATA_value %d\012", 0);
  fprintf(file, "\012");
  I32 pos_x, pos_y;
  for (pos_y = min_y; pos_y <= max_y; pos_y++)
  {
    for (pos_x = min_x; pos_x <= max_x; pos_x++)
    {
      if (occupied(pos_x, pos_y))
      {
        fprintf(file, "1 ");
      }
      else
      {
        fprintf(file, "0 ");
      }
    }
    fprintf(file, "\012");
  }
  fclose(file);
  return TRUE;
}

LASoccupancyGrid::LASoccupancyGrid(F32 grid_spacing)
{
  min_x = min_y = max_x = max_y = 0;
  this->grid_spacing = -grid_spacing;
  minus_ankers = 0;
  minus_minus_size = 0;
  minus_minus = 0; 
  minus_minus_sizes = 0;
  minus_plus_size = 0;
  minus_plus = 0;
  minus_plus_sizes = 0;
  plus_ankers = 0;
  plus_minus_size = 0;
  plus_minus = 0;
  plus_minus_sizes = 0;
  plus_plus_size = 0;
  plus_plus = 0;
  plus_plus_sizes = 0;
  num_occupied = 0;
}

LASoccupancyGrid::~LASoccupancyGrid()
{
  reset();
}

