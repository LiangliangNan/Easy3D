/*
===============================================================================

  FILE:  laspoint.hpp
  
  CONTENTS:
  
    This class describes an LAS point and offers helper functions to access,
    convert, and set the default (and any additional) point attributes.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2017, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    15 June 2018 -- fix in flag copy from legacy (0-5) to extended (6-10) type
    10 March 2017 -- fix in copy_to() and copy_from() new LAS 1.4 point types
    10 October 2016 -- small fixes for NIR and extended scanner channel
    19 July 2015 -- created after FOSS4GE in the train back from Lake Como
  
===============================================================================
*/
#ifndef LAS_POINT_HPP
#define LAS_POINT_HPP

#include "lasquantizer.hpp"
#include "lasattributer.hpp"

class LASwavepacket
{
public:
  LASwavepacket() {zero();};
  void zero() {memset(data, 0, 29);};
  inline U8 getIndex() const {return data[0];};
  inline U64 getOffset() const {return ((U64*)&(data[1]))[0];};
  inline U32 getSize() const {return ((U32*)&(data[9]))[0];};
  inline F32 getLocation() const {return ((F32*)&(data[13]))[0];};
  inline F32 getXt() const {return ((F32*)&(data[17]))[0];};
  inline F32 getYt() const {return ((F32*)&(data[21]))[0];};
  inline F32 getZt() const {return ((F32*)&(data[25]))[0];};
  inline void setIndex(U8 index) {data[0] = index;};
  inline void setOffset(U64 offset) {((U64*)&(data[1]))[0] = offset;};
  inline void setSize(U32 size) {((U32*)&(data[9]))[0] = size;};
  inline void setLocation(F32 location) { ((F32*)&(data[13]))[0] = location;};
  inline void setXt(F32 xt) {((F32*)&(data[17]))[0] = xt;};
  inline void setYt(F32 yt) {((F32*)&(data[21]))[0] = yt;};
  inline void setZt(F32 zt) {((F32*)&(data[25]))[0] = zt;};
  inline void flipDirection() {((F32*)&(data[17]))[0] *= -1; ((F32*)&(data[21]))[0] *= -1; ((F32*)&(data[25]))[0] *= -1;};
private:
  U8 data[29];
};

class LASpoint
{
public:

// these fields contain the data that describe each point

  I32 X;
  I32 Y;
  I32 Z;
  U16 intensity;
  U8 return_number : 3;
  U8 number_of_returns : 3;
  U8 scan_direction_flag : 1;
  U8 edge_of_flight_line : 1;
  U8 classification : 5;
  U8 synthetic_flag : 1;
  U8 keypoint_flag  : 1;
  U8 withheld_flag  : 1;
  I8 scan_angle_rank;
  U8 user_data;
  U16 point_source_ID;

  // LAS 1.4 only
  I16 extended_scan_angle;
  U8 extended_point_type : 2;
  U8 extended_scanner_channel : 2;
  U8 extended_classification_flags : 4;
  U8 extended_classification;
  U8 extended_return_number : 4;
  U8 extended_number_of_returns : 4;

  // LASlib internal use only
  U8 deleted_flag;

  // for 8 byte alignment of the GPS time
  U8 dummy[2];

  // compressed LASzip 1.4 points only
  BOOL gps_time_change;

  F64 gps_time;
  U16 rgb[4];
  LASwavepacket wavepacket;

  U8* extra_bytes;

// for converting between x,y,z integers and scaled/translated coordinates

  const LASquantizer* quantizer;
  F64 coordinates[3];

// for attributed access to the extra bytes

  const LASattributer* attributer;

// this field provides generic access to the point data

  U8** point;

// these fields describe the point format LAS specific

  BOOL have_gps_time;
  BOOL have_rgb;
  BOOL have_nir;
  BOOL have_wavepacket;
  I32 extra_bytes_number;
  U32 total_point_size;

// these fields describe the point format terms of generic items

  U16 num_items;
  LASitem* items;

// copy functions

  LASpoint(const LASpoint & other)
  {
    *this = other;
  }

  LASpoint & operator=(const LASpoint & other)
  {
    X = other.X;
    Y = other.Y;
    Z = other.Z;
    intensity = other.intensity;
    return_number = other.return_number;
    number_of_returns = other.number_of_returns;
    scan_direction_flag = other.scan_direction_flag;
    edge_of_flight_line = other.edge_of_flight_line;
    classification = other.classification;
    synthetic_flag = other.synthetic_flag;
    keypoint_flag = other.keypoint_flag;
    withheld_flag = other.withheld_flag;
    scan_angle_rank = other.scan_angle_rank;
    user_data = other.user_data;
    point_source_ID = other.point_source_ID;
    deleted_flag = other.deleted_flag;

    if (other.have_gps_time)
    {
      gps_time = other.gps_time;
    }
    if (other.have_rgb)
    {
      rgb[0] = other.rgb[0];
      rgb[1] = other.rgb[1];
      rgb[2] = other.rgb[2];
      if (other.have_nir)
      {
        rgb[3] = other.rgb[3];
      }
    }
    if (other.have_wavepacket)
    {
      wavepacket = other.wavepacket;
    }
    if (other.extra_bytes && extra_bytes)
    {
      memcpy(extra_bytes, other.extra_bytes, extra_bytes_number);
    }
    if (other.extended_point_type)
    {
      extended_classification = other.extended_classification;
      extended_classification_flags = other.extended_classification_flags;
      extended_number_of_returns = other.extended_number_of_returns;
      extended_return_number = other.extended_return_number;
      extended_scan_angle = other.extended_scan_angle;
      extended_scanner_channel = other.extended_scanner_channel;
    }
    else if (extended_point_type)
    {
      extended_classification = other.classification;
      extended_classification_flags = ((other.withheld_flag) << 2) | ((other.keypoint_flag) << 1) | (other.synthetic_flag);
      extended_number_of_returns = other.number_of_returns;
      extended_return_number = other.return_number;
      extended_scan_angle = I16_QUANTIZE(((F32)other.scan_angle_rank)/0.006);
      extended_scanner_channel = other.extended_scanner_channel;
    }

    return *this;
  };

  void copy_to(U8* buffer) const
  {
    if (extended_point_type)
    {
      memcpy(buffer, &X, 14);
      buffer[14] = ((U8*)&X)[24]; // extended return number and number of returns
      buffer[15] = (((U8*)&X)[14] & 0xC0) | (extended_scanner_channel << 4) | (extended_classification_flags & 0x08) | ((((U8*)&X)[15]) >> 5);
      buffer[16] = ((U8*)&X)[23]; // extended classification
      buffer[17] = ((U8*)&X)[17]; // user data
      ((I16*)buffer)[9] = ((I16*)&X)[10]; // extended scan angle
      ((U16*)buffer)[10] = ((U16*)&X)[9]; // point source ID
      memcpy(buffer+22, &gps_time, 8);
    }
    else
    {
      memcpy(buffer, &X, 20);
    }
    U32 i;
    U32 b = items[0].size;
    for (i = 1; i < num_items; i++)
    {
      memcpy(&buffer[b], point[i], items[i].size);
      b += items[i].size;
    }
  };

  void copy_from(const U8* buffer)
  {
    if (extended_point_type)
    {
      memcpy(&X, buffer, 14);
      ((U8*)&X)[24] = buffer[14]; // extended return number and number of returns
      extended_classification_flags = buffer[15] & 0x0F;
      ((U8*)&X)[15] = (buffer[15] & 0x07) << 5; // legacy classification flags
      extended_scanner_channel = (buffer[15] >> 4) & 0x03;
      scan_direction_flag = (buffer[15] >> 6) & 0x01;
      edge_of_flight_line = (buffer[15] >> 7) & 0x01;
      ((U8*)&X)[23] = buffer[16]; // extended classification
      if (extended_classification < 32) classification = extended_classification;
      ((U8*)&X)[17] = buffer[17]; // user data
      ((I16*)&X)[10] = ((I16*)buffer)[9]; // extended scan angle
      ((U16*)&X)[9] = ((U16*)buffer)[10]; // point source ID
      memcpy(&gps_time, buffer+22, 8);
    }
    else
    {
      memcpy(&X, buffer, 20);
    }
    U32 i;
    U32 b = items[0].size;
    for (i = 1; i < num_items; i++)
    {
      memcpy(point[i], &buffer[b], items[i].size);
      b += items[i].size;
    }
  };

// these functions set the desired point format (and maybe add on attributes in extra bytes)

  BOOL init(const LASquantizer* quantizer, const U8 point_type, const U16 point_size, const LASattributer* attributer=0)
  {
    // clean the point

    clean();

    // switch over the point types we know

    if (!LASzip().setup(&num_items, &items, point_type, point_size, LASZIP_COMPRESSOR_NONE))
    {
      fprintf(stderr,"ERROR: unknown point type %d with point size %d\n", (I32)point_type, (I32)point_size);
      return FALSE;
    }

    // create point's item pointers

    point = new U8*[num_items];

    U16 i;
    for (i = 0; i < num_items; i++)
    {
      total_point_size += items[i].size;
      switch (items[i].type)
      {
      case LASitem::POINT14:
        have_gps_time = TRUE;
        extended_point_type = 1;
      case LASitem::POINT10:
        this->point[i] = (U8*)&(this->X);
        break;
      case LASitem::GPSTIME11:
        have_gps_time = TRUE;
        this->point[i] = (U8*)&(this->gps_time);
        break;
      case LASitem::RGBNIR14:
        have_nir = TRUE;
      case LASitem::RGB12:
      case LASitem::RGB14:
        have_rgb = TRUE;
        this->point[i] = (U8*)(this->rgb);
        break;
      case LASitem::WAVEPACKET13:
      case LASitem::WAVEPACKET14:
        have_wavepacket = TRUE;
        this->point[i] = (U8*)&(this->wavepacket);
        break;
      case LASitem::BYTE:
      case LASitem::BYTE14:
        extra_bytes_number = items[i].size;
        extra_bytes = new U8[extra_bytes_number];
        this->point[i] = extra_bytes;
        break;
      default:
        return FALSE;
      }
    }
    this->quantizer = quantizer;
    this->attributer = attributer;
    return TRUE;
  };

  BOOL init(const LASquantizer* quantizer, const U32 num_items, const LASitem* items, const LASattributer* attributer=0)
  {
    U32 i;

    // clean the point

    clean();

    // create item description

    this->num_items = num_items;
    if (this->items) delete [] this->items;
    this->items = new LASitem[num_items];
    if (this->point) delete [] this->point;
    this->point = new U8*[num_items];

    for (i = 0; i < num_items; i++)
    {
      this->items[i] = items[i];
      total_point_size += items[i].size;
      switch (items[i].type)
      {
      case LASitem::POINT14:
        have_gps_time = TRUE;
        extended_point_type = 1;
      case LASitem::POINT10:
        this->point[i] = (U8*)&(this->X);
        break;
      case LASitem::GPSTIME11:
        have_gps_time = TRUE;
        this->point[i] = (U8*)&(this->gps_time);
        break;
      case LASitem::RGBNIR14:
        have_nir = TRUE;
      case LASitem::RGB12:
      case LASitem::RGB14:
        have_rgb = TRUE;
        this->point[i] = (U8*)(this->rgb);
        break;
      case LASitem::WAVEPACKET13:
      case LASitem::WAVEPACKET14:
        have_wavepacket = TRUE;
        this->point[i] = (U8*)&(this->wavepacket);
        break;
      case LASitem::BYTE:
      case LASitem::BYTE14:
        extra_bytes_number = items[i].size;
        extra_bytes = new U8[extra_bytes_number];
        this->point[i] = extra_bytes;
        break;
      default:
        return FALSE;
      }
    }
    this->quantizer = quantizer;
    this->attributer = attributer;
    return TRUE;
  };

  BOOL inside_rectangle(const F64 r_min_x, const F64 r_min_y, const F64 r_max_x, const F64 r_max_y) const
  {
    F64 xy;
    xy = get_x();
    if (xy < r_min_x || xy >= r_max_x) return FALSE;
    xy = get_y();
    if (xy < r_min_y || xy >= r_max_y) return FALSE;
    return TRUE;
  }

  BOOL inside_tile(const F32 ll_x, const F32 ll_y, const F32 ur_x, const F32 ur_y) const
  {
    F64 xy;
    xy = get_x();
    if (xy < ll_x || xy >= ur_x) return FALSE;
    xy = get_y();
    if (xy < ll_y || xy >= ur_y) return FALSE;
    return TRUE;
  }

  BOOL inside_circle(const F64 center_x, const F64 center_y, F64 squared_radius) const
  {
    F64 dx = center_x - get_x();
    F64 dy = center_y - get_y();
    return ((dx*dx+dy*dy) < squared_radius);
  }

  BOOL inside_box(const F64 min_x, const F64 min_y, const F64 min_z, const F64 max_x, const F64 max_y, const F64 max_z) const
  {
    F64 xyz;
    xyz = get_x();
    if (xyz < min_x || xyz >= max_x) return FALSE;
    xyz = get_y();
    if (xyz < min_y || xyz >= max_y) return FALSE;
    xyz = get_z();
    if (xyz < min_z || xyz >= max_z) return FALSE;
    return TRUE;
  }

  BOOL inside_bounding_box(const F64 min_x, const F64 min_y, const F64 min_z, const F64 max_x, const F64 max_y, const F64 max_z) const
  {
    F64 xyz;
    xyz = get_x();
    if (xyz < min_x || xyz > max_x) return FALSE;
    xyz = get_y();
    if (xyz < min_y || xyz > max_y) return FALSE;
    xyz = get_z();
    if (xyz < min_z || xyz > max_z) return FALSE;
    return TRUE;
  }

  BOOL is_zero() const
  {
    if (((U32*)&(this->X))[0] || ((U32*)&(this->X))[1] || ((U32*)&(this->X))[2] || ((U32*)&(this->X))[3] || ((U32*)&(this->X))[4])
    {
      return FALSE;
    }
    if (have_gps_time)
    {
      if (this->gps_time)
      {
        return FALSE;
      }
    }
    if (have_rgb)
    {
      if (this->rgb[0] || this->rgb[1] || this->rgb[2])
      {
        return FALSE;
      }
      if (have_nir)
      {
        if (this->rgb[3])
        {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  void zero()
  {
    X = 0;
    Y = 0;
    Z = 0;
    intensity = 0;
    return_number = 1;
    number_of_returns = 1;
    scan_direction_flag = 0;
    edge_of_flight_line = 0;
    classification = 0;
    synthetic_flag = 0;
    keypoint_flag = 0;
    withheld_flag = 0;
    scan_angle_rank = 0;
    user_data = 0;
    point_source_ID = 0;

    // LAS 1.4 only
    extended_scan_angle = 0;
    extended_scanner_channel = 0;
    extended_classification_flags = 0;
    extended_classification = 0;
    extended_return_number = 1;
    extended_number_of_returns = 1;

    // LASlib only
    deleted_flag = 0;

    gps_time = 0.0;
    rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0;
    wavepacket.zero();
  };

  void clean()
  {
    zero();

    if (extra_bytes)
    {
      delete [] extra_bytes;
      extra_bytes = 0;
    };

    if (point) delete [] point;
    point = 0;

    have_gps_time = FALSE;
    have_rgb = FALSE;
    have_wavepacket = FALSE;
    have_nir = FALSE;
    extra_bytes_number = 0;
    total_point_size = 0;
    
    num_items = 0;
    if (items) delete [] items;
    items = 0;

    // LAS 1.4 only
    extended_point_type = 0;
  };

  LASpoint()
  {
    extra_bytes = 0;
    point = 0;
    items = 0;
    clean();
  };

  inline BOOL is_first() const { return get_return_number() <= 1; };
  inline BOOL is_intermediate() const { return (!is_first() && !is_last()); };
  inline BOOL is_last() const { return get_return_number() >= get_number_of_returns(); };
  inline BOOL is_single() const { return get_number_of_returns() <= 1; };

  inline BOOL is_first_of_many() const { return !is_single() && is_first(); };
  inline BOOL is_last_of_many() const { return !is_single() && is_last(); };

  inline I32 get_X() const { return X; };
  inline I32 get_Y() const { return Y; };
  inline I32 get_Z() const { return Z; };
  inline U16 get_intensity() const { return intensity; };
  inline U8 get_return_number() const { return return_number; };
  inline U8 get_number_of_returns() const { return number_of_returns; };
  inline U8 get_scan_direction_flag() const { return scan_direction_flag; };
  inline U8 get_edge_of_flight_line() const { return edge_of_flight_line; };
  inline U8 get_classification() const { return classification; };
  inline U8 get_synthetic_flag() const { return synthetic_flag; };
  inline U8 get_keypoint_flag() const { return keypoint_flag; };
  inline U8 get_withheld_flag() const { return withheld_flag; };
  inline I8 get_scan_angle_rank() const { return scan_angle_rank; };
  inline U8 get_user_data() const { return user_data; };
  inline U16 get_point_source_ID() const { return point_source_ID; };
  inline U8 get_deleted_flag() const { return deleted_flag; };
  inline F64 get_gps_time() const { return gps_time; };
  inline const U16* get_rgb() const { return rgb; };
  inline U16 get_R() const { return rgb[0]; };
  inline U16 get_G() const { return rgb[1]; };
  inline U16 get_B() const { return rgb[2]; };
  inline U16 get_I() const { return rgb[3]; };
  inline U16 get_NIR() const { return rgb[3]; };
  
  inline void set_X(const I32 X) { this->X = X; };
  inline void set_Y(const I32 Y) { this->Y = Y; };
  inline void set_Z(const I32 Z) { this->Z = Z; };
  inline void set_intensity(const U16 intensity) { this->intensity = intensity; };
  inline void set_return_number(const U8 return_number) { this->return_number = (return_number > 7 ? 7 : return_number); };
  inline void set_number_of_returns(const U8 number_of_returns) { this->number_of_returns = (number_of_returns > 7 ? 7 : number_of_returns); };
  inline void set_scan_direction_flag(const U8 scan_direction_flag) { this->scan_direction_flag = scan_direction_flag; };
  inline void set_edge_of_flight_line(const U8 edge_of_flight_line) { this->edge_of_flight_line = edge_of_flight_line; };
  inline void set_classification(U8 classification) { if (classification < 32) { this->classification = classification; this->extended_classification = classification; } };
  inline void set_synthetic_flag(U8 synthetic_flag) { if (synthetic_flag) { this->synthetic_flag = 1; this->extended_classification_flags |= 0x01; } else { this->synthetic_flag = 0; this->extended_classification_flags &= 0x0E; } };
  inline void set_keypoint_flag(U8 keypoint_flag) { if (keypoint_flag) { this->keypoint_flag = 1; this->extended_classification_flags |= 0x02; } else { this->keypoint_flag = 0; this->extended_classification_flags &= 0x0D; } };
  inline void set_withheld_flag(U8 withheld_flag) { if (withheld_flag) { this->withheld_flag = 1; this->extended_classification_flags |= 0x04; } else { this->withheld_flag = 0; this->extended_classification_flags &= 0x0B; } };
  inline void set_scan_angle_rank(I8 scan_angle_rank) { this->scan_angle_rank = scan_angle_rank; };
  inline void set_user_data(U8 user_data) { this->user_data = user_data; };
  inline void set_point_source_ID(U16 point_source_ID) { this->point_source_ID = point_source_ID; };
  inline void set_deleted_flag(U8 deleted_flag) { this->deleted_flag = deleted_flag; };
  inline void set_gps_time(const F64 gps_time) { this->gps_time = gps_time; };
  inline void set_RGB(const U16* rgb) { memcpy(this->rgb, rgb, sizeof(U16) * 3); };
  inline void set_RGBI(const U16* rgb) { memcpy(this->rgb, rgb, sizeof(U16) * 4); };
  inline void set_R(const U16 R) { this->rgb[0] = R; };
  inline void set_G(const U16 G) { this->rgb[1] = G; };
  inline void set_B(const U16 B) { this->rgb[2] = B; };
  inline void set_I(const U16 I) { this->rgb[3] = I; };
  inline void set_NIR(const U16 NIR) { this->rgb[3] = NIR; };

  inline F64 get_x() const { return quantizer->get_x(X); };
  inline F64 get_y() const { return quantizer->get_y(Y); };
  inline F64 get_z() const { return quantizer->get_z(Z); };

  inline void set_x(const F64 x) { this->X = quantizer->get_X(x); };
  inline void set_y(const F64 y) { this->Y = quantizer->get_Y(y); };
  inline void set_z(const F64 z) { this->Z = quantizer->get_Z(z); };

  inline BOOL is_extended_point_type() const { return extended_point_type; };

  inline U8 get_extended_classification() const { return extended_classification; };
  inline U8 get_extended_return_number() const { return extended_return_number; };
  inline U8 get_extended_number_of_returns() const { return extended_number_of_returns; };
  inline I16 get_extended_scan_angle() const { return extended_scan_angle; };
  inline U8 get_extended_overlap_flag() const { return (extended_classification_flags >> 3); };
  inline U8 get_extended_scanner_channel() const { return extended_scanner_channel; };

  inline void set_extended_classification(U8 extended_classification) { this->extended_classification = extended_classification; if (extended_classification > 31) this->classification = 0; else this->classification = extended_classification; };
  inline void set_extended_return_number(U8 extended_return_number) { this->extended_return_number = extended_return_number; };
  inline void set_extended_number_of_returns(U8 extended_number_of_returns) { this->extended_number_of_returns = extended_number_of_returns; };
  inline void set_extended_scan_angle(I16 extended_scan_angle) { this->extended_scan_angle = extended_scan_angle; };
  inline void set_extended_overlap_flag(U8 extended_overlap_flag) { this->extended_classification_flags = (extended_overlap_flag << 3) | (this->extended_classification_flags & 7); };
  inline void set_extended_scanner_channel(U8 extended_scanner_channel) { this->extended_scanner_channel = extended_scanner_channel; };

  inline F32 get_scan_angle() const { if (extended_point_type) return 0.006f*extended_scan_angle; else return (F32)scan_angle_rank; };
  inline F32 get_abs_scan_angle() const { if (extended_point_type) return (extended_scan_angle < 0 ? -0.006f*extended_scan_angle : 0.006f*extended_scan_angle) ; else return (scan_angle_rank < 0 ? (F32)-scan_angle_rank : (F32)scan_angle_rank); };

  inline void set_scan_angle(F32 scan_angle) { if (extended_point_type) set_extended_scan_angle(I16_QUANTIZE(scan_angle/0.006f)); else set_scan_angle_rank(I8_QUANTIZE(scan_angle)); };

  inline void compute_coordinates()
  {
    coordinates[0] = get_x();
    coordinates[1] = get_y();
    coordinates[2] = get_z();
  };

  inline void compute_XYZ()
  {
    set_x(coordinates[0]);
    set_y(coordinates[1]);
    set_z(coordinates[2]);
  };

  inline void compute_XYZ(const LASquantizer* quantizer)
  {
    X = quantizer->get_X(coordinates[0]);
    Y = quantizer->get_Y(coordinates[1]);
    Z = quantizer->get_Z(coordinates[2]);
  };

  // generic functions for attributes in extra bytes

  inline BOOL has_attribute(U32 index) const
  {
    if (attributer)
    {
      if (((I32)index) < attributer->number_attributes)
      {
        return TRUE;
      }
    }
    return FALSE;
  };

  inline BOOL get_attribute(U32 index, U8* data) const
  {
    if (has_attribute(index))
    {
      memcpy(data, extra_bytes + attributer->attribute_starts[index], attributer->attribute_sizes[index]);
      return TRUE;
    }
    return FALSE;
  };

  inline BOOL set_attribute(U32 index, const U8* data) 
  {
    if (has_attribute(index))
    {
      memcpy(extra_bytes + attributer->attribute_starts[index], data, attributer->attribute_sizes[index]);
      return TRUE;
    }
    return FALSE;
  };

  inline const CHAR* get_attribute_name(U32 index) const
  {
    if (has_attribute(index))
    {
      return attributer->attributes[index].name;
    }
    return 0;
  };

  inline F64 get_attribute_as_float(U32 index) const
  {
    if (has_attribute(index))
    {
      return attributer->attributes[index].get_value_as_float(extra_bytes + attributer->attribute_starts[index]);
    }
    return 0.0;
  };

  // typed and offset functions for attributes in extra bytes (more efficient)

  inline void get_attribute(I32 start, U8 &data) const { data = extra_bytes[start]; };
  inline void set_attribute(I32 start, U8 data) { extra_bytes[start] = data; };
  inline void get_attribute(I32 start, I8 &data) const { data = (I8)(extra_bytes[start]); };
  inline void set_attribute(I32 start, I8 data) { extra_bytes[start] = data; };
  inline void get_attribute(I32 start, U16 &data) const { data = *((U16*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U16 data) { *((U16*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I16 &data) const { data = *((I16*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I16 data) { *((I16*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, U32 &data) const { data = *((U32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U32 data) { *((U32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I32 &data) const { data = *((I32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I32 data) { *((I32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, U64 &data) const { data = *((U64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, U64 data) { *((U64*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, I64 &data) const { data = *((I64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, I64 data) { *((I64*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, F32 &data) const { data = *((F32*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, F32 data) { *((F32*)(extra_bytes + start)) = data; };
  inline void get_attribute(I32 start, F64 &data) const { data = *((F64*)(extra_bytes + start)); };
  inline void set_attribute(I32 start, F64 data) { *((F64*)(extra_bytes + start)) = data; };

  ~LASpoint()
  {
    clean();
  };
};

#endif
