/*
===============================================================================

  FILE:  laswriteitem.hpp
  
  CONTENTS:
  
    Common interface for all classes that write the items that compose a point.

  PROGRAMMERS:

    info@rapidlasso.de  -  https://rapidlasso.de

  COPYRIGHT:

    (c) 2007-2022, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 August 2017 -- moving 'context' from global development hack to interface  
    23 August 2016 -- layering of items for selective decompression in LAS 1.4 
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- refactored after watching two movies with silke
  
===============================================================================
*/
#ifndef LAS_WRITE_ITEM_HPP
#define LAS_WRITE_ITEM_HPP

#include "mydefs.hpp"

class ByteStreamOut;

class LASwriteItem
{
public:
  virtual BOOL write(const U8* item, U32& context)=0;

  virtual ~LASwriteItem(){};
};

class LASwriteItemRaw : public LASwriteItem
{
public:
  LASwriteItemRaw()
  {
    outstream = 0;
  };
  BOOL init(ByteStreamOut* outstream)
  {
    if (!outstream) return FALSE;
    this->outstream = outstream;
    return TRUE;
  };
  virtual ~LASwriteItemRaw(){};
protected:
  ByteStreamOut* outstream;
};

class LASwriteItemCompressed : public LASwriteItem
{
public:
  virtual BOOL init(const U8* item, U32& context)=0;
  virtual BOOL chunk_sizes() { return FALSE; };
  virtual BOOL chunk_bytes() { return FALSE; };

  virtual ~LASwriteItemCompressed(){};
};

#endif
