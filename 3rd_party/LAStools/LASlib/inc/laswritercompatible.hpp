/*
===============================================================================

  FILE:  laswritercompatible.hpp
  
  CONTENTS:
  
    Interface to write LIDAR points to the LAS format versions 1.0 - 1.4 and
    per on-the-fly conversion to simple ASCII files.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    29 March 2015 -- created on the last PHIL LiDAR tour 2015 day in Ali Mall
  
===============================================================================
*/
#ifndef LAS_WRITER_COMPATIBLE_HPP
#define LAS_WRITER_COMPATIBLE_HPP

#include "laswriter.hpp"

class LASwriterCompatibleDown : public LASwriter
{
public:
  BOOL open(LASheader* header, LASwriteOpener* laswriteopener, BOOL moveCRSfromEVLRtoVLR=FALSE, BOOL moveEVLRtoVLR=FALSE);

  BOOL write_point(const LASpoint* point);
  BOOL chunk() { return FALSE; };

  BOOL update_header(const LASheader* header, BOOL use_inventory=FALSE, BOOL update_extra_bytes=FALSE);
  I64 close(BOOL update_npoints=TRUE);

  LASwriterCompatibleDown();
  ~LASwriterCompatibleDown();

private:
  LASpoint pointCompatibleDown;
  LASheader* header;
  LASwriter* writer;
  I32 start_scan_angle;
  I32 start_extended_returns;
  I32 start_classification;
  I32 start_flags_and_channel;
  I32 start_NIR_band;
};

class LASwriterCompatibleUp : public LASwriter
{
public:
  BOOL open(LASheader* header, LASwriteOpener* laswriteopener);

  BOOL write_point(const LASpoint* point);
  BOOL chunk() { return FALSE; };

  BOOL update_header(const LASheader* header, BOOL use_inventory=FALSE, BOOL update_extra_bytes=FALSE);
  I64 close(BOOL update_npoints=TRUE);

  LASwriterCompatibleUp();
  ~LASwriterCompatibleUp();

private:
  LASpoint pointCompatibleUp;
  LASheader* header;
  LASwriter* writer;
  I32 start_scan_angle;
  I32 start_extended_returns;
  I32 start_classification;
  I32 start_flags_and_channel;
  I32 start_NIR_band;
};

#endif
