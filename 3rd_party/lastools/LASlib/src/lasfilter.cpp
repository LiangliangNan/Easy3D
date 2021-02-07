/*
===============================================================================

  FILE:  lasfilter.cpp
  
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
#include "lasfilter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <set>
using namespace std;

typedef multimap<I64,F64> my_I64_F64_map;
typedef set<I64> my_I64_set;

class LAScriterionAnd : public LAScriterion
{
public:
  inline const CHAR* name() const { return "filter_and"; };
  inline I32 get_command(CHAR* string) const { int n = 0; n += one->get_command(&string[n]); n += two->get_command(&string[n]); n += sprintf(&string[n], "-%s ", name()); return n; };
  inline U32 get_decompress_selective() const { return (one->get_decompress_selective() | two->get_decompress_selective()); };
  inline BOOL filter(const LASpoint* point) { return one->filter(point) && two->filter(point); };
  LAScriterionAnd(LAScriterion* one, LAScriterion* two) { this->one = one; this->two = two; };
private:
  LAScriterion* one;
  LAScriterion* two;
};

class LAScriterionOr : public LAScriterion
{
public:
  inline const CHAR* name() const { return "filter_or"; };
  inline I32 get_command(CHAR* string) const { int n = 0; n += one->get_command(&string[n]); n += two->get_command(&string[n]); n += sprintf(&string[n], "-%s ", name()); return n; };
  inline U32 get_decompress_selective() const { return (one->get_decompress_selective() | two->get_decompress_selective()); };
  inline BOOL filter(const LASpoint* point) { return one->filter(point) || two->filter(point); };
  LAScriterionOr(LAScriterion* one, LAScriterion* two) { this->one = one; this->two = two; };
private:
  LAScriterion* one;
  LAScriterion* two;
};

class LAScriterionKeepTile : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_tile"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), ll_x, ll_y, tile_size); };
  inline BOOL filter(const LASpoint* point) { return (!point->inside_tile(ll_x, ll_y, ur_x, ur_y)); };
  LAScriterionKeepTile(F32 ll_x, F32 ll_y, F32 tile_size) { this->ll_x = ll_x; this->ll_y = ll_y; this->ur_x = ll_x+tile_size; this->ur_y = ll_y+tile_size; this->tile_size = tile_size; };
private:
  F32 ll_x, ll_y, ur_x, ur_y, tile_size;
};

class LAScriterionKeepCircle : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_circle"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf %lf ", name(), center_x, center_y, radius); };
  inline BOOL filter(const LASpoint* point) { return (!point->inside_circle(center_x, center_y, radius_squared)); };
  LAScriterionKeepCircle(F64 x, F64 y, F64 radius) { this->center_x = x; this->center_y = y; this->radius = radius; this->radius_squared = radius*radius; };
private:
  F64 center_x, center_y, radius, radius_squared;
};

class LAScriterionKeepxyz : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_xyz"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf %lf %lf %lf %lf ", name(), min_x, min_y, min_z, max_x, max_y, max_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY | LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (!point->inside_box(min_x, min_y, min_z, max_x, max_y, max_z)); };
  LAScriterionKeepxyz(F64 min_x, F64 min_y, F64 min_z, F64 max_x, F64 max_y, F64 max_z) { this->min_x = min_x; this->min_y = min_y; this->min_z = min_z; this->max_x = max_x; this->max_y = max_y; this->max_z = max_z; };
private:
  F64 min_x, min_y, min_z, max_x, max_y, max_z;
};

class LAScriterionDropxyz : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_xyz"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf %lf %lf %lf %lf ", name(), min_x, min_y, min_z, max_x, max_y, max_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY | LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->inside_box(min_x, min_y, min_z, max_x, max_y, max_z)); };
  LAScriterionDropxyz(F64 min_x, F64 min_y, F64 min_z, F64 max_x, F64 max_y, F64 max_z) { this->min_x = min_x; this->min_y = min_y; this->min_z = min_z; this->max_x = max_x; this->max_y = max_y; this->max_z = max_z; };
private:
  F64 min_x, min_y, min_z, max_x, max_y, max_z;
};

class LAScriterionKeepxy : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_xy"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf %lf %lf ", name(), below_x, below_y, above_x, above_y); };
  inline BOOL filter(const LASpoint* point) { return (!point->inside_rectangle(below_x, below_y, above_x, above_y)); };
  LAScriterionKeepxy(F64 below_x, F64 below_y, F64 above_x, F64 above_y) { this->below_x = below_x; this->below_y = below_y; this->above_x = above_x; this->above_y = above_y; };
private:
  F64 below_x, below_y, above_x, above_y;
};

class LAScriterionDropxy : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_xy"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf %lf %lf ", name(), below_x, below_y, above_x, above_y); };
  inline BOOL filter(const LASpoint* point) { return (point->inside_rectangle(below_x, below_y, above_x, above_y)); };
  LAScriterionDropxy(F64 below_x, F64 below_y, F64 above_x, F64 above_y) { this->below_x = below_x; this->below_y = below_y; this->above_x = above_x; this->above_y = above_y; };
private:
  F64 below_x, below_y, above_x, above_y;
};

class LAScriterionKeepx : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_x"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_x, above_x); };
  inline BOOL filter(const LASpoint* point) { F64 x = point->get_x(); return (x < below_x) || (x >= above_x); };
  LAScriterionKeepx(F64 below_x, F64 above_x) { this->below_x = below_x; this->above_x = above_x; };
private:
  F64 below_x, above_x;
};

class LAScriterionDropx : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_x"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_x, above_x); };
  inline BOOL filter(const LASpoint* point) { F64 x = point->get_x(); return ((below_x <= x) && (x < above_x)); };
  LAScriterionDropx(F64 below_x, F64 above_x) { this->below_x = below_x; this->above_x = above_x; };
private:
  F64 below_x, above_x;
};

class LAScriterionKeepy : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_y"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_y, above_y); };
  inline BOOL filter(const LASpoint* point) { F64 y = point->get_y(); return (y < below_y) || (y >= above_y); };
  LAScriterionKeepy(F64 below_y, F64 above_y) { this->below_y = below_y; this->above_y = above_y; };
private:
  F64 below_y, above_y;
};

class LAScriterionDropy : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_y"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_y, above_y); };
  inline BOOL filter(const LASpoint* point) { F64 y = point->get_y(); return ((below_y <= y) && (y < above_y)); };
  LAScriterionDropy(F64 below_y, F64 above_y) { this->below_y = below_y; this->above_y = above_y; };
private:
  F64 below_y, above_y;
};

class LAScriterionKeepz : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_z"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_z, above_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { F64 z = point->get_z(); return (z < below_z) || (z >= above_z); };
  LAScriterionKeepz(F64 below_z, F64 above_z) { this->below_z = below_z; this->above_z = above_z; };
private:
  F64 below_z, above_z;
};

class LAScriterionDropz : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_z"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_z, above_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { F64 z = point->get_z(); return ((below_z <= z) && (z < above_z)); };
  LAScriterionDropz(F64 below_z, F64 above_z) { this->below_z = below_z; this->above_z = above_z; };
private:
  F64 below_z, above_z;
};

class LAScriterionDropxBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_x_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), below_x); };
  inline BOOL filter(const LASpoint* point) { return (point->get_x() < below_x); };
  LAScriterionDropxBelow(F64 below_x) { this->below_x = below_x; };
private:
  F64 below_x;
};

class LAScriterionDropxAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_x_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), above_x); };
  inline BOOL filter(const LASpoint* point) { return (point->get_x() >= above_x); };
  LAScriterionDropxAbove(F64 above_x) { this->above_x = above_x; };
private:
  F64 above_x;
};

class LAScriterionDropyBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_y_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), below_y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_y() < below_y); };
  LAScriterionDropyBelow(F64 below_y) { this->below_y = below_y; };
private:
  F64 below_y;
};

class LAScriterionDropyAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_y_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), above_y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_y() >= above_y); };
  LAScriterionDropyAbove(F64 above_y) { this->above_y = above_y; };
private:
  F64 above_y;
};

class LAScriterionDropzBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_z_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), below_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->get_z() < below_z); };
  LAScriterionDropzBelow(F64 below_z) { this->below_z = below_z; };
private:
  F64 below_z;
};

class LAScriterionDropzAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_z_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), above_z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->get_z() >= above_z); };
  LAScriterionDropzAbove(F64 above_z) { this->above_z = above_z; };
private:
  F64 above_z;
};

class LAScriterionKeepXY : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_XY"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d %d %d ", name(), below_X, below_Y, above_X, above_Y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_X() < below_X) || (point->get_Y() < below_Y) || (point->get_X() >= above_X) || (point->get_Y() >= above_Y); };
  LAScriterionKeepXY(I32 below_X, I32 below_Y, I32 above_X, I32 above_Y) { this->below_X = below_X; this->below_Y = below_Y; this->above_X = above_X; this->above_Y = above_Y; };
private:
  I32 below_X, below_Y, above_X, above_Y;
};

class LAScriterionKeepX : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_X"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_X, above_X); };
  inline BOOL filter(const LASpoint* point) { return (point->get_X() < below_X) || (above_X <= point->get_X()); };
  LAScriterionKeepX(I32 below_X, I32 above_X) { this->below_X = below_X; this->above_X = above_X; };
private:
  I32 below_X, above_X;
};

class LAScriterionDropX : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_X"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_X, above_X); };
  inline BOOL filter(const LASpoint* point) { return ((below_X <= point->get_X()) && (point->get_X() < above_X)); };
  LAScriterionDropX(I32 below_X, I32 above_X) { this->below_X = below_X; this->above_X = above_X; };
private:
  I32 below_X;
  I32 above_X;
};

class LAScriterionKeepY : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_Y"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_Y, above_Y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_Y() < below_Y) || (above_Y <= point->get_Y()); };
  LAScriterionKeepY(I32 below_Y, I32 above_Y) { this->below_Y = below_Y; this->above_Y = above_Y; };
private:
  I32 below_Y, above_Y;
};

class LAScriterionDropY : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Y"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_Y, above_Y); };
  inline BOOL filter(const LASpoint* point) { return ((below_Y <= point->get_Y()) && (point->get_Y() < above_Y)); };
  LAScriterionDropY(I32 below_Y, I32 above_Y) { this->below_Y = below_Y; this->above_Y = above_Y; };
private:
  I32 below_Y;
  I32 above_Y;
};

class LAScriterionKeepZ : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_Z"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_Z, above_Z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->get_Z() < below_Z) || (above_Z <= point->get_Z()); };
  LAScriterionKeepZ(I32 below_Z, I32 above_Z) { this->below_Z = below_Z; this->above_Z = above_Z; };
private:
  I32 below_Z, above_Z;
};

class LAScriterionDropZ : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Z"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_Z, above_Z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return ((below_Z <= point->get_Z()) && (point->get_Z() < above_Z)); };
  LAScriterionDropZ(I32 below_Z, I32 above_Z) { this->below_Z = below_Z; this->above_Z = above_Z; };
private:
  I32 below_Z;
  I32 above_Z;
};

class LAScriterionDropXBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_X_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_X); };
  inline BOOL filter(const LASpoint* point) { return (point->get_X() < below_X); };
  LAScriterionDropXBelow(I32 below_X) { this->below_X = below_X; };
private:
  I32 below_X;
};

class LAScriterionDropXAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_X_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_X); };
  inline BOOL filter(const LASpoint* point) { return (point->get_X() >= above_X); };
  LAScriterionDropXAbove(I32 above_X) { this->above_X = above_X; };
private:
  I32 above_X;
};

class LAScriterionDropYBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Y_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_Y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_Y() < below_Y); };
  LAScriterionDropYBelow(I32 below_Y) { this->below_Y = below_Y; };
private:
  I32 below_Y;
};

class LAScriterionDropYAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Y_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_Y); };
  inline BOOL filter(const LASpoint* point) { return (point->get_Y() >= above_Y); };
  LAScriterionDropYAbove(I32 above_Y) { this->above_Y = above_Y; };
private:
  I32 above_Y;
};

class LAScriterionDropZBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Z_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_Z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->get_Z() < below_Z); };
  LAScriterionDropZBelow(I32 below_Z) { this->below_Z = below_Z; };
private:
  I32 below_Z;
};

class LAScriterionDropZAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_Z_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_Z); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_Z; };
  inline BOOL filter(const LASpoint* point) { return (point->get_Z() >= above_Z); };
  LAScriterionDropZAbove(I32 above_Z) { this->above_Z = above_Z; };
private:
  I32 above_Z;
};

class LAScriterionKeepFirstReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_first"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->return_number > 1); };
};

class LAScriterionKeepFirstOfManyReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_first_of_many"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->number_of_returns == 1) || (point->return_number > 1)); };
};

class LAScriterionKeepMiddleReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_middle"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->return_number == 1) || (point->return_number >= point->number_of_returns)); };
};

class LAScriterionKeepLastReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_last"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->return_number < point->number_of_returns); };
};

class LAScriterionKeepLastOfManyReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_last_of_many"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->return_number == 1) || (point->return_number < point->number_of_returns)); };
};

class LAScriterionKeepSecondLast : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_second_last"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->number_of_returns <= 1) || (point->return_number != (point->number_of_returns-1))); };
};

class LAScriterionDropFirstReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_first"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->return_number == 1); };
};

class LAScriterionDropFirstOfManyReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_first_of_many"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->number_of_returns > 1) && (point->return_number == 1)); };
};

class LAScriterionDropMiddleReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_middle"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->return_number > 1) && (point->return_number < point->number_of_returns)); };
};

class LAScriterionDropLastReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_last"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->return_number >= point->number_of_returns); };
};

class LAScriterionDropLastOfManyReturn : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_last_of_many"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->number_of_returns > 1) && (point->return_number >= point->number_of_returns)); };
};

class LAScriterionDropSecondLast : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_second_last"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return ((point->number_of_returns > 1) && (point->return_number == (point->number_of_returns-1))); };
};

class LAScriterionKeepReturns : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_return_mask"; };
  inline I32 get_command(CHAR* string) const { 
    U32 i;
    U32 n = sprintf(string, "-keep_return ");
    U16 keep_return_mask = ~drop_return_mask;
    for (i = 0; i < 16; i++) if ((1 << i) & keep_return_mask) n += sprintf(string + n, "%u ", i);
    return n;
  };
  inline BOOL filter(const LASpoint* point) { return ((1 << point->get_return_number()) & drop_return_mask); };
  LAScriterionKeepReturns(U16 keep_return_mask) { drop_return_mask = ~keep_return_mask; };
  inline U16 get_keep_return_mask() const { return ~drop_return_mask; };
private:
  U16 drop_return_mask;
};

class LAScriterionDropReturns : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_return_mask"; };
  inline I32 get_command(CHAR* string) const { 
    U32 i;
    U32 n = sprintf(string, "-drop_return ");
    for (i = 0; i < 16; i++) if ((1 << i) & drop_return_mask) n += sprintf(string + n, "%u ", i);
    return n;
  };
  inline BOOL filter(const LASpoint* point) { return ((1 << point->get_return_number()) & drop_return_mask); };
  LAScriterionDropReturns(U16 drop_return_mask) { this->drop_return_mask = drop_return_mask; };
  inline U16 get_drop_return_mask() const { return drop_return_mask; };
private:
  U16 drop_return_mask;
};

class LAScriterionKeepSpecificNumberOfReturns : public LAScriterion
{
public:
  inline const CHAR* name() const { return (number_of_returns == 1 ? "keep_single" : (number_of_returns == 2 ? "keep_double" : (number_of_returns == 3 ? "keep_triple" : (number_of_returns == 4 ? "keep_quadruple" : "keep_quintuple")))); };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->get_number_of_returns() != number_of_returns); };
  LAScriterionKeepSpecificNumberOfReturns(U32 number_of_returns) { this->number_of_returns = number_of_returns; };
private:
  U32 number_of_returns;
};

class LAScriterionDropSpecificNumberOfReturns : public LAScriterion
{
public:
  inline const CHAR* name() const { return (number_of_returns == 1 ? "drop_single" : (number_of_returns == 2 ? "drop_double" : (number_of_returns == 3 ? "drop_triple" : (number_of_returns == 4 ? "drop_quadruple" : "drop_quintuple")))); };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline BOOL filter(const LASpoint* point) { return (point->get_number_of_returns() == number_of_returns); };
  LAScriterionDropSpecificNumberOfReturns(U32 number_of_returns) { this->number_of_returns = number_of_returns; };
private:
  U32 number_of_returns;
};

class LAScriterionDropScanDirection : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_scan_direction"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), scan_direction); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (scan_direction == point->scan_direction_flag); };
  LAScriterionDropScanDirection(I32 scan_direction) { this->scan_direction = scan_direction; };
private:
  I32 scan_direction;
};

class LAScriterionKeepScanDirectionChange : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_scan_direction_change"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { if (scan_direction_flag == point->scan_direction_flag) return TRUE; I32 s = scan_direction_flag; scan_direction_flag = point->scan_direction_flag; return s == -1; };
  void reset() { scan_direction_flag = -1; };
  LAScriterionKeepScanDirectionChange() { reset(); };
private:
  I32 scan_direction_flag;
};

class LAScriterionKeepEdgeOfFlightLine : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_edge_of_flight_line"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->edge_of_flight_line == 0); };
};

class LAScriterionKeepScannerChannel : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_scanner_channel"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), scanner_channel); };
  inline BOOL filter(const LASpoint* point) { return (point->get_extended_scanner_channel() != scanner_channel); };
  LAScriterionKeepScannerChannel(I32 scanner_channel) { this->scanner_channel = scanner_channel; };
private:
  I32 scanner_channel;
};

class LAScriterionDropScannerChannel : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_scanner_channel"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), scanner_channel); };
  inline BOOL filter(const LASpoint* point) { return (point->get_extended_scanner_channel() == scanner_channel); };
  LAScriterionDropScannerChannel(I32 scanner_channel) { this->scanner_channel = scanner_channel; };
private:
  I32 scanner_channel;
};

class LAScriterionKeepRGB : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_RGB"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s_%s %d %d ", name(), (channel == 0 ? "red" : (channel == 1 ? "green" : (channel == 2 ? "blue" : "nir"))),  below_RGB, above_RGB); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_RGB; };
  inline BOOL filter(const LASpoint* point) { return ((point->rgb[channel] < below_RGB) || (above_RGB < point->rgb[channel])); };
  LAScriterionKeepRGB(I32 below_RGB, I32 above_RGB, I32 channel) { if (above_RGB < below_RGB) { this->below_RGB = above_RGB; this->above_RGB = below_RGB; } else { this->below_RGB = below_RGB; this->above_RGB = above_RGB; }; this->channel = channel; };
private:
  I32 below_RGB, above_RGB, channel;
};

class LAScriterionDropRGB : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_RGB"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s_%s %d %d ", name(), (channel == 0 ? "red" : (channel == 1 ? "green" : (channel == 2 ? "blue" : "nir"))),  below_RGB, above_RGB); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_RGB; };
  inline BOOL filter(const LASpoint* point) { return ((below_RGB <= point->rgb[channel]) && (point->rgb[channel] <= above_RGB)); };
  LAScriterionDropRGB(I32 below_RGB, I32 above_RGB, I32 channel) { if (above_RGB < below_RGB) { this->below_RGB = above_RGB; this->above_RGB = below_RGB; } else { this->below_RGB = below_RGB; this->above_RGB = above_RGB; }; this->channel = channel; };
private:
  I32 below_RGB, above_RGB, channel;
};

class LAScriterionKeepNDVI : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_NDVI"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s%s %g %g ", name(), (NIR == 3 ? "" : (NIR == 1 ? "_green_is_NIR" : "_blue_is_NIR")),  below_NDVI, above_NDVI); };
  inline U32 get_decompress_selective() const { if (NIR == 3) return LASZIP_DECOMPRESS_SELECTIVE_RGB | LASZIP_DECOMPRESS_SELECTIVE_NIR; else return LASZIP_DECOMPRESS_SELECTIVE_RGB; };
  inline BOOL filter(const LASpoint* point)
  { 
    F32 NDVI = ((F32)(point->rgb[NIR] - point->get_R())) / ((F32)(point->rgb[NIR] + point->get_R()));
    return (NDVI < below_NDVI) || (above_NDVI < NDVI);
  };
  LAScriterionKeepNDVI(F32 below_NDVI, F32 above_NDVI, I32 NIR) { if (above_NDVI < below_NDVI) { this->below_NDVI = above_NDVI; this->above_NDVI = below_NDVI; } else { this->below_NDVI = below_NDVI; this->above_NDVI = above_NDVI; }; this->NIR = NIR; };
private:
  F32 below_NDVI, above_NDVI;
  I32 NIR;
};

class LAScriterionKeepNDVIfromCIR : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_NDVI_from_CIR"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(),  below_NDVI, above_NDVI); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_RGB; };
  inline BOOL filter(const LASpoint* point)
  { 
    F32 NDVI = ((F32)(point->get_R() - point->get_G())) / ((F32)(point->get_R() + point->get_G()));
    return (NDVI < below_NDVI) || (above_NDVI < NDVI);
  };
  LAScriterionKeepNDVIfromCIR(F32 below_NDVI, F32 above_NDVI) { if (above_NDVI < below_NDVI) { this->below_NDVI = above_NDVI; this->above_NDVI = below_NDVI; } else { this->below_NDVI = below_NDVI; this->above_NDVI = above_NDVI; }; };
private:
  F32 below_NDVI, above_NDVI;
};

class LAScriterionKeepNDVIintensityIsNIR : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_NDVI_intensity_is_NIR"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(),  below_NDVI, above_NDVI); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_RGB | LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point)
  { 
    F32 NDVI = ((F32)(point->get_intensity() - point->get_R())) / ((F32)(point->get_intensity() + point->get_R()));
    return (NDVI < below_NDVI) || (above_NDVI < NDVI);
  };
  LAScriterionKeepNDVIintensityIsNIR(F32 below_NDVI, F32 above_NDVI) { if (above_NDVI < below_NDVI) { this->below_NDVI = above_NDVI; this->above_NDVI = below_NDVI; } else { this->below_NDVI = below_NDVI; this->above_NDVI = above_NDVI; }; };
private:
  F32 below_NDVI, above_NDVI;
};

class LAScriterionKeepScanAngle : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_scan_angle"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_scan, above_scan); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE; };
  inline BOOL filter(const LASpoint* point) { return (point->scan_angle_rank < below_scan) || (above_scan < point->scan_angle_rank); };
  LAScriterionKeepScanAngle(I32 below_scan, I32 above_scan) { if (above_scan < below_scan) { this->below_scan = above_scan; this->above_scan = below_scan; } else { this->below_scan = below_scan; this->above_scan = above_scan; } };
private:
  I32 below_scan, above_scan;
};

class LAScriterionDropScanAngleBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_scan_angle_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_scan); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE; };
  inline BOOL filter(const LASpoint* point) { return (point->scan_angle_rank < below_scan); };
  LAScriterionDropScanAngleBelow(I32 below_scan) { this->below_scan = below_scan; };
private:
  I32 below_scan;
};

class LAScriterionDropScanAngleAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_scan_angle_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_scan); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE; };
  inline BOOL filter(const LASpoint* point) { return (point->scan_angle_rank > above_scan); };
  LAScriterionDropScanAngleAbove(I32 above_scan) { this->above_scan = above_scan; };
private:
  I32 above_scan;
};

class LAScriterionDropScanAngleBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_scan_angle_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_scan, above_scan); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_SCAN_ANGLE; };
  inline BOOL filter(const LASpoint* point) { return (below_scan <= point->scan_angle_rank) && (point->scan_angle_rank <= above_scan); };
  LAScriterionDropScanAngleBetween(I32 below_scan, I32 above_scan) { if (above_scan < below_scan) { this->below_scan = above_scan; this->above_scan = below_scan; } else { this->below_scan = below_scan; this->above_scan = above_scan; } };
private:
  I32 below_scan, above_scan;
};

class LAScriterionKeepIntensity : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_intensity"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_intensity, above_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (point->get_intensity() < below_intensity) || (point->get_intensity() > above_intensity); };
  LAScriterionKeepIntensity(U16 below_intensity, U16 above_intensity) { this->below_intensity = below_intensity; this->above_intensity = above_intensity; };
private:
  U16 below_intensity, above_intensity;
};

class LAScriterionKeepIntensityBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_intensity_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (point->get_intensity() >= below_intensity); };
  LAScriterionKeepIntensityBelow(U16 below_intensity) { this->below_intensity = below_intensity; };
private:
  U16 below_intensity;
};

class LAScriterionKeepIntensityAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_intensity_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (point->get_intensity() <= above_intensity); };
  LAScriterionKeepIntensityAbove(U16 above_intensity) { this->above_intensity = above_intensity; };
private:
  U16 above_intensity;
};

class LAScriterionDropIntensityBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_intensity_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (point->get_intensity() < below_intensity); };
  LAScriterionDropIntensityBelow(I32 below_intensity) { this->below_intensity = below_intensity; };
private:
  I32 below_intensity;
};

class LAScriterionDropIntensityAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_intensity_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (point->get_intensity() > above_intensity); };
  LAScriterionDropIntensityAbove(I32 above_intensity) { this->above_intensity = above_intensity; };
private:
  I32 above_intensity;
};

class LAScriterionDropIntensityBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_intensity_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_intensity, above_intensity); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY; };
  inline BOOL filter(const LASpoint* point) { return (below_intensity <= point->get_intensity()) && (point->get_intensity() <= above_intensity); };
  LAScriterionDropIntensityBetween(I32 below_intensity, I32 above_intensity) { this->below_intensity = below_intensity; this->above_intensity = above_intensity; };
private:
  I32 below_intensity, above_intensity;
};

class LAScriterionKeepClassifications : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_classification_mask"; };
  inline I32 get_command(CHAR* string) const { 
    U32 i;
    U32 n = sprintf(string, "-keep_class ");
    U32 keep_classification_mask = ~drop_classification_mask;
    for (i = 0; i < 32; i++) if ((1u << i) & keep_classification_mask) n += sprintf(string + n, "%u ", i);
    return n;
  };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION; };
  inline BOOL filter(const LASpoint* point) { return ((1u << point->classification) & drop_classification_mask); };
  LAScriterionKeepClassifications(U32 keep_classification_mask) { drop_classification_mask = ~keep_classification_mask; };
  inline U32 get_keep_classification_mask() const { return ~drop_classification_mask; };
private:
  U32 drop_classification_mask;
};

class LAScriterionDropClassifications : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_classification_mask"; };
  inline I32 get_command(CHAR* string) const { 
    U32 i;
    U32 n = sprintf(string, "-drop_class ");
    for (i = 0; i < 32; i++) if ((1 << i) & drop_classification_mask) n += sprintf(string + n, "%u ", i);
    return n;
  };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION; };
  inline BOOL filter(const LASpoint* point) { return ((1 << point->classification) & drop_classification_mask); };
  LAScriterionDropClassifications(U32 drop_classification_mask) { this->drop_classification_mask = drop_classification_mask; };
  inline U32 get_drop_classification_mask() const { return drop_classification_mask; };
private:
  U32 drop_classification_mask;
};

class LAScriterionDropExtendedClassifications : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_extended_classification_mask"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %u %u %u %u %u %u %u ", name(), drop_extended_classification_mask[7], drop_extended_classification_mask[6], drop_extended_classification_mask[5], drop_extended_classification_mask[4], drop_extended_classification_mask[3], drop_extended_classification_mask[2], drop_extended_classification_mask[1], drop_extended_classification_mask[0]); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION; };
  inline BOOL filter(const LASpoint* point) { return ((1 << (point->extended_classification - (32 * (point->extended_classification / 32)))) & drop_extended_classification_mask[point->extended_classification / 32]); };
  LAScriterionDropExtendedClassifications(U32 drop_extended_classification_mask[8]) { for (I32 i = 0; i < 8; i++) this->drop_extended_classification_mask[i] = drop_extended_classification_mask[i]; };
private:
  U32 drop_extended_classification_mask[8];
};

class LAScriterionDropSynthetic : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_synthetic"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_synthetic_flag() == 1); };
};

class LAScriterionKeepSynthetic : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_synthetic"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_synthetic_flag() == 0); };
};

class LAScriterionDropKeypoint : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_keypoint"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_keypoint_flag() == 1); };
};

class LAScriterionKeepKeypoint : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_keypoint"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_keypoint_flag() == 0); };
};

class LAScriterionDropWithheld : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_withheld"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_withheld_flag() == 1); };
};

class LAScriterionKeepWithheld : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_withheld"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_withheld_flag() == 0); };
};

class LAScriterionDropOverlap : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_overlap"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_extended_overlap_flag() == 1); };
};

class LAScriterionKeepOverlap : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_overlap"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_FLAGS; };
  inline BOOL filter(const LASpoint* point) { return (point->get_extended_overlap_flag() == 0); };
};

class LAScriterionKeepUserData : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_user_data"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data != user_data); };
  LAScriterionKeepUserData(U8 user_data) { this->user_data = user_data; };
private:
  U8 user_data;
};

class LAScriterionKeepUserDataBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_user_data_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data >= below_user_data); };
  LAScriterionKeepUserDataBelow(U8 below_user_data) { this->below_user_data = below_user_data; };
private:
  U8 below_user_data;
};

class LAScriterionKeepUserDataAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_user_data_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data <= above_user_data); };
  LAScriterionKeepUserDataAbove(U8 above_user_data) { this->above_user_data = above_user_data; };
private:
  U8 above_user_data;
};

class LAScriterionKeepUserDataBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_user_data_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_user_data, above_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data < below_user_data) || (above_user_data < point->user_data); };
  LAScriterionKeepUserDataBetween(U8 below_user_data, U8 above_user_data) { this->below_user_data = below_user_data; this->above_user_data = above_user_data; };
private:
  U8 below_user_data, above_user_data;
};

class LAScriterionDropUserData : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_user_data"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data == user_data); };
  LAScriterionDropUserData(U8 user_data) { this->user_data = user_data; };
private:
  U8 user_data;
};

class LAScriterionDropUserDataBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_user_data_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data < below_user_data) ; };
  LAScriterionDropUserDataBelow(U8 below_user_data) { this->below_user_data = below_user_data; };
private:
  U8 below_user_data;
};

class LAScriterionDropUserDataAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_user_data_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (point->user_data > above_user_data); };
  LAScriterionDropUserDataAbove(U8 above_user_data) { this->above_user_data = above_user_data; };
private:
  U8 above_user_data;
};

class LAScriterionDropUserDataBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_user_data_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_user_data, above_user_data); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_USER_DATA; };
  inline BOOL filter(const LASpoint* point) { return (below_user_data <= point->user_data) && (point->user_data <= above_user_data); };
  LAScriterionDropUserDataBetween(U8 below_user_data, U8 above_user_data) { this->below_user_data = below_user_data; this->above_user_data = above_user_data; };
private:
  U8 below_user_data, above_user_data;
};

class LAScriterionKeepPointSource : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_point_source"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (point->get_point_source_ID() != point_source_id); };
  LAScriterionKeepPointSource(U16 point_source_id) { this->point_source_id = point_source_id; };
private:
  U16 point_source_id;
};

class LAScriterionKeepPointSourceBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_point_source_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_point_source_id, above_point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (point->get_point_source_ID() < below_point_source_id) || (above_point_source_id < point->get_point_source_ID()); };
  LAScriterionKeepPointSourceBetween(U16 below_point_source_id, U16 above_point_source_id) { this->below_point_source_id = below_point_source_id; this->above_point_source_id = above_point_source_id; };
private:
  U16 below_point_source_id, above_point_source_id;
};

class LAScriterionDropPointSource : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_point_source"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (point->get_point_source_ID() == point_source_id) ; };
  LAScriterionDropPointSource(U16 point_source_id) { this->point_source_id = point_source_id; };
private:
  U16 point_source_id;
};

class LAScriterionDropPointSourceBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_point_source_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), below_point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (point->get_point_source_ID() < below_point_source_id) ; };
  LAScriterionDropPointSourceBelow(U16 below_point_source_id) { this->below_point_source_id = below_point_source_id; };
private:
  U16 below_point_source_id;
};

class LAScriterionDropPointSourceAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_point_source_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), above_point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (point->get_point_source_ID() > above_point_source_id); };
  LAScriterionDropPointSourceAbove(U16 above_point_source_id) { this->above_point_source_id = above_point_source_id; };
private:
  U16 above_point_source_id;
};

class LAScriterionDropPointSourceBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_point_source_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below_point_source_id, above_point_source_id); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_POINT_SOURCE; };
  inline BOOL filter(const LASpoint* point) { return (below_point_source_id <= point->get_point_source_ID()) && (point->get_point_source_ID() <= above_point_source_id); };
  LAScriterionDropPointSourceBetween(U16 below_point_source_id, U16 above_point_source_id) { this->below_point_source_id = below_point_source_id; this->above_point_source_id = above_point_source_id; };
private:
  U16 below_point_source_id, above_point_source_id;
};

class LAScriterionKeepGpsTime : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_gps_time"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_gpstime, above_gpstime); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point) { return (point->have_gps_time && ((point->gps_time < below_gpstime) || (point->gps_time > above_gpstime))); };
  LAScriterionKeepGpsTime(F64 below_gpstime, F64 above_gpstime) { this->below_gpstime = below_gpstime; this->above_gpstime = above_gpstime; };
private:
  F64 below_gpstime, above_gpstime;
};

class LAScriterionDropGpsTimeBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_gps_time_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), below_gpstime); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point) { return (point->have_gps_time && (point->gps_time < below_gpstime)); };
  LAScriterionDropGpsTimeBelow(F64 below_gpstime) { this->below_gpstime = below_gpstime; };
private:
  F64 below_gpstime;
};

class LAScriterionDropGpsTimeAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_gps_time_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), above_gpstime); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point) { return (point->have_gps_time && (point->gps_time > above_gpstime)); };
  LAScriterionDropGpsTimeAbove(F64 above_gpstime) { this->above_gpstime = above_gpstime; };
private:
  F64 above_gpstime;
};

class LAScriterionDropGpsTimeBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_gps_time_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf %lf ", name(), below_gpstime, above_gpstime); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point) { return (point->have_gps_time && ((below_gpstime <= point->gps_time) && (point->gps_time <= above_gpstime))); };
  LAScriterionDropGpsTimeBetween(F64 below_gpstime, F64 above_gpstime) { this->below_gpstime = below_gpstime; this->above_gpstime = above_gpstime; };
private:
  F64 below_gpstime, above_gpstime;
};

class LAScriterionKeepWavepacket : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_wavepacket"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), keep_wavepacket); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_WAVEPACKET; };
  inline BOOL filter(const LASpoint* point) { return (point->wavepacket.getIndex() != keep_wavepacket); };
  LAScriterionKeepWavepacket(U32 keep_wavepacket) { this->keep_wavepacket = keep_wavepacket; };
private:
  U32 keep_wavepacket;
};

class LAScriterionDropWavepacket : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_wavepacket"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), drop_wavepacket); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_WAVEPACKET; };
  inline BOOL filter(const LASpoint* point) { return (point->wavepacket.getIndex() == drop_wavepacket); };
  LAScriterionDropWavepacket(U32 drop_wavepacket) { this->drop_wavepacket = drop_wavepacket; };
private:
  U32 drop_wavepacket;
};

class LAScriterionKeepAttributeBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_attribute_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf ", name(), index, below_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { return (point->get_attribute_as_float(index) >= below_attribute); };
  LAScriterionKeepAttributeBelow(U32 index, F64 below_attribute) { this->index = index; this->below_attribute = below_attribute; };
private:
  U32 index;
  F64 below_attribute;
};

class LAScriterionKeepAttributeAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_attribute_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf ", name(), index, above_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { return (point->get_attribute_as_float(index) <= above_attribute); };
  LAScriterionKeepAttributeAbove(U32 index, F64 above_attribute) { this->index = index; this->above_attribute = above_attribute; };
private:
  U32 index;
  F64 above_attribute;
};

class LAScriterionKeepAttributeBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_attribute_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf %lf ", name(), index, below_attribute, above_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { F64 attribute = point->get_attribute_as_float(index); return (attribute < below_attribute) || (above_attribute < attribute); };
  LAScriterionKeepAttributeBetween(U32 index, F64 below_attribute, F64 above_attribute) { this->index = index; this->below_attribute = below_attribute; this->above_attribute = above_attribute; };
private:
  U32 index;
  F64 below_attribute;
  F64 above_attribute;
};

class LAScriterionDropAttributeBelow : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_attribute_below"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf ", name(), index, below_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { return (point->get_attribute_as_float(index) < below_attribute); };
  LAScriterionDropAttributeBelow(U32 index, F64 below_attribute) { this->index = index; this->below_attribute = below_attribute; };
private:
  U32 index;
  F64 below_attribute;
};

class LAScriterionDropAttributeAbove : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_attribute_above"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf ", name(), index, above_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { return (point->get_attribute_as_float(index) > above_attribute); };
  LAScriterionDropAttributeAbove(U32 index, F64 above_attribute) { this->index = index; this->above_attribute = above_attribute; };
private:
  U32 index;
  F64 above_attribute;
};

class LAScriterionDropAttributeBetween : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_attribute_between"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u %lf %lf ", name(), index, below_attribute, above_attribute); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_EXTRA_BYTES; };
  inline BOOL filter(const LASpoint* point) { F64 attribute = point->get_attribute_as_float(index); return (below_attribute <= attribute) && (attribute <= above_attribute); };
  LAScriterionDropAttributeBetween(U32 index, F64 below_attribute, F64 above_attribute) { this->index = index; this->below_attribute = below_attribute; this->above_attribute = above_attribute; };
private:
  U32 index;
  F64 below_attribute;
  F64 above_attribute;
};

class LAScriterionKeepEveryNth : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_every_nth"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), every); };
  inline BOOL filter(const LASpoint* point) { if (counter == every) { counter = 1; return FALSE; } else { counter++; return TRUE; } };
  LAScriterionKeepEveryNth(U32 every) { this->every = every; counter = 1; };
private:
  U32 counter;
  U32 every;
};

class LAScriterionDropEveryNth : public LAScriterion
{
public:
  inline const CHAR* name() const { return "drop_every_nth"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), every); };
  inline BOOL filter(const LASpoint* point) { if (counter == every) { counter = 1; return TRUE; } else { counter++; return FALSE; } };
  LAScriterionDropEveryNth(U32 every) { this->every = every; counter = 1; };
private:
  U32 counter;
  U32 every;
};

class LAScriterionKeepRandomFraction : public LAScriterion
{
public:
  inline const CHAR* name() const { return "keep_random_fraction"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), fraction); };
  inline BOOL filter(const LASpoint* point)
  {
    srand(seed);
    seed = rand();
    return ((F32)seed/(F32)RAND_MAX) > fraction;
  };
  void reset() { seed = requested_seed; };
  LAScriterionKeepRandomFraction(F32 fraction) { requested_seed = seed = 0; this->fraction = fraction; };
  LAScriterionKeepRandomFraction(U32 seed, F32 fraction) { requested_seed = this->seed = seed; this->fraction = fraction; };
private:
  U32 requested_seed;
  U32 seed;
  F32 fraction;
};

class LAScriterionThinWithGrid : public LAScriterion
{
public:
  inline const CHAR* name() const { return "thin_with_grid"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), (grid_spacing > 0 ? grid_spacing : -grid_spacing)); };
  inline BOOL filter(const LASpoint* point)
  { 
    if (grid_spacing < 0)
    {
      grid_spacing = -grid_spacing;
      anker = I32_FLOOR(point->get_y() / grid_spacing);
    }
    I32 pos_x = I32_FLOOR(point->get_x() / grid_spacing);
    I32 pos_y = I32_FLOOR(point->get_y() / grid_spacing) - anker;
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
    if ((*array)[pos_y][pos_x_pos] & pos_x_bit) return TRUE;
    (*array)[pos_y][pos_x_pos] |= pos_x_bit;
    return FALSE;
  }
  void reset()
  {
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
  };
  LAScriterionThinWithGrid(F32 grid_spacing)
  {
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
  };
  ~LAScriterionThinWithGrid() { reset(); };
private:
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
};

class LAScriterionThinPulsesWithTime : public LAScriterion
{
public:
  inline const CHAR* name() const { return "thin_pulses_with_time"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), (time_spacing > 0 ? time_spacing : -time_spacing)); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point)
  { 
    I64 pos_t = I64_FLOOR(point->get_gps_time() / time_spacing);
    my_I64_F64_map::iterator map_element = times.find(pos_t);
    if (map_element == times.end())
    {
      times.insert(my_I64_F64_map::value_type(pos_t, point->get_gps_time()));
      return FALSE;
    }
    else if ((*map_element).second == point->get_gps_time())
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
  void reset()
  {
    times.clear();
  };
  LAScriterionThinPulsesWithTime(F64 time_spacing)
  {
    this->time_spacing = time_spacing;
  };
  ~LAScriterionThinPulsesWithTime() { reset(); };
private:
  F64 time_spacing;
  my_I64_F64_map times;
};

class LAScriterionThinPointsWithTime : public LAScriterion
{
public:
  inline const CHAR* name() const { return "thin_points_with_time"; };
  inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s %lf ", name(), (time_spacing > 0 ? time_spacing : -time_spacing)); };
  inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_GPS_TIME; };
  inline BOOL filter(const LASpoint* point)
  { 
    I64 pos_t = I64_FLOOR(point->get_gps_time() / time_spacing);
    my_I64_set::iterator map_element = times.find(pos_t);
    if (map_element == times.end())
    {
      times.insert(my_I64_set::value_type(pos_t));
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
  void reset()
  {
    times.clear();
  };
  LAScriterionThinPointsWithTime(F64 time_spacing)
  {
    this->time_spacing = time_spacing;
  };
  ~LAScriterionThinPointsWithTime() { reset(); };
private:
  F64 time_spacing;
  my_I64_set times;
};

void LASfilter::clean()
{
  U32 i;
  for (i = 0; i < num_criteria; i++)
  {
    delete criteria[i];
  }
  if (criteria) delete [] criteria;
  if (counters) delete [] counters;
  alloc_criteria = 0;
  num_criteria = 0;
  criteria = 0;
  counters = 0;
}

void LASfilter::usage() const
{
  fprintf(stderr,"Filter points based on their coordinates.\n");
  fprintf(stderr,"  -keep_tile 631000 4834000 1000 (ll_x ll_y size)\n");
  fprintf(stderr,"  -keep_circle 630250.00 4834750.00 100 (x y radius)\n");
  fprintf(stderr,"  -keep_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)\n");
  fprintf(stderr,"  -drop_xy 630000 4834000 631000 4836000 (min_x min_y max_x max_y)\n");
  fprintf(stderr,"  -keep_x 631500.50 631501.00 (min_x max_x)\n");
  fprintf(stderr,"  -drop_x 631500.50 631501.00 (min_x max_x)\n");
  fprintf(stderr,"  -drop_x_below 630000.50 (min_x)\n");
  fprintf(stderr,"  -drop_x_above 630500.50 (max_x)\n");
  fprintf(stderr,"  -keep_y 4834500.25 4834550.25 (min_y max_y)\n");
  fprintf(stderr,"  -drop_y 4834500.25 4834550.25 (min_y max_y)\n");
  fprintf(stderr,"  -drop_y_below 4834500.25 (min_y)\n");
  fprintf(stderr,"  -drop_y_above 4836000.75 (max_y)\n");
  fprintf(stderr,"  -keep_z 11.125 130.725 (min_z max_z)\n");
  fprintf(stderr,"  -drop_z 11.125 130.725 (min_z max_z)\n");
  fprintf(stderr,"  -drop_z_below 11.125 (min_z)\n");
  fprintf(stderr,"  -drop_z_above 130.725 (max_z)\n");
  fprintf(stderr,"  -keep_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)\n");
  fprintf(stderr,"  -drop_xyz 620000 4830000 100 621000 4831000 200 (min_x min_y min_z max_x max_y max_z)\n");
  fprintf(stderr,"Filter points based on their return numbering.\n");
  fprintf(stderr,"  -keep_first -first_only -drop_first\n");
  fprintf(stderr,"  -keep_last -last_only -drop_last\n");
  fprintf(stderr,"  -keep_second_last -drop_second_last\n");
  fprintf(stderr,"  -keep_first_of_many -keep_last_of_many\n");
  fprintf(stderr,"  -drop_first_of_many -drop_last_of_many\n");
  fprintf(stderr,"  -keep_middle -drop_middle\n");
  fprintf(stderr,"  -keep_return 1 2 3\n");
  fprintf(stderr,"  -drop_return 3 4\n");
  fprintf(stderr,"  -keep_single -drop_single\n");
  fprintf(stderr,"  -keep_double -drop_double\n");
  fprintf(stderr,"  -keep_triple -drop_triple\n");
  fprintf(stderr,"  -keep_quadruple -drop_quadruple\n");
  fprintf(stderr,"  -keep_number_of_returns 5\n");
  fprintf(stderr,"  -drop_number_of_returns 0\n");
  fprintf(stderr,"Filter points based on the scanline flags.\n");
  fprintf(stderr,"  -drop_scan_direction 0\n");
  fprintf(stderr,"  -keep_scan_direction_change\n");
  fprintf(stderr,"  -keep_edge_of_flight_line\n");
  fprintf(stderr,"Filter points based on their intensity.\n");
  fprintf(stderr,"  -keep_intensity 20 380\n");
  fprintf(stderr,"  -drop_intensity_below 20\n");
  fprintf(stderr,"  -drop_intensity_above 380\n");
  fprintf(stderr,"  -drop_intensity_between 4000 5000\n");
  fprintf(stderr,"Filter points based on classifications or flags.\n");
  fprintf(stderr,"  -keep_class 1 3 7\n");
  fprintf(stderr,"  -drop_class 4 2\n");
  fprintf(stderr,"  -keep_extended_class 43\n");
  fprintf(stderr,"  -drop_extended_class 129 135\n");
  fprintf(stderr,"  -drop_synthetic -keep_synthetic\n");
  fprintf(stderr,"  -drop_keypoint -keep_keypoint\n");
  fprintf(stderr,"  -drop_withheld -keep_withheld\n");
  fprintf(stderr,"  -drop_overlap -keep_overlap\n");
  fprintf(stderr,"Filter points based on their user data.\n");
  fprintf(stderr,"  -keep_user_data 1\n");
  fprintf(stderr,"  -drop_user_data 255\n");
  fprintf(stderr,"  -keep_user_data_below 50\n");
  fprintf(stderr,"  -keep_user_data_above 150\n");
  fprintf(stderr,"  -keep_user_data_between 10 20\n");
  fprintf(stderr,"  -drop_user_data_below 1\n");
  fprintf(stderr,"  -drop_user_data_above 100\n");
  fprintf(stderr,"  -drop_user_data_between 10 40\n");
  fprintf(stderr,"Filter points based on their point source ID.\n");
  fprintf(stderr,"  -keep_point_source 3\n");
  fprintf(stderr,"  -keep_point_source_between 2 6\n");
  fprintf(stderr,"  -drop_point_source 27\n");
  fprintf(stderr,"  -drop_point_source_below 6\n");
  fprintf(stderr,"  -drop_point_source_above 15\n");
  fprintf(stderr,"  -drop_point_source_between 17 21\n");
  fprintf(stderr,"Filter points based on their scan angle.\n");
  fprintf(stderr,"  -keep_scan_angle -15 15\n");
  fprintf(stderr,"  -drop_abs_scan_angle_above 15\n");
  fprintf(stderr,"  -drop_abs_scan_angle_below 1\n");
  fprintf(stderr,"  -drop_scan_angle_below -15\n");
  fprintf(stderr,"  -drop_scan_angle_above 15\n");
  fprintf(stderr,"  -drop_scan_angle_between -25 -23\n");
  fprintf(stderr,"Filter points based on their gps time.\n");
  fprintf(stderr,"  -keep_gps_time 11.125 130.725\n");
  fprintf(stderr,"  -drop_gps_time_below 11.125\n");
  fprintf(stderr,"  -drop_gps_time_above 130.725\n");
  fprintf(stderr,"  -drop_gps_time_between 22.0 48.0\n");
  fprintf(stderr,"Filter points based on their RGB/CIR/NIR channels.\n");
  fprintf(stderr,"  -keep_RGB_red 1 1\n");
  fprintf(stderr,"  -drop_RGB_red 5000 20000\n");
  fprintf(stderr,"  -keep_RGB_green 30 100\n");
  fprintf(stderr,"  -drop_RGB_green 2000 10000\n");
  fprintf(stderr,"  -keep_RGB_blue 0 0\n");
  fprintf(stderr,"  -keep_RGB_nir 64 127\n");
  fprintf(stderr,"  -keep_NDVI 0.2 0.7 -keep_NDVI_from_CIR -0.1 0.5\n");
  fprintf(stderr,"  -keep_NDVI_intensity_is_NIR 0.4 0.8 -keep_NDVI_green_is_NIR -0.2 0.2\n");
  fprintf(stderr,"Filter points based on their wavepacket.\n");
  fprintf(stderr,"  -keep_wavepacket 0\n");
  fprintf(stderr,"  -drop_wavepacket 3\n");
  fprintf(stderr,"Filter points based on extra attributes.\n");
  fprintf(stderr,"  -keep_attribute_above 0 5.0\n");
  fprintf(stderr,"  -drop_attribute_below 1 1.5\n");
  fprintf(stderr,"Filter points with simple thinning.\n");
  fprintf(stderr,"  -keep_every_nth 2 -drop_every_nth 3\n");
  fprintf(stderr,"  -keep_random_fraction 0.1\n");
  fprintf(stderr,"  -keep_random_fraction 0.1 4711\n");
  fprintf(stderr,"  -thin_with_grid 1.0\n");
  fprintf(stderr,"  -thin_pulses_with_time 0.0001\n");
  fprintf(stderr,"  -thin_points_with_time 0.000001\n");
  fprintf(stderr,"Boolean combination of filters.\n");
  fprintf(stderr,"  -filter_and\n");
}

BOOL LASfilter::parse(int argc, char* argv[])
{
  int i;

  U32 keep_extended_classification_mask[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  U32 drop_extended_classification_mask[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      usage();
      return TRUE;
    }
    else if (strncmp(argv[i],"-clip_", 6) == 0)
    {
      if (strcmp(argv[i], "-clip_z_below") == 0)
      {
        fprintf(stderr,"WARNING: '%s' will not be supported in the future. check documentation with '-h'.\n", argv[i]);
        fprintf(stderr,"  rename '-clip_z_below' to '-drop_z_below'.\n");
        fprintf(stderr,"  rename '-clip_z_above' to '-drop_z_above'.\n");
        fprintf(stderr,"  rename '-clip_z_between' to '-drop_z'.\n");
        fprintf(stderr,"  rename '-clip' to '-keep_xy'.\n");
        fprintf(stderr,"  rename '-clip_tile' to '-keep_tile'.\n");
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionDropzBelow(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i], "-clip_z_above") == 0)
      {
        fprintf(stderr,"WARNING: '%s' will not be supported in the future. check documentation with '-h'.\n", argv[i]);
        fprintf(stderr,"  rename '-clip_z_below' to '-drop_z_below'.\n");
        fprintf(stderr,"  rename '-clip_z_above' to '-drop_z_above'.\n");
        fprintf(stderr,"  rename '-clip_z_between' to '-drop_z'.\n");
        fprintf(stderr,"  rename '-clip' to '-keep_xy'.\n");
        fprintf(stderr,"  rename '-clip_tile' to '-keep_tile'.\n");
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionDropzAbove(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if ((strcmp(argv[i], "-clip_to_bounding_box") != 0) && (strcmp(argv[i],"-clip_to_bb") != 0))
      {
        fprintf(stderr,"ERROR: '%s' is no longer recognized. check documentation with '-h'.\n", argv[i]);
        fprintf(stderr,"  rename '-clip' to '-keep_xy'.\n");
        fprintf(stderr,"  rename '-clip_box' to '-keep_xyz'.\n");
        fprintf(stderr,"  rename '-clip_tile' to '-keep_tile'.\n");
        fprintf(stderr,"  rename '-clip_z_below' to '-drop_z_below'.\n");
        fprintf(stderr,"  rename '-clip_z_above' to '-drop_z_above'.\n");
        fprintf(stderr,"  rename '-clip_z_between' to '-drop_z'.\n");
        fprintf(stderr,"  etc ...\n");
        return FALSE;
      }
    }
    else if (strncmp(argv[i],"-keep_", 6) == 0)
    {
      if (strncmp(argv[i],"-keep_first", 11) == 0)
      {
        if (strcmp(argv[i],"-keep_first") == 0)
        {
          add_criterion(new LAScriterionKeepFirstReturn());
          *argv[i]='\0';
        }
        else if (strcmp(argv[i],"-keep_first_of_many") == 0)
        {
          add_criterion(new LAScriterionKeepFirstOfManyReturn());
          *argv[i]='\0';
        }
      }
      else if (strncmp(argv[i],"-keep_last", 10) == 0)
      {
        if (strcmp(argv[i],"-keep_last") == 0)
        {
          add_criterion(new LAScriterionKeepLastReturn());
          *argv[i]='\0';
        }
        else if (strcmp(argv[i],"-keep_last_of_many") == 0)
        {
          add_criterion(new LAScriterionKeepLastOfManyReturn());
          *argv[i]='\0';
        }
      }
      else if (strcmp(argv[i],"-keep_middle") == 0)
      {
        add_criterion(new LAScriterionKeepMiddleReturn());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_second_last") == 0)
      {
        add_criterion(new LAScriterionKeepSecondLast());
        *argv[i]='\0';
      }
      else if (strncmp(argv[i],"-keep_class", 11) == 0)
      {
        if (strcmp(argv[i],"-keep_classification") == 0 || strcmp(argv[i],"-keep_class") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: classification\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 classification;
          U32 keep_classification_mask = 0;
          do
          {
            if (sscanf(argv[i], "%u", &classification) != 1)
            {
              fprintf(stderr,"ERROR: '-keep_classification' needs at least 1 argument: classification but '%s' is no valid classification\n", argv[i]);
              return FALSE;
            }
            if (classification > 31)
            {
              fprintf(stderr,"ERROR: cannot keep classification %u because it is larger than 31\n", classification);
              return FALSE;
            }
            keep_classification_mask |= (1u << classification);
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
          // if last criteria was also keep_class filter then combine them
          if (num_criteria && (strcmp(criteria[num_criteria-1]->name(), "keep_classification_mask") == 0))
          {
            LAScriterionKeepClassifications* criterion = (LAScriterionKeepClassifications*)(criteria[num_criteria-1]);
            num_criteria--;
            keep_classification_mask |= criterion->get_keep_classification_mask();
            delete criterion;
          }
          add_criterion(new LAScriterionKeepClassifications(keep_classification_mask));
        }
        else if (strcmp(argv[i],"-keep_classification_mask") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: mask\n", argv[i]);
            return FALSE;
          }
          U32 keep_classification_mask;
          if (sscanf(argv[i+1], "%u", &keep_classification_mask) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: mask but '%s' is no valid mask\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepClassifications(keep_classification_mask));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-keep_extended_", 15) == 0)
      {
        if (strcmp(argv[i],"-keep_extended_classification") == 0 || strcmp(argv[i],"-keep_extended_class") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least a argument: classification\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 extended_classification;
          do
          {
            if (sscanf(argv[i], "%u", &extended_classification) != 1)
            {
              fprintf(stderr,"ERROR: '-keep_extended_classification' needs at least 1 argument: classification but '%s' is no valid classification\n", argv[i]);
              return FALSE;
            }
            if (extended_classification > 255)
            {
              fprintf(stderr,"ERROR: cannot keep extended classification %u because it is larger than 255\n", extended_classification);
              return FALSE;
            }
            keep_extended_classification_mask[extended_classification/32] |= (1u << (extended_classification%32));
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
        }
      }
      else if (strncmp(argv[i],"-keep_x", 7) == 0)
      {
        if (strcmp(argv[i],"-keep_xy") == 0)
        {
          if ((i+4) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+2], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid min_y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+3], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid max_x\n", argv[i], argv[i+3]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+4], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid max_y\n", argv[i], argv[i+4]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepxy(min_x, min_y, max_x, max_y));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4; 
        }
        else if (strcmp(argv[i],"-keep_xyz") == 0)
        {
          if ((i+6) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+2], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+3], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_z\n", argv[i], argv[i+3]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+4], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_x\n", argv[i], argv[i+4]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+5], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_y\n", argv[i], argv[i+5]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+6], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_z\n", argv[i], argv[i+6]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepxyz(min_x, min_y, min_z, max_x, max_y, max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; i+=6; 
        }
        else if (strcmp(argv[i],"-keep_x") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+2], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x but '%s' is no valid max_x\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepx(min_x, max_x));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strcmp(argv[i],"-keep_y") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y\n", argv[i]);
          return FALSE;
        }
        F64 min_y;
        if (sscanf(argv[i+1], "%lf", &min_y) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y but '%s' is no valid min_y\n", argv[i], argv[i+1]);
          return FALSE;
        }
        F64 max_y;
        if (sscanf(argv[i+2], "%lf", &max_y) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y but '%s' is no valid max_y\n", argv[i], argv[i+2]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepy(min_y, max_y));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
      }
      else if (strncmp(argv[i],"-keep_z", 7) == 0)
      {
        if (strcmp(argv[i],"-keep_z") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z\n", argv[i]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+1], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z but '%s' is no valid min_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+2], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z but '%s' is no valid max_z\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepz(min_z, max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-keep_z_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_z\n", argv[i]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+1], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_z but '%s' is no valid min_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropzBelow(min_z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_z_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z\n", argv[i]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+1], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z but '%s' is no valid max_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropzAbove(max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-keep_X", 7) == 0)
      {
        if (strcmp(argv[i],"-keep_XY") == 0)
        {
          if ((i+4) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_X min_Y max_X max_Y\n", argv[i]);
            return FALSE;
          }
          I32 min_X;
          if (sscanf(argv[i+1], "%d", &min_X) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_X min_Y max_X max_Y but '%s' is no valid min_X\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 min_Y;
          if (sscanf(argv[i+2], "%d", &min_Y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_X min_Y max_X max_Y but '%s' is no valid min_Y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          I32 max_X;
          if (sscanf(argv[i+3], "%d", &max_X) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_X min_Y max_X max_Y but '%s' is no valid max_X\n", argv[i], argv[i+3]);
            return FALSE;
          }
          I32 max_Y;
          if (sscanf(argv[i+4], "%d", &max_Y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_X min_Y max_X max_Y but '%s' is no valid max_Y\n", argv[i], argv[i+4]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepXY(min_X, min_Y, max_X, max_Y));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4; 
        }
        else if (strcmp(argv[i],"-keep_X") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_X max_X\n", argv[i]);
            return FALSE;
          }
          I32 min_X;
          if (sscanf(argv[i+1], "%d", &min_X) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_X max_X but '%s' is no valid min_X\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max_X;
          if (sscanf(argv[i+2], "%d", &max_X) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_X max_X but '%s' is no valid max_X\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepX(min_X, max_X));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
        }
      }
      else if (strcmp(argv[i],"-keep_Y") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Y max_Y\n", argv[i]);
          return FALSE;
        }
        I32 min_Y;
        if (sscanf(argv[i+1], "%d", &min_Y) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Y max_Y but '%s' is no valid min_Y\n", argv[i], argv[i+1]);
          return FALSE;
        }
        I32 max_Y;
        if (sscanf(argv[i+2], "%d", &max_Y) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Y max_Y but '%s' is no valid max_Y\n", argv[i], argv[i+2]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepY(min_Y, max_Y));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strncmp(argv[i],"-keep_Z", 7) == 0)
      {
        if (strcmp(argv[i],"-keep_Z") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Z max_Z\n", argv[i]);
            return FALSE;
          }
          I32 min_Z;
          if (sscanf(argv[i+1], "%d", &min_Z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Z max_Z but '%s' is no valid min_Z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max_Z;
          if (sscanf(argv[i+2], "%d", &max_Z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Z max_Z but '%s' is no valid max_Z\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepZ(min_Z, max_Z));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
        }
        else if (strcmp(argv[i],"-keep_Z_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_Z\n", argv[i]);
            return FALSE;
          }
          I32 min_Z;
          if (sscanf(argv[i+1], "%d", &min_Z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_Z but '%s' is no valid min_Z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropZBelow(min_Z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_Z_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_Z\n", argv[i]);
            return FALSE;
          }
          I32 max_Z;
          if (sscanf(argv[i+1], "%d", &max_Z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_Z but '%s' is no valid max_Z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropZAbove(max_Z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strcmp(argv[i],"-keep_tile") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: llx lly size\n", argv[i]);
          return FALSE;
        }
        F32 llx;
        if (sscanf(argv[i+1], "%f", &llx) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: llx lly size but '%s' is no valid llx\n", argv[i], argv[i+1]);
          return FALSE;
        }
        F32 lly;
        if (sscanf(argv[i+2], "%f", &lly) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: llx lly size but '%s' is no valid lly\n", argv[i], argv[i+2]);
          return FALSE;
        }
        F32 size;
        if (sscanf(argv[i+3], "%f", &size) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: llx lly size but '%s' is no valid size\n", argv[i], argv[i+3]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepTile(llx, lly, size));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-keep_circle") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: center_x center_y radius\n", argv[i]);
          return FALSE;
        }
        F64 center_x;
        if (sscanf(argv[i+1], "%lf", &center_x) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: center_x center_y radius but '%s' is no valid center_x\n", argv[i], argv[i+1]);
          return FALSE;
        }
        F64 center_y;
        if (sscanf(argv[i+2], "%lf", &center_y) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: center_x center_y radius but '%s' is no valid center_y\n", argv[i], argv[i+2]);
          return FALSE;
        }
        F64 radius;
        if (sscanf(argv[i+3], "%lf", &radius) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: center_x center_y radius but '%s' is no valid radius\n", argv[i], argv[i+3]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepCircle(center_x, center_y, radius));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
      }
      else if (strncmp(argv[i],"-keep_return", 12) == 0)
      {
        if (strcmp(argv[i],"-keep_return") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: return_number\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 return_number;
          U16 keep_return_mask = 0;
          do
          {
            if (sscanf(argv[i], "%u", &return_number) != 1)
            {
              fprintf(stderr,"ERROR: '-keep_return' needs at least 1 argument: return_number but '%s' is no valid return_number\n", argv[i]);
              return FALSE;
            }
            if (return_number > 15)
            {
              fprintf(stderr,"ERROR: cannot keep return_number %u because it is larger than 15\n", return_number);
              return FALSE;
            }
            keep_return_mask |= (1u << return_number);
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
          // if last criteria was also keep_return filter then combine them
          if (num_criteria && (strcmp(criteria[num_criteria-1]->name(), "keep_return_mask") == 0))
          {
            LAScriterionKeepReturns* criterion = (LAScriterionKeepReturns*)(criteria[num_criteria-1]);
            num_criteria--;
            keep_return_mask |= criterion->get_keep_return_mask();
            delete criterion;
          }
          add_criterion(new LAScriterionKeepReturns(keep_return_mask));
        }
        else if (strcmp(argv[i],"-keep_return_mask") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: return_mask\n", argv[i]);
            return FALSE;
          }
          U32 keep_return_mask;
          if (sscanf(argv[i+1], "%u", &keep_return_mask) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: return_mask but '%s' is no valid return_mask\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (keep_return_mask > U16_MAX)
          {
            fprintf(stderr,"ERROR: '%s' needs a return_mask between 0 and %u but return_mask is %u\n", argv[i], U16_MAX, keep_return_mask);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepReturns((U16)keep_return_mask));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strcmp(argv[i],"-keep_number_of_returns") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number_of_returns\n", argv[i]);
          return FALSE;
        }
        U32 number_of_returns;
        if (sscanf(argv[i+1], "%u", &number_of_returns) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number_of_returns but '%s' is no valid number_of_returns\n", argv[i], argv[i+1]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(number_of_returns));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-keep_single") == 0)
      {
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(1));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_double") == 0)
      {
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(2));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_triple") == 0)
      {
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(3));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_quadruple") == 0)
      {
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(4));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_quintuple") == 0)
      {
        add_criterion(new LAScriterionKeepSpecificNumberOfReturns(5));
        *argv[i]='\0';
      }
      else if (strncmp(argv[i],"-keep_intensity", 15) == 0)
      {
        if (strcmp(argv[i],"-keep_intensity") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          U32 min;
          if (sscanf(argv[i+1], "%u", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          U32 max;
          if (sscanf(argv[i+2], "%u", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          if (min > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep intensity because min of %u is larger than %u\n", min, U16_MAX);
            return FALSE;
          }
          if (max > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep intensity because max of %u is larger than %u\n", max, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepIntensity((U16)min, (U16)max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-keep_intensity_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
            return FALSE;
          }
          U32 max;
          if (sscanf(argv[i+1], "%u", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max but '%s' is no valid max\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (max > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep intensity above max of %u because it is larger than %u\n", max, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepIntensityAbove((U16)max));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_intensity_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min\n", argv[i]);
            return FALSE;
          }
          U32 min;
          if (sscanf(argv[i+1], "%u", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (min > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep intensity below min of %u because it is larger than %u\n", min, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepIntensityBelow((U16)min));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-keep_RGB_", 10) == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
          return FALSE;
        }
        I32 min;
        if (sscanf(argv[i+1], "%d", &min) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
          return FALSE;
        }
        I32 max;
        if (sscanf(argv[i+2], "%d", &max) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
          return FALSE;
        }
        if (strcmp(argv[i]+10,"red") == 0)
        {
          add_criterion(new LAScriterionKeepRGB(min, max, 0));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"green") == 0)
        {
          add_criterion(new LAScriterionKeepRGB(min, max, 1));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"blue") == 0)
        {
          add_criterion(new LAScriterionKeepRGB(min, max, 2));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"nir") == 0)
        {
          add_criterion(new LAScriterionKeepRGB(min, max, 3));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-keep_NDVI", 10) == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
          return FALSE;
        }
        F32 min;
        if (sscanf(argv[i+1], "%f", &min) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
          return FALSE;
        }
        F32 max;
        if (sscanf(argv[i+2], "%f", &max) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
          return FALSE;
        }
        if (strcmp(argv[i]+10,"") == 0)
        {
          add_criterion(new LAScriterionKeepNDVI(min, max, 3));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"_from_CIR") == 0)
        {
          add_criterion(new LAScriterionKeepNDVIfromCIR(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"_green_is_NIR") == 0)
        {
          add_criterion(new LAScriterionKeepNDVI(min, max, 1));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"_blue_is_NIR") == 0)
        {
          add_criterion(new LAScriterionKeepNDVI(min, max, 2));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"_intensity_is_NIR") == 0)
        {
          add_criterion(new LAScriterionKeepNDVIintensityIsNIR(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-keep_scan_angle", 16) == 0)
      {
        if ((strcmp(argv[i],"-keep_scan_angle") == 0) || (strcmp(argv[i],"-keep_scan_angle_between") == 0))
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+2], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepScanAngle(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strcmp(argv[i],"-keep_synthetic") == 0)
      {
        add_criterion(new LAScriterionKeepSynthetic());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_keypoint") == 0)
      {
        add_criterion(new LAScriterionKeepKeypoint());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_withheld") == 0)
      {
        add_criterion(new LAScriterionKeepWithheld());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_overlap") == 0)
      {
        add_criterion(new LAScriterionKeepOverlap());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_wavepacket") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
          return FALSE;
        }
        *argv[i]='\0';
        i+=1;
        add_criterion(new LAScriterionKeepWavepacket(atoi(argv[i])));
        *argv[i]='\0';
      }
      else if (strncmp(argv[i],"-keep_user_data", 15) == 0)
      {
        if (strcmp(argv[i],"-keep_user_data") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
            return FALSE;
          }
          I32 value;
          if (sscanf(argv[i+1], "%d", &value) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value but '%s' is no valid value\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepUserData(value));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_user_data_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
            return FALSE;
          }
          I32 value;
          if (sscanf(argv[i+1], "%d", &value) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value but '%s' is no valid value\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepUserDataBelow(value));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_user_data_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
            return FALSE;
          }
          I32 value;
          if (sscanf(argv[i+1], "%d", &value) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: value but '%s' is no valid value\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepUserDataAbove(value));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_user_data_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+2], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepUserDataBetween(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-keep_point_source", 18) == 0)
      {
        if (strcmp(argv[i],"-keep_point_source") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: ID\n", argv[i]);
            return FALSE;
          }
          U32 ID;
          if (sscanf(argv[i+1], "%u", &ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: ID but '%s' is no valid ID\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep_point_source ID %u because it is larger than %u\n", ID, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepPointSource(ID));
          while (((i+2) < argc) && ('0' <= argv[i+2][0]) && (argv[i+2][0] <= '9'))
          {
            *argv[i]='\0';
            if (sscanf(argv[i+2], "%d", &ID) != 1)
            {
              fprintf(stderr,"ERROR: '-keep_point_source' takes one or more IDs but '%s' is no valid ID\n", argv[i+2]);
              return FALSE;
            }
            if (ID > U16_MAX)
            {
              fprintf(stderr,"ERROR: cannot keep_point_source ID %u because it is larger than %u\n", ID, U16_MAX);
              return FALSE;
            }
            add_criterion(new LAScriterionKeepPointSource(ID));
            LAScriterion* filter_criterion = new LAScriterionAnd(criteria[num_criteria-2], criteria[num_criteria-1]);
            num_criteria--;
            criteria[num_criteria] = 0;
            num_criteria--;
            criteria[num_criteria] = 0;
            add_criterion(filter_criterion);
            i+=1;
          } 
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-keep_point_source_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID\n", argv[i]);
            return FALSE;
          }
          U32 min_ID;
          if (sscanf(argv[i+1], "%u", &min_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID but '%s' is no valid min_ID\n", argv[i], argv[i+1]);
            return FALSE;
          }
          U32 max_ID;
          if (sscanf(argv[i+2], "%u", &max_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID but '%s' is no valid max_ID\n", argv[i], argv[i+2]);
            return FALSE;
          }
          if (min_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep point source because min_ID of %u is larger than %u\n", min_ID, U16_MAX);
            return FALSE;
          }
          if (max_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot keep point source because max_ID of %u is larger than %u\n", max_ID, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepPointSourceBetween((U16)min_ID, (U16)max_ID));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-keep_gps", 9) == 0)
      {
        if (strcmp(argv[i],"-keep_gps_time") == 0 || strcmp(argv[i],"-keep_gpstime") == 0 || strcmp(argv[i],"-keep_gps_time_between") == 0 || strcmp(argv[i],"-keep_gpstime_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: start end\n", argv[i]);
            return FALSE;
          }
          F64 start = 0.0;
          if (sscanf(argv[i+1], "%lf", &start) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: start end but '%s' is no valid start\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 end = 0.0;
          if (sscanf(argv[i+2], "%lf", &end) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: start end but '%s' is no valid end\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepGpsTime(start, end));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-keep_gps_time_above") == 0 || strcmp(argv[i],"-keep_gpstime_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: time\n", argv[i]);
            return FALSE;
          }
          F64 time = 0.0;
          if (sscanf(argv[i+1], "%lf", &time) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: time but '%s' is no valid time\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropGpsTimeBelow(time));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
         }
        else if (strcmp(argv[i],"-keep_gps_time_below") == 0 || strcmp(argv[i],"-keep_gpstime_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: time\n", argv[i]);
            return FALSE;
          }
          F64 time = 0.0;
          if (sscanf(argv[i+1], "%lf", &time) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: time but '%s' is no valid time\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropGpsTimeAbove(time));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-keep_attribute", 15) == 0)
      {
        if (strcmp(argv[i],"-keep_attribute_below") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value\n", argv[i]);
            return FALSE;
          }
          I32 index;
          if (sscanf(argv[i+1], "%d", &index) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value but '%s' is no valid index\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 value;
          if (sscanf(argv[i+2], "%lf", &value) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid value\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepAttributeBelow(index, value));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-keep_attribute_above") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value\n", argv[i]);
            return FALSE;
          }
          I32 index;
          if (sscanf(argv[i+1], "%d", &index) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value but '%s' is no valid index\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 value;
          if (sscanf(argv[i+2], "%lf", &value) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid value\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepAttributeAbove(index, value));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-keep_attribute_between") == 0)
        {
          if ((i+3) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index min max\n", argv[i]);
            return FALSE;
          }
          I32 index;
          if (sscanf(argv[i+1], "%d", &index) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index min max but '%s' is no valid index\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 min;
          if (sscanf(argv[i+2], "%lf", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index min max but '%s' is no valid min\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 max;
          if (sscanf(argv[i+3], "%lf", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index min max but '%s' is no valid max\n", argv[i], argv[i+3]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepAttributeBetween(index, min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
        }
      }
      else if (strcmp(argv[i],"-keep_every_nth") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: nth\n", argv[i]);
          return FALSE;
        }
        U32 nth;
        if (sscanf(argv[i+1], "%u", &nth) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: nth but '%s' is no valid nth\n", argv[i], argv[i+1]);
          return FALSE;
        }
        if (nth == 0)
        {
          fprintf(stderr,"ERROR: %u is no valid nth for '%s'\n", nth, argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepEveryNth(nth));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-keep_random_fraction") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: fraction\n", argv[i]);
          return FALSE;
        }
        F32 fraction;
        if (sscanf(argv[i+1], "%f", &fraction) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs at least 1 argument: fraction [seed] but '%s' is no valid fraction\n", argv[i], argv[i+1]);
          return FALSE;
        }
        if (((i+2) < argc) && ('0' <= argv[i+2][0]) && (argv[i+2][0] <= '9')) // maybe a seed was specified
        {
          U32 seed;
          if (sscanf(argv[i+2], "%u", &seed) != 1)
          {
            fprintf(stderr,"ERROR: '%s' takes seed as second argument but '%s' is no valid seed\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepRandomFraction(seed, fraction));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else // no seed was specified
        {
          add_criterion(new LAScriterionKeepRandomFraction(fraction));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strcmp(argv[i],"-keep_scan_direction_change") == 0)
      {
        add_criterion(new LAScriterionKeepScanDirectionChange());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_edge_of_flight_line") == 0)
      {
        add_criterion(new LAScriterionKeepEdgeOfFlightLine());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-keep_scanner_channel") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          return FALSE;
        }
        I32 number;
        if (sscanf(argv[i+1], "%d", &number) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number but '%s' is no valid number\n", argv[i], argv[i+1]);
          return FALSE;
        }
        if ((number < 0) || (number > 3))
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number between 0 and 3 so '%s' is not valid\n", argv[i], argv[i+1]);
          return FALSE;
        }
        add_criterion(new LAScriterionKeepScannerChannel(number));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
    }
    else if (strncmp(argv[i],"-drop_", 6) == 0)
    {
      if (strncmp(argv[i],"-drop_first", 11) == 0)
      {
        if (strcmp(argv[i],"-drop_first") == 0)
        {
          add_criterion(new LAScriterionDropFirstReturn());
          *argv[i]='\0';
        }
        else if (strcmp(argv[i],"-drop_first_of_many") == 0)
        {
          add_criterion(new LAScriterionDropFirstOfManyReturn());
          *argv[i]='\0';
        }
      }
      else if (strncmp(argv[i],"-drop_last", 10) == 0)
      {
        if (strcmp(argv[i],"-drop_last") == 0)
        {
          add_criterion(new LAScriterionDropLastReturn());
          *argv[i]='\0';
        }
        else if (strcmp(argv[i],"-drop_last_of_many") == 0)
        {
          add_criterion(new LAScriterionDropLastOfManyReturn());
          *argv[i]='\0';
        }
      }
      else if (strcmp(argv[i],"-drop_middle") == 0)
      {
        add_criterion(new LAScriterionDropMiddleReturn());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_second_last") == 0)
      {
        add_criterion(new LAScriterionDropSecondLast());
        *argv[i]='\0';
      }
      else if (strncmp(argv[i],"-drop_class", 11) == 0)
      {
        if (strcmp(argv[i],"-drop_classification") == 0 || strcmp(argv[i],"-drop_class") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: classification\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 classification;
          U32 drop_classification_mask = 0;
          do
          {
            if (sscanf(argv[i], "%u", &classification) != 1)
            {
              fprintf(stderr,"ERROR: '-drop_classification' needs at least 1 argument: classification but '%s' is no valid classification\n", argv[i]);
              return FALSE;
            }
            if (classification > 31)
            {
              fprintf(stderr,"ERROR: cannot drop classification %u because it is larger than 31\n", classification);
              return FALSE;
            }
            drop_classification_mask |= (1u << classification);
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
          // if last criteria was also drop_class filter then combine them
          if (num_criteria && (strcmp(criteria[num_criteria-1]->name(), "drop_classification_mask") == 0))
          {
            LAScriterionDropClassifications* criterion = (LAScriterionDropClassifications*)(criteria[num_criteria-1]);
            num_criteria--;
            drop_classification_mask |= criterion->get_drop_classification_mask();
            delete criterion;
          }
          add_criterion(new LAScriterionDropClassifications(drop_classification_mask));
        }
        else if (strcmp(argv[i],"-drop_classification_mask") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: mask\n", argv[i]);
            return FALSE;
          }
          U32 drop_classification_mask;
          if (sscanf(argv[i+1], "%u", &drop_classification_mask) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: mask but '%s' is no valid mask\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropClassifications(drop_classification_mask));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_extended_", 15) == 0)
      {
        if (strcmp(argv[i],"-drop_extended_classification") == 0 || strcmp(argv[i],"-drop_extended_class") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: classification\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 extended_classification;
          do
          {
            if (sscanf(argv[i], "%u", &extended_classification) != 1)
            {
              fprintf(stderr,"ERROR: '-drop_extended_classification' needs at least 1 argument: classification but '%s' is no valid classification\n", argv[i]);
              return FALSE;
            }
            if (extended_classification > 255)
            {
              fprintf(stderr,"ERROR: cannot drop extended classification %u because it is larger than 255\n", extended_classification);
              return FALSE;
            }
            drop_extended_classification_mask[extended_classification/32] |= (1u << (extended_classification%32));
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
        }
        else if (strcmp(argv[i],"-drop_extended_classification_mask") == 0)
        {
          if ((i+8) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 8 arguments: mask7 mask6 mask5 mask4 mask3 mask2 mask1 mask0\n", argv[i]);
            return FALSE;
          }
          drop_extended_classification_mask[7] = atoi(argv[i+1]);
          drop_extended_classification_mask[6] = atoi(argv[i+2]);
          drop_extended_classification_mask[5] = atoi(argv[i+3]);
          drop_extended_classification_mask[4] = atoi(argv[i+4]);
          drop_extended_classification_mask[3] = atoi(argv[i+5]);
          drop_extended_classification_mask[2] = atoi(argv[i+6]);
          drop_extended_classification_mask[1] = atoi(argv[i+7]);
          drop_extended_classification_mask[0] = atoi(argv[i+8]);
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; *argv[i+7]='\0'; *argv[i+8]='\0'; i+=8;
        }
      }
      else if (strncmp(argv[i],"-drop_x", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_xy") == 0)
        {
          if ((i+4) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+2], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid min_y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+3], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid max_x\n", argv[i], argv[i+3]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+4], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: min_x min_y max_x max_y but '%s' is no valid max_y\n", argv[i], argv[i+4]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropxy(min_x, min_y, max_x, max_y));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4; 
        }
        else if (strcmp(argv[i],"-drop_xyz") == 0)
        {
          if ((i+6) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+2], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+3], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid min_z\n", argv[i], argv[i+3]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+4], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_x\n", argv[i], argv[i+4]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+5], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_y\n", argv[i], argv[i+5]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+6], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 6 arguments: min_x min_y min_z max_x max_y max_z but '%s' is no valid max_z\n", argv[i], argv[i+6]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropxyz(min_x, min_y, min_z, max_x, max_y, max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; *argv[i+5]='\0'; *argv[i+6]='\0'; i+=6; 
        }
        else if (strcmp(argv[i],"-drop_x") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+2], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_x max_x but '%s' is no valid max_x\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropx(min_x, max_x));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_x_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_x\n", argv[i]);
            return FALSE;
          }
          F64 min_x;
          if (sscanf(argv[i+1], "%lf", &min_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_x but '%s' is no valid min_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropxBelow(min_x));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_x_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_x\n", argv[i]);
            return FALSE;
          }
          F64 max_x;
          if (sscanf(argv[i+1], "%lf", &max_x) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_x but '%s' is no valid max_x\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropxAbove(max_x));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_y", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_y") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y\n", argv[i]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+1], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y but '%s' is no valid min_y\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+2], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_y max_y but '%s' is no valid max_y\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropy(min_y, max_y));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_y_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_y\n", argv[i]);
            return FALSE;
          }
          F64 min_y;
          if (sscanf(argv[i+1], "%lf", &min_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_y but '%s' is no valid min_y\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropyBelow(min_y));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_y_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_y\n", argv[i]);
            return FALSE;
          }
          F64 max_y;
          if (sscanf(argv[i+1], "%lf", &max_y) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_y but '%s' is no valid max_y\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropyAbove(max_y));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_z", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_z") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z\n", argv[i]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+1], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z but '%s' is no valid min_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+2], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_z max_z but '%s' is no valid max_z\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropz(min_z, max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_z_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_z\n", argv[i]);
            return FALSE;
          }
          F64 min_z;
          if (sscanf(argv[i+1], "%lf", &min_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_z but '%s' is no valid min_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropzBelow(min_z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_z_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z\n", argv[i]);
            return FALSE;
          }
          F64 max_z;
          if (sscanf(argv[i+1], "%lf", &max_z) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_z but '%s' is no valid max_z\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropzAbove(max_z));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_X", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_X") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_X max_X\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropX(atoi(argv[i+1]), atoi(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_X_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_X\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropXBelow(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_X_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_X\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropXAbove(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_Y", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_Y") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Y max_Y\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropY(atoi(argv[i+1]), atoi(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_Y_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_Y\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropYBelow(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_Y_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_Y\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropYAbove(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_Z", 7) == 0)
      {
        if (strcmp(argv[i],"-drop_Z") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_Z max_Z\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropZ(atoi(argv[i+1]), atoi(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_Z_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_Z\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropZBelow(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_Z_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_Z\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropZAbove(atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_return", 12) == 0)
      {
        if (strcmp(argv[i],"-drop_return") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: return_number\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          U32 return_number;
          U16 drop_return_mask = 0;
          do
          {
            if (sscanf(argv[i], "%u", &return_number) != 1)
            {
              fprintf(stderr,"ERROR: '-drop_return' needs at least 1 argument: return_number but '%s' is no valid return_number\n", argv[i]);
              return FALSE;
            }
            if (return_number > 15)
            {
              fprintf(stderr,"ERROR: cannot drop return_number %u because it is larger than 15\n", return_number);
              return FALSE;
            }
            drop_return_mask |= (1u << return_number);
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
          // if last criteria was also drop_return filter then combine them
          if (num_criteria && (strcmp(criteria[num_criteria-1]->name(), "drop_return_mask") == 0))
          {
            LAScriterionDropReturns* criterion = (LAScriterionDropReturns*)(criteria[num_criteria-1]);
            num_criteria--;
            drop_return_mask |= criterion->get_drop_return_mask();
            delete criterion;
          }
          add_criterion(new LAScriterionDropReturns(drop_return_mask));
        }
        else if (strcmp(argv[i],"-drop_return_mask") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: return_mask\n", argv[i]);
            return FALSE;
          }
          U32 drop_return_mask;
          if (sscanf(argv[i+1], "%u", &drop_return_mask) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: return_mask but '%s' is no valid return_mask\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (drop_return_mask > U16_MAX)
          {
            fprintf(stderr,"ERROR: '%s' needs a return_mask between 0 and %u but return_mask is %u\n", argv[i], U16_MAX, drop_return_mask);
            return FALSE;
          }
          add_criterion(new LAScriterionDropReturns((U16)drop_return_mask));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strcmp(argv[i],"-drop_number_of_returns") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number of returns\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-drop_single") == 0)
      {
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(1));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_double") == 0)
      {
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(2));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_triple") == 0)
      {
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(3));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_quadruple") == 0)
      {
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(4));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_quintuple") == 0)
      {
        add_criterion(new LAScriterionDropSpecificNumberOfReturns(5));
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_scan_direction") == 0)
      {
        add_criterion(new LAScriterionDropScanDirection(atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strncmp(argv[i],"-drop_intensity",15) == 0)
      {
        if (strcmp(argv[i],"-drop_intensity_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+1], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max but '%s' is no valid max\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropIntensityAbove(max));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_intensity_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropIntensityBelow(min));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_intensity_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+2], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropIntensityBetween(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-drop_RGB_", 10) == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
          return FALSE;
        }
        I32 min;
        if (sscanf(argv[i+1], "%d", &min) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
          return FALSE;
        }
        I32 max;
        if (sscanf(argv[i+2], "%d", &max) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
          return FALSE;
        }
        if (strcmp(argv[i]+10,"red") == 0)
        {
          add_criterion(new LAScriterionDropRGB(min, max, 0));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"green") == 0)
        {
          add_criterion(new LAScriterionDropRGB(min, max, 1));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"blue") == 0)
        {
          add_criterion(new LAScriterionDropRGB(min, max, 2));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i]+10,"nir") == 0)
        {
          add_criterion(new LAScriterionDropRGB(min, max, 3));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-drop_abs_scan_angle_",21) == 0)
      {
        if (strcmp(argv[i],"-drop_abs_scan_angle_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+1], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max but '%s' is no valid max\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionKeepScanAngle(-max, max));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_abs_scan_angle_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropScanAngleBetween(-min+1, min-1));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
      }
      else if (strncmp(argv[i],"-drop_scan_angle_",17) == 0)
      {
        if (strcmp(argv[i],"-drop_scan_angle_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+1], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max but '%s' is no valid max\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropScanAngleAbove(max));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_scan_angle_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropScanAngleBelow(min));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }    
        else if (strcmp(argv[i],"-drop_scan_angle_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+2], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropScanAngleBetween(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strcmp(argv[i],"-drop_synthetic") == 0)
      {
        add_criterion(new LAScriterionDropSynthetic());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_keypoint") == 0)
      {
        add_criterion(new LAScriterionDropKeypoint());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_withheld") == 0)
      {
        add_criterion(new LAScriterionDropWithheld());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_overlap") == 0)
      {
        add_criterion(new LAScriterionDropOverlap());
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-drop_wavepacket") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index\n", argv[i]);
          return FALSE;
        }
        *argv[i]='\0';
        i+=1;
        add_criterion(new LAScriterionDropWavepacket(atoi(argv[i])));
        *argv[i]='\0';
      }
      else if (strncmp(argv[i],"-drop_user_data", 15) == 0)
      {
        if (strcmp(argv[i],"-drop_user_data") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: value\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          do
          {
            add_criterion(new LAScriterionDropUserData(atoi(argv[i])));
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
        }
        else if (strcmp(argv[i],"-drop_user_data_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropUserDataBelow(min));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_user_data_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max\n", argv[i]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+1], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max but '%s' is no valid max\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropUserDataAbove(max));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_user_data_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          I32 min;
          if (sscanf(argv[i+1], "%d", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          I32 max;
          if (sscanf(argv[i+2], "%d", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropUserDataBetween(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-drop_point_source", 18) == 0)
      {
        if (strcmp(argv[i],"-drop_point_source") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs at least 1 argument: ID\n", argv[i]);
            return FALSE;
          }
          *argv[i]='\0';
          i+=1;
          do
          {
            U32 ID;
            if (sscanf(argv[i], "%u", &ID) != 1)
            {
              fprintf(stderr,"ERROR: '-drop_point_source' needs at least 1 argument: ID but '%s' is no valid ID\n", argv[i]);
              return FALSE;
            }
            if (ID > U16_MAX)
            {
              fprintf(stderr,"ERROR: cannot drop_point_source ID %u because it is larger than %u\n", ID, U16_MAX);
              return FALSE;
            }
            add_criterion(new LAScriterionDropPointSource(ID));
            *argv[i]='\0';
            i+=1;
          } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
          i-=1;
        }
        else if (strcmp(argv[i],"-drop_point_source_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_ID\n", argv[i]);
            return FALSE;
          }
          U32 min_ID;
          if (sscanf(argv[i+1], "%u", &min_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_ID max_ID but '%s' is no valid min_ID\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (min_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot drop point source below min_ID of %u because it is larger than %u\n", min_ID, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionDropPointSourceBelow((U16)min_ID));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_point_source_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_ID\n", argv[i]);
            return FALSE;
          }
          U32 max_ID;
          if (sscanf(argv[i+1], "%u", &max_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_ID but '%s' is no valid max_ID\n", argv[i], argv[i+1]);
            return FALSE;
          }
          if (max_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot drop point source above max_ID of %u because it is larger than %u\n", max_ID, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionDropPointSourceAbove((U16)max_ID));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_point_source_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID\n", argv[i]);
            return FALSE;
          }
          U32 min_ID;
          if (sscanf(argv[i+1], "%u", &min_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID but '%s' is no valid min_ID\n", argv[i], argv[i+1]);
            return FALSE;
          }
          U32 max_ID;
          if (sscanf(argv[i+2], "%d", &max_ID) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min_ID max_ID but '%s' is no valid max_ID\n", argv[i], argv[i+2]);
            return FALSE;
          }
          if (min_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot drop point source because min_ID of %u is larger than %u\n", min_ID, U16_MAX);
            return FALSE;
          }
          if (max_ID > U16_MAX)
          {
            fprintf(stderr,"ERROR: cannot drop point source because max_ID of %u is larger than %u\n", max_ID, U16_MAX);
            return FALSE;
          }
          add_criterion(new LAScriterionDropPointSourceBetween((U16)min_ID, (U16)max_ID));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-drop_gps", 9) == 0)
      {
        if (strcmp(argv[i],"-drop_gps_time_above") == 0 || strcmp(argv[i],"-drop_gpstime_above") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_gps_time\n", argv[i]);
            return FALSE;
          }
          F64 max_gps_time;
          if (sscanf(argv[i+1], "%lf", &max_gps_time) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: max_gps_time but '%s' is no valid max_gps_time\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropGpsTimeAbove(max_gps_time));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_gps_time_below") == 0 || strcmp(argv[i],"-drop_gpstime_below") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_gps_time\n", argv[i]);
            return FALSE;
          }
          F64 min_gps_time;
          if (sscanf(argv[i+1], "%lf", &min_gps_time) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: min_gps_time but '%s' is no valid min_gps_time\n", argv[i], argv[i+1]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropGpsTimeBelow(min_gps_time));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
        }
        else if (strcmp(argv[i],"-drop_gps_time_between") == 0 || strcmp(argv[i],"-drop_gpstime_between") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max\n", argv[i]);
            return FALSE;
          }
          F64 min;
          if (sscanf(argv[i+1], "%lf", &min) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid min\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 max;
          if (sscanf(argv[i+2], "%lf", &max) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: min max but '%s' is no valid max\n", argv[i], argv[i+2]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropGpsTimeBetween(min, max));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
      }
      else if (strncmp(argv[i],"-drop_attribute", 15) == 0)
      {
        if (strcmp(argv[i],"-drop_attribute_below") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropAttributeBelow(atoi(argv[i+1]), atof(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_attribute_above") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: index value\n", argv[i]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropAttributeAbove(atoi(argv[i+1]), atof(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2;
        }
        else if (strcmp(argv[i],"-drop_attribute_between") == 0)
        {
          if ((i+3) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index below above\n", argv[i]);
            return FALSE;
          }
          U32 index;
          if (sscanf(argv[i+1], "%u", &index) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index below above but '%s' is no valid index\n", argv[i], argv[i+1]);
            return FALSE;
          }
          F64 below;
          if (sscanf(argv[i+2], "%lf", &below) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index below above but '%s' is no valid below\n", argv[i], argv[i+2]);
            return FALSE;
          }
          F64 above;
          if (sscanf(argv[i+3], "%lf", &above) != 1)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: index below above but '%s' is no valid above\n", argv[i], argv[i+3]);
            return FALSE;
          }
          add_criterion(new LAScriterionDropAttributeBetween(index, below, above));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3;
        }
      }
      else if (strcmp(argv[i],"-drop_every_nth") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: nth\n", argv[i]);
          return FALSE;
        }
        U32 nth;
        if (sscanf(argv[i+1], "%u", &nth) != 1)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument nth but '%s' is no valid nth\n", argv[i], argv[i+1]);
          return FALSE;
        }
        if (nth == 0)
        {
          fprintf(stderr,"ERROR: %u is no valid nth for '%s'\n", nth, argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionDropEveryNth(nth));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }

      else if (strcmp(argv[i],"-drop_scanner_channel") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionDropScannerChannel((I32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
    }
    else if (strcmp(argv[i],"-first_only") == 0)
    {
      add_criterion(new LAScriterionKeepFirstReturn());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-last_only") == 0)
    {
      add_criterion(new LAScriterionKeepLastReturn());
      *argv[i]='\0';
    }
    else if (strncmp(argv[i],"-thin_", 6) == 0)
    {
      if (strcmp(argv[i],"-thin_with_grid") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: grid_spacing\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionThinWithGrid((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-thin_pulses_with_time") == 0 || strcmp(argv[i],"-thin_with_time") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: time_spacing\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionThinPulsesWithTime(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
      else if (strcmp(argv[i],"-thin_points_with_time") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: time_spacing\n", argv[i]);
          return FALSE;
        }
        add_criterion(new LAScriterionThinPointsWithTime(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
      }
    }
    else if (strncmp(argv[i],"-filter_", 8) == 0)
    {
      if (strcmp(argv[i],"-filter_and") == 0)
      {
        if (num_criteria < 2)
        {
          fprintf(stderr,"ERROR: '%s' needs to be preceeded by at least two filters\n", argv[i]);
          return FALSE;
        }
        LAScriterion* filter_criterion = new LAScriterionAnd(criteria[num_criteria-2], criteria[num_criteria-1]);
        num_criteria--;
        criteria[num_criteria] = 0;
        num_criteria--;
        criteria[num_criteria] = 0;
        add_criterion(filter_criterion);
        *argv[i]='\0';
      }
      else if (strcmp(argv[i],"-filter_or") == 0)
      {
        if (num_criteria < 2)
        {
          fprintf(stderr,"ERROR: '%s' needs to be preceeded by at least two filters\n", argv[i]);
          return FALSE;
        }
        LAScriterion* filter_criterion = new LAScriterionOr(criteria[num_criteria-2], criteria[num_criteria-1]);
        num_criteria--;
        criteria[num_criteria] = 0;
        num_criteria--;
        criteria[num_criteria] = 0;
        add_criterion(filter_criterion);
        *argv[i]='\0';
      }
    }
  }

  if (keep_extended_classification_mask[0] || keep_extended_classification_mask[1] || keep_extended_classification_mask[2] || keep_extended_classification_mask[3] || keep_extended_classification_mask[4] || keep_extended_classification_mask[5] || keep_extended_classification_mask[6] || keep_extended_classification_mask[7])
  {
    if (drop_extended_classification_mask[0] || drop_extended_classification_mask[1] || drop_extended_classification_mask[2] || drop_extended_classification_mask[3] || drop_extended_classification_mask[4] || drop_extended_classification_mask[5] || drop_extended_classification_mask[6] || drop_extended_classification_mask[7])
    {
      fprintf(stderr,"ERROR: cannot use '-drop_extended_class' and '-keep_extended_class' simultaneously\n");
      return FALSE;
    }
    else
    {
      drop_extended_classification_mask[0] = ~keep_extended_classification_mask[0];
      drop_extended_classification_mask[1] = ~keep_extended_classification_mask[1];
      drop_extended_classification_mask[2] = ~keep_extended_classification_mask[2];
      drop_extended_classification_mask[3] = ~keep_extended_classification_mask[3];
      drop_extended_classification_mask[4] = ~keep_extended_classification_mask[4];
      drop_extended_classification_mask[5] = ~keep_extended_classification_mask[5];
      drop_extended_classification_mask[6] = ~keep_extended_classification_mask[6];
      drop_extended_classification_mask[7] = ~keep_extended_classification_mask[7];
    }
  }
  if (drop_extended_classification_mask[0] || drop_extended_classification_mask[1] || drop_extended_classification_mask[2] || drop_extended_classification_mask[3] || drop_extended_classification_mask[4] || drop_extended_classification_mask[5] || drop_extended_classification_mask[6] || drop_extended_classification_mask[7])
  {
    add_criterion(new LAScriterionDropExtendedClassifications(drop_extended_classification_mask));
  }

  return TRUE;
}

BOOL LASfilter::parse(CHAR* string)
{
  int p = 0;
  int argc = 1;
  char* argv[64];
  I32 len = (I32)strlen(string);

  while (p < len)
  {
    while ((p < len) && (string[p] == ' ')) p++;
    if (p < len)
    {
      argv[argc] = string + p;
      argc++;
      while ((p < len) && (string[p] != ' ')) p++;
      string[p] = '\0';
      p++;
    }
  }

  return parse(argc, argv);
}

I32 LASfilter::unparse(CHAR* string) const
{
  U32 i;
  I32 n = 0;
  for (i = 0; i < num_criteria; i++)
  {
    n += criteria[i]->get_command(&string[n]);
  }
  return n;
}

U32 LASfilter::get_decompress_selective() const
{
  U32 decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY;
  U32 i;
  for (i = 0; i < num_criteria; i++)
  {
    decompress_selective |= criteria[i]->get_decompress_selective();
  }
  return decompress_selective;
}

void LASfilter::addClipCircle(F64 x, F64 y, F64 radius)
{
  add_criterion(new LAScriterionKeepCircle(x, y, radius));
}

void LASfilter::addClipBox(F64 min_x, F64 min_y, F64 min_z, F64 max_x, F64 max_y, F64 max_z)
{
  add_criterion(new LAScriterionKeepxyz(min_x, min_y, min_z, max_x, max_y, max_z));
}

void LASfilter::addKeepScanDirectionChange()
{
  add_criterion(new LAScriterionKeepScanDirectionChange());
}

BOOL LASfilter::filter(const LASpoint* point)
{
  U32 i;

  for (i = 0; i < num_criteria; i++)
  {
    if (criteria[i]->filter(point))
    {
      counters[i]++;
      return TRUE; // point was filtered
    }
  }
  return FALSE; // point survived
}

void LASfilter::reset()
{
  U32 i;
  for (i = 0; i < num_criteria; i++)
  {
    criteria[i]->reset();
  }
}

LASfilter::LASfilter()
{
  alloc_criteria = 0;
  num_criteria = 0;
  criteria = 0;
  counters = 0;
}

LASfilter::~LASfilter()
{
  if (criteria) clean();
}

void LASfilter::add_criterion(LAScriterion* filter_criterion)
{
  if (num_criteria == alloc_criteria)
  {
    U32 i;
    alloc_criteria += 16;
    LAScriterion** temp_criteria = new LAScriterion*[alloc_criteria];
    int* temp_counters = new int[alloc_criteria];
    if (criteria)
    {
      for (i = 0; i < num_criteria; i++)
      {
        temp_criteria[i] = criteria[i];
        temp_counters[i] = counters[i];
      }
      delete [] criteria;
      delete [] counters;
    }
    criteria = temp_criteria;
    counters = temp_counters;
  }
  criteria[num_criteria] = filter_criterion;
  counters[num_criteria] = 0;
  num_criteria++;
}
