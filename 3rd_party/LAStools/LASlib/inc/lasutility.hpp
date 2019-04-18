/*
===============================================================================

  FILE:  lasutility.hpp
  
  CONTENTS:
  
    Simple utilities that come in handy when using the laslib API.

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
  
    27 August 2017 -- added '-histo scanner_channel 1'
     1 June 2017 -- improved "fluff" detection
     3 May 2015 -- updated LASinventory to handle LAS 1.4 content 
    25 December 2010 -- created after swinging in Mara's hammock for hours
  
===============================================================================
*/
#ifndef LAS_UTILITY_HPP
#define LAS_UTILITY_HPP

#include "lasdefinitions.hpp"

class LASinventory
{
public:
  BOOL active() const { return (first == FALSE); }; 
  I64 extended_number_of_point_records;
  I64 extended_number_of_points_by_return[16];
  I32 max_X;
  I32 min_X;
  I32 max_Y;
  I32 min_Y;
  I32 max_Z;
  I32 min_Z;
  BOOL init(const LASheader* header);
  BOOL add(const LASpoint* point);
  BOOL update_header(LASheader* header) const;
  LASinventory();
private:
  BOOL first;
};

class LASsummary
{
public:
  BOOL active() const { return (first == FALSE); }; 
  I64 number_of_point_records;
  I64 number_of_points_by_return[16];
  I64 number_of_returns[16];
  I64 classification[32];
  I64 extended_classification[256];
  I64 classification_synthetic;
  I64 classification_keypoint;
  I64 classification_withheld;
  I64 classification_extended_overlap;
  LASpoint min;
  LASpoint max;
  U16 xyz_low_digits_10[3];
  U16 xyz_low_digits_100[3];
  U16 xyz_low_digits_1000[3];
  U16 xyz_low_digits_10000[3];
  I64 xyz_fluff_10[3];
  I64 xyz_fluff_100[3];
  I64 xyz_fluff_1000[3];
  I64 xyz_fluff_10000[3];
  BOOL add(const LASpoint* point);
  BOOL has_fluff() const { return has_fluff(0) || has_fluff(1) || has_fluff(2); };
  BOOL has_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_10[i])); };
  BOOL has_serious_fluff() const { return has_serious_fluff(0) || has_serious_fluff(1) || has_serious_fluff(2); };
  BOOL has_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_100[i])); };
  BOOL has_very_serious_fluff() const { return has_very_serious_fluff(0) || has_very_serious_fluff(1) || has_very_serious_fluff(2); };
  BOOL has_very_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_1000[i])); };
  BOOL has_extremely_serious_fluff() const { return has_extremely_serious_fluff(0) || has_extremely_serious_fluff(1) || has_extremely_serious_fluff(2); };
  BOOL has_extremely_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_10000[i])); };
  LASsummary();
private:
  BOOL first;
};

class LASbin
{
public:
  void add(I32 item);
  void add(I64 item);
  void add(F64 item);
  void add(I32 item, I32 value);
  void add(F64 item, F64 value);
  void report(FILE* file, const CHAR* name=0, const CHAR* name_avg=0) const;
  void reset();
  F64 get_step() const;
  LASbin(F64 step, F64 clamp_min=F64_MIN, F64 clamp_max=F64_MAX);
  ~LASbin();
private:
  void add_to_bin(I32 bin);
  F64 total;
  I64 count;
  F64 step;
  F64 clamp_min;
  F64 clamp_max;
  F64 one_over_step;
  BOOL first;
  I32 anker;
  I32 size_pos;
  I32 size_neg;
  U32* bins_pos;
  U32* bins_neg;
  F64* values_pos;
  F64* values_neg;
};

class LAShistogram
{
public:
  BOOL active() const { return is_active; }; 
  BOOL parse(int argc, char* argv[]);
  I32 unparse(CHAR* string) const;
  BOOL histo(const CHAR* name, F64 step);
  BOOL histo_avg(const CHAR* name, F64 step, const CHAR* name_avg);
  void add(const LASpoint* point);
  void report(FILE* file) const;
  void reset();
  LAShistogram();
  ~LAShistogram();
private:
  BOOL is_active;
  // counter bins
  LASbin* x_bin;
  LASbin* y_bin;
  LASbin* z_bin;
  LASbin* X_bin;
  LASbin* Y_bin;
  LASbin* Z_bin;
  LASbin* intensity_bin;
  LASbin* classification_bin;
  LASbin* scan_angle_bin;
  LASbin* extended_scan_angle_bin;
  LASbin* return_number_bin;
  LASbin* number_of_returns_bin;
  LASbin* user_data_bin;
  LASbin* point_source_id_bin;
  LASbin* gps_time_bin;
  LASbin* scanner_channel_bin;
  LASbin* R_bin;
  LASbin* G_bin;
  LASbin* B_bin;
  LASbin* I_bin;
  LASbin* attribute0_bin;
  LASbin* attribute1_bin;
  LASbin* attribute2_bin;
  LASbin* attribute3_bin;
  LASbin* attribute4_bin;
  LASbin* wavepacket_index_bin;
  LASbin* wavepacket_offset_bin;
  LASbin* wavepacket_size_bin;
  LASbin* wavepacket_location_bin;
  // averages bins
  LASbin* classification_bin_intensity;
  LASbin* classification_bin_scan_angle;
  LASbin* scan_angle_bin_z;
  LASbin* scan_angle_bin_number_of_returns;
  LASbin* scan_angle_bin_intensity;
  LASbin* return_map_bin_intensity;
};

class LASoccupancyGrid
{
public:
  void reset();
  BOOL add(const LASpoint* point);
  BOOL add(I32 pos_x, I32 pos_y);
  BOOL occupied(const LASpoint* point) const;
  BOOL occupied(I32 pos_x, I32 pos_y) const;
  BOOL active() const;
  U32 get_num_occupied() const { return num_occupied; };
  BOOL write_asc_grid(const CHAR* file_name) const;

  // read from file or write to file
//  BOOL read(ByteStreamIn* stream);
//  BOOL write(ByteStreamOut* stream) const;

  LASoccupancyGrid(F32 grid_spacing);
  ~LASoccupancyGrid();
  I32 min_x, min_y, max_x, max_y;
private:
  BOOL add_internal(I32 pos_x, I32 pos_y);
  F32 grid_spacing;
  I32 anker;
  I32* minus_ankers;
  U32 minus_minus_size;
  U32** minus_minus;
  U16* minus_minus_sizes;
  U32 minus_plus_size;
  U32** minus_plus;
  U16* minus_plus_sizes;
  I32* plus_ankers;
  U32 plus_minus_size;
  U32** plus_minus;
  U16* plus_minus_sizes;
  U32 plus_plus_size;
  U32** plus_plus;
  U16* plus_plus_sizes;
  U32 num_occupied;
};

#endif
