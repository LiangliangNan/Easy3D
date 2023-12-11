/*
===============================================================================

  FILE:  lasreaditem.hpp
  
  CONTENTS:
  
    Common interface for all classes that read the items that compose a point.

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
    7 December 2010 -- refactored after getting invited to KAUST in Saudi Arabia
  
===============================================================================
*/
#ifndef LAS_READ_ITEM_HPP
#define LAS_READ_ITEM_HPP

#include "mydefs.hpp"

class ByteStreamIn;

class LASreadItem
{
public:
  virtual void read(U8* item, U32& context)=0;

  virtual ~LASreadItem(){};
};

class LASreadItemRaw : public LASreadItem
{
public:
  LASreadItemRaw()
  {
    instream = 0;
  };
  BOOL init(ByteStreamIn* instream)
  {
    if (!instream) return FALSE;
    this->instream = instream;
    return TRUE;
  };
  virtual ~LASreadItemRaw(){};
protected:
  ByteStreamIn* instream;
};

class LASreadItemCompressed : public LASreadItem
{
public:
  virtual BOOL chunk_sizes() { return FALSE; };
  virtual BOOL init(const U8* item, U32& context)=0;

  virtual ~LASreadItemCompressed(){};
};

#endif
