/*
===============================================================================

  FILE:  lasreadpoint.hpp
  
  CONTENTS:
  
    Common interface for the classes that read points raw or compressed.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    28 August 2017 -- moving 'context' from global development hack to interface  
    18 July 2017 -- bug fix for spatial-indexed reading of native compressed LAS 1.4 
    19 April 2017 -- support for selective decompression for new LAS 1.4 points 
    23 August 2016 -- layering of items for selective decompression in LAS 1.4 
    6 September 2014 -- removed inheritance of EntropyEncoder and EntropyDecoder
    24 August 2014 -- delay read of chunk table until first read() or seek() is called
    6 October 2011 -- large file support & reading with missing chunk table
    9 May 2011 -- the chunked compressor now allows variable chunk sizes
    25 April 2011 -- added chunked laszip for random access decompression
    10 January 2011 -- licensing change for LGPL release and liblas integration
    7 December 2010 -- adapted from LASpointReader for better code modularity
    3 December 2010 -- updated to (somewhat) support LAS format 1.3
    7 September 2008 -- updated to support LAS format 1.2 
    22 February 2007 -- created about an hour before henna's birthday
  
===============================================================================
*/
#ifndef LAS_READ_POINT_HPP
#define LAS_READ_POINT_HPP

#include "mydefs.hpp"
#include "laszip.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "bytestreamin.hpp"

class LASreadItem;
class ArithmeticDecoder;

class LASreadPoint
{
public:
  LASreadPoint(U32 decompress_selective=LASZIP_DECOMPRESS_SELECTIVE_ALL);
  ~LASreadPoint();

  // should only be called *once*
  BOOL setup(const U32 num_items, const LASitem* items, const LASzip* laszip=0);

  BOOL init(ByteStreamIn* instream);
  BOOL seek(const U32 current, const U32 target);
  BOOL read(U8* const * point);
  BOOL check_end();
  BOOL done();

  inline const CHAR* error() const { return last_error; };
  inline const CHAR* warning() const { return last_warning; };

private:
  ByteStreamIn* instream;
  U32 num_readers;
  LASreadItem** readers;
  LASreadItem** readers_raw;
  LASreadItem** readers_compressed;
  ArithmeticDecoder* dec;
  BOOL layered_las14_compression;
  // used for chunking
  U32 chunk_size;
  U32 chunk_count;
  U32 current_chunk;
  U32 number_chunks;
  U32 tabled_chunks;
  I64* chunk_starts;
  U32* chunk_totals;
  BOOL init_dec();
  BOOL read_chunk_table();
  U32 search_chunk_table(const U32 index, const U32 lower, const U32 upper);
  // used for selective decompression (new LAS 1.4 point types only)
  U32 decompress_selective;
  // used for seeking
  I64 point_start;
  U32 point_size;
  U8** seek_point;
  // used for error and warning reporting
  CHAR* last_error;
  CHAR* last_warning;
};

#endif
