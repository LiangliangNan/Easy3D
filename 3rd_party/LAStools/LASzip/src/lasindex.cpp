/*
===============================================================================

  FILE:  lasindex.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2011-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "lasindex.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasquadtree.hpp"
#include "lasinterval.hpp"
#ifdef LASZIPDLL_EXPORTS
#include "lasreadpoint.hpp"
#else
#include "lasreader.hpp"
#endif
#include "bytestreamin_file.hpp"
#include "bytestreamout_file.hpp"

#ifdef UNORDERED
   // Figure out whether <unordered_map> is in tr1
#  ifdef __has_include
#    if __has_include(<unordered_map>)
#     include <unordered_map>
      using namespace std;
#     define UNORDERED_FOUND
#    endif
#  endif
#  ifdef HAVE_UNORDERED_MAP
#     include <unordered_map>
      using namespace std;
#  elif defined(UNORDERED_FOUND)
#    include <tr1/unordered_map>
    using namespace std;
    using namespace tr1;
#  endif
typedef unordered_map<I32,U32> my_cell_hash;
#elif defined(LZ_WIN32_VC6)
#include <hash_map>
using namespace std;
typedef hash_map<I32,U32> my_cell_hash;
#else
#include <unordered_map>
using namespace std;
typedef unordered_map<I32, U32> my_cell_hash;
#endif

LASindex::LASindex()
{
  spatial = 0;
  interval = 0;
  have_interval = FALSE;
  start = 0;
  end = 0;
  full = 0;
  total = 0;
  cells = 0;
}

LASindex::~LASindex()
{
  if (spatial) delete spatial;
  if (interval) delete interval;
}

void LASindex::prepare(LASquadtree* spatial, I32 threshold)
{
  if (this->spatial) delete this->spatial;
  this->spatial = spatial;
  if (this->interval) delete this->interval;
  this->interval = new LASinterval(threshold);
}

BOOL LASindex::add(const F64 x, const F64 y, const U32 p_index)
{
  I32 cell = spatial->get_cell_index(x, y);
  return interval->add(p_index, cell);
}

void LASindex::complete(U32 minimum_points, I32 maximum_intervals, const BOOL verbose)
{
  if (verbose)
  {
    fprintf(stderr,"before complete %d %d\n", minimum_points, maximum_intervals);
    print(FALSE);
  }
  if (minimum_points)
  {
    I32 hash1 = 0;
    my_cell_hash cell_hash[2];
    // insert all cells into hash1
    interval->get_cells();
    while (interval->has_cells())
    {
      cell_hash[hash1][interval->index] = interval->full;
    }
    while (cell_hash[hash1].size())
    {
      I32 hash2 = (hash1+1)%2;
      cell_hash[hash2].clear();
      // coarsen if a coarser cell will still have fewer than minimum_points (and points in all subcells)
      BOOL coarsened = FALSE;
      U32 i, full;
      I32 coarser_index;
      U32 num_indices;
      U32 num_filled;
      I32* indices;
      my_cell_hash::iterator hash_element_inner;
      my_cell_hash::iterator hash_element_outer = cell_hash[hash1].begin();
      while (hash_element_outer != cell_hash[hash1].end())
      {
        if ((*hash_element_outer).second)
        {
          if (spatial->coarsen((*hash_element_outer).first, &coarser_index, &num_indices, &indices))
          {
            full = 0;
            num_filled = 0;
            for (i = 0; i < num_indices; i++)
            {
              if ((*hash_element_outer).first == indices[i])
              {
                hash_element_inner = hash_element_outer;
              }
              else
              {
                hash_element_inner = cell_hash[hash1].find(indices[i]);
              }
              if (hash_element_inner != cell_hash[hash1].end())
              {
                full += (*hash_element_inner).second;
                (*hash_element_inner).second = 0;
                num_filled++;
              }
            }
            if ((full < minimum_points) && (num_filled == num_indices))
            {
              interval->merge_cells(num_indices, indices, coarser_index);
              coarsened = TRUE;
              cell_hash[hash2][coarser_index] = full;
            }
          }
        }
        hash_element_outer++;
      }
      if (!coarsened) break;
      hash1 = (hash1+1)%2;
    }
    // tell spatial about the existing cells
    interval->get_cells();
    while (interval->has_cells())
    {
      spatial->manage_cell(interval->index);
    }
    if (verbose)
    {
      fprintf(stderr,"after minimum_points %d\n", minimum_points);
      print(FALSE);
    }
  }
  if (maximum_intervals < 0)
  {
    maximum_intervals = -maximum_intervals*interval->get_number_cells();
  }
  if (maximum_intervals)
  {
    interval->merge_intervals(maximum_intervals, verbose);
    if (verbose)
    {
      fprintf(stderr,"after maximum_intervals %d\n", maximum_intervals);
      print(FALSE);
    }
  }
}

void LASindex::print(BOOL verbose)
{
  U32 total_cells = 0;
  U32 total_full = 0;
  U32 total_total = 0;
  U32 total_intervals = 0;
  U32 total_check;
  U32 intervals;
  interval->get_cells();
  while (interval->has_cells())
  {
    total_check = 0;
    intervals = 0;
    while (interval->has_intervals())
    {
      total_check += interval->end-interval->start+1;
      intervals++;
    }
    if (total_check != interval->total)
    {
      fprintf(stderr,"ERROR: total_check %d != interval->total %d\n", total_check, interval->total);
    }
    if (verbose) fprintf(stderr,"cell %d intervals %d full %d total %d (%.2f)\n", interval->index, intervals, interval->full, interval->total, 100.0f*interval->full/interval->total);
    total_cells++;
    total_full += interval->full;
    total_total += interval->total;
    total_intervals += intervals;
  }
  if (verbose) fprintf(stderr,"total cells/intervals %d/%d full %d (%.2f)\n", total_cells, total_intervals, total_full, 100.0f*total_full/total_total);
}

LASquadtree* LASindex::get_spatial() const
{
  return spatial;
}

LASinterval* LASindex::get_interval() const
{
  return interval;
}

BOOL LASindex::intersect_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y)
{
  have_interval = FALSE;
  cells = spatial->intersect_rectangle(r_min_x, r_min_y, r_max_x, r_max_y);
//  fprintf(stderr,"%d cells of %g/%g %g/%g intersect rect %g/%g %g/%g\n", num_cells, spatial->get_min_x(), spatial->get_min_y(), spatial->get_max_x(), spatial->get_max_y(), r_min_x, r_min_y, r_max_x, r_max_y);
  if (cells)
    return merge_intervals();
  return FALSE;
}

BOOL LASindex::intersect_tile(const F32 ll_x, const F32 ll_y, const F32 size)
{
  have_interval = FALSE;
  cells = spatial->intersect_tile(ll_x, ll_y, size);
//  fprintf(stderr,"%d cells of %g/%g %g/%g intersect tile %g/%g/%g\n", num_cells, spatial->get_min_x(), spatial->get_min_y(), spatial->get_max_x(), spatial->get_max_y(), ll_x, ll_y, size);
  if (cells)
    return merge_intervals();
  return FALSE;
}

BOOL LASindex::intersect_circle(const F64 center_x, const F64 center_y, const F64 radius)
{
  have_interval = FALSE;
  cells = spatial->intersect_circle(center_x, center_y, radius);
//  fprintf(stderr,"%d cells of %g/%g %g/%g intersect circle %g/%g/%g\n", num_cells, spatial->get_min_x(), spatial->get_min_y(), spatial->get_max_x(), spatial->get_max_y(), center_x, center_y, radius);
  if (cells)
    return merge_intervals();
  return FALSE;
}

BOOL LASindex::get_intervals()
{
  have_interval = FALSE;
  return interval->get_merged_cell();
}

BOOL LASindex::has_intervals()
{
  if (interval->has_intervals())
  {
    start = interval->start;
    end = interval->end;
    full = interval->full;
    have_interval = TRUE;
    return TRUE;
  }
  have_interval = FALSE;
  return FALSE;
}

BOOL LASindex::read(FILE* file)
{
  if (file == 0) return FALSE;
  ByteStreamIn* stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamInFileLE(file);
  else
    stream = new ByteStreamInFileBE(file);
  if (!read(stream))
  {
    delete stream;
    return FALSE;
  }
  delete stream;
  return TRUE;
}

BOOL LASindex::write(FILE* file) const
{
  if (file == 0) return FALSE;
  ByteStreamOut* stream;
  if (IS_LITTLE_ENDIAN())
    stream = new ByteStreamOutFileLE(file);
  else
    stream = new ByteStreamOutFileBE(file);
  if (!write(stream))
  {
    delete stream;
    return FALSE;
  }
  delete stream;
  return TRUE;
}

BOOL LASindex::read(const char* file_name)
{
  if (file_name == 0) return FALSE;
  char* name = LASCopyString(file_name);
  if (strstr(file_name, ".las") || strstr(file_name, ".laz"))
  {
    name[strlen(name)-1] = 'x';
  }
  else if (strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
  {
    name[strlen(name)-1] = 'X';
  }
  else
  {
    name[strlen(name)-3] = 'l';
    name[strlen(name)-2] = 'a';
    name[strlen(name)-1] = 'x';
  }
  FILE* file = fopen(name, "rb");
  if (file == 0)
  {
    free(name);
    return FALSE;
  }
  if (!read(file))
  {
    fprintf(stderr,"ERROR (LASindex): cannot read '%s'\n", name);
    fclose(file);
    free(name);
    return FALSE;
  }
  fclose(file);
  free(name);
  return TRUE;
}

BOOL LASindex::append(const char* file_name) const
{
#ifdef LASZIPDLL_EXPORTS
  return FALSE;
#else
  LASreadOpener lasreadopener;

  if (file_name == 0) return FALSE;

  // open reader

  LASreader* lasreader = lasreadopener.open(file_name);
  if (lasreader == 0) return FALSE;
  if (lasreader->header.laszip == 0) return FALSE;

  // close reader

  lasreader->close();

  FILE* file = fopen(file_name, "rb");
  ByteStreamIn* bytestreamin = 0;
  if (IS_LITTLE_ENDIAN())
    bytestreamin = new ByteStreamInFileLE(file);
  else
    bytestreamin = new ByteStreamInFileBE(file);

  // maybe write LASindex EVLR start position into LASzip VLR

  I64 offset_laz_vlr = -1;

  // where to write LASindex EVLR that will contain the LAX file

  I64 number_of_special_evlrs = lasreader->header.laszip->number_of_special_evlrs;
  I64 offset_to_special_evlrs = lasreader->header.laszip->offset_to_special_evlrs;

  if ((number_of_special_evlrs == -1) && (offset_to_special_evlrs == -1))
  {
    bytestreamin->seekEnd();
    number_of_special_evlrs = 1;
    offset_to_special_evlrs = bytestreamin->tell();

    // find LASzip VLR

    I64 total = lasreader->header.header_size + 2;
    U32 number_of_variable_length_records = lasreader->header.number_of_variable_length_records + 1 + (lasreader->header.vlr_lastiling != 0) + (lasreader->header.vlr_lasoriginal != 0);

    for (U32 u = 0; u < number_of_variable_length_records; u++)
    {
      bytestreamin->seek(total);

      CHAR user_id[16];
      try { bytestreamin->getBytes((U8*)user_id, 16); } catch(...)
      {
        fprintf(stderr,"ERROR: reading header.vlrs[%d].user_id\n", u);
        return FALSE;
      }
      if (strcmp(user_id, "laszip encoded") == 0)
      {
        offset_laz_vlr = bytestreamin->tell() - 18;
        break;
      }
      U16 record_id;
      try { bytestreamin->get16bitsLE((U8*)&record_id); } catch(...)
      {
        fprintf(stderr,"ERROR: reading header.vlrs[%d].record_id\n", u);
        return FALSE;
      }
      U16 record_length_after_header;
      try { bytestreamin->get16bitsLE((U8*)&record_length_after_header); } catch(...)
      {
        fprintf(stderr,"ERROR: reading header.vlrs[%d].record_length_after_header\n", u);
        return FALSE;
      }
      total += (54 + record_length_after_header);
    }

    if (number_of_special_evlrs == -1) return FALSE;
  }

  delete bytestreamin;
  fclose(file);

  ByteStreamOut* bytestreamout;
  file = fopen(file_name, "rb+");
  if (IS_LITTLE_ENDIAN())
    bytestreamout = new ByteStreamOutFileLE(file);
  else
    bytestreamout = new ByteStreamOutFileBE(file);
  bytestreamout->seek(offset_to_special_evlrs);

  LASevlr lax_evlr;
  sprintf(lax_evlr.user_id, "LAStools");
  lax_evlr.record_id = 30;
  sprintf(lax_evlr.description, "LAX spatial indexing (LASindex)");

  bytestreamout->put16bitsLE((U8*)&(lax_evlr.reserved));
  bytestreamout->putBytes((U8*)lax_evlr.user_id, 16);
  bytestreamout->put16bitsLE((U8*)&(lax_evlr.record_id));
  bytestreamout->put64bitsLE((U8*)&(lax_evlr.record_length_after_header));
  bytestreamout->putBytes((U8*)lax_evlr.description, 32);

  if (!write(bytestreamout))
  {
    fprintf(stderr,"ERROR (LASindex): cannot append LAX to '%s'\n", file_name);
    delete bytestreamout;
    fclose(file);
    delete lasreader;
    return FALSE;
  }

  // update LASindex EVLR

  lax_evlr.record_length_after_header = bytestreamout->tell() - offset_to_special_evlrs - 60;
  bytestreamout->seek(offset_to_special_evlrs + 20);
  bytestreamout->put64bitsLE((U8*)&(lax_evlr.record_length_after_header));

  // maybe update LASzip VLR

  if (number_of_special_evlrs != -1)
  {
    bytestreamout->seek(offset_laz_vlr + 54 + 16);
    bytestreamout->put64bitsLE((U8*)&number_of_special_evlrs);
    bytestreamout->put64bitsLE((U8*)&offset_to_special_evlrs);
  }

  // close writer

  bytestreamout->seekEnd();
  delete bytestreamout;
  fclose(file);

  // delete reader

  delete lasreader;

  return TRUE;
#endif
}

BOOL LASindex::write(const char* file_name) const
{
  if (file_name == 0) return FALSE;
  char* name = LASCopyString(file_name);
  if (strstr(file_name, ".las") || strstr(file_name, ".laz"))
  {
    name[strlen(name)-1] = 'x';
  }
  else if (strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
  {
    name[strlen(name)-1] = 'X';
  }
  else
  {
    name[strlen(name)-3] = 'l';
    name[strlen(name)-2] = 'a';
    name[strlen(name)-1] = 'x';
  }
  FILE* file = fopen(name, "wb");
  if (file == 0)
  {
    fprintf(stderr,"ERROR (LASindex): cannot open '%s' for write\n", name);
    free(name);
    return FALSE;
  }
  if (!write(file))
  {
    fprintf(stderr,"ERROR (LASindex): cannot write '%s'\n", name);
    fclose(file);
    free(name);
    return FALSE;
  }
  fclose(file);
  free(name);
  return TRUE;
}

BOOL LASindex::read(ByteStreamIn* stream)
{
  if (spatial)
  {
    delete spatial;
    spatial = 0;
  }
  if (interval)
  {
    delete interval;
    interval = 0;
  }
  char signature[4];
  try { stream->getBytes((U8*)signature, 4); } catch (...)
  {
    fprintf(stderr,"ERROR (LASindex): reading signature\n");
    return FALSE;
  }
  if (strncmp(signature, "LASX", 4) != 0)
  {
    fprintf(stderr,"ERROR (LASindex): wrong signature %4s instead of 'LASX'\n", signature);
    return FALSE;
  }
  U32 version;
  try { stream->get32bitsLE((U8*)&version); } catch (...)
  {
    fprintf(stderr,"ERROR (LASindex): reading version\n");
    return FALSE;
  }
  // read spatial quadtree
  spatial = new LASquadtree();
  if (!spatial->read(stream))
  {
    fprintf(stderr,"ERROR (LASindex): cannot read LASspatial (LASquadtree)\n");
    return FALSE;
  }
  // read interval
  interval = new LASinterval();
  if (!interval->read(stream))
  {
    fprintf(stderr,"ERROR (LASindex): reading LASinterval\n");
    return FALSE;
  }
  // tell spatial about the existing cells
  interval->get_cells();
  while (interval->has_cells())
  {
    spatial->manage_cell(interval->index);
  }
  return TRUE;
}

BOOL LASindex::write(ByteStreamOut* stream) const
{
  if (!stream->putBytes((U8*)"LASX", 4))
  {
    fprintf(stderr,"ERROR (LASindex): writing signature\n");
    return FALSE;
  }
  U32 version = 0;
  if (!stream->put32bitsLE((U8*)&version))
  {
    fprintf(stderr,"ERROR (LASindex): writing version\n");
    return FALSE;
  }
  // write spatial quadtree
  if (!spatial->write(stream))
  {
    fprintf(stderr,"ERROR (LASindex): cannot write LASspatial (LASquadtree)\n");
    return FALSE;
  }
  // write interval
  if (!interval->write(stream))
  {
    fprintf(stderr,"ERROR (LASindex): writing LASinterval\n");
    return FALSE;
  }
  return TRUE;
}

// seek to next interval point

#ifdef LASZIPDLL_EXPORTS
BOOL LASindex::seek_next(LASreadPoint* reader, I64 &p_count)
{
  if (!have_interval)
  {
    if (!has_intervals()) return FALSE;
    reader->seek((U32)p_count, start);
    p_count = start;
  }
  if (p_count == end)
  {
    have_interval = FALSE;
  }
  return TRUE;
}
#else
BOOL LASindex::seek_next(LASreader* lasreader)
{
  if (!have_interval)
  {
    if (!has_intervals()) return FALSE;
    lasreader->seek(start);
  }
  if (lasreader->p_count == end)
  {
    have_interval = FALSE;
  }
  return TRUE;
}
#endif

// merge the intervals of non-empty cells
BOOL LASindex::merge_intervals()
{
  if (spatial->get_intersected_cells())
  {
    U32 used_cells = 0;
    while (spatial->has_more_cells())
    {
      if (interval->get_cell(spatial->current_cell))
      {
        interval->add_current_cell_to_merge_cell_set();
        used_cells++;
      }
    }
//    fprintf(stderr,"LASindex: used %d cells of total %d\n", used_cells, interval->get_number_cells());
    if (used_cells)
    {
      BOOL r = interval->merge();
      full = interval->full;
      total = interval->total;
      interval->clear_merge_cell_set();
      return r;
    }
  }
  return FALSE;
}
