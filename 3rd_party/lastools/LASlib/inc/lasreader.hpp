/*
===============================================================================

	FILE:  lasreader.hpp

	CONTENTS:

		Interface to read LIDAR points from the LAS format versions 1.0 - 1.3 and
		per on-the-fly conversion from simple ASCII files.

	PROGRAMMERS:

		info@rapidlasso.de  -  https://rapidlasso.de

	COPYRIGHT:

		(c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

		This is free software; you can redistribute and/or modify it under the
		terms of the GNU Lesser General Licence as published by the Free Software
		Foundation. See the LICENSE.txt file for more information.

		This software is distributed WITHOUT ANY WARRANTY and without even the
		implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	CHANGE HISTORY:

		18 April 2023 -- adding support of COPC spatial index standard
		10 March 2022 -- added '-iptx_transform' option
		31 October 2019 -- adding kdtree of bounding boxes for large number of LAS/LAZ files
		19 August 2019 -- unify '-ignore_class 7', '-ignore_withheld', etc ... in LASignore
		 7 September 2018 -- replaced calls to _strdup with calls to the LASCopyString macro
		 8 February 2018 -- new LASreaderStored via '-stored' option to allow piped operation
		15 December 2017 -- optional '-files_are_flightline 101' start number like '-faf 101'
		21 November 2017 -- allow adding up to 32 (from 10) additional extra bytes attributes
		 5 August 2017 -- unless '-buffered 25' just created buffers always '-remain_buffered'
		 5 August 2017 -- removed option '-unbuffered' because it makes too many assumptions
		 7 February 2014 -- added option '-apply_file_source_ID' when reading LAS/LAZ
		22 August 2012 -- added the '-pipe_on' option for a multi-stage LAStools pipeline
		11 August 2012 -- added on-the-fly buffered reading of LiDAR files (efficient with LAX)
		5 September 2011 -- support for reading Terrasolid's BIN format
		11 June 2011 -- billion point support: p_count & npoints are 64 bit counters
		9 April 2011 -- added capability to read on-the-fly conversion from ASCII
		24 January 2011 -- introduced LASreadOpener
		21 January 2011 -- turned into abstract reader to support multiple files
		3 December 2010 -- updated to (somewhat) support LAS format 1.3
		7 September 2008 -- updated to support LAS format 1.2
		18 February 2007 -- created after repairing 2 vacuum cleaners in the garden

===============================================================================
*/
#ifndef LAS_READER_HPP
#define LAS_READER_HPP

#include "lasdefinitions.hpp"
#include "lasignore.hpp"
#include "lastransform.hpp"

class LASindex;
class COPCindex;
class LASfilter;
class LAStransform;
class ByteStreamIn;
class LASkdtreeRectangles;

class LASLIB_DLL LASreader
{
public:
	LASheader header;
	LASpoint point;
	I64 npoints;
	I64 p_count;
	LASTransformMatrix transform_matrix;

	virtual I32 get_format() const = 0;
	virtual BOOL has_layers() const { return FALSE; };

	void set_index(LASindex* index);
	inline LASindex* get_index() const { return index; };
 	void set_copcindex(COPCindex* copc_index);
  	inline COPCindex* get_copcindex() const { return copc_index; };
	virtual void set_filter(LASfilter* filter);
	inline LASfilter* get_filter() const { return filter; };
	virtual void set_transform(LAStransform* transform);
	inline LAStransform* get_transform() const { return transform; };
	void set_ignore(LASignore* ignore);
	inline LASignore* get_ignore() const { return ignore; };

	inline U32 get_inside() const { return inside; };
	virtual BOOL inside_none();
	virtual BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
	inline F32 get_t_ll_x() const { return t_ll_x; };
	inline F32 get_t_ll_y() const { return t_ll_y; };
	inline F32 get_t_size() const { return t_size; };
	virtual BOOL inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
	inline F64 get_c_center_x() const { return c_center_x; };
	inline F64 get_c_center_y() const { return c_center_y; };
	inline F64 get_c_radius() const { return c_radius; };
	virtual BOOL inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);
	inline F64 get_r_min_x() const { return r_min_x; };
	inline F64 get_r_min_y() const { return r_min_y; };
	inline F64 get_r_max_x() const { return r_max_x; };
	inline F64 get_r_max_y() const { return r_max_y; };
	virtual BOOL inside_copc_depth(const U8 mode, const I32 depth, const F32 resolution);
	inline I32 get_copc_depth() const { return copc_depth; };
	inline F32 get_copc_resolution() const { return copc_resolution; };

	virtual BOOL seek(const I64 p_index) = 0;
	BOOL read_point() { return (this->*read_simple)(); };

	inline BOOL ignore_point() { return (ignore ? ignore->ignore(&point) : FALSE); };

	inline void compute_coordinates() { point.compute_coordinates(); };

	inline F64 get_min_x() const { return header.min_x; };
	inline F64 get_min_y() const { return header.min_y; };
	inline F64 get_min_z() const { return header.min_z; };

	inline F64 get_max_x() const { return header.max_x; };
	inline F64 get_max_y() const { return header.max_y; };
	inline F64 get_max_z() const { return header.max_z; };

	inline F64 get_x() const { return get_x(point.get_X()); };
	inline F64 get_y() const { return get_y(point.get_Y()); };
	inline F64 get_z() const { return get_z(point.get_Z()); };

	inline F64 get_x(const I32 x) const { return header.get_x(x); };
	inline F64 get_y(const I32 y) const { return header.get_y(y); };
	inline F64 get_z(const I32 z) const { return header.get_z(z); };

	inline I64 get_X(const F64 x) const { return header.get_X(x); };
	inline I64 get_Y(const F64 y) const { return header.get_Y(y); };
	inline I64 get_Z(const F64 z) const { return header.get_Z(z); };

	virtual ByteStreamIn* get_stream() const = 0;
	virtual void close(BOOL close_stream = TRUE) = 0;

	LASreader();
	virtual ~LASreader();

	void dealloc();

protected:
	virtual BOOL read_point_default() = 0;

	LASindex* index;
	COPCindex* copc_index;
	LASfilter* filter;
	LAStransform* transform;
	LASignore* ignore;

	U32 inside;
	F32 t_ll_x, t_ll_y, t_size, t_ur_x, t_ur_y;
	F64 c_center_x, c_center_y, c_radius, c_radius_squared;
	F64 r_min_x, r_min_y, r_max_x, r_max_y;
	F64 orig_min_x, orig_min_y, orig_max_x, orig_max_y;

	 // optional resolution-of-interest query (copc indexed)
 	U8  inside_depth;  // 0 all, 1 max depth, 2 resolution
	U8  copc_stream_order; // 0 normal, 1 spatially, 2 depth
	F32 copc_resolution;
	I32 copc_depth;

private:
	BOOL(LASreader::*read_simple)();
	BOOL(LASreader::*read_complex)();

	BOOL read_point_none();
	BOOL read_point_filtered();
	BOOL read_point_transformed();
	BOOL read_point_filtered_and_transformed();

	BOOL read_point_inside_tile();
	BOOL read_point_inside_tile_indexed();
	BOOL read_point_inside_circle();
	BOOL read_point_inside_circle_indexed();
	BOOL read_point_inside_rectangle();
	BOOL read_point_inside_rectangle_indexed();

	// COPC specialized readers
	BOOL read_point_inside_circle_copc_indexed();
	BOOL read_point_inside_rectangle_copc_indexed();
	BOOL read_point_inside_depth_copc_indexed();
};

#include "laswaveform13reader.hpp"

class LASLIB_DLL LASreadOpener
{
public:
	void set_io_ibuffer_size(const U32 buffer_size);
	inline U32 get_io_ibuffer_size() const { return io_ibuffer_size; };
	U32 get_file_name_number() const;
	U32 get_file_name_current() const;
	const CHAR* get_file_name() const;
	const CHAR* get_file_name_only() const;
	const CHAR* get_file_extension_only() const;
	const CHAR* get_file_name(U32 number) const;
	const CHAR* get_file_name_only(U32 number) const;
	const CHAR* get_file_extension_only(U32 number) const;
	CHAR* get_file_name_base() const;
	CHAR* get_file_name_base(U32 number) const;
	void set_file_name(const CHAR* file_name, BOOL unique = FALSE);
	BOOL add_file_name(const CHAR* file_name, BOOL unique = FALSE);
	BOOL add_list_of_files(const CHAR* list_of_files, BOOL unique = FALSE);
	void delete_file_name(U32 file_name_id);
	BOOL set_file_name_current(U32 file_name_id);
	I32 get_file_format(U32 number) const;
	void set_merged(const BOOL merged);
	BOOL is_merged() const { return merged; };
	void set_stored(const BOOL stored);
	BOOL is_stored() const { return stored; };
	void set_buffer_size(const F32 buffer_size);
	F32 get_buffer_size() const;
	BOOL add_neighbor_file_name(const CHAR* neighbor_file_name, BOOL unique = FALSE);
	BOOL add_neighbor_file_name(const CHAR* file_name, I64 npoints, F64 min_x, F64 min_y, F64 max_x, F64 max_y, BOOL unique = FALSE);
	BOOL add_neighbor_list_of_files(const CHAR* list_of_files, BOOL unique = FALSE);
	void set_auto_reoffset(const BOOL auto_reoffset);
	inline BOOL is_auto_reoffset() const { return auto_reoffset; };
	void set_files_are_flightlines(const I32 files_are_flightlines);
	inline I32 are_files_flightlines() const { return files_are_flightlines; };
	void set_files_are_flightlines_index(const I32 files_are_flightlines_index);
	inline I32 get_files_flight_index() const { return files_are_flightlines_index; };
	void set_apply_file_source_ID(const BOOL apply_file_source_ID);
	inline BOOL applying_file_source_ID() const { return apply_file_source_ID; };
	void set_scale_factor(const F64* scale_factor);
	inline const F64* get_scale_factor() const { return scale_factor; };
	void set_offset(const F64* offset);
	inline const F64* get_offset() const { return offset; };
	void set_translate_intensity(const F32 translation);
	void set_scale_intensity(const F32 scale);
	void set_translate_scan_angle(const F32 translate_scan_angle);
	void set_scale_scan_angle(const F32 scale_scan_angle);
	void add_attribute(I32 data_type, const CHAR* name, const CHAR* description = 0, F64 scale = 1.0, F64 offset = 0.0, F64 pre_scale = 1.0, F64 pre_offset = 0.0, F64 no_data = F64_MAX);
	BOOL set_point_type(U8 point_type);
	void set_parse_string(const CHAR* parse_string);
	void set_skip_lines(const U32 number_of_lines);
	void set_populate_header(BOOL populate_header);
	void set_keep_lastiling(BOOL keep_lastiling);
	void set_keep_copc(BOOL keep_copc);
	void set_pipe_on(BOOL pipe_on);
	const CHAR* get_parse_string() const;
	void usage() const;
	void set_decompress_selective(U32 decompress_selective);
	void set_inside_tile(const F32 ll_x, const F32 ll_y, const F32 size);
	void set_inside_circle(const F64 center_x, const F64 center_y, const F64 radius);
	void set_inside_rectangle(const F64 min_x, const F64 min_y, const F64 max_x, const F64 max_y);
	void set_max_depth(const I32 max_depth);
	void set_resolution(const F32 resolution);
	BOOL parse(int argc, char* argv[], BOOL parse_ignore = FALSE);
	BOOL is_piped() const;
	BOOL is_buffered() const;
	BOOL is_header_populated() const;
	BOOL active() const;
	BOOL is_inside() const;
	I32 unparse(CHAR* string) const;
	void set_filter(LASfilter* filter);
	inline LASfilter* get_filter() { return filter; };
	void set_transform(LAStransform* transform);
	inline LAStransform* get_transform() { return transform; };
	void set_ignore(LASignore* ignore);
	inline LASignore* get_ignore() { return ignore; };
	void reset();
	const CHAR* get_temp_file_base() const { return temp_file_base; };
	LASreader* open(const CHAR* other_file_name = 0, BOOL reset_after_other = TRUE);
	BOOL reopen(LASreader* lasreader, BOOL remain_buffered = TRUE);
	LASwaveform13reader* open_waveform13(const LASheader* lasheader);
	I32 get_number_attributes() const { return number_attributes; };
	I32 get_attribute_data_type(U32 index) const { return attribute_data_types[index]; };
	const CHAR* get_attribute_name(U32 index) const { return attribute_names[index]; };
	const CHAR* get_attribute_description(U32 index) const { return attribute_descriptions[index]; };
	F64 get_attribute_scale(U32 index) const { return attribute_scales[index]; };
	F64 get_attribute_offset(U32 index) const { return attribute_offsets[index]; };
	F64 get_attribute_no_data(U32 index) const { return attribute_no_datas[index]; };
	void set_copc_stream_ordered_by_chunk() { copc_stream_order = 0; };
	void set_copc_stream_ordered_spatially() { copc_stream_order = 1; };
	void set_copc_stream_ordered_by_level() { copc_stream_order = 2; };
	LASreadOpener();
	~LASreadOpener();
private:
#ifdef _WIN32
	BOOL add_file_name_single(const CHAR* file_name, BOOL unique = FALSE);
	BOOL add_neighbor_file_name_single(const CHAR* neighbor_file_name, BOOL unique = FALSE);
#endif
	BOOL add_file_name(const CHAR* file_name, U32 ID, BOOL unique);
	BOOL add_file_name(const CHAR* file_name, U32 ID, I64 npoints, F64 min_x, F64 min_y, F64 max_x, F64 max_y, BOOL unique = FALSE);
	U32 io_ibuffer_size;
	const CHAR* file_name;
	BOOL merged;
	BOOL stored;
	U32 file_name_current;
	CHAR** file_names;
	U32 file_name_number;
	U32 file_name_allocated;
	U32* file_names_ID;
	I64* file_names_npoints;
	F64* file_names_min_x;
	F64* file_names_min_y;
	F64* file_names_max_x;
	F64* file_names_max_y;
	LASkdtreeRectangles* kdtree_rectangles;
	F32 buffer_size;
	CHAR* temp_file_base;
	CHAR** neighbor_file_names;
	U32 neighbor_file_name_number;
	U32 neighbor_file_name_allocated;
	I64* neighbor_file_names_npoints;
	F64* neighbor_file_names_min_x;
	F64* neighbor_file_names_min_y;
	F64* neighbor_file_names_max_x;
	F64* neighbor_file_names_max_y;
	LASkdtreeRectangles* neighbor_kdtree_rectangles;
	BOOL comma_not_point;
	F64* scale_factor;
	F64* offset;
	BOOL auto_reoffset;
	I32 files_are_flightlines;
	I32 files_are_flightlines_index;
	BOOL apply_file_source_ID;
	BOOL itxt;
	BOOL ipts;
	BOOL iptx;
	BOOL iptx_transform;
	F32 translate_intensity;
	F32 scale_intensity;
	F32 translate_scan_angle;
	F32 scale_scan_angle;
	I32 number_attributes;
	I32 attribute_data_types[32];
	CHAR* attribute_names[32];
	CHAR* attribute_descriptions[32];
	F64 attribute_scales[32];
	F64 attribute_offsets[32];
	F64 attribute_pre_scales[32];
	F64 attribute_pre_offsets[32];
	F64 attribute_no_datas[32];
	U8 point_type;
	CHAR* parse_string;
	U32 skip_lines;
	BOOL populate_header;
	BOOL keep_lastiling;
	BOOL keep_copc;
	BOOL pipe_on;
	BOOL use_stdin;
	BOOL unique;

	// optional extras
	LASindex* index;
	LASfilter* filter;
	LAStransform* transform;
	LASignore* ignore;

	// optional selective decompression (compressed new LAS 1.4 point types only)
	U32 decompress_selective;

	// optional area-of-interest query (spatially indexed)
	F32* inside_tile;
	F64* inside_circle;
	F64* inside_rectangle;

	// optional resolution-of-interest query (copc indexed)
	U8  inside_depth; // 0 all, 1 max depth, 2 resolution
	U8  copc_stream_order; // 0 normal, 1 spatially, 2 depth
	F32 copc_resolution;
	I32 copc_depth;
};

#endif
