/*
===============================================================================

  FILE:  lastransform.hpp
  
  CONTENTS:
  
    Transforms LIDAR points with a number of different operations.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2018, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     6 March 2018 -- changed '%g' to '%lf' for all sprintf() of F64 values
    28 February 2017 -- now '-set_RGB_of_class' also works for classifications > 31
     1 February 2017 -- new '-copy_intensity_into_z' for use in lasgrid or lascanopy
     9 May 2016 -- new '-translate_raw_xy_at_random 2 2' for random pertubation
    20 April 2016 -- new '-switch_R_G', '-switch_R_B' and '-set_RGB 32768 16384 0'
    25 January 2016 -- brand-new opportunity to do a '-filtered_transform' 
    18 December 2011 -- added '-flip_waveform_direction' to deal with Riegl's data 
    20 March 2011 -- added -translate_raw_xyz after the fullest of full moons
    21 January 2011 -- re-created after matt told me about the optech dashmap bug
  
===============================================================================
*/
#ifndef LAS_TRANSFORM_HPP
#define LAS_TRANSFORM_HPP

#include "lasdefinitions.hpp"
#include "laszip_decompress_selective_v3.hpp"

class LASfilter;

class LASoperation
{
public:
  virtual const CHAR * name() const = 0;
  virtual I32 get_command(CHAR* string) const = 0;
  virtual U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY; };
  virtual void transform(LASpoint* point) = 0;
  virtual void reset(){};
  virtual ~LASoperation(){};
};

class LAStransform
{
public:

  BOOL change_coordinates;

  void usage() const;
  void clean();
  BOOL parse(int argc, char* argv[]);
  BOOL parse(CHAR* string);
  I32 unparse(CHAR* string) const;
  inline BOOL active() const { return (num_operations != 0); };
  U32 get_decompress_selective() const;
  inline BOOL filtered() const { return is_filtered; };

  void setFilter(LASfilter* filter);

  void setPointSource(U16 value);

  void transform(LASpoint* point);
  void reset();

  LAStransform();
  ~LAStransform();

private:

  void add_operation(LASoperation* operation);
  U32 num_operations;
  U32 alloc_operations;
  LASoperation** operations;
  BOOL is_filtered;
  LASfilter* filter;
};

#endif
