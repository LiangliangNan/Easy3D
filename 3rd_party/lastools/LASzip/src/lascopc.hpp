/*
 ===============================================================================

 FILE:  lascopc.hpp

 CONTENTS:

 Interface to support COPC standard https://copc.io/

 PROGRAMMERS:

 Jean-Romain Roussel

 COPYRIGHT:

 (c) 2023 rapidlasso - fast tools to catch reality

 This is free software; you can redistribute and/or modify it under the
 terms of the GNU Lesser General Licence as published by the Free Software
 Foundation. See the LICENSE.txt file for more information.

 This software is distributed WITHOUT ANY WARRANTY and without even the
 implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 CHANGE HISTORY:

 17 April 2023 -- created to support copc standard

 ===============================================================================
 */

#ifndef LAS_COPC_HPP
#define LAS_COPC_HPP

#include <array>
#include <vector>
#include <unordered_map>

#include "lasdefinitions.hpp"
#include "mydefs.hpp"

#ifdef LASZIPDLL_EXPORTS
class LASreadPoint;
#else
class LASreader;
#endif

struct Range
{
  U64 start;
  U64 end;
};

inline bool operator<(const Range& a, const Range& b) { return a.start < b.start; }

struct EPTkey
{
  EPTkey();
  EPTkey(I32 d, I32 x, I32 y, I32 z);
  static EPTkey root() { return EPTkey(0, 0, 0, 0); }
  bool is_valid() const { return d >= 0 && x >= 0 && y >= 0 && z >= 0; }
  std::array<EPTkey, 8> get_children() const;
  EPTkey get_parent() const;

  I32 d;
  I32 x;
  I32 y;
  I32 z;
};

struct EPTKeyHasher
{
  // PDAL hash method copied
  std::size_t operator()(const EPTkey &k) const
  {
    std::hash<U64> h;
    U64 k1 = ((U64)k.d << 32) | k.x;
    U64 k2 = ((U64)k.y << 32) | k.z;
    return h(k1) ^ (h(k2) << 1);
  }
};

inline bool operator==(const EPTkey& a, const EPTkey& b) { return a.d == b.d && a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator!=(const EPTkey& a, const EPTkey& b) { return !(a == b); }
inline bool operator<(const EPTkey& a, const EPTkey& b)
{
  if (a.x < b.x) return true;
  if (a.x > b.x) return false;
  if (a.y < b.y) return true;
  if (a.y > b.y) return false;
  if (a.z < b.z) return true;
  if (a.z > b.z) return false;
  if (a.d < b.d) return true;
  if (a.d > b.d) return false;
  return false;
}

struct EPToctant : public EPTkey
{
  EPToctant();
  EPToctant(const LASvlr_copc_entry entry, const F64 xmin, const F64 ymin, const F64 zmin, const F64 xmax, const F64 ymax, const F64 zmax, const U64 start, const U64 end);

  Range offset;   // Address in file
  Range position; // Point position in file

  // Bounding box of the entry
  F64 xmin;
  F64 ymin;
  F64 zmin;
  F64 xmax;
  F64 ymax;
  F64 zmax;
};

inline bool spatial_order(const EPToctant& a, const EPToctant& b)
{
  // 2D spatial order
  F64 dx_a = (F64)a.x/(1 << a.d);
  F64 dx_b = (F64)b.x/(1 << b.d);
  if (dx_a < dx_b) return true;
  if (dx_a > dx_b) return false;

  F64 dy_a = (F64)a.y/(1 << a.d);
  F64 dy_b = (F64)b.y/(1 << b.d);
  if (dy_a < dy_b) return true;
  if (dy_a > dy_b) return false;

  if (a.d < b.d) return true;
  if (a.d > b.d) return false;

  F64 dz_a = (F64)a.z/(1 << a.d);
  F64 dz_b = (F64)b.z/(1 << b.d);
  if (dz_a < dz_b) return true;
  if (dz_a > dz_b) return false;

  return false;
}

inline bool depth_order(const EPToctant& a, const EPToctant& b)
{
  if (a.d < b.d) return true;
  if (a.d > b.d) return false;
  if (a.x < b.x) return true;
  if (a.x > b.x) return false;
  if (a.y < b.y) return true;
  if (a.y > b.y) return false;
  if (a.z < b.z) return true;
  if (a.z > b.z) return false;
  return false;
}

inline bool file_order(const EPToctant& a, const EPToctant& b)
{
  if (a.offset.start < b.offset.start) return true;
  return false;
}

class EPToctree
{
public:
  EPToctree(const LASheader& header);
  static BOOL set_vlr_entries(const U8* data, const U64 offset_to_first_copc_entry, LASheader& header);
  static I32 compute_max_depth(const LASheader& header, const U64 max_points_per_octant);
  EPTkey get_key(const LASpoint* p, const I32 depth) const;
  I32 get_cell(const LASpoint*p, const EPTkey& key) const;
  inline I32 get_max_depth() const { return max_depth; };
  inline F64 get_center_x() const { return (xmin+xmax)/2; };
  inline F64 get_center_y() const { return (ymin+ymax)/2; };
  inline F64 get_center_z() const { return (zmin+zmax)/2; };
  inline F64 get_halfsize() const { return (xmax-xmin)/2; };
  inline F64 get_size() const { return xmax-xmin; };
  inline F64 get_xmin() const { return xmin; };
  inline F64 get_ymin() const { return ymin; };
  inline F64 get_zmin() const { return zmin; };
  inline F64 get_xmax() const { return xmax; };
  inline F64 get_ymax() const { return ymax; };
  inline F64 get_zmax() const { return zmax; };
  inline I32 get_gridsize() const { return grid_size; };
  inline void set_gridsize(I32 size) { if (size > 2) grid_size = size; };

protected:
  F64 xmin;
  F64 ymin;
  F64 zmin;
  F64 xmax;
  F64 ymax;
  F64 zmax;
  F64 point_spacing;
  I32 max_depth;
  I32 grid_size;
  std::unordered_map<EPTkey, EPToctant, EPTKeyHasher> registry;
};

class COPCindex : public EPToctree
{
public:
  COPCindex(const LASheader& header);
  void set_depth_limit(const I32 depth);
  void set_resolution(const F64 resolution);
  void set_stream_ordered_by_chunk() { sort_octants = &file_order; };
  void set_stream_ordered_spatially() { sort_octants = &spatial_order; };
  void set_stream_ordered_by_depth() { sort_octants = &depth_order; };
  void intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y);
  void intersect_cuboid(const F64 r_min_x, const F64 r_min_y, const F64 r_min_z, const F64 r_max_x, const F64 r_max_y, const F64 r_max_z);
  void intersect_circle(const F64 center_x, const F64 center_y, const F64 radius);
  void intersect_sphere(const F64 center_x, const F64 center_y, const F64 center_z, const F64 radius);
  std::vector<Range> get_points_intervals();
  std::vector<Range> get_offsets_intervals();
  U64 get_number_of_points();

  // seek to next interval
#ifdef LASZIPDLL_EXPORTS
  BOOL seek_next(LASreadPoint* reader, I64 &p_count);
#else
  BOOL seek_next(LASreader* lasreader);
#endif

private:
  void query_intervals(const EPTkey& key);
  void merge_intervals();
  void merge_intervals(std::vector<Range>& x);
  void clear_intervals();
  bool query_intervals();
  bool has_intervals();
  bool (*sort_octants)(const EPToctant& a, const EPToctant& b);

private:
  F64 r_min_x;
  F64 r_min_y;
  F64 r_min_z;
  F64 r_max_x;
  F64 r_max_y;
  F64 r_max_z;
  I32 q_depth;

  bool have_interval;
  I64 start;
  I64 end;
  U32 current_interval;
  std::vector<Range> points_intervals;
  std::vector<Range> offsets_intervals;
  std::vector<EPToctant> query;
};

#endif


