/*
===============================================================================

  FILE:  laskdtree.cpp

  CONTENTS:
  
    see corresponding header file
  
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
  
    see corresponding header file
  
===============================================================================
*/

#include "laskdtree.hpp"

#include <stdio.h>

BOOL LASkdtreeRectangle::overlap(const LASkdtreeRectangle &rectangle) const
{
  if (max[0] < rectangle.min[0]) return FALSE;
  if (max[1] < rectangle.min[1]) return FALSE;
  if (rectangle.max[0] < min[0]) return FALSE;
  if (rectangle.max[1] < min[1]) return FALSE;
  return TRUE;
}

LASkdtreeRectangle::LASkdtreeRectangle(F64 min_x, F64 min_y, F64 max_x, F64 max_y, U32 index)
{
  min[0] = min_x;
  min[1] = min_y;
  max[0] = max_x;
  max[1] = max_y;
  idx = index;
}

LASkdtreeRectangle::LASkdtreeRectangle(F64 min_x, F64 min_y, F64 max_x, F64 max_y)
{
  min[0] = min_x;
  min[1] = min_y;
  max[0] = max_x;
  max[1] = max_y;
  idx = 0;
}

LASkdtreeRectangle::LASkdtreeRectangle()
{
  min[0] = 0;
  min[1] = 0;
  max[0] = 0;
  max[1] = 0;
  idx = 0;
}

BOOL LASkdtreePoint::overlap(const LASkdtreeRectangle &rectangle) const
{
  if (pos[0] < rectangle.min[0]) return FALSE;
  if (pos[1] < rectangle.min[1]) return FALSE;
  if (rectangle.max[0] < pos[0]) return FALSE;
  if (rectangle.max[1] < pos[1]) return FALSE;
  return TRUE;
}

LASkdtreePoint::LASkdtreePoint(F64 x, F64 y)
{
  pos[0] = x;
  pos[1] = y;
}

LASkdtreePoint::LASkdtreePoint()
{
  pos[0] = 0;
  pos[1] = 0;
}

LASkdtreeRectanglesNode::LASkdtreeRectanglesNode()
{
  left = 0;
  right = 0;
  list = 0;
}

LASkdtreeRectanglesNode::~LASkdtreeRectanglesNode()
{
  if (left) delete left;
  if (right) delete right;
  if (list) delete list;
}

BOOL LASkdtreeRectangles::init()
{
  bb.min[0] = F64_MAX;
  bb.min[1] = F64_MAX;
  bb.max[0] = F64_MIN;
  bb.max[1] = F64_MIN;
  num_rectangles = 0;
  if (rectangle_list) delete rectangle_list;
  rectangle_list = new my_rectangle_list;
  if (rectangle_list == 0)
  {
    return FALSE;
  }
  if (root) delete root;
  root = 0;
  if (overlap_set) delete overlap_set;
  overlap_set = 0;
  return TRUE;
}

void LASkdtreeRectangles::add(F64 min_x, F64 min_y, F64 max_x, F64 max_y)
{
  // update bounding box

  if (min_x < bb.min[0]) bb.min[0] = min_x;
  if (min_y < bb.min[1]) bb.min[1] = min_y;
  if (max_x > bb.max[0]) bb.max[0] = max_x;
  if (min_y > bb.max[1]) bb.max[1] = max_y;
  
  // create rectangle

  LASkdtreeRectangle rectangle(min_x, min_y, max_x, max_y, (U32)rectangle_list->size());

  // add rectangle to list

  rectangle_list->push_back(rectangle);

  // increate size meaning number of elements

  num_rectangles++;
}

BOOL LASkdtreeRectangles::build()
{
  if (root) delete root;
  root = new LASkdtreeRectanglesNode();
  if (root == 0)
  {
    return FALSE;
  }
  
  build_recursive(root, 0, bb, rectangle_list, 0);
  rectangle_list = 0;

  overlap_set = new my_index_set;
  if (overlap_set == 0)
  {
    return FALSE;
  }
  return TRUE;
}

BOOL LASkdtreeRectangles::was_built() const
{
  if (root && overlap_set)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

BOOL LASkdtreeRectangles::overlap(F64 min_x, F64 min_y, F64 max_x, F64 max_y)
{
  if (overlap_set == 0)
  {
    return FALSE;
  }
  overlap_set->clear();
  LASkdtreeRectangle rectangle(min_x, min_y, max_x, max_y);
  overlap_rectangles(root, 0, rectangle, overlap_set);
  return TRUE;
}

BOOL LASkdtreeRectangles::overlap(F64 x, F64 y)
{
  if (overlap_set == 0)
  {
    return FALSE;
  }
  overlap_set->clear();
  LASkdtreePoint point(x, y);
  overlap_rectangles(root, 0, point, overlap_set);
  return TRUE;
}

BOOL LASkdtreeRectangles::has_overlaps()
{
  if (overlap_set && overlap_set->size())
  {
    set_element = overlap_set->begin();
    return TRUE;
  }
  return FALSE;
}

BOOL LASkdtreeRectangles::get_overlap(U32& index)
{
  if (overlap_set)
  {
    if (set_element == overlap_set->end())
    {
      return FALSE;
    }
    index = (*set_element);
    set_element++;
    return TRUE;
  }
  return FALSE;
}

void LASkdtreeRectangles::build_recursive(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreeRectangle curr_bb, my_rectangle_list* insertion_list, I32 unchanged)
{
  // is list small enough? 
  if (insertion_list->size() <= 4)
  {
    node->list = insertion_list;
    return;
  }

  // did last 4 recursions not make list smaller?
  if (unchanged >= 4)
  {
    node->list = insertion_list;
    return;
  }

  F64 split = (curr_bb.min[plane] + curr_bb.max[plane]) / 2;

  my_rectangle_list* list_left = new my_rectangle_list;
  my_rectangle_list* list_right = new my_rectangle_list;

  my_rectangle_list::iterator list_element = insertion_list->begin();

  U32 total = 0;
  U32 total_left = 0;
  U32 total_right = 0;

  while (TRUE)
  {
    if (list_element == insertion_list->end())
    {
      break;
    }

    LASkdtreeRectangle rectangle = (*list_element);

    total++;

    if (rectangle.min[plane] < split)
    {
      list_left->push_back(rectangle);
      total_left++;
    }
    if (split <= rectangle.max[plane])
    {
      list_right->push_back(rectangle);
      total_right++;
    }

    list_element++;
  }

  // input list was divided into two output lists and no longer useful

  delete insertion_list;

  // split the bounding box

  LASkdtreeRectangle bb_left = curr_bb;
  bb_left.max[plane] = split;

  LASkdtreeRectangle bb_right = curr_bb;
  bb_right.min[plane] = split;

  // create the child nodes

  LASkdtreeRectanglesNode* left = new LASkdtreeRectanglesNode();
  LASkdtreeRectanglesNode* right = new LASkdtreeRectanglesNode();

  // start recursive calls

  if (total_left < total)
  {
    build_recursive(left, (plane + 1) % 2, bb_left, list_left, 0);
  }
  else
  {
    build_recursive(left, (plane + 1) % 2, bb_left, list_left, unchanged+1);
  }

  if (total_right < total)
  {
    build_recursive(right, (plane + 1) % 2, bb_right, list_right, 0);
  }
  else
  {
    build_recursive(right, (plane + 1) % 2, bb_right, list_right, unchanged+1);
  }

  // attach children to their parent

  node->split = split;
  node->left = left;
  node->right = right;
}

void LASkdtreeRectangles::overlap_rectangles(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreeRectangle rectangle, my_index_set* overlap_set)
{
  if (node->list)
  {
    my_rectangle_list::iterator list_element = node->list->begin();
    while (TRUE)
    {
      if (list_element == node->list->end())
      {
        break;
      }

      LASkdtreeRectangle overlap_candidate = (*list_element);

      if (rectangle.overlap(overlap_candidate))
      {
        overlap_set->insert(overlap_candidate.idx);
      }
      list_element++;
    }
  }
  else
  {
    // maybe recurse left

    if (rectangle.min[plane] < node->split)
    {
      overlap_rectangles(node->left, (plane + 1) % 2, rectangle, overlap_set);
    }

    // maybe recurse right
    
    if (node->split <= rectangle.max[plane])
    {
      overlap_rectangles(node->right, (plane + 1) % 2, rectangle, overlap_set);
    }
  }
}

void LASkdtreeRectangles::overlap_rectangles(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreePoint point, my_index_set* overlap_set)
{
  if (node->list)
  {
    my_rectangle_list::iterator list_element = node->list->begin();
    while (TRUE)
    {
      if (list_element == node->list->end())
      {
        break;
      }

      LASkdtreeRectangle overlap_candidate = (*list_element);

      if (point.overlap(overlap_candidate))
      {
        overlap_set->insert(overlap_candidate.idx);
      }
      list_element++;
    }
  }
  else
  {
    // maybe recurse left

    if (point.pos[plane] < node->split)
    {
      overlap_rectangles(node->left, (plane + 1) % 2, point, overlap_set);
    }

    // maybe recurse right
    
    if (node->split <= point.pos[plane])
    {
      overlap_rectangles(node->right, (plane + 1) % 2, point, overlap_set);
    }
  }
}

void LASkdtreeRectangles::print_overlap()
{
  fprintf(stderr, "overlap elements: %u\n", (U32)overlap_set->size());
  my_index_set::iterator set_element = overlap_set->begin();
  while (TRUE)
  {
    if (set_element == overlap_set->end())
    {
      break;
    }

    U32 idx = (*set_element);

    fprintf(stderr, "overlap %u\n", idx);

    set_element++;
  }
}

LASkdtreeRectangles::LASkdtreeRectangles()
{
  rectangle_list = 0;
  overlap_set = 0;
  root = 0;
}

LASkdtreeRectangles::~LASkdtreeRectangles()
{
  if (rectangle_list) delete rectangle_list;
  if (root) delete root;
  if (overlap_set) delete overlap_set;
}
