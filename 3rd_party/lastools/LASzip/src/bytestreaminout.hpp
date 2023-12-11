/*
===============================================================================

  FILE:  bytestreaminout.hpp
  
  CONTENTS:
      
    Abstract base class for streams that both input and output with endian handling.

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
  
    29 December 2013 -- created after helping a client to QA their Optech LiDAR
  
===============================================================================
*/
#ifndef BYTE_STREAM_INOUT_HPP
#define BYTE_STREAM_INOUT_HPP

#include "mydefs.hpp"
#include "bytestreamin.hpp"
#include "bytestreamout.hpp"

class ByteStreamInOut : public ByteStreamIn, ByteStreamOut
{
public:
/* destructor                                                */
  virtual ~ByteStreamInOut() {};
};

#endif
