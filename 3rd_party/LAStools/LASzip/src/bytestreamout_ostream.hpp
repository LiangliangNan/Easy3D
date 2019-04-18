/*
===============================================================================

  FILE:  bytestreamout_ostream.hpp
  
    Class for ostream-based output streams with endian handling.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     1 October 2011 -- added 64 bit file support in MSVC 6.0 at McCafe at Hbf Linz
    10 January 2011 -- licensing change for LGPL release and liblas integration
    12 December 2010 -- created from ByteStreamOutFile after Howard got pushy (-;
  
===============================================================================
*/
#ifndef BYTE_STREAM_OUT_OSTREAM_H
#define BYTE_STREAM_OUT_OSTREAM_H

#include "bytestreamout.hpp"

#ifdef LZ_WIN32_VC6
#include <fstream.h>
#else
#include <istream>
#include <fstream>
using namespace std;
#endif

class ByteStreamOutOstream : public ByteStreamOut
{
public:
  ByteStreamOutOstream(ostream& stream);
/* write a single byte                                       */
  BOOL putByte(U8 byte);
/* write an array of bytes                                   */
  BOOL putBytes(const U8* bytes, U32 num_bytes);
/* is the stream seekable (e.g. standard out is not)         */
  BOOL isSeekable() const;
/* get current position of stream                            */
  I64 tell() const;
/* seek to this position in the stream                       */
  BOOL seek(const I64 position);
/* seek to the end of the file                               */
  BOOL seekEnd();
/* destructor                                                */
  ~ByteStreamOutOstream(){};
protected:
  ostream& stream;
};

class ByteStreamOutOstreamLE : public ByteStreamOutOstream
{
public:
  ByteStreamOutOstreamLE(ostream& stream);
/* write 16 bit low-endian field                             */
  BOOL put16bitsLE(const U8* bytes);
/* write 32 bit low-endian field                             */
  BOOL put32bitsLE(const U8* bytes);
/* write 64 bit low-endian field                             */
  BOOL put64bitsLE(const U8* bytes);
/* write 16 bit big-endian field                             */
  BOOL put16bitsBE(const U8* bytes);
/* write 32 bit big-endian field                             */
  BOOL put32bitsBE(const U8* bytes);
/* write 64 bit big-endian field                             */
  BOOL put64bitsBE(const U8* bytes);
private:
  U8 swapped[8];
};

class ByteStreamOutOstreamBE : public ByteStreamOutOstream
{
public:
  ByteStreamOutOstreamBE(ostream& stream);
/* write 16 bit low-endian field                             */
  BOOL put16bitsLE(const U8* bytes);
/* write 32 bit low-endian field                             */
  BOOL put32bitsLE(const U8* bytes);
/* write 64 bit low-endian field                             */
  BOOL put64bitsLE(const U8* bytes);
/* write 16 bit big-endian field                             */
  BOOL put16bitsBE(const U8* bytes);
/* write 32 bit big-endian field                             */
  BOOL put32bitsBE(const U8* bytes);
/* write 64 bit big-endian field                             */
  BOOL put64bitsBE(const U8* bytes);
private:
  U8 swapped[8];
};

inline ByteStreamOutOstream::ByteStreamOutOstream(ostream& stream_param) :
    stream(stream_param)
{
}

inline BOOL ByteStreamOutOstream::putByte(U8 byte)
{
  stream.put(byte);
  return stream.good();
}

inline BOOL ByteStreamOutOstream::putBytes(const U8* bytes, U32 num_bytes)
{
  stream.write((const char*)bytes, num_bytes);
  return stream.good();
}

inline BOOL ByteStreamOutOstream::isSeekable() const
{
  return !!(static_cast<ofstream&>(stream));
}

inline I64 ByteStreamOutOstream::tell() const
{
  return (I64)stream.tellp();
}

inline BOOL ByteStreamOutOstream::seek(I64 position)
{
  stream.seekp(static_cast<streamoff>(position));
  return stream.good();
}

inline BOOL ByteStreamOutOstream::seekEnd()
{
  stream.seekp(0, ios::end);
  return stream.good();
}

inline ByteStreamOutOstreamLE::ByteStreamOutOstreamLE(ostream& stream) : ByteStreamOutOstream(stream)
{
}

inline BOOL ByteStreamOutOstreamLE::put16bitsLE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutOstreamLE::put32bitsLE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutOstreamLE::put64bitsLE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

inline BOOL ByteStreamOutOstreamLE::put16bitsBE(const U8* bytes)
{
  swapped[0] = bytes[1];
  swapped[1] = bytes[0];
  return putBytes(swapped, 2);
}

inline BOOL ByteStreamOutOstreamLE::put32bitsBE(const U8* bytes)
{
  swapped[0] = bytes[3];
  swapped[1] = bytes[2];
  swapped[2] = bytes[1];
  swapped[3] = bytes[0];
  return putBytes(swapped, 4);
}

inline BOOL ByteStreamOutOstreamLE::put64bitsBE(const U8* bytes)
{
  swapped[0] = bytes[7];
  swapped[1] = bytes[6];
  swapped[2] = bytes[5];
  swapped[3] = bytes[4];
  swapped[4] = bytes[3];
  swapped[5] = bytes[2];
  swapped[6] = bytes[1];
  swapped[7] = bytes[0];
  return putBytes(swapped, 8);
}

inline ByteStreamOutOstreamBE::ByteStreamOutOstreamBE(ostream& stream) : ByteStreamOutOstream(stream)
{
}

inline BOOL ByteStreamOutOstreamBE::put16bitsLE(const U8* bytes)
{
  swapped[0] = bytes[1];
  swapped[1] = bytes[0];
  return putBytes(swapped, 2);
}

inline BOOL ByteStreamOutOstreamBE::put32bitsLE(const U8* bytes)
{
  swapped[0] = bytes[3];
  swapped[1] = bytes[2];
  swapped[2] = bytes[1];
  swapped[3] = bytes[0];
  return putBytes(swapped, 4);
}

inline BOOL ByteStreamOutOstreamBE::put64bitsLE(const U8* bytes)
{
  swapped[0] = bytes[7];
  swapped[1] = bytes[6];
  swapped[2] = bytes[5];
  swapped[3] = bytes[4];
  swapped[4] = bytes[3];
  swapped[5] = bytes[2];
  swapped[6] = bytes[1];
  swapped[7] = bytes[0];
  return putBytes(swapped, 8);
}

inline BOOL ByteStreamOutOstreamBE::put16bitsBE(const U8* bytes)
{
  return putBytes(bytes, 2);
}

inline BOOL ByteStreamOutOstreamBE::put32bitsBE(const U8* bytes)
{
  return putBytes(bytes, 4);
}

inline BOOL ByteStreamOutOstreamBE::put64bitsBE(const U8* bytes)
{
  return putBytes(bytes, 8);
}

#endif
