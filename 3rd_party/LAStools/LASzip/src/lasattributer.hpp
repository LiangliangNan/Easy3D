/*
===============================================================================

  FILE:  lasattributer.hpp
  
  CONTENTS:
  
    This class assists with handling the "extra bytes" that allow storing 
    additional per point attributes.

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
  
    13 September 2018 -- removed tuples and triple support from attributes
    19 July 2015 -- created after FOSS4GE in the train back from Lake Como
  
===============================================================================
*/
#ifndef LAS_ATTRIBUTER_HPP
#define LAS_ATTRIBUTER_HPP

#include "mydefs.hpp"

#define LAS_ATTRIBUTE_U8  0
#define LAS_ATTRIBUTE_I8  1
#define LAS_ATTRIBUTE_U16 2
#define LAS_ATTRIBUTE_I16 3
#define LAS_ATTRIBUTE_U32 4
#define LAS_ATTRIBUTE_I32 5
#define LAS_ATTRIBUTE_U64 6
#define LAS_ATTRIBUTE_I64 7
#define LAS_ATTRIBUTE_F32 8
#define LAS_ATTRIBUTE_F64 9

class LASattribute
{
public:
  U8 reserved[2];           // 2 bytes
  U8 data_type;             // 1 byte
  U8 options;               // 1 byte
  CHAR name[32];            // 32 bytes
  U8 unused[4];             // 4 bytes
  U64I64F64 no_data[3];     // 24 = 3*8 bytes
  U64I64F64 min[3];         // 24 = 3*8 bytes
  U64I64F64 max[3];         // 24 = 3*8 bytes
  F64 scale[3];             // 24 = 3*8 bytes
  F64 offset[3];            // 24 = 3*8 bytes
  CHAR description[32];     // 32 bytes

  LASattribute(U8 size)
  {
    if (size == 0) throw;
    memset(this, 0, sizeof(LASattribute));
    scale[0] = scale[1] = scale[2] = 1.0;
    this->options = size;
  };

  LASattribute(U32 type, const CHAR* name, const CHAR* description=0)
  {
    if (type > LAS_ATTRIBUTE_F64) throw;
    if (name == 0) throw;
    memset(this, 0, sizeof(LASattribute));
    scale[0] = scale[1] = scale[2] = 1.0;
    this->data_type = type+1;
    strncpy(this->name, name, 32);
    if (description) strncpy(this->description, description, 32);
  };

  inline BOOL set_no_data(U8  no_data) { if (0 == get_type()) { this->no_data[0].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(I8  no_data) { if (1 == get_type()) { this->no_data[0].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(U16 no_data) { if (2 == get_type()) { this->no_data[0].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(I16 no_data) { if (3 == get_type()) { this->no_data[0].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(U32 no_data) { if (4 == get_type()) { this->no_data[0].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(I32 no_data) { if (5 == get_type()) { this->no_data[0].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(U64 no_data) { if (6 == get_type()) { this->no_data[0].u64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(I64 no_data) { if (7 == get_type()) { this->no_data[0].i64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(F32 no_data) { if (8 == get_type()) { this->no_data[0].f64 = no_data; options |= 0x01; return TRUE; } return FALSE; };
  inline BOOL set_no_data(F64 no_data)
  {
    switch (get_type())
    {
    case 0:
    case 2:
    case 4:
    case 6:
      this->no_data[0].u64 = (U64)no_data;
      options |= 0x01;
      return TRUE;
    case 1:
    case 3:
    case 5:
    case 7:
      this->no_data[0].i64 = (I64)no_data;
      options |= 0x01;
      return TRUE;
    case 8:
    case 9:
      this->no_data[0].f64 = no_data;
      options |= 0x01;
      return TRUE;
    }
    return FALSE;
  };

  inline void set_min(U8* min) { this->min[0] = cast(min); options |= 0x02; };
  inline void update_min(U8* min) { this->min[0] = smallest(cast(min), this->min[0]); };
  inline BOOL set_min(U8  min) { if (0 == get_type()) { this->min[0].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(I8  min) { if (1 == get_type()) { this->min[0].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(U16 min) { if (2 == get_type()) { this->min[0].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(I16 min) { if (3 == get_type()) { this->min[0].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(U32 min) { if (4 == get_type()) { this->min[0].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(I32 min) { if (5 == get_type()) { this->min[0].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(U64 min) { if (6 == get_type()) { this->min[0].u64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(I64 min) { if (7 == get_type()) { this->min[0].i64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(F32 min) { if (8 == get_type()) { this->min[0].f64 = min; options |= 0x02; return TRUE; } return FALSE; };
  inline BOOL set_min(F64 min) { if (9 == get_type()) { this->min[0].f64 = min; options |= 0x02; return TRUE; } return FALSE; };

  inline void set_max(U8* max) { this->max[0] = cast(max); options |= 0x04; };
  inline void update_max(U8* max) { this->max[0] = biggest(cast(max), this->max[0]); };
  inline BOOL set_max(U8  max) { if (0 == get_type()) { this->max[0].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(I8  max) { if (1 == get_type()) { this->max[0].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(U16 max) { if (2 == get_type()) { this->max[0].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(I16 max) { if (3 == get_type()) { this->max[0].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(U32 max) { if (4 == get_type()) { this->max[0].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(I32 max) { if (5 == get_type()) { this->max[0].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(U64 max) { if (6 == get_type()) { this->max[0].u64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(I64 max) { if (7 == get_type()) { this->max[0].i64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(F32 max) { if (8 == get_type()) { this->max[0].f64 = max; options |= 0x04; return TRUE; } return FALSE; };
  inline BOOL set_max(F64 max) { if (9 == get_type()) { this->max[0].f64 = max; options |= 0x04; return TRUE; } return FALSE; };

  inline BOOL set_scale(F64 scale) { if (data_type) { this->scale[0] = scale; options |= 0x08; return TRUE; } return FALSE; };
  inline BOOL set_offset(F64 offset) { if (data_type) { this->offset[0] = offset; options |= 0x10; return TRUE; } return FALSE; };

  inline BOOL has_no_data() const { return options & 0x01; };
  inline BOOL has_min() const { return options & 0x02; };
  inline BOOL has_max() const { return options & 0x04; };
  inline BOOL has_scale() const { return options & 0x08; };
  inline BOOL has_offset() const { return options & 0x10; };

  inline U32 get_size() const
  {
    if (data_type)
    {
      const U32 size_table[10] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8 };
      U32 type = get_type();
      return size_table[type];
    }
    else
    {
      return options;
    }
  };

  inline F64 get_value_as_float(U8* value) const
  {
    F64 casted_value;
    I32 type = get_type();
    if (type == 0)
      casted_value = (F64)*((U8*)value);
    else if (type == 1)
      casted_value = (F64)*((I8*)value);
    else if (type == 2)
      casted_value = (F64)*((U16*)value);
    else if (type == 3)
      casted_value = (F64)*((I16*)value);
    else if (type == 4)
      casted_value = (F64)*((U32*)value);
    else if (type == 5)
      casted_value = (F64)*((I32*)value);
    else if (type == 6)
      casted_value = (F64)(I64)*((U64*)value);
    else if (type == 7)
      casted_value = (F64)*((I64*)value);
    else if (type == 8)
      casted_value = (F64)*((F32*)value);
    else
      casted_value = *((F64*)value);
    return offset[0]+scale[0]*casted_value;
  };

private:
  inline I32 get_type() const
  {
    return ((I32)data_type - 1)%10;
  };
  inline I32 get_dim() const
  {
    return 1;
  };
  inline U64I64F64 cast(U8* value) const
  {
    I32 type = get_type();
    U64I64F64 casted_value;
    if (type == 0)
      casted_value.u64 = *((U8*)value);
    else if (type == 1)
      casted_value.i64 = *((I8*)value);
    else if (type == 2)
      casted_value.u64 = *((U16*)value);
    else if (type == 3)
      casted_value.i64 = *((I16*)value);
    else if (type == 4)
      casted_value.u64 = *((U32*)value);
    else if (type == 5)
      casted_value.i64 = *((I32*)value);
    else if (type == 6)
      casted_value.u64 = *((U64*)value);
    else if (type == 7)
      casted_value.i64 = *((I64*)value);
    else if (type == 8)
      casted_value.f64 = *((F32*)value);
    else
      casted_value.f64 = *((F64*)value);
    return casted_value;
  };
  inline U64I64F64 smallest(U64I64F64 a, U64I64F64 b) const
  {
    I32 type = get_type();
    if (type >= 8) // float compare
    {
      if (a.f64 < b.f64) return a;
      else               return b;
    }
    if (type & 1) // int compare
    {
      if (a.i64 < b.i64) return a;
      else               return b;
    }
    if (a.u64 < b.u64) return a;
    else               return b;
  };
  inline U64I64F64 biggest(U64I64F64 a, U64I64F64 b) const
  {
    I32 type = get_type();
    if (type >= 8) // float compare
    {
      if (a.f64 > b.f64) return a;
      else               return b;
    }
    if (type & 1) // int compare
    {
      if (a.i64 > b.i64) return a;
      else               return b;
    }
    if (a.u64 > b.u64) return a;
    else               return b;
  };
};

class LASattributer
{
public:
  BOOL attributes_linked;
  I32 number_attributes;
  LASattribute* attributes;
  I32* attribute_starts;
  I32* attribute_sizes;

  LASattributer()
  {
    attributes_linked = TRUE;
    number_attributes = 0;
    attributes = 0;
    attribute_starts = 0;
    attribute_sizes = 0;
  };

  ~LASattributer()
  {
    clean_attributes();
  };

  void clean_attributes()
  {
    if (attributes_linked)
    {
      if (attributes)
      {
        number_attributes = 0;
        free(attributes); attributes = 0;
        free(attribute_starts); attribute_starts = 0;
        free(attribute_sizes); attribute_sizes = 0;
      }
    }
  };

  BOOL init_attributes(U32 number_attributes, LASattribute* attributes)
  {
    U32 i;
    clean_attributes();
    this->number_attributes = number_attributes;
    this->attributes = (LASattribute*)malloc(sizeof(LASattribute)*number_attributes);
    if (this->attributes == 0)
    {
      return FALSE;
    }
    memcpy(this->attributes, attributes, sizeof(LASattribute)*number_attributes);
    attribute_starts = (I32*)malloc(sizeof(I32)*number_attributes);
    if (attribute_starts == 0)
    {
      return FALSE;
    }
    attribute_sizes = (I32*)malloc(sizeof(I32)*number_attributes);
    if (attribute_sizes == 0)
    {
      return FALSE;
    }
    attribute_starts[0] = 0;
    attribute_sizes[0] = attributes[0].get_size();
    for (i = 1; i < number_attributes; i++)
    {
      attribute_starts[i] = attribute_starts[i-1] + attribute_sizes[i-1];
      attribute_sizes[i] = attributes[i].get_size();
    }
    return TRUE;
  };

  I32 add_attribute(const LASattribute attribute)
  {
    if (attribute.get_size())
    {
      if (attributes)
      {
        number_attributes++;
        attributes = (LASattribute*)realloc(attributes, sizeof(LASattribute)*number_attributes);
        if (attributes == 0)
        {
          return -1;
        }
        attribute_starts = (I32*)realloc(attribute_starts, sizeof(I32)*number_attributes);
        if (attribute_starts == 0)
        {
          return -1;
        }
        attribute_sizes = (I32*)realloc(attribute_sizes, sizeof(I32)*number_attributes);
        if (attribute_sizes == 0)
        {
          return -1;
        }
        attributes[number_attributes-1] = attribute;
        attribute_starts[number_attributes-1] = attribute_starts[number_attributes-2] + attribute_sizes[number_attributes-2];
        attribute_sizes[number_attributes-1] = attributes[number_attributes-1].get_size();
      }
      else
      {
        number_attributes = 1;
        attributes = (LASattribute*)malloc(sizeof(LASattribute));
        if (attributes == 0)
        {
          return -1;
        }
        attribute_starts = (I32*)malloc(sizeof(I32));
        if (attribute_starts == 0)
        {
          return -1;
        }
        attribute_sizes = (I32*)malloc(sizeof(I32));
        if (attribute_sizes == 0)
        {
          return -1;
        }
        attributes[0] = attribute;
        attribute_starts[0] = 0;
        attribute_sizes[0] = attributes[0].get_size();
      }
      return number_attributes-1;
    }
    return -1;
  };

  inline I16 get_attributes_size() const
  {
    return (attributes ? attribute_starts[number_attributes-1] + attribute_sizes[number_attributes-1] : 0);
  }

  I32 get_attribute_index(const CHAR* name) const
  {
    I32 i;
    for (i = 0; i < number_attributes; i++)
    {
      if (strcmp(attributes[i].name, name) == 0)
      {
        return i;
      }
    }
    return -1;
  }

  I32 get_attribute_start(const CHAR* name) const
  {
    I32 i;
    for (i = 0; i < number_attributes; i++)
    {
      if (strcmp(attributes[i].name, name) == 0)
      {
        return attribute_starts[i];
      }
    }
    return -1;
  }

  I32 get_attribute_start(I32 index) const
  {
    if (index < number_attributes)
    {
      return attribute_starts[index];
    }
    return -1;
  }

  I32 get_attribute_size(I32 index) const
  {
    if (index < number_attributes)
    {
      return attribute_sizes[index];
    }
    return -1;
  }

  BOOL remove_attribute(I32 index)
  {
    if (index < 0 || index >= number_attributes)
    {
      return FALSE;
    }
    for (index = index + 1; index < number_attributes; index++)
    {
      attributes[index-1] = attributes[index];
      if (index > 1)
      {
        attribute_starts[index-1] = attribute_starts[index-2] + attribute_sizes[index-2];
      }
      else
      {
        attribute_starts[index-1] = 0;
      }
      attribute_sizes[index-1] = attribute_sizes[index];
    }
    number_attributes--;
    if (number_attributes)
    {
      attributes = (LASattribute*)realloc(attributes, sizeof(LASattribute)*number_attributes);
      attribute_starts = (I32*)realloc(attribute_starts, sizeof(I32)*number_attributes);
      attribute_sizes = (I32*)realloc(attribute_sizes, sizeof(I32)*number_attributes);
    }
    else
    {
      free(attributes); attributes = 0;
      free(attribute_starts); attribute_starts = 0;
      free(attribute_sizes); attribute_sizes = 0;
    }
    return TRUE;
  }

  BOOL remove_attribute(const CHAR* name)
  {
    I32 index = get_attribute_index(name);
    if (index != -1)
    { 
      return remove_attribute(index);
    }
    return FALSE;
  }
};

#endif
