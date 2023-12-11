/*
===============================================================================

  FILE:  laskdtree.hpp
  
  CONTENTS:
  
    Tree structure for fast overlap checks of points or rectangles with list
    of rectangles 

  PROGRAMMERS:

    info@rapidlasso.de - https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2021, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:

     8 December 2023 -- Fix memory leak
    26 June 2021 -- new LASkdtreePoint after four weeks of memoy pain in Samara
    26 October 2019 -- created at LoCoworking after three days of rain in Samara
  
===============================================================================
*/
#ifndef LAS_KD_TREE_HPP
#define LAS_KD_TREE_HPP

#include "mydefs.hpp"

#include <stdlib.h>
#include <string.h>

#include <list>
#include <set>
using namespace std;

typedef set<U32> my_index_set;

class LASkdtreeRectangle
{
public:
  F64 min[2];
  F64 max[2];
  U32 idx;

  BOOL overlap(const LASkdtreeRectangle &rectangle) const;

  LASkdtreeRectangle();
  LASkdtreeRectangle(F64 min_x, F64 min_y, F64 max_x, F64 max_y);
  LASkdtreeRectangle(F64 min_x, F64 min_y, F64 max_x, F64 max_y, U32 index);
};

class LASkdtreePoint
{
public:
  F64 pos[2];

  BOOL overlap(const LASkdtreeRectangle &rectangle) const;

  LASkdtreePoint();
  LASkdtreePoint(F64 x, F64 y);
};

typedef list<LASkdtreeRectangle> my_rectangle_list;

class LASkdtreeRectanglesNode
{
public:
  F64 split;
  LASkdtreeRectanglesNode* left;
  LASkdtreeRectanglesNode* right;
  my_rectangle_list* list;

  LASkdtreeRectanglesNode();
  ~LASkdtreeRectanglesNode();
};

class LASkdtreeRectangles
{
public:
  BOOL init();
  void add(F64 min_x, F64 min_y, F64 max_x, F64 max_y);
  BOOL build();
  inline U32 size() const { return num_rectangles; };
  BOOL was_built() const;
  BOOL overlap(F64 min_x, F64 min_y, F64 max_x, F64 max_y); // rectangle
  BOOL overlap(F64 x, F64 y);                               // point
  void print_overlap();
  BOOL has_overlaps();
  BOOL get_overlap(U32& index);

  LASkdtreeRectangles();
  ~LASkdtreeRectangles();
private:
  U32 num_rectangles;
  LASkdtreeRectangle bb;
  my_rectangle_list* rectangle_list;
  LASkdtreeRectanglesNode* root;
  my_index_set* overlap_set;
  my_index_set::iterator set_element;

  void build_recursive(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreeRectangle bb, my_rectangle_list* insertion_list, I32 unchanged);
  void overlap_rectangles(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreeRectangle rectangle, my_index_set* overlap_set);
  void overlap_rectangles(LASkdtreeRectanglesNode* node, I32 plane, LASkdtreePoint point, my_index_set* overlap_set);
};

#endif
