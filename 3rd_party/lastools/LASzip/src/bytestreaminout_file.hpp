/*
===============================================================================

  FILE:  bytestreaminout_file.hpp
  
  CONTENTS:
      
    Class for FILE*-based streams that both input and output with endian handling.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    29 December 2013 -- created after helping a client to QA their Optech LiDAR
  
===============================================================================
*/
#ifndef BYTE_STREAM_INOUT_FILE_HPP
#define BYTE_STREAM_INOUT_FILE_HPP

#include "bytestreamin_file.hpp"
#include "bytestreamout_file.hpp"

class ByteStreamInOutFileLE : public ByteStreamInFileLE, public ByteStreamOutFileLE
{
public:
  ByteStreamInOutFileLE(FILE* file);
};

class ByteStreamInOutFileBE : public ByteStreamInFileBE, public ByteStreamOutFileBE
{
public:
  ByteStreamInOutFileBE(FILE* file);
};

inline ByteStreamInOutFileLE::ByteStreamInOutFileLE(FILE* file) : ByteStreamInFileLE(file), ByteStreamOutFileLE(file) 
{
}

inline ByteStreamInOutFileBE::ByteStreamInOutFileBE(FILE* file) : ByteStreamInFileBE(file), ByteStreamOutFileBE(file)
{
}

#endif
