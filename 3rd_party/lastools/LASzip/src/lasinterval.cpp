/*
===============================================================================

  FILE:  lasinterval.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2011-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/
#include "lasinterval.hpp"
#include "laszip.hpp"

#include "bytestreamin.hpp"
#include "bytestreamout.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <map>
#include <set>
using namespace std;

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
typedef unordered_map<I32, LASintervalStartCell*> my_cell_hash;
#elif defined(LZ_WIN32_VC6)
#include <hash_map>
using namespace std;
typedef hash_map<I32, LASintervalStartCell*> my_cell_hash;
#else
#include <unordered_map>
using namespace std;
typedef unordered_map<I32, LASintervalStartCell*> my_cell_hash;
#endif

typedef multimap<U32, LASintervalCell*> my_cell_map;
typedef set<LASintervalStartCell*> my_cell_set;

LASintervalCell::LASintervalCell()
{
  start = 0;
  end = 0;
  next = 0;
}

LASintervalCell::LASintervalCell(const U32 p_index)
{
  start = p_index;
  end = p_index;
  next = 0;
}

LASintervalCell::LASintervalCell(const LASintervalCell* cell)
{
  start = cell->start;
  end = cell->end;
  next = 0;
}

LASintervalStartCell::LASintervalStartCell() : LASintervalCell()
{
  full = 0;
  total = 0;
  last = 0;
}

LASintervalStartCell::LASintervalStartCell(const U32 p_index) : LASintervalCell(p_index)
{
  full = 1;
  total = 1;
  last = 0;
}

BOOL LASintervalStartCell::add(const U32 p_index, const U32 threshold)
{
  U32 current_end = (last ? last->end : end);
  assert(p_index > current_end);
  U32 diff = p_index - current_end;
  full++;
  if (diff > threshold)
  {
    if (last)
    {
      last->next = new LASintervalCell(p_index);
      last = last->next;
    }
    else
    {
      next = new LASintervalCell(p_index);
      last = next;
    }
    total++;
    return TRUE; // created new interval
  }
  if (last)
  {
    last->end = p_index;
  }
  else
  {
    end = p_index;
  }
  total += diff;
  return FALSE; // added to interval
}

BOOL LASinterval::add(const U32 p_index, const I32 c_index)
{
  if (last_cell == 0 || last_index != c_index)
  {
    last_index = c_index;
    my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->find(c_index);
    if (hash_element == ((my_cell_hash*)cells)->end())
    {
      last_cell = new LASintervalStartCell(p_index);
      ((my_cell_hash*)cells)->insert(my_cell_hash::value_type(c_index, last_cell));
      number_intervals++;
      return TRUE;
    }
    last_cell = (*hash_element).second;
  }
  if (last_cell->add(p_index, threshold))
  {
    number_intervals++;
    return TRUE;
  }
  return FALSE;
}

// get total number of cells
U32 LASinterval::get_number_cells() const
{
  return (U32)((my_cell_hash*)cells)->size();
}

// get total number of intervals
U32 LASinterval::get_number_intervals() const
{
  return number_intervals;
}

// merge cells (and their intervals) into one cell
BOOL LASinterval::merge_cells(const U32 num_indices, const I32* indices, const I32 new_index)
{
  U32 i;
  if (num_indices == 1)
  {
    my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->find(indices[0]);
    if (hash_element == ((my_cell_hash*)cells)->end())
    {
      return FALSE;
    }
    ((my_cell_hash*)cells)->insert(my_cell_hash::value_type(new_index, (*hash_element).second));
    ((my_cell_hash*)cells)->erase(hash_element);
  }
  else
  {
    if (cells_to_merge) ((my_cell_set*)cells_to_merge)->clear();
    for (i = 0; i < num_indices; i++)
    {
      add_cell_to_merge_cell_set(indices[i], TRUE);
    }
    if (!merge(TRUE)) return FALSE;
    ((my_cell_hash*)cells)->insert(my_cell_hash::value_type(new_index, merged_cells));
    merged_cells = 0;
  }
  return TRUE;
}

// merge adjacent intervals with small gaps in cells to reduce total interval number to maximum
void LASinterval::merge_intervals(U32 maximum_intervals, const BOOL verbose)
{
  U32 diff;
  LASintervalCell* cell;
  LASintervalCell* delete_cell;

  // each cell has minimum one interval

  if (maximum_intervals < get_number_cells())
  {
    maximum_intervals = 0;
  }
  else
  {
    maximum_intervals -= get_number_cells();
  }

  // order intervals by smallest gap

  my_cell_map map;
  my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->begin();
  while (hash_element != ((my_cell_hash*)cells)->end())
  {
    cell = (*hash_element).second;
    while (cell->next)
    {
      diff = cell->next->start - cell->end - 1;
      map.insert(my_cell_map::value_type(diff, cell));
      cell = cell->next;
    }
    hash_element++;
  }

  // maybe nothing to do
  if (map.size() <= maximum_intervals)
  {
    if (verbose)
    {
      if (map.size() == 0)
      {
        fprintf(stderr,"maximum_intervals: %u number of interval gaps: 0 \n", maximum_intervals);
      }
      else
      {
        diff = (*(map.begin())).first;
        fprintf(stderr,"maximum_intervals: %u number of interval gaps: %u next largest interval gap %u\n", maximum_intervals, (U32)map.size(), diff);
      }
    }
    return;
  }

  my_cell_map::iterator map_element;
  U32 size = (U32)map.size();

  while (size > maximum_intervals)
  {
    map_element = map.begin();
    diff = (*map_element).first;
    cell = (*map_element).second;
    map.erase(map_element);
    if ((cell->start == 1) && (cell->end == 0)) // the (start == 1 && end == 0) signals that the cell is to be deleted
    {
      number_intervals--;
      delete cell;
    }
    else
    {
      delete_cell = cell->next;
      cell->end = delete_cell->end;
      cell->next = delete_cell->next;
      if (cell->next)
      {
        map.insert(my_cell_map::value_type(cell->next->start - cell->end - 1, cell));
        delete_cell->start = 1; delete_cell->end = 0; // the (start == 1 && end == 0) signals that the cell is to be deleted
      }
      else
      {
        number_intervals--;
        delete delete_cell;
      }
      size--;
    }
  }
  map_element = map.begin();
  while (true)
  {
    if (map_element == map.end()) break;
    cell = (*map_element).second;
    if ((cell->start == 1) && (cell->end == 0)) // the (start == 1 && end == 0) signals that the cell is to be deleted
    {
      number_intervals--;
      delete cell;
    }
    map_element++;
  }
  fprintf(stderr,"largest interval gap increased to %u\n", diff);

  // update totals

  LASintervalStartCell* start_cell;
  hash_element = ((my_cell_hash*)cells)->begin();
  while (hash_element != ((my_cell_hash*)cells)->end())
  {
    start_cell = (*hash_element).second;
    start_cell->total = 0;
    cell = start_cell;
    while (cell)
    {
      start_cell->total += (cell->end - cell->start + 1);
      cell = cell->next;
    }
    hash_element++;
  }
}

void LASinterval::get_cells()
{
  last_index = I32_MIN;
  current_cell = 0;
}

BOOL LASinterval::has_cells()
{
  my_cell_hash::iterator hash_element;
  if (last_index == I32_MIN)
  {
    hash_element = ((my_cell_hash*)cells)->begin();
  }
  else
  {
    hash_element = ((my_cell_hash*)cells)->find(last_index);
    hash_element++;
  }
  if (hash_element == ((my_cell_hash*)cells)->end())
  {
    last_index = I32_MIN;
    current_cell = 0;
    return FALSE;
  }
  last_index = (*hash_element).first;
  index = (*hash_element).first;
  full = (*hash_element).second->full;
  total = (*hash_element).second->total;
  current_cell = (*hash_element).second;
  return TRUE;
}

BOOL LASinterval::get_cell(const I32 c_index)
{
  my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->find(c_index);
  if (hash_element == ((my_cell_hash*)cells)->end())
  {
    current_cell = 0;
    return FALSE;
  }
  index = (*hash_element).first;
  full = (*hash_element).second->full;
  total = (*hash_element).second->total;
  current_cell = (*hash_element).second;
  return TRUE;
}

BOOL LASinterval::add_current_cell_to_merge_cell_set()
{
  if (current_cell == 0)
  {
    return FALSE;
  }
  if (cells_to_merge == 0)
  {
    cells_to_merge = (void*) new my_cell_set;
  }
  ((my_cell_set*)cells_to_merge)->insert((LASintervalStartCell*)current_cell);
  return TRUE;
}

BOOL LASinterval::add_cell_to_merge_cell_set(const I32 c_index, const BOOL erase)
{
  my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->find(c_index);
  if (hash_element == ((my_cell_hash*)cells)->end())
  {
    return FALSE;
  }
  if (cells_to_merge == 0)
  {
    cells_to_merge = (void*) new my_cell_set;
  }
  ((my_cell_set*)cells_to_merge)->insert((*hash_element).second);
  if (erase) ((my_cell_hash*)cells)->erase(hash_element);
  return TRUE;
}

BOOL LASinterval::merge(const BOOL erase)
{
  // maybe delete temporary merge cells from the previous merge
  if (merged_cells)
  {
    if (merged_cells_temporary)
    {
      LASintervalCell* next_next;
      LASintervalCell* next = merged_cells->next;
      while (next)
      {
        next_next = next->next;
        delete next;
        next = next_next;
      }
      delete merged_cells;
    }
    merged_cells = 0;
  }
  // are there cells to merge
  if (cells_to_merge == 0) return FALSE;
  if (((my_cell_set*)cells_to_merge)->size() == 0) return FALSE;
  // is there just one cell
  if (((my_cell_set*)cells_to_merge)->size() == 1)
  {
    merged_cells_temporary = FALSE;
    // simply use this cell as the merge cell
    my_cell_set::iterator set_element = ((my_cell_set*)cells_to_merge)->begin();
    merged_cells = (*set_element);
  }
  else
  {
    merged_cells_temporary = TRUE;
    merged_cells = new LASintervalStartCell();
    // iterate over all cells and add their intervals to map
    LASintervalCell* cell;
    my_cell_map map;
    my_cell_set::iterator set_element = ((my_cell_set*)cells_to_merge)->begin();
    while (true)
    {
      if (set_element == ((my_cell_set*)cells_to_merge)->end()) break;
      cell = (*set_element);
      merged_cells->full += ((LASintervalStartCell*)cell)->full;
      while (cell)
      {
        map.insert(my_cell_map::value_type(cell->start, cell));
        cell = cell->next;
      }
      set_element++;
    }
    // initialize merged_cells with first interval
    my_cell_map::iterator map_element = map.begin();
    cell = (*map_element).second;
    map.erase(map_element);
    merged_cells->start = cell->start;
    merged_cells->end = cell->end;
    merged_cells->total = cell->end - cell->start + 1;
    if (erase) delete cell;
    // merge intervals
    LASintervalCell* last_cell = merged_cells;
    I32 diff;
    while (map.size())
    {
      map_element = map.begin();
      cell = (*map_element).second;
      map.erase(map_element);
      diff = cell->start - last_cell->end;
      if (diff > (I32)threshold)
      {
        last_cell->next = new LASintervalCell(cell);
        last_cell = last_cell->next;
        merged_cells->total += (cell->end - cell->start + 1);
      }
      else
      {
        diff = cell->end - last_cell->end;
        if (diff > 0)
        {
          last_cell->end = cell->end;
          merged_cells->total += diff;
        }
        number_intervals--;
      }
      if (erase) delete cell;
    }
  }
  current_cell = merged_cells;
  full = merged_cells->full;
  total = merged_cells->total;
  return TRUE;
}

void LASinterval::clear_merge_cell_set()
{
  if (cells_to_merge)
  {
    ((my_cell_set*)cells_to_merge)->clear();
  }
}

BOOL LASinterval::get_merged_cell()
{
  if (merged_cells)
  {
    full = merged_cells->full;
    total = merged_cells->total;
    current_cell = merged_cells;
    return TRUE;
  }
  return FALSE;
}

BOOL LASinterval::has_intervals()
{
  if (current_cell)
  {
    start = current_cell->start;
    end = current_cell->end;
    current_cell = current_cell->next;
    return TRUE;
  }
  return FALSE;
}

LASinterval::LASinterval(const U32 threshold)
{
  cells = new my_cell_hash;
  cells_to_merge = 0;
  this->threshold = threshold;
  number_intervals = 0;
  last_index = I32_MIN;
  last_cell = 0;
  current_cell = 0;
  merged_cells = 0;
  merged_cells_temporary = FALSE;
}

LASinterval::~LASinterval()
{
  // loop over all cells
  my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->begin();
  while (hash_element != ((my_cell_hash*)cells)->end())
  {
    LASintervalCell* previous_cell = (*hash_element).second;
    LASintervalCell* cell = previous_cell->next;
    while (cell)
    {
      delete previous_cell;
      previous_cell = cell;
      cell = cell->next;
    }
    delete previous_cell;
    hash_element++;
  }
  delete ((my_cell_hash*)cells);
  // maybe delete temporary merge cells from the previous merge
  if (merged_cells)
  {
    if (merged_cells_temporary)
    {
      LASintervalCell* next_next;
      LASintervalCell* next = merged_cells->next;
      while (next)
      {
        next_next = next->next;
        delete next;
        next = next_next;
      }
      delete merged_cells;
    }
    merged_cells = 0;
  }
  if (cells_to_merge) delete ((my_cell_set*)cells_to_merge);
}

BOOL LASinterval::read(ByteStreamIn* stream)
{
  char signature[4];
  try { stream->getBytes((U8*)signature, 4); } catch (...)
  {
    fprintf(stderr,"ERROR (LASinterval): reading signature\n");
    return FALSE;
  }
  if (strncmp(signature, "LASV", 4) != 0)
  {
    fprintf(stderr,"ERROR (LASinterval): wrong signature %4s instead of 'LASV'\n", signature);
    return FALSE;
  }
  U32 version;
  try { stream->get32bitsLE((U8*)&version); } catch (...)
  {
    fprintf(stderr,"ERROR (LASinterval): reading version\n");
    return FALSE;
  }
  // read number of cells
  U32 number_cells;
  try { stream->get32bitsLE((U8*)&number_cells); } catch (...)
  {
    fprintf(stderr,"ERROR (LASinterval): reading number of cells\n");
    return FALSE;
  }
  // loop over all cells
  while (number_cells)
  {
    // read index of cell
    I32 cell_index;
    try { stream->get32bitsLE((U8*)&cell_index); } catch (...)
    {
      fprintf(stderr,"ERROR (LASinterval): reading cell index\n");
      return FALSE;
    }
    // create cell and insert into hash
    LASintervalStartCell* start_cell = new LASintervalStartCell();
    ((my_cell_hash*)cells)->insert(my_cell_hash::value_type(cell_index, start_cell));
    LASintervalCell* cell = start_cell;
    // read number of intervals in cell
    U32 number_intervals;
    try { stream->get32bitsLE((U8*)&number_intervals); } catch (...)
    {
      fprintf(stderr,"ERROR (LASinterval): reading number of intervals in cell\n");
      return FALSE;
    }
    // read number of points in cell
    U32 number_points;
    try { stream->get32bitsLE((U8*)&number_points); } catch (...)
    {
      fprintf(stderr,"ERROR (LASinterval): reading number of points in cell\n");
      return FALSE;
    }
    start_cell->full = number_points;
    start_cell->total = 0;
    while (number_intervals)
    {
      // read start of interval
      try { stream->get32bitsLE((U8*)&(cell->start)); } catch (...)
      {
        fprintf(stderr,"ERROR (LASinterval): reading start %d of interval\n", cell->start);
        return FALSE;
      }
      // read end of interval
      try { stream->get32bitsLE((U8*)&(cell->end)); } catch (...)
      {
        fprintf(stderr,"ERROR (LASinterval): reading end %d of interval\n", cell->end);
        return FALSE;
      }
      start_cell->total += (cell->end - cell->start + 1);
      number_intervals--;
      if (number_intervals)
      {
        cell->next = new LASintervalCell();
        cell = cell->next;
      }
    }
    number_cells--;
  }

  return TRUE;
}

BOOL LASinterval::write(ByteStreamOut* stream) const
{
  if (!stream->putBytes((U8*)"LASV", 4))
  {
    fprintf(stderr,"ERROR (LASinterval): writing signature\n");
    return FALSE;
  }
  U32 version = 0;
  if (!stream->put32bitsLE((U8*)&version))
  {
    fprintf(stderr,"ERROR (LASinterval): writing version\n");
    return FALSE;
  }
  // write number of cells
  U32 number_cells = (U32)((my_cell_hash*)cells)->size();
  if (!stream->put32bitsLE((U8*)&number_cells))
  {
    fprintf(stderr,"ERROR (LASinterval): writing number of cells %d\n", number_cells);
    return FALSE;
  }
  // loop over all cells
  my_cell_hash::iterator hash_element = ((my_cell_hash*)cells)->begin();
  while (hash_element != ((my_cell_hash*)cells)->end())
  {
    LASintervalCell* cell = (*hash_element).second;
    // count number of intervals and points in cell
    U32 number_intervals = 0;
    U32 number_points = ((LASintervalStartCell*)cell)->full;
    while (cell)
    {
      number_intervals++;
      cell = cell->next;
    }
    // write index of cell
    I32 cell_index = (*hash_element).first;
    if (!stream->put32bitsLE((U8*)&cell_index))
    {
      fprintf(stderr,"ERROR (LASinterval): writing cell index %d\n", cell_index);
      return FALSE;
    }
    // write number of intervals in cell
    if (!stream->put32bitsLE((U8*)&number_intervals))
    {
      fprintf(stderr,"ERROR (LASinterval): writing number of intervals %d in cell\n", number_intervals);
      return FALSE;
    }
    // write number of points in cell
    if (!stream->put32bitsLE((U8*)&number_points))
    {
      fprintf(stderr,"ERROR (LASinterval): writing number of points %d in cell\n", number_points);
      return FALSE;
    }
    // write intervals
    cell = (*hash_element).second;
    while (cell)
    {
      // write start of interval
      if (!stream->put32bitsLE((U8*)&(cell->start)))
      {
        fprintf(stderr,"ERROR (LASinterval): writing start %d of interval\n", cell->start);
        return FALSE;
      }
      // write end of interval
      if (!stream->put32bitsLE((U8*)&(cell->end)))
      {
        fprintf(stderr,"ERROR (LASinterval): writing end %d of interval\n", cell->end);
        return FALSE;
      }
      cell = cell->next;
    }
    hash_element++;
  }
  return TRUE;
}
