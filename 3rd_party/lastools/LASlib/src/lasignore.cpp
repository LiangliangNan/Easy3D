/*
===============================================================================

  FILE:  lasignore.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:
  
    info@rapidlasso.de  -  https://rapidlasso.de
  
  COPYRIGHT:
  
    (c) 2007-2019, rapidlasso GmbH - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasignore.hpp"

#include "laszip_decompress_selective_v3.hpp"

void LASignore::usage() const
{
  fprintf(stderr,"Ignore points based on classifications.\n");
  fprintf(stderr,"  -ignore_class 7\n");
  fprintf(stderr,"  -ignore_class 0 1 7 33\n");
  fprintf(stderr,"Ignore points based on return type.\n");
  fprintf(stderr,"  -ignore_first -ignore_first_of_many\n");
  fprintf(stderr,"  -ignore_last -ignore_last_of_many\n");
  fprintf(stderr,"  -ignore_intermediate\n");
  fprintf(stderr,"  -ignore_single\n");
  fprintf(stderr,"Ignore points based on flags.\n");
  fprintf(stderr,"  -ignore_synthetic -ignore_keypoint\n");
  fprintf(stderr,"  -ignore_withheld -ignore_overlap\n");
}

void LASignore::ignore_class(U8 classification)
{
  U32 byte = classification >> 5;
  U32 rest = classification & 31;
  ignore_classification_mask[byte] |= (1u << rest);
  ignore_mask |= (1u << byte);
}

void LASignore::dont_ignore_class(U8 classification)
{
  U32 byte = classification >> 5;
  U32 rest = classification & 31;
  ignore_classification_mask[byte] &= ~(1u << rest);
  if (ignore_classification_mask[byte] == 0)
  {
    ignore_mask &= ~(1u << byte);
  }
}

BOOL LASignore::parse(int& i, int argc, char *argv[])
{
  if ((strcmp(argv[i],"-ignore_class") == 0) || (strcmp(argv[i],"-ignore_extended_class") == 0))
  {
    if ((i+1) >= argc)
    {
      fprintf(stderr,"ERROR: '%s' needs 1 at least argument: classification\n", argv[i]);
      return FALSE;
    }
    int i_in = i;
    *argv[i]='\0';
    i+=1;
    do
    {
      U32 classification;
      if (sscanf(argv[i], "%u", &classification) != 1)
      {
        fprintf(stderr,"ERROR: '%s' needs arguments between 0 and 255 but '%s' is no valid code\n", argv[i_in], argv[i]);
        return FALSE;
      }
      if (classification > 255)
      {
        fprintf(stderr,"ERROR: '%s' needs arguments between 0 and 255 but '%u' is out of range\n", argv[i_in], classification);
        return FALSE;
      }
      ignore_mask |= (1u << (classification >> 5));
      ignore_classification_mask[(classification >> 5)] |= (1u << (classification & 31));
      *argv[i]='\0';
      i+=1;
    } while ((i < argc) && ('0' <= *argv[i]) && (*argv[i] <= '9'));
    i-=1;
  }
  else if (strcmp(argv[i],"-ignore_class_mask") == 0)
  {
    if ((i+1) >= argc)
    {
      fprintf(stderr,"ERROR: '%s' needs 1 argument: mask\n", argv[i]);
      return FALSE;
    }
    U32 classification_mask;
    if (sscanf(argv[i+1], "%u", &classification_mask) != 1)
    {
      fprintf(stderr,"ERROR: '%s' needs argument between 0x00000000 and 0xFFFFFFFF but '%s' is no valid mask\n", argv[i], argv[i+1]);
      return FALSE;
    }
    if (classification_mask == 0x00000000)
    {
      fprintf(stderr,"ERROR: '%s' needs argument between 0x00000000 and 0xFFFFFFFF but %u makes no sense\n", argv[i], classification_mask);
      return FALSE;
    }
    if (classification_mask == 0xFFFFFFFF)
    {
      fprintf(stderr,"ERROR: '%s' needs argument between 0x00000000 and 0xFFFFFFFF but %u makes no sense\n", argv[i], classification_mask);
      return FALSE;
    }
    ignore_mask |= 1;
    ignore_classification_mask[0] = classification_mask;
    *argv[i]='\0'; *argv[i+1]='\0'; i+=1;
  }
  else if (strcmp(argv[i],"-ignore_first_of_many") == 0)
  {
    ignore_mask |= LASIGNORE_FIRST_OF_MANY;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_last_of_many") == 0)
  {
    ignore_mask |= LASIGNORE_LAST_OF_MANY;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_intermediate") == 0)
  {
    ignore_mask |= LASIGNORE_INTERMEDIATE;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_single") == 0)
  {
    ignore_mask |= LASIGNORE_SINGLE;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_first") == 0)
  {
    ignore_mask |= LASIGNORE_FIRST_OF_MANY;
    ignore_mask |= LASIGNORE_SINGLE;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_last") == 0)
  {
    ignore_mask |= LASIGNORE_LAST_OF_MANY;
    ignore_mask |= LASIGNORE_SINGLE;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_synthetic") == 0)
  {
    ignore_mask |= LASIGNORE_SYNTHETIC;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_keypoint") == 0)
  {
    ignore_mask |= LASIGNORE_KEYPOINT;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_withheld") == 0)
  {
    ignore_mask |= LASIGNORE_WITHHELD;
    *argv[i]='\0';
  }
  else if (strcmp(argv[i],"-ignore_overlap") == 0)
  {
    ignore_mask |= LASIGNORE_OVERLAP;
    *argv[i]='\0';
  }
  else
  {
    fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
    return FALSE;
  }
  return TRUE;
}

I32 LASignore::unparse(CHAR* string) const
{
  I32 n = 0;
  if (ignore_mask)
  {
    if (ignore_mask & LASIGNORE_CLASSIFICATIONS)
    {
      n += sprintf(&string[n], "-ignore_class ");
      for (int i = 0; i < 8; i++)
      {
        if (ignore_mask & (1u << i))
        {
          for (int j = 0; j < 32; j++)
          {
            if (ignore_classification_mask[i] & (1u << j))
            {
              n += sprintf(&string[n], "%d ", 32*i + j);
            }
          }
        }
      }
    }
    if (ignore_mask & LASIGNORE_RETURNS)
    {
      if (ignore_mask & LASIGNORE_SINGLE)
      {
        if (ignore_mask & (LASIGNORE_FIRST_OF_MANY | LASIGNORE_LAST_OF_MANY))
        {
          if (ignore_mask & LASIGNORE_FIRST_OF_MANY)
          {
            n += sprintf(&string[n], "-ignore_first ");
          }
          if (ignore_mask & LASIGNORE_LAST_OF_MANY)
          {
            n += sprintf(&string[n], "-ignore_last ");
          }
        }
        else
        {
          n += sprintf(&string[n], "-ignore_single ");
        }
      }
      else
      {
        if (ignore_mask & LASIGNORE_FIRST_OF_MANY)
        {
          n += sprintf(&string[n], "-ignore_first_of_many ");
        }
        if (ignore_mask & LASIGNORE_LAST_OF_MANY)
        {
          n += sprintf(&string[n], "-ignore_last_of_many ");
        }
      }
      if (ignore_mask & LASIGNORE_INTERMEDIATE)
      {
        n += sprintf(&string[n], "-ignore_intermediate ");
      }
    }
    if (ignore_mask & LASIGNORE_FLAGS)
    {
      if (ignore_mask & LASIGNORE_SYNTHETIC)
      {
        n += sprintf(&string[n], "-ignore_snthetic ");
      }
      if (ignore_mask & LASIGNORE_KEYPOINT)
      {
        n += sprintf(&string[n], "-ignore_keypoint ");
      }
      if (ignore_mask & LASIGNORE_WITHHELD)
      {
        n += sprintf(&string[n], "-ignore_withheld ");
      }
      if (ignore_mask & LASIGNORE_OVERLAP)
      {
        n += sprintf(&string[n], "-ignore_overlap ");
      }
    }
  }
  return n;
}

U32 LASignore::get_decompress_selective() const
{
  U32 decompress_selective = LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY;
  if (ignore_mask & LASIGNORE_CLASSIFICATIONS)
  {
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_CLASSIFICATION;
  }
  if (ignore_mask & LASIGNORE_FLAGS)
  {
    decompress_selective |= LASZIP_DECOMPRESS_SELECTIVE_FLAGS;
  }
  return decompress_selective;
}

BOOL LASignore::parse(U32 curr_parameter, const U32 num_parameters, const F64* parameters)
{
  if (curr_parameter == num_parameters)
  {
    return TRUE; // nothing to ignore
  }
  else if (curr_parameter > num_parameters) // sanity check
  {
    return FALSE; // invalid state
  }
  if ((parameters[curr_parameter] < U32_MIN) || (U32_MAX < parameters[curr_parameter])) // sanity check
  {
    return FALSE; // invalid state
  }
  ignore_mask = (U32)parameters[curr_parameter];
  curr_parameter++;
  if (ignore_mask & LASIGNORE_CLASSIFICATIONS)
  {
    for (int i = 0; i < 8; i++)
    {
      if (ignore_mask & (1u << i))
      {
        if (curr_parameter == num_parameters) // sanity check
        {
          return FALSE; // invalid state
        }
        ignore_classification_mask[i] = (U32)(parameters[curr_parameter]);
        curr_parameter++;
      }
    }
  }
  return TRUE;
}

void LASignore::unparse(U32& num_parameters, F64* parameters) const
{
  if (ignore_mask)
  {
    parameters[num_parameters] = ignore_mask;
    num_parameters++;

    if (ignore_mask & LASIGNORE_CLASSIFICATIONS)
    {
      for (int i = 0; i < 8; i++)
      {
        if (ignore_mask & (1u << i))
        {
          parameters[num_parameters] = ignore_classification_mask[i];
          num_parameters++;
        }
      }
    }
  }
}

BOOL LASignore::ignore(const LASpoint* point) const
{
  if (ignore_mask)
  {
    if (ignore_mask & LASIGNORE_CLASSIFICATIONS)
    {
      U32 classification = (point->is_extended_point_type() ? point->get_extended_classification() : point->get_classification());
      U32 byte = classification >> 5;
      U32 rest = classification & 31;
      if (ignore_classification_mask[byte] & (1u << rest))
      {
        return TRUE;
      }
    }
    if (ignore_mask & LASIGNORE_RETURNS)
    {
      if ((ignore_mask & LASIGNORE_SINGLE) && point->is_single())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_FIRST_OF_MANY) && point->is_first_of_many())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_LAST_OF_MANY) && point->is_last_of_many())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_INTERMEDIATE) && point->is_intermediate())
      {
        return TRUE;
      }
    }
    if (ignore_mask & LASIGNORE_FLAGS)
    {
      if ((ignore_mask & LASIGNORE_SYNTHETIC) && point->get_synthetic_flag())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_KEYPOINT) && point->get_keypoint_flag())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_WITHHELD) && point->get_withheld_flag())
      {
        return TRUE;
      }
      if ((ignore_mask & LASIGNORE_OVERLAP) && point->get_extended_overlap_flag())
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

LASignore::LASignore()
{
  ignore_mask = 0;
  ignore_classification_mask[0] = ignore_classification_mask[1] = ignore_classification_mask[2] = ignore_classification_mask[3] = ignore_classification_mask[4] = ignore_classification_mask[5] = ignore_classification_mask[6] = ignore_classification_mask[7] = 0;
}

LASignore::~LASignore()
{
}
