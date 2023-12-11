/*
===============================================================================

	FILE:  lastransform.hpp

	CONTENTS:

		Transforms LIDAR points with a number of different operations.

	PROGRAMMERS:

		info@rapidlasso.de  -  https://rapidlasso.de

	COPYRIGHT:

		(c) 2007-2021, rapidlasso GmbH - fast tools to catch reality

		This is free software; you can redistribute and/or modify it under the
		terms of the GNU Lesser General Licence as published by the Free Software
		Foundation. See the LICENSE.txt file for more information.

		This software is distributed WITHOUT ANY WARRANTY and without even the
		implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	CHANGE HISTORY:

		10 March 2022 -- added TransformMatrix operation
		18 November 2021 -- new '-forceRGB' to use RGB values also in non-RGB point versions
		15 June 2021 -- new '-clamp_RGB_to_8bit' transform useful to avoid 8 bit overflow
		10 May 2019 -- checking for overflows in X, Y, Z 32 bit integers of fixed-point LAS
		 6 March 2018 -- changed '%g' to '%lf' for all sprintf() of F64 values
		28 February 2017 -- now '-set_RGB_of_class' also works for classifications > 31
		 1 February 2017 -- new '-copy_intensity_into_z' for use in lasgrid or lascanopy
		 9 May 2016 -- new '-translate_raw_xy_at_random 2 2' for random pertubation
		20 April 2016 -- new '-switch_R_G', '-switch_R_B' and '-set_RGB 32768 16384 0'
		25 January 2016 -- brand-new opportunity to do a '-filtered_transform'
		18 December 2011 -- added '-flip_waveform_direction' to deal with Riegl's data
		20 March 2011 -- added -translate_raw_xyz after the fullest of full moons
		21 January 2011 -- re-created after matt told me about the optech dashmap bug

===============================================================================
*/
#ifndef LAS_TRANSFORM_HPP
#define LAS_TRANSFORM_HPP

#include "lasdefinitions.hpp"
#include "laszip_decompress_selective_v3.hpp"

class LASfilter;

struct LASTransformMatrix {
	F64 r11;
	F64 r12;
	F64 r13;
	F64 r21;
	F64 r22;
	F64 r23;
	F64 r31;
	F64 r32;
	F64 r33;
	F64 tr1;
	F64 tr2;
	F64 tr3;
};

class LASoperation
{
public:
	virtual const CHAR * name() const = 0;
	virtual I32 get_command(CHAR* string) const = 0;
	virtual U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY; };
	inline I64 get_overflow() const { return overflow; };
	inline void zero_overflow() { overflow = 0; };
	virtual void transform(LASpoint* point) = 0;
	virtual void reset() { overflow = 0; };
	inline LASoperation() { overflow = 0; };
	virtual ~LASoperation() {};
protected:
	I64 overflow;
};

// this operation is public cause used out of reader in ptx header
class LASoperationTransformMatrix : public LASoperation
{
public:
	inline const CHAR* name() const { return "transform_matrix"; };
	inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf", name(), r11, r12, r13, r21, r22, r23, r31, r32, r33, tr1, tr2, tr3); };
	inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY | LASZIP_DECOMPRESS_SELECTIVE_Z; };
	inline void transform(LASpoint* point) {
		F64 x = point->get_x();
		F64 y = point->get_y();
		F64 z = point->get_z();
		F64 xr = x * r11 + y * r12 + z * r13 + tr1;
		F64 yr = x * r21 + y * r22 + z * r23 + tr2;
		F64 zr = x * r31 + y * r32 + z * r33 + tr3;
		if (!point->set_x(xr))
		{
			overflow++;
		}
		if (!point->set_y(yr))
		{
			overflow++;
		}
		if (!point->set_z(zr))
		{
			overflow++;
		}
	};
	LASoperationTransformMatrix(F64 r11, F64 r12, F64 r13, F64 r21, F64 r22, F64 r23, F64 r31, F64 r32, F64 r33, F64 tr1, F64 tr2, F64 tr3)
	{
		this->r11 = r11; this->r12 = r12; this->r13 = r13;
		this->r21 = r21; this->r22 = r22; this->r23 = r23;
		this->r31 = r31; this->r32 = r32; this->r33 = r33;
		this->tr1 = tr1; this->tr2 = tr2; this->tr3 = tr3;
	};
	LASoperationTransformMatrix(LASTransformMatrix tm)
	{
		this->r11 = tm.r11; this->r12 = tm.r12; this->r13 = tm.r13;
		this->r21 = tm.r21; this->r22 = tm.r22; this->r23 = tm.r23;
		this->r31 = tm.r31; this->r32 = tm.r32; this->r33 = tm.r33;
		this->tr1 = tm.tr1; this->tr2 = tm.tr2; this->tr3 = tm.tr3;
	};
private:
	F64 r11, r12, r13, r21, r22, r23, r31, r32, r33, tr1, tr2, tr3;
};

#define LASTRANSFORM_X_COORDINATE 0x00000001
#define LASTRANSFORM_Y_COORDINATE 0x00000002
#define LASTRANSFORM_Z_COORDINATE 0x00000004
#define LASTRANSFORM_INTENSITY    0x00000008
#define LASTRANSFORM_RGB          0x00020000
#define LASTRANSFORM_NIR          0x02000000

#define LASTRANSFORM_XY_COORDINATE (LASTRANSFORM_X_COORDINATE | LASTRANSFORM_Y_COORDINATE)
#define LASTRANSFORM_XYZ_COORDINATE (LASTRANSFORM_XY_COORDINATE | LASTRANSFORM_Z_COORDINATE)

class LAStransform
{
public:
	U32 transformed_fields;
	F64 registers[16];

	void usage() const;
	void clean();
	BOOL parse(int argc, char* argv[]);
	BOOL parse(CHAR* string);
	I32 unparse(CHAR* string) const;
	inline BOOL active() const { return (num_operations != 0); };
	U32 get_decompress_selective() const;
	inline BOOL filtered() const { return is_filtered; };

	void setFilter(LASfilter* filter);

	void setPointSource(U16 value);
	void unsetPointSource();

	void transform(LASpoint* point);

	void check_for_overflow() const;

	void reset();
	void add_operation(LASoperation* operation);

	LAStransform();
	~LAStransform();

private:

	void delete_operation(const CHAR* name);
	U32 num_operations;
	U32 alloc_operations;
	LASoperation** operations;
	BOOL is_filtered;
	LASfilter* filter;
};

#endif
