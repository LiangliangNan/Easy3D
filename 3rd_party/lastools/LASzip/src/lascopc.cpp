/*
 ===============================================================================

 FILE:  lascopc.cpp

 CONTENTS:

 see corresponding header file

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

 see corresponding header file

 ===============================================================================
 */
#include "lascopc.hpp"

#ifdef LASZIPDLL_EXPORTS
#include "lasreadpoint.hpp"
#else
#include "lasreader.hpp"
#endif

#include <deque>
#include <cmath>
#include <algorithm>

#define MAX(a, b, c) ((a) <= (b)? (b) <= (c)? (c) : (b) : (a) <= (c)? (c) : (a)) // MSVC does not like std::max({a, b, c})

EPTkey::EPTkey(I32 d, I32 x, I32 y, I32 z) : d(d), x(x), y(y), z(z) {}
EPTkey::EPTkey() : EPTkey(-1, -1, -1, -1) {}

std::array<EPTkey, 8> EPTkey::get_children() const
{
  std::array<EPTkey, 8> children;
  for (U8 direction = 0 ; direction < 8 ; direction++)
  {
    EPTkey key(*this);

    key.d++;
    key.x *= 2;
    key.y *= 2;
    key.z *= 2;

    if (direction & (((U8)1) << 0)) key.x++;
    if (direction & (((U8)1) << 1)) key.y++;
    if (direction & (((U8)1) << 2)) key.z++;

    children[direction] = key;
  }

  return children;
}

EPTkey EPTkey::get_parent() const
{
  if (!is_valid()) return EPTkey();
  if (d == 0) return EPTkey();
  return EPTkey(d - 1, x >> 1, y >> 1, z >> 1);
}

EPToctant::EPToctant()
{
  offset.start = 0;
  offset.end = 0;
  position.start = 0;
  position.end = 0;
  xmin = 0;
  ymin = 0;
  zmin = 0;
  xmax = 0;
  ymax = 0;
  zmax = 0;
}

EPToctant::EPToctant(const LASvlr_copc_entry entry, const F64 xmin, const F64 ymin, const F64 zmin, const F64 xmax, const F64 ymax, const F64 zmax, const U64 start, const U64 end)
{
  d = entry.key.depth;
  x = entry.key.x;
  y = entry.key.y;
  z = entry.key.z;

  // The address range in the file
  offset.start = entry.offset;
  offset.end = entry.offset + entry.byte_size;

  // The point range in the file
  position.start = start;
  position.end = end;

  // Step size accounts for depth level
  F64 size = MAX(xmax - xmin, ymax - ymin, zmax - zmin);
  F64 res  = size / std::pow(2, d);

  // Bounding box
  this->xmin = res * x + xmin;
  this->ymin = res * y + ymin;
  this->zmin = res * z + zmin;
  this->xmax = this->xmin + res;
  this->ymax = this->ymin + res;
  this->zmax = this->zmin + res;
}

EPToctree::EPToctree(const LASheader& header)
{
  if (header.vlr_copc_info)
  {
    xmin = header.vlr_copc_info->center_x - header.vlr_copc_info->halfsize;
    ymin = header.vlr_copc_info->center_y - header.vlr_copc_info->halfsize;
    zmin = header.vlr_copc_info->center_z - header.vlr_copc_info->halfsize;
    xmax = header.vlr_copc_info->center_x + header.vlr_copc_info->halfsize;
    ymax = header.vlr_copc_info->center_y + header.vlr_copc_info->halfsize;
    zmax = header.vlr_copc_info->center_z + header.vlr_copc_info->halfsize;
    point_spacing = header.vlr_copc_info->spacing;
    max_depth = 0;
  }
  else
  {
    F64 center_x = (header.min_x + header.max_x)/2;
    F64 center_y = (header.min_y + header.max_y)/2;
    F64 center_z = (header.min_z + header.max_z)/2;
    F64 halfsize = MAX(header.max_x - header.min_x, header.max_y - header.min_y, header.max_z - header.min_z)/2;

    xmin = center_x - halfsize;
    ymin = center_y - halfsize;
    zmin = center_z - halfsize;
    xmax = center_x + halfsize;
    ymax = center_y + halfsize;
    zmax = center_z + halfsize;
    point_spacing = 0;
    max_depth = 0;
  }

  if (header.vlr_copc_info && header.vlr_copc_entries)
  {
    registry.reserve(header.number_of_copc_entries);

    U64 ni = 0;
    U64 nf = 0;
    for (U32 i = 0 ; i < header.number_of_copc_entries ; i++)
    {
      if (header.vlr_copc_entries[i].point_count > 0)
      {
        ni = nf;
        nf = ni + header.vlr_copc_entries[i].point_count-1;
        EPToctant octant(header.vlr_copc_entries[i], xmin, ymin, zmin, xmax, ymax, zmax, ni, nf);
        registry[(EPTkey)octant] = octant;
        if (octant.d > max_depth) max_depth = octant.d;
        nf++;
      }
      else
      {
        // Octants with 0 point must be added to be able to reccurse the octree
        EPToctant octant(header.vlr_copc_entries[i], xmin, ymin, zmin, xmax, ymax, zmax, 0, 0);
        registry[(EPTkey)octant] = octant;
      }
    }
  }
}

// Static function used to parse and assigned data to the LASheader in LASreaderLAS. The reasons of this function are:
// 1. Keep the copc logic code here, adding minimal modifications in the legacy LASlib/LASzip core code.
// 2. Make sure that the legacy code style (name convention, usage raw pointer and so on) stay unchanged.
//    (the LASvlr_copc_entry pointer allocated on heap is deleted by the LASheader)
BOOL EPToctree::set_vlr_entries(const U8* data, const U64 offset_to_first_copc_entry, LASheader& header)
{
  if (!data) return FALSE;

  std::vector<LASvlr_copc_entry> page;
  std::vector<LASvlr_copc_entry> entries;
  std::deque<LASvlr_copc_entry> child_entries;
  LASvlr_copc_entry* payload = (LASvlr_copc_entry*)data;

  // Read the root page
  U64 n_root_entries = header.vlr_copc_info->root_hier_size/sizeof(LASvlr_copc_entry);
  U64 root_offset = header.vlr_copc_info->root_hier_offset - offset_to_first_copc_entry;
  U64 n_entries_before_root = root_offset/sizeof(LASvlr_copc_entry);
  page = std::vector<LASvlr_copc_entry>(payload + n_entries_before_root, payload + n_entries_before_root + n_root_entries);

  for (size_t j = 0; j < page.size(); j++)
  {
    if (page[j].point_count >= 0) entries.push_back(page[j]);
    else if (page[j].point_count == -1) child_entries.push_back(page[j]);
  }

  // Recurse the tree of entries
  while (child_entries.size())
  {
    LASvlr_copc_entry e = child_entries.front();
    child_entries.pop_front();

    U64 page_size = e.byte_size;
    U64 n_entry = page_size/sizeof(LASvlr_copc_entry);
    U64 page_offset = e.offset - offset_to_first_copc_entry;
    U64 n_entries_before_page = page_offset/sizeof(LASvlr_copc_entry);
    page = std::vector<LASvlr_copc_entry>(payload + n_entries_before_page, payload + n_entries_before_page + n_entry);

    for (size_t j = 0; j < page.size(); j++)
    {
      if (page[j].point_count >= 0) entries.push_back(page[j]);
      else if (page[j].point_count == -1) child_entries.push_back(page[j]);
    }
  }

  std::sort(entries.begin(), entries.end(), [](const LASvlr_copc_entry& e1, const LASvlr_copc_entry& e2) { return e1.offset < e2.offset; });

  U64 sum = 0;
  for (size_t j = 0; j < entries.size(); j++)
    sum += entries[j].point_count;

  if (sum != header.extended_number_of_point_records)
  {
    fprintf(stderr, "WARNING: COPC EPT hierarchy EVLR number of point does not match with the header.\n");
    return FALSE;
  }

  header.number_of_copc_entries = (U32)entries.size();
  header.vlr_copc_entries = new LASvlr_copc_entry[header.number_of_copc_entries];
  for (size_t j = 0; j < header.number_of_copc_entries; j++)
    header.vlr_copc_entries[j] = entries[j];

  return TRUE;
}

I32 EPToctree::compute_max_depth(const LASheader& header, U64 max_points_per_octant)
{
  // strategy to regulate the maximum depth of the octree
  F64 xsize = header.max_x - header.min_x;
  F64 ysize = header.max_y - header.min_y;
  F64 zsize = header.max_z - header.min_z;
  F64 size  = MAX(xsize, ysize, zsize);
  U64 npts  = MAX((U64)header.number_of_point_records, header.extended_number_of_point_records, 0);
  I32 max_depth = 0;

  while (npts > max_points_per_octant)
  {
    if (xsize >= size) { npts /= 2; }
    if (ysize >= size) { npts /= 2; }
    if (zsize >= size) { npts /= 2; }
    size /= 2;
    max_depth++;
  }

  return max_depth;
}

EPTkey EPToctree::get_key(const LASpoint* p, const I32 depth) const
{
  I32 grid_size = (I32)std::pow(2, depth);
  F64 grid_resolution = (xmax - xmin) / grid_size;

  I32 xi = I32_FLOOR((p->get_x() - xmin) / grid_resolution);
  I32 yi = I32_FLOOR((p->get_y() - ymin) / grid_resolution);
  I32 zi = I32_FLOOR((p->get_z() - zmin) / grid_resolution);

  if (xi < 0) xi = 0;
  if (yi < 0) yi = 0;
  if (zi < 0) zi = 0;
  if (xi >= grid_size) xi = grid_size - 1;
  if (yi >= grid_size) yi = grid_size - 1;
  if (zi >= grid_size) zi = grid_size - 1;

  return EPTkey(depth, xi, yi, zi);
}

I32 EPToctree::get_cell(const LASpoint*p, const EPTkey& key) const
{
  F64 size = get_halfsize()*2;
  F64 res  = size / (1 << key.d);

  // Bounding box of the octant
  F64 minx = res * key.x + (get_center_x() - get_halfsize());
  F64 miny = res * key.y + (get_center_y() - get_halfsize());
  F64 minz = res * key.z + (get_center_z() - get_halfsize());
  F64 maxx = minx + res;
  //F64 maxy = miny + res;
  //F64 maxz = minz + res;

  // Get cell id in this octant
  F64 grid_resolution = (maxx - minx) / grid_size;
  I32 xi = (I32)std::floor((p->get_x() - minx) / grid_resolution);
  I32 yi = (I32)std::floor((p->get_y() - miny) / grid_resolution);
  I32 zi = (I32)std::floor((p->get_z() - minz) / grid_resolution);
  xi = (std::min)((std::max)(0, xi), grid_size - 1);
  yi = (std::min)((std::max)(0, yi), grid_size - 1);
  zi = (std::min)((std::max)(0, zi), grid_size - 1);

  return zi * grid_size * grid_size + yi * grid_size + xi;
}

COPCindex::COPCindex(const LASheader& header) : EPToctree(header)
{
  start = 0;
  end = 0;
  current_interval = 0;
  have_interval = false;

  r_min_x = F64_MIN;
  r_min_y = F64_MIN;
  r_min_z = F64_MIN;
  r_max_x = F64_MAX;
  r_max_y = F64_MAX;
  r_max_z = F64_MAX;
  q_depth = max_depth;

  sort_octants = &spatial_order;
}

void COPCindex::set_depth_limit(const I32 depth)
{
  q_depth = (depth < 0) ? max_depth : depth;
  query_intervals();
}

void COPCindex::set_resolution(const F64 resolution)
{
  q_depth = max_depth;

  if (resolution <= 0.0)
    return;

  F64 current_resolution = point_spacing;
  for (I32 i = 0; i <= max_depth; i++)
  {
    if (current_resolution <= resolution)
    {
      q_depth = i;
      break;
    }
    current_resolution /= 2.0;
  }

  query_intervals();
}

void COPCindex::intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y)
{
  this->r_min_x = r_min_x;
  this->r_min_y = r_min_y;
  this->r_max_x = r_max_x;
  this->r_max_y = r_max_y;
  query_intervals();
}

void  COPCindex::intersect_cuboid(const F64 r_min_x, const F64 r_min_y, const F64 r_min_z, const F64 r_max_x, const F64 r_max_y, const F64 r_max_z)
{
  this->r_min_z = r_min_z;
  this->r_max_z = r_max_z;
  intersect_rectangle(r_min_x, r_min_y, r_max_x, r_max_y);
}

void COPCindex::intersect_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  F64 r_min_x = center_x - radius;
  F64 r_min_y = center_y - radius;
  F64 r_max_x = center_x + radius;
  F64 r_max_y = center_y + radius;
  intersect_rectangle(r_min_x, r_min_y, r_max_x, r_max_y);
}

void COPCindex::intersect_sphere(const F64 center_x, const F64 center_y, const F64 center_z, const F64 radius)
{
  F64 r_min_x = center_x - radius;
  F64 r_min_y = center_y - radius;
  F64 r_min_z = center_z - radius;
  F64 r_max_x = center_x + radius;
  F64 r_max_y = center_y + radius;
  F64 r_max_z = center_z + radius;
  intersect_cuboid(r_min_x, r_min_y, r_min_z, r_max_x, r_max_y, r_max_z);
}

bool COPCindex::query_intervals()
{
  clear_intervals();
  query_intervals(EPTkey::root());
  std::sort(query.begin(), query.end(), sort_octants);

  for (const EPToctant& oct : query)
  {
    points_intervals.push_back(oct.position);
    offsets_intervals.push_back(oct.offset);
  }
  merge_intervals();
  return query.size() > 0;
}

void COPCindex::query_intervals(const EPTkey& key)
{
  auto it = registry.find(key);
  if (it != registry.end())
  {
    EPToctant const &oct = it->second;
    bool inside = !(oct.xmin > r_max_x || oct.xmax < r_min_x || oct.ymin > r_max_y || oct.ymax < r_min_y || oct.zmin > r_max_z || oct.zmax < r_min_z);
    bool indepth = oct.d <= q_depth;
    if (indepth && inside)
    {
      if (oct.offset.start > 0) query.push_back(oct); // if start == 0 it is an octant with 0 points
      for (EPTkey const &k : key.get_children()) query_intervals(k); // octants with 0 points may have childs with non 0 points
    }
  }
}

void COPCindex::merge_intervals()
{
  merge_intervals(points_intervals);
  merge_intervals(offsets_intervals);
}

void COPCindex::merge_intervals(std::vector<Range>& x)
{
  if (x.size() < 2) return;

  std::vector<Range> ans;
  ans.reserve(x.size()/2);
  Range prev = x[0];

  for (U32 i = 1 ; i < x.size() ; i++)
  {
    Range current = x[i];
    if ((current.start-prev.end) <= 1)
    {
      prev.end = current.end;
    }
    else
    {
      ans.push_back(prev);
      prev = current;
    }
  }

  ans.push_back(prev);
  x.swap(ans);
}

void COPCindex::clear_intervals()
{
  start = 0;
  end = 0;
  current_interval = 0;
  points_intervals.clear();
  offsets_intervals.clear();
  query.clear();
}

bool COPCindex::has_intervals()
{
  if (current_interval < points_intervals.size())
  {
    start = points_intervals[current_interval].start;
    end = points_intervals[current_interval].end;
    current_interval++;
    have_interval = true;
    return true;
  }
  else
  {
    have_interval = false;
    return false;
  }
}

std::vector<Range> COPCindex::get_points_intervals()
{
  if (query.empty()) query_intervals();
  return points_intervals;
}

std::vector<Range> COPCindex::get_offsets_intervals()
{
  if (query.empty()) query_intervals();
  return offsets_intervals;
}

U64  COPCindex::get_number_of_points()
{
  if (query.empty()) query_intervals();

  U64 sum = 0;
  for (const Range& r : points_intervals)
    sum += r.end-r.start+1;

  return sum;
}

#ifdef LASZIPDLL_EXPORTS
BOOL COPCindex::seek_next(LASreadPoint* reader, I64 &p_count)
{
  if (!have_interval)
  {
    if (!has_intervals()) return FALSE;
    reader->seek((U32)p_count, start);
    p_count = start;
  }
  if (p_count == (I64)end)
  {
    have_interval = FALSE;
  }
  return TRUE;
}
#else
BOOL COPCindex::seek_next(LASreader* reader)
{
  if (!have_interval)
  {
    if (!has_intervals()) return FALSE;
    reader->seek(start);
  }
  if (reader->p_count == (I64)end)
  {
    have_interval = FALSE;
  }
  return TRUE;
}
#endif

