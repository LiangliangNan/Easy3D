/*
===============================================================================

  FILE:  lasreader_dtm.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreader_dtm.hpp"

// used to map GeoTIFF codes to GCTP codes

static const unsigned short PCS_NAD83_Alabama_East = 26929;
static const unsigned short PCS_NAD83_Alabama_West = 26930;
static const unsigned short PCS_NAD83_Alaska_zone_1 = 26931; /* Hotine Oblique Mercator Projection not supported*/
static const unsigned short PCS_NAD83_Alaska_zone_2 = 26932;
static const unsigned short PCS_NAD83_Alaska_zone_3 = 26933;
static const unsigned short PCS_NAD83_Alaska_zone_4 = 26934;
static const unsigned short PCS_NAD83_Alaska_zone_5 = 26935;
static const unsigned short PCS_NAD83_Alaska_zone_6 = 26936;
static const unsigned short PCS_NAD83_Alaska_zone_7 = 26937;
static const unsigned short PCS_NAD83_Alaska_zone_8 = 26938;
static const unsigned short PCS_NAD83_Alaska_zone_9 = 26939;
static const unsigned short PCS_NAD83_Alaska_zone_10 = 26940;
static const unsigned short PCS_NAD83_California_1 = 26941;
static const unsigned short PCS_NAD83_California_2 = 26942;
static const unsigned short PCS_NAD83_California_3 = 26943;
static const unsigned short PCS_NAD83_California_4 = 26944;
static const unsigned short PCS_NAD83_California_5 = 26945;
static const unsigned short PCS_NAD83_California_6 = 26946;
static const unsigned short PCS_NAD83_Arizona_East = 26948;
static const unsigned short PCS_NAD83_Arizona_Central = 26949;
static const unsigned short PCS_NAD83_Arizona_West = 26950;
static const unsigned short PCS_NAD83_Arkansas_North = 26951;
static const unsigned short PCS_NAD83_Arkansas_South = 26952;
static const unsigned short PCS_NAD83_Colorado_North = 26953;
static const unsigned short PCS_NAD83_Colorado_Central = 26954;
static const unsigned short PCS_NAD83_Colorado_South = 26955;
static const unsigned short PCS_NAD83_Connecticut = 26956;
static const unsigned short PCS_NAD83_Delaware = 26957;
static const unsigned short PCS_NAD83_Florida_East = 26958;
static const unsigned short PCS_NAD83_Florida_West = 26959;
static const unsigned short PCS_NAD83_Florida_North = 26960;
static const unsigned short PCS_NAD83_Hawaii_zone_1 = 26961;
static const unsigned short PCS_NAD83_Hawaii_zone_2 = 26962;
static const unsigned short PCS_NAD83_Hawaii_zone_3 = 26963;
static const unsigned short PCS_NAD83_Hawaii_zone_4 = 26964;
static const unsigned short PCS_NAD83_Hawaii_zone_5 = 26965;
static const unsigned short PCS_NAD83_Georgia_East = 26966;
static const unsigned short PCS_NAD83_Georgia_West = 26967;
static const unsigned short PCS_NAD83_Idaho_East = 26968;
static const unsigned short PCS_NAD83_Idaho_Central = 26969;
static const unsigned short PCS_NAD83_Idaho_West = 26970;
static const unsigned short PCS_NAD83_Illinois_East = 26971;
static const unsigned short PCS_NAD83_Illinois_West = 26972;
static const unsigned short PCS_NAD83_Indiana_East = 26973;
static const unsigned short PCS_NAD83_Indiana_West = 26974;
static const unsigned short PCS_NAD83_Iowa_North = 26975;
static const unsigned short PCS_NAD83_Iowa_South = 26976;
static const unsigned short PCS_NAD83_Kansas_North = 26977;
static const unsigned short PCS_NAD83_Kansas_South = 26978;
static const unsigned short PCS_NAD83_Kentucky_North = 2205;
static const unsigned short PCS_NAD83_Kentucky_South = 26980;
static const unsigned short PCS_NAD83_Louisiana_North = 26981;
static const unsigned short PCS_NAD83_Louisiana_South = 26982;
static const unsigned short PCS_NAD83_Maine_East = 26983;
static const unsigned short PCS_NAD83_Maine_West = 26984;
static const unsigned short PCS_NAD83_Maryland = 26985;
static const unsigned short PCS_NAD83_Massachusetts = 26986;
static const unsigned short PCS_NAD83_Massachusetts_Is = 26987;
static const unsigned short PCS_NAD83_Michigan_North = 26988;
static const unsigned short PCS_NAD83_Michigan_Central = 26989;
static const unsigned short PCS_NAD83_Michigan_South = 26990;
static const unsigned short PCS_NAD83_Minnesota_North = 26991;
static const unsigned short PCS_NAD83_Minnesota_Central = 26992;
static const unsigned short PCS_NAD83_Minnesota_South = 26993;
static const unsigned short PCS_NAD83_Mississippi_East = 26994;
static const unsigned short PCS_NAD83_Mississippi_West = 26995;
static const unsigned short PCS_NAD83_Missouri_East = 26996;
static const unsigned short PCS_NAD83_Missouri_Central = 26997;
static const unsigned short PCS_NAD83_Missouri_West = 26998;
static const unsigned short PCS_NAD83_Montana = 32100;
static const unsigned short PCS_NAD83_Nebraska = 32104;
static const unsigned short PCS_NAD83_Nevada_East = 32107;
static const unsigned short PCS_NAD83_Nevada_Central = 32108;
static const unsigned short PCS_NAD83_Nevada_West = 32109;
static const unsigned short PCS_NAD83_New_Hampshire = 32110;
static const unsigned short PCS_NAD83_New_Jersey = 32111;
static const unsigned short PCS_NAD83_New_Mexico_East = 32112;
static const unsigned short PCS_NAD83_New_Mexico_Central = 32113;
static const unsigned short PCS_NAD83_New_Mexico_West = 32114;
static const unsigned short PCS_NAD83_New_York_East = 32115;
static const unsigned short PCS_NAD83_New_York_Central = 32116;
static const unsigned short PCS_NAD83_New_York_West = 32117;
static const unsigned short PCS_NAD83_New_York_Long_Is = 32118;
static const unsigned short PCS_NAD83_North_Carolina = 32119;
static const unsigned short PCS_NAD83_North_Dakota_N = 32120;
static const unsigned short PCS_NAD83_North_Dakota_S = 32121;
static const unsigned short PCS_NAD83_Ohio_North = 32122;
static const unsigned short PCS_NAD83_Ohio_South = 32123;
static const unsigned short PCS_NAD83_Oklahoma_North = 32124;
static const unsigned short PCS_NAD83_Oklahoma_South = 32125;
static const unsigned short PCS_NAD83_Oregon_North = 32126;
static const unsigned short PCS_NAD83_Oregon_South = 32127;
static const unsigned short PCS_NAD83_Pennsylvania_N = 32128;
static const unsigned short PCS_NAD83_Pennsylvania_S = 32129;
static const unsigned short PCS_NAD83_Rhode_Island = 32130;
static const unsigned short PCS_NAD83_South_Carolina = 32133;
static const unsigned short PCS_NAD83_South_Dakota_N = 32134;
static const unsigned short PCS_NAD83_South_Dakota_S = 32135;
static const unsigned short PCS_NAD83_Tennessee = 32136;
static const unsigned short PCS_NAD83_Texas_North = 32137;
static const unsigned short PCS_NAD83_Texas_North_Central = 32138;
static const unsigned short PCS_NAD83_Texas_Central = 32139;
static const unsigned short PCS_NAD83_Texas_South_Central = 32140;
static const unsigned short PCS_NAD83_Texas_South = 32141;
static const unsigned short PCS_NAD83_Utah_North = 32142;
static const unsigned short PCS_NAD83_Utah_Central = 32143;
static const unsigned short PCS_NAD83_Utah_South = 32144;
static const unsigned short PCS_NAD83_Vermont = 32145;
static const unsigned short PCS_NAD83_Virginia_North = 32146;
static const unsigned short PCS_NAD83_Virginia_South = 32147;
static const unsigned short PCS_NAD83_Washington_North = 32148;
static const unsigned short PCS_NAD83_Washington_South = 32149;
static const unsigned short PCS_NAD83_West_Virginia_N = 32150;
static const unsigned short PCS_NAD83_West_Virginia_S = 32151;
static const unsigned short PCS_NAD83_Wisconsin_North = 32152;
static const unsigned short PCS_NAD83_Wisconsin_Central = 32153;
static const unsigned short PCS_NAD83_Wisconsin_South = 32154;
static const unsigned short PCS_NAD83_Wyoming_East = 32155;
static const unsigned short PCS_NAD83_Wyoming_East_Central = 32156;
static const unsigned short PCS_NAD83_Wyoming_West_Central = 32157;
static const unsigned short PCS_NAD83_Wyoming_West = 32158;
static const unsigned short PCS_NAD83_Puerto_Rico = 32161;

static const unsigned short GCTP_NAD83_Alabama_East = 101;
static const unsigned short GCTP_NAD83_Alabama_West = 102;
static const unsigned short GCTP_NAD83_Alaska_zone_1 = 5001; /* Hotine Oblique Mercator Projection not supported*/
static const unsigned short GCTP_NAD83_Alaska_zone_2 = 5002;
static const unsigned short GCTP_NAD83_Alaska_zone_3 = 5003;
static const unsigned short GCTP_NAD83_Alaska_zone_4 = 5004;
static const unsigned short GCTP_NAD83_Alaska_zone_5 = 5005;
static const unsigned short GCTP_NAD83_Alaska_zone_6 = 5006;
static const unsigned short GCTP_NAD83_Alaska_zone_7 = 5007;
static const unsigned short GCTP_NAD83_Alaska_zone_8 = 5008;
static const unsigned short GCTP_NAD83_Alaska_zone_9 = 5009;
static const unsigned short GCTP_NAD83_Alaska_zone_10 = 5010;
static const unsigned short GCTP_NAD83_California_1 = 401;
static const unsigned short GCTP_NAD83_California_2 = 402;
static const unsigned short GCTP_NAD83_California_3 = 403;
static const unsigned short GCTP_NAD83_California_4 = 404;
static const unsigned short GCTP_NAD83_California_5 = 405;
static const unsigned short GCTP_NAD83_California_6 = 406;
static const unsigned short GCTP_NAD83_Arizona_East = 201;
static const unsigned short GCTP_NAD83_Arizona_Central = 202;
static const unsigned short GCTP_NAD83_Arizona_West = 203;
static const unsigned short GCTP_NAD83_Arkansas_North = 301;
static const unsigned short GCTP_NAD83_Arkansas_South = 302;
static const unsigned short GCTP_NAD83_Colorado_North = 501;
static const unsigned short GCTP_NAD83_Colorado_Central = 502;
static const unsigned short GCTP_NAD83_Colorado_South = 503;
static const unsigned short GCTP_NAD83_Connecticut = 600;
static const unsigned short GCTP_NAD83_Delaware = 700;
static const unsigned short GCTP_NAD83_Florida_East = 901;
static const unsigned short GCTP_NAD83_Florida_West = 902;
static const unsigned short GCTP_NAD83_Florida_North = 903;
static const unsigned short GCTP_NAD83_Hawaii_zone_1 = 5101;
static const unsigned short GCTP_NAD83_Hawaii_zone_2 = 5102;
static const unsigned short GCTP_NAD83_Hawaii_zone_3 = 5103;
static const unsigned short GCTP_NAD83_Hawaii_zone_4 = 5104;
static const unsigned short GCTP_NAD83_Hawaii_zone_5 = 5105;
static const unsigned short GCTP_NAD83_Georgia_East = 1001;
static const unsigned short GCTP_NAD83_Georgia_West = 1002;
static const unsigned short GCTP_NAD83_Idaho_East = 1101;
static const unsigned short GCTP_NAD83_Idaho_Central = 1102;
static const unsigned short GCTP_NAD83_Idaho_West = 1103;
static const unsigned short GCTP_NAD83_Illinois_East = 1201;
static const unsigned short GCTP_NAD83_Illinois_West = 1202;
static const unsigned short GCTP_NAD83_Indiana_East = 1301;
static const unsigned short GCTP_NAD83_Indiana_West = 1302;
static const unsigned short GCTP_NAD83_Iowa_North = 1401;
static const unsigned short GCTP_NAD83_Iowa_South = 1402;
static const unsigned short GCTP_NAD83_Kansas_North = 1501;
static const unsigned short GCTP_NAD83_Kansas_South = 1502;
static const unsigned short GCTP_NAD83_Kentucky_North = 1601;
static const unsigned short GCTP_NAD83_Kentucky_South = 1602;
static const unsigned short GCTP_NAD83_Louisiana_North = 1701;
static const unsigned short GCTP_NAD83_Louisiana_South = 1702;
static const unsigned short GCTP_NAD83_Maine_East = 1801;
static const unsigned short GCTP_NAD83_Maine_West = 1802;
static const unsigned short GCTP_NAD83_Maryland = 1900;
static const unsigned short GCTP_NAD83_Massachusetts = 2001;
static const unsigned short GCTP_NAD83_Massachusetts_Is = 2002;
static const unsigned short GCTP_NAD83_Michigan_North = 2111;
static const unsigned short GCTP_NAD83_Michigan_Central = 2112;
static const unsigned short GCTP_NAD83_Michigan_South = 2113;
static const unsigned short GCTP_NAD83_Minnesota_North = 2201;
static const unsigned short GCTP_NAD83_Minnesota_Central = 2202;
static const unsigned short GCTP_NAD83_Minnesota_South = 2203;
static const unsigned short GCTP_NAD83_Mississippi_East = 2301;
static const unsigned short GCTP_NAD83_Mississippi_West = 2302;
static const unsigned short GCTP_NAD83_Missouri_East = 2401;
static const unsigned short GCTP_NAD83_Missouri_Central = 2402;
static const unsigned short GCTP_NAD83_Missouri_West = 2403;
static const unsigned short GCTP_NAD83_Montana = 2500;
static const unsigned short GCTP_NAD83_Nebraska = 2600;
static const unsigned short GCTP_NAD83_Nevada_East = 2701;
static const unsigned short GCTP_NAD83_Nevada_Central = 2702;
static const unsigned short GCTP_NAD83_Nevada_West = 2703;
static const unsigned short GCTP_NAD83_New_Hampshire = 2800;
static const unsigned short GCTP_NAD83_New_Jersey = 2900;
static const unsigned short GCTP_NAD83_New_Mexico_East = 3001;
static const unsigned short GCTP_NAD83_New_Mexico_Central = 3002;
static const unsigned short GCTP_NAD83_New_Mexico_West = 3003;
static const unsigned short GCTP_NAD83_New_York_East = 3101;
static const unsigned short GCTP_NAD83_New_York_Central = 3102;
static const unsigned short GCTP_NAD83_New_York_West = 3103;
static const unsigned short GCTP_NAD83_New_York_Long_Is = 3104;
static const unsigned short GCTP_NAD83_North_Carolina = 3200;
static const unsigned short GCTP_NAD83_North_Dakota_N = 3301;
static const unsigned short GCTP_NAD83_North_Dakota_S = 3302;
static const unsigned short GCTP_NAD83_Ohio_North = 3401;
static const unsigned short GCTP_NAD83_Ohio_South = 3402;
static const unsigned short GCTP_NAD83_Oklahoma_North = 3501;
static const unsigned short GCTP_NAD83_Oklahoma_South = 3502;
static const unsigned short GCTP_NAD83_Oregon_North = 3601;
static const unsigned short GCTP_NAD83_Oregon_South = 3602;
static const unsigned short GCTP_NAD83_Pennsylvania_N = 3701;
static const unsigned short GCTP_NAD83_Pennsylvania_S = 3702;
static const unsigned short GCTP_NAD83_Rhode_Island = 3800;
static const unsigned short GCTP_NAD83_South_Carolina = 3900;
static const unsigned short GCTP_NAD83_South_Dakota_N = 4001;
static const unsigned short GCTP_NAD83_South_Dakota_S = 4002;
static const unsigned short GCTP_NAD83_Tennessee = 4100;
static const unsigned short GCTP_NAD83_Texas_North = 4201;
static const unsigned short GCTP_NAD83_Texas_North_Central = 4202;
static const unsigned short GCTP_NAD83_Texas_Central = 4203;
static const unsigned short GCTP_NAD83_Texas_South_Central = 4204;
static const unsigned short GCTP_NAD83_Texas_South = 4205;
static const unsigned short GCTP_NAD83_Utah_North = 4301;
static const unsigned short GCTP_NAD83_Utah_Central = 4302;
static const unsigned short GCTP_NAD83_Utah_South = 4303;
static const unsigned short GCTP_NAD83_Vermont = 4400;
static const unsigned short GCTP_NAD83_Virginia_North = 4501;
static const unsigned short GCTP_NAD83_Virginia_South = 4502;
static const unsigned short GCTP_NAD83_Washington_North = 4601;
static const unsigned short GCTP_NAD83_Washington_South = 4602;
static const unsigned short GCTP_NAD83_West_Virginia_N = 4701;
static const unsigned short GCTP_NAD83_West_Virginia_S = 4702;
static const unsigned short GCTP_NAD83_Wisconsin_North = 4801;
static const unsigned short GCTP_NAD83_Wisconsin_Central = 4802;
static const unsigned short GCTP_NAD83_Wisconsin_South = 4803;
static const unsigned short GCTP_NAD83_Wyoming_East = 4901;
static const unsigned short GCTP_NAD83_Wyoming_East_Central = 4902;
static const unsigned short GCTP_NAD83_Wyoming_West_Central = 4903;
static const unsigned short GCTP_NAD83_Wyoming_West = 4904;
static const unsigned short GCTP_NAD83_Puerto_Rico = 5200;

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

BOOL LASreaderDTM::open(const CHAR* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  // clean header

  clean();

  // open the DTM file

  file = fopen(file_name, "rb");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot open file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 2*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 2*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  // read the 200 byte header

  CHAR signature[21];
  if (fread(signature, 1, 21, file) != 21)
  {
    fprintf(stderr, "ERROR: reading 21 byte signature for '%s'\n", file_name);
    return FALSE;
  }

  // do the first 21 bytes contain the right signature

  if (strncmp(signature, "PLANS-PC BINARY .DTM", 21) != 0)
  {
    fprintf(stderr, "ERROR: cannot open DTM file '%s', wrong signature '%21s'\n", file_name, signature);
    return FALSE;
  }

  // read the next 61 bytes containing the description

  CHAR description[61];
  if (fread(description, 1, 61, file) != 61)
  {
    fprintf(stderr, "ERROR: reading 61 byte description for '%s'\n", file_name);
    return FALSE;
  }

  // read version

  F32 version;
  if (fread(&version, 4, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 4 byte version for '%s'\n", file_name);
    return FALSE;
  }

  // check the version

  if (version != 3.1f)
  {
    fprintf(stderr, "WARNING: expected 3.1 but version is %1.1f\n", version);
  }

  // read lower left x

  if (fread(&ll_x, 8, 1, file) != 1)
  {
  }

  // read lower left y

  if (fread(&ll_y, 8, 1, file) != 1)
  {
  }

  // read min_z 

  F64 min_z;
  if (fread(&min_z, 8, 1, file) != 1)
  {
  }

  // read max_z

  F64 max_z;
  if (fread(&max_z, 8, 1, file) != 1)
  {
  }

  // read rotation

  F64 rotation;
  if (fread(&rotation, 8, 1, file) != 1)
  {
  }

  // check rotation

  if (rotation != 0.0)
  {
    fprintf(stderr, "WARNING: expected 0.0 but rotation is %g\n", rotation);
  }

  F64 column_spacing;
  if (fread(&column_spacing, 8, 1, file) != 1)
  {
  }
  xdim = (F32)column_spacing;

  F64 row_spacing;
  if (fread(&row_spacing, 8, 1, file) != 1)
  {
  }
  ydim = (F32)row_spacing;

  if (fread(&ncols, 4, 1, file) != 1)
  {
  }
  
  if (fread(&nrows, 4, 1, file) != 1)
  {
  }

  ll_x += 0.5*xdim;
  ll_y += 0.5*ydim;

  I16 horizontal_units = 0;
  I16 vertical_units = 0;
  data_type = -1;
  I16 coordinate_system = 0;
  I16 coordinate_zone = 0;
  I16 horizontal_datum = 0;
  I16 vertical_datum = 0;

  if (fread(&horizontal_units, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte horizontal_units for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&vertical_units, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte vertical_units for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&data_type, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte data_type for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&coordinate_system, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte horizontal_units for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&coordinate_zone, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte coordinate_zone for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&horizontal_datum, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte horizontal_datum for '%s'\n", file_name);
    return FALSE;
  }

  if (fread(&vertical_datum, 2, 1, file) != 1)
  {
    fprintf(stderr, "ERROR: reading 2 byte vertical_datum for '%s'\n", file_name);
    return FALSE;
  }

  int num_geo_keys = 0;
  LASvlr_key_entry geo_keys[6];

  if ((coordinate_system == 2) && (coordinate_zone >= 1) && (coordinate_zone <= 160)) // UTM
  {
    unsigned short geokey = 0;

    if (horizontal_datum == 1) // GEO_DATUM_NAD27
    {
      if ((3 <= coordinate_zone) && (coordinate_zone <= 22))
      {
        geokey = coordinate_zone + 26700;
      }
      else
      {
        fprintf(stderr, "UTM zone %d for NAD27 out-of-range\n", (int)coordinate_zone);
      }
    }
    else if (horizontal_datum == 2) // GEO_DATUM_NAD83
    {
      if ((3 <= coordinate_zone) && (coordinate_zone <= 22))
      {
        geokey = coordinate_zone + 26900;
      }
      else if ((28 <= coordinate_zone) && (coordinate_zone <= 38))
      {
        geokey = coordinate_zone + 25800;
      }
      else
      {
        fprintf(stderr, "UTM zone %d for NAD83 out-of-range\n", (int)coordinate_zone);
      }
    }
    else if (horizontal_datum == 3) // GEO_DATUM_WGS84
    {
      if (coordinate_zone < 100)
      {
        geokey = coordinate_zone + 32600; // northern_hemisphere
      }
      else
      {
        geokey = coordinate_zone - 100 + 32700; // southern_hemisphere
      }
    }

    if (geokey)
    {
      num_geo_keys = 2;

      // projected coordinates
      geo_keys[0].key_id = 1024; // GTModelTypeGeoKey
      geo_keys[0].tiff_tag_location = 0;
      geo_keys[0].count = 1;
      geo_keys[0].value_offset = 1; // ModelTypeProjected

      // projection
      geo_keys[1].key_id = 3072; // ProjectedCSTypeGeoKey
      geo_keys[1].tiff_tag_location = 0;
      geo_keys[1].count = 1;
      geo_keys[1].value_offset = geokey;
    }
  }
  else if (coordinate_system == 3) // state plane
  {
    unsigned short geokey = 0;
    if (horizontal_datum == 2) // GEO_DATUM_NAD83
    {
      switch(coordinate_zone)
      {
      case GCTP_NAD83_Alabama_East:
        geokey = PCS_NAD83_Alabama_East;
        break;
      case GCTP_NAD83_Alabama_West:
        geokey = PCS_NAD83_Alabama_West;
        break;
      case GCTP_NAD83_Alaska_zone_1:
        geokey = PCS_NAD83_Alaska_zone_1;
        break;
      case GCTP_NAD83_Alaska_zone_2:
        geokey = PCS_NAD83_Alaska_zone_2;
        break;
      case GCTP_NAD83_Alaska_zone_3:
        geokey = PCS_NAD83_Alaska_zone_3;
        break;
      case GCTP_NAD83_Alaska_zone_4:
        geokey = PCS_NAD83_Alaska_zone_4;
        break;
      case GCTP_NAD83_Alaska_zone_5:
        geokey = PCS_NAD83_Alaska_zone_5;
        break;
      case GCTP_NAD83_Alaska_zone_6:
        geokey = PCS_NAD83_Alaska_zone_6;
        break;
      case GCTP_NAD83_Alaska_zone_7:
        geokey = PCS_NAD83_Alaska_zone_7;
        break;
      case GCTP_NAD83_Alaska_zone_8:
        geokey = PCS_NAD83_Alaska_zone_8;
        break;
      case GCTP_NAD83_Alaska_zone_9:
        geokey = PCS_NAD83_Alaska_zone_9;
        break;
      case GCTP_NAD83_Alaska_zone_10:
        geokey = PCS_NAD83_Alaska_zone_10;
        break;
      case GCTP_NAD83_California_1:
        geokey = PCS_NAD83_California_1;
        break;
      case GCTP_NAD83_California_2:
        geokey = PCS_NAD83_California_2;
        break;
      case GCTP_NAD83_California_3:
        geokey = PCS_NAD83_California_3;
        break;
      case GCTP_NAD83_California_4:
        geokey = PCS_NAD83_California_4;
        break;
      case GCTP_NAD83_California_5:
        geokey = PCS_NAD83_California_5;
        break;
      case GCTP_NAD83_California_6:
        geokey = PCS_NAD83_California_6;
        break;
      case GCTP_NAD83_Arizona_East:
        geokey = PCS_NAD83_Arizona_East;
        break;
      case GCTP_NAD83_Arizona_Central:
        geokey = PCS_NAD83_Arizona_Central;
        break;
      case GCTP_NAD83_Arizona_West:
        geokey = PCS_NAD83_Arizona_West;
        break;
      case GCTP_NAD83_Arkansas_North:
        geokey = PCS_NAD83_Arkansas_North;
        break;
      case GCTP_NAD83_Arkansas_South:
        geokey = PCS_NAD83_Arkansas_South;
        break;
      case GCTP_NAD83_Colorado_North:
        geokey = PCS_NAD83_Colorado_North;
        break;
      case GCTP_NAD83_Colorado_Central:
        geokey = PCS_NAD83_Colorado_Central;
        break;
      case GCTP_NAD83_Colorado_South:
        geokey = PCS_NAD83_Colorado_South;
        break;
      case GCTP_NAD83_Connecticut:
        geokey = PCS_NAD83_Connecticut;
        break;
      case GCTP_NAD83_Delaware:
        geokey = PCS_NAD83_Delaware;
        break;
      case GCTP_NAD83_Florida_East:
        geokey = PCS_NAD83_Florida_East;
        break;
      case GCTP_NAD83_Florida_West:
        geokey = PCS_NAD83_Florida_West;
        break;
      case GCTP_NAD83_Florida_North:
        geokey = PCS_NAD83_Florida_North;
        break;
      case GCTP_NAD83_Hawaii_zone_1:
        geokey = PCS_NAD83_Hawaii_zone_1;
        break;
      case GCTP_NAD83_Hawaii_zone_2:
        geokey = PCS_NAD83_Hawaii_zone_2;
        break;
      case GCTP_NAD83_Hawaii_zone_3:
        geokey = PCS_NAD83_Hawaii_zone_3;
        break;
      case GCTP_NAD83_Hawaii_zone_4:
        geokey = PCS_NAD83_Hawaii_zone_4;
        break;
      case GCTP_NAD83_Hawaii_zone_5:
        geokey = PCS_NAD83_Hawaii_zone_5;
        break;
      case GCTP_NAD83_Georgia_East:
        geokey = PCS_NAD83_Georgia_East;
        break;
      case GCTP_NAD83_Georgia_West:
        geokey = PCS_NAD83_Georgia_West;
        break;
      case GCTP_NAD83_Idaho_East:
        geokey = PCS_NAD83_Idaho_East;
        break;
      case GCTP_NAD83_Idaho_Central:
        geokey = PCS_NAD83_Idaho_Central;
        break;
      case GCTP_NAD83_Idaho_West:
        geokey = PCS_NAD83_Idaho_West;
        break;
      case GCTP_NAD83_Illinois_East:
        geokey = PCS_NAD83_Illinois_East;
        break;
      case GCTP_NAD83_Illinois_West:
        geokey = PCS_NAD83_Illinois_West;
        break;
      case GCTP_NAD83_Indiana_East:
        geokey = PCS_NAD83_Indiana_East;
        break;
      case GCTP_NAD83_Indiana_West:
        geokey = PCS_NAD83_Indiana_West;
        break;
      case GCTP_NAD83_Iowa_North:
        geokey = PCS_NAD83_Iowa_North;
        break;
      case GCTP_NAD83_Iowa_South:
        geokey = PCS_NAD83_Iowa_South;
        break;
      case GCTP_NAD83_Kansas_North:
        geokey = PCS_NAD83_Kansas_North;
        break;
      case GCTP_NAD83_Kansas_South:
        geokey = PCS_NAD83_Kansas_South;
        break;
      case GCTP_NAD83_Kentucky_North:
        geokey = PCS_NAD83_Kentucky_North;
        break;
      case GCTP_NAD83_Kentucky_South:
        geokey = PCS_NAD83_Kentucky_North;
        break;
      case GCTP_NAD83_Louisiana_North:
        geokey = PCS_NAD83_Louisiana_North;
        break;
      case GCTP_NAD83_Louisiana_South:
        geokey = PCS_NAD83_Louisiana_South;
        break;
      case GCTP_NAD83_Maine_East:
        geokey = PCS_NAD83_Maine_East;
        break;
      case GCTP_NAD83_Maine_West:
        geokey = PCS_NAD83_Maine_West;
        break;
      case GCTP_NAD83_Maryland:
        geokey = PCS_NAD83_Maryland;
        break;
      case GCTP_NAD83_Massachusetts:
        geokey = PCS_NAD83_Massachusetts;
        break;
      case GCTP_NAD83_Massachusetts_Is:
        geokey = PCS_NAD83_Massachusetts_Is;
        break;
      case GCTP_NAD83_Michigan_North:
        geokey = PCS_NAD83_Michigan_North;
        break;
      case GCTP_NAD83_Michigan_Central:
        geokey = PCS_NAD83_Michigan_Central;
        break;
      case GCTP_NAD83_Michigan_South:
        geokey = PCS_NAD83_Michigan_South;
        break;
      case GCTP_NAD83_Minnesota_North:
        geokey = PCS_NAD83_Minnesota_North;
        break;
      case GCTP_NAD83_Minnesota_Central:
        geokey = PCS_NAD83_Minnesota_Central;
        break;
      case GCTP_NAD83_Minnesota_South:
        geokey = PCS_NAD83_Minnesota_South;
        break;
      case GCTP_NAD83_Mississippi_East:
        geokey = PCS_NAD83_Mississippi_East;
        break;
      case GCTP_NAD83_Mississippi_West:
        geokey = PCS_NAD83_Mississippi_West;
        break;
      case GCTP_NAD83_Missouri_East:
        geokey = PCS_NAD83_Missouri_East;
        break;
      case GCTP_NAD83_Missouri_Central:
        geokey = PCS_NAD83_Missouri_Central;
        break;
      case GCTP_NAD83_Missouri_West:
        geokey = PCS_NAD83_Missouri_West;
        break;
      case GCTP_NAD83_Montana:
        geokey = PCS_NAD83_Montana;
        break;
      case GCTP_NAD83_Nebraska:
        geokey = PCS_NAD83_Nebraska;
        break;
      case GCTP_NAD83_Nevada_East:
        geokey = PCS_NAD83_Nevada_East;
        break;
      case GCTP_NAD83_Nevada_Central:
        geokey = PCS_NAD83_Nevada_Central;
        break;
      case GCTP_NAD83_Nevada_West:
        geokey = PCS_NAD83_Nevada_West;
        break;
      case GCTP_NAD83_New_Hampshire:
        geokey = PCS_NAD83_New_Hampshire;
        break;
      case GCTP_NAD83_New_Jersey:
        geokey = PCS_NAD83_New_Jersey;
        break;
      case GCTP_NAD83_New_Mexico_East:
        geokey = PCS_NAD83_New_Mexico_East;
        break;
      case GCTP_NAD83_New_Mexico_Central:
        geokey = PCS_NAD83_New_Mexico_Central;
        break;
      case GCTP_NAD83_New_Mexico_West:
        geokey = PCS_NAD83_New_Mexico_West;
        break;
      case GCTP_NAD83_New_York_East:
        geokey = PCS_NAD83_New_York_East;
        break;
      case GCTP_NAD83_New_York_Central:
        geokey = PCS_NAD83_New_York_Central;
        break;
      case GCTP_NAD83_New_York_West:
        geokey = PCS_NAD83_New_York_West;
        break;
      case GCTP_NAD83_New_York_Long_Is:
        geokey = PCS_NAD83_New_York_Long_Is;
        break;
      case GCTP_NAD83_North_Carolina:
        geokey = PCS_NAD83_North_Carolina;
        break;
      case GCTP_NAD83_North_Dakota_N:
        geokey = PCS_NAD83_North_Dakota_N;
        break;
      case GCTP_NAD83_North_Dakota_S:
        geokey = PCS_NAD83_North_Dakota_S;
        break;
      case GCTP_NAD83_Ohio_North:
        geokey = PCS_NAD83_Ohio_North;
        break;
      case GCTP_NAD83_Ohio_South:
        geokey = PCS_NAD83_Ohio_South;
        break;
      case GCTP_NAD83_Oklahoma_North:
        geokey = PCS_NAD83_Oklahoma_North;
        break;
      case GCTP_NAD83_Oklahoma_South:
        geokey = PCS_NAD83_Oklahoma_South;
        break;
      case GCTP_NAD83_Oregon_North:
        geokey = PCS_NAD83_Oregon_North;
        break;
      case GCTP_NAD83_Oregon_South:
        geokey = PCS_NAD83_Oregon_South;
        break;
      case GCTP_NAD83_Pennsylvania_N:
        geokey = PCS_NAD83_Pennsylvania_N;
        break;
      case GCTP_NAD83_Pennsylvania_S:
        geokey = PCS_NAD83_Pennsylvania_S;
        break;
      case GCTP_NAD83_Rhode_Island:
        geokey = PCS_NAD83_Rhode_Island;
        break;
      case GCTP_NAD83_South_Carolina:
        geokey = PCS_NAD83_South_Carolina;
        break;
      case GCTP_NAD83_South_Dakota_N:
        geokey = PCS_NAD83_South_Dakota_N;
        break;
      case GCTP_NAD83_South_Dakota_S:
        geokey = PCS_NAD83_South_Dakota_S;
        break;
      case GCTP_NAD83_Tennessee:
        geokey = PCS_NAD83_Tennessee;
        break;
      case GCTP_NAD83_Texas_North:
        geokey = PCS_NAD83_Texas_North;
        break;
      case GCTP_NAD83_Texas_North_Central:
        geokey = PCS_NAD83_Texas_North_Central;
        break;
      case GCTP_NAD83_Texas_Central:
        geokey = PCS_NAD83_Texas_Central;
        break;
      case GCTP_NAD83_Texas_South_Central:
        geokey = PCS_NAD83_Texas_South_Central;
        break;
      case GCTP_NAD83_Texas_South:
        geokey = PCS_NAD83_Texas_South;
        break;
      case GCTP_NAD83_Utah_North:
        geokey = PCS_NAD83_Utah_North;
        break;
      case GCTP_NAD83_Utah_Central:
        geokey = PCS_NAD83_Utah_Central;
        break;
      case GCTP_NAD83_Utah_South:
        geokey = PCS_NAD83_Utah_South;
        break;
      case GCTP_NAD83_Vermont:
        geokey = PCS_NAD83_Vermont;
        break;
      case GCTP_NAD83_Virginia_North:
        geokey = PCS_NAD83_Virginia_North;
        break;
      case GCTP_NAD83_Virginia_South:
        geokey = PCS_NAD83_Virginia_South;
        break;
      case GCTP_NAD83_Washington_North:
        geokey = PCS_NAD83_Washington_North;
        break;
      case GCTP_NAD83_Washington_South:
        geokey = PCS_NAD83_Washington_South;
        break;
      case GCTP_NAD83_West_Virginia_N:
        geokey = PCS_NAD83_West_Virginia_N;
        break;
      case GCTP_NAD83_West_Virginia_S:
        geokey = PCS_NAD83_West_Virginia_S;
        break;
      case GCTP_NAD83_Wisconsin_North:
        geokey = PCS_NAD83_Wisconsin_North;
        break;
      case GCTP_NAD83_Wisconsin_Central:
        geokey = PCS_NAD83_Wisconsin_Central;
        break;
      case GCTP_NAD83_Wisconsin_South:
        geokey = PCS_NAD83_Wisconsin_South;
        break;
      case GCTP_NAD83_Wyoming_East:
        geokey = PCS_NAD83_Wyoming_East;
        break;
      case GCTP_NAD83_Wyoming_East_Central:
        geokey = PCS_NAD83_Wyoming_East_Central;
        break;
      case GCTP_NAD83_Wyoming_West_Central:
        geokey = PCS_NAD83_Wyoming_West_Central;
        break;
      case GCTP_NAD83_Wyoming_West:
        geokey = PCS_NAD83_Wyoming_West;
        break;
      case GCTP_NAD83_Puerto_Rico:
        geokey = PCS_NAD83_Puerto_Rico;
        break;
      default:
        fprintf(stderr, "state plane NAD83 zone %d not implemented\n", (int)coordinate_zone);
      }
    }

    if (geokey)
    {
      num_geo_keys = 2;

      // projected coordinates
      geo_keys[0].key_id = 1024; // GTModelTypeGeoKey
      geo_keys[0].tiff_tag_location = 0;
      geo_keys[0].count = 1;
      geo_keys[0].value_offset = 1; // ModelTypeProjected

      // projection
      geo_keys[1].key_id = 3072; // ProjectedCSTypeGeoKey
      geo_keys[1].tiff_tag_location = 0;
      geo_keys[1].count = 1;
      geo_keys[1].value_offset = geokey;
    }
  }

  if (horizontal_units < 2)
  {
    // horizontal units
    geo_keys[num_geo_keys].key_id = 3076; // ProjLinearUnitsGeoKey
    geo_keys[num_geo_keys].tiff_tag_location = 0;
    geo_keys[num_geo_keys].count = 1;
    geo_keys[num_geo_keys].value_offset = (horizontal_units == 1 ? 9001 : 9002);
    num_geo_keys++;
  }

  if (vertical_units < 2)
  {
    // vertical units
    geo_keys[num_geo_keys].key_id = 4099; // VerticalUnitsGeoKey
    geo_keys[num_geo_keys].tiff_tag_location = 0;
    geo_keys[num_geo_keys].count = 1;
    geo_keys[num_geo_keys].value_offset = (vertical_units == 1 ? 9001 : 9002);
    num_geo_keys++;
  }
  
  if (vertical_datum)
  {
    // vertical datum
    geo_keys[num_geo_keys].key_id = 4096; // VerticalCSTypeGeoKey
    geo_keys[num_geo_keys].tiff_tag_location = 0;
    geo_keys[num_geo_keys].count = 1;
    if (vertical_datum == 2)
    {
      geo_keys[num_geo_keys].value_offset = 5103;// GEO_VERTICAL_NAVD88;
    }
    else if (vertical_datum == 3)
    {
      geo_keys[num_geo_keys].value_offset = 5030; // GEO_VERTICAL_WGS84;
    }
    else if (vertical_datum == 1)
    {
      geo_keys[num_geo_keys].value_offset = 5102; // GEO_VERTICAL_NAVD29;
    }
    num_geo_keys++;
  }
      
  if (num_geo_keys)
  {
    header.set_geo_keys(num_geo_keys, geo_keys);
  }

  // get remaining zero bytes of header

  I32 i;
  for (i = 164; i < 200; i++)
  {
    fgetc(file);
  }

  // check that we have all the needed info

  if (xdim <= 0)
  {
    fprintf(stderr,"WARNING: xdim was %g. setting to 1.0\n", xdim);
    xdim = 1;
  }

  if (ydim <= 0)
  {
    ydim = 1;
    fprintf(stderr,"WARNING: ydim was %g. setting to 1.0\n", ydim);
  }

  // populate the header as much as it makes sense

  sprintf(header.system_identifier, "LAStools (c) by rapidlasso GmbH");
  sprintf(header.generating_software, "via LASreaderDTM (%d)", LAS_TOOLS_VERSION);

  // maybe set creation date

#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA attr;
	SYSTEMTIME creation;
  GetFileAttributesEx(file_name, GetFileExInfoStandard, &attr);
	FileTimeToSystemTime(&attr.ftCreationTime, &creation);
  int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  header.file_creation_day = startday[creation.wMonth] + creation.wDay;
  header.file_creation_year = creation.wYear;
  // leap year handling
  if ((((creation.wYear)%4) == 0) && (creation.wMonth > 2)) header.file_creation_day++;
#else
  header.file_creation_day = 111;
  header.file_creation_year = 2013;
#endif

  // initialize point format in header

  header.point_data_format = 0;
  header.point_data_record_length = 20;

  // initialize point

  point.init(&header, header.point_data_format, header.point_data_record_length, &header);

  // init the bounding box x y

  header.min_x = ll_x;
  header.min_y = ll_y;
  header.max_x = ll_x + (ncols-1)*xdim;
  header.max_y = ll_y + (nrows-1)*ydim;
  header.min_z = F64_MAX;
  header.max_z = F64_MIN;

  // init the bounding box z and count the rasters

  npoints = 0;

  if (data_type == 2) // F32
  {
    F32 elevation = 0;
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        if (fread((void*)&elevation, 4, 1, file) == 1)
        {
          if (elevation != nodata)
          {
            if (header.max_z < elevation) header.max_z = elevation;
            if (header.min_z > elevation) header.min_z = elevation;
            npoints++;
          }
        }
        else
        {
          col = ncols;
          row = nrows;
        }
      }
    }
  }
  else if (data_type == 1) // I32
  {
    I32 elevation = 0;
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        if (fread((void*)&elevation, 4, 1, file) == 1)
        {
          if (((F32)elevation) != nodata)
          {
            if (header.max_z < elevation) header.max_z = elevation;
            if (header.min_z > elevation) header.min_z = elevation;
            npoints++;
          }
        }
        else
        {
          col = ncols;
          row = nrows;
        }
      }
    }
  }
  else if (data_type == 0) // I16
  {
    I16 elevation;
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        if (fread((void*)&elevation, 2, 1, file) == 1)
        {
          if (((F32)elevation) != nodata)
          {
            if (header.max_z < elevation) header.max_z = elevation;
            if (header.min_z > elevation) header.min_z = elevation;
            npoints++;
          }
        }
        else
        {
          col = ncols;
          row = nrows;
        }
      }
    }
  }
  else if (data_type == 3) // F64
  {
    F64 elevation;
    for (row = 0; row < nrows; row++)
    {
      for (col = 0; col < ncols; col++)
      {
        if (fread((void*)&elevation, 8, 1, file) == 1)
        {
          if (((F32)elevation) != nodata)
          {
            if (header.max_z < elevation) header.max_z = elevation;
            if (header.min_z > elevation) header.min_z = elevation;
            npoints++;
          }
        }
        else
        {
          col = ncols;
          row = nrows;
        }
      }
    }
  }
  else
  {
    fprintf(stderr,"ERROR: unknown data type %d\n", (I32)data_type);
    return FALSE;
  }

  // update the header point count

  header.number_of_point_records = (U32)npoints;

  if (npoints)
  {
    // populate scale and offset

    populate_scale_and_offset();

    // check bounding box for this scale and offset

    populate_bounding_box();
  }
  else
  {
    fprintf(stderr,"WARNING: DTM raster contains only no data values\n");
    header.min_z = 0;
    header.max_z = 0;
  }

  // reopen

  return reopen(file_name);
}

void LASreaderDTM::set_scale_factor(const F64* scale_factor)
{
  if (scale_factor)
  {
    if (this->scale_factor == 0) this->scale_factor = new F64[3];
    this->scale_factor[0] = scale_factor[0];
    this->scale_factor[1] = scale_factor[1];
    this->scale_factor[2] = scale_factor[2];
  }
  else if (this->scale_factor)
  {
    delete [] this->scale_factor;
    this->scale_factor = 0;
  }
}

void LASreaderDTM::set_offset(const F64* offset)
{
  if (offset)
  {
    if (this->offset == 0) this->offset = new F64[3];
    this->offset[0] = offset[0];
    this->offset[1] = offset[1];
    this->offset[2] = offset[2];
  }
  else if (this->offset)
  {
    delete [] this->offset;
    this->offset = 0;
  }
}

BOOL LASreaderDTM::seek(const I64 p_index)
{
  return FALSE;
}

BOOL LASreaderDTM::read_point_default()
{
  while (p_count < npoints)
  {
    if (row == nrows)
    {
      row = 0;
      col++;
    }

    F32 elevation;

    if (data_type == 2) // F32
    {
      if (fread((void*)&elevation, 4, 1, file) != 1)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        npoints = p_count;
        return FALSE;
      }
    }
    else if (data_type == 1) // I32
    {
      I32 elev;
      if (fread((void*)&elev, 4, 1, file) != 1)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        npoints = p_count;
        return FALSE;
      }
      elevation = (F32)elev;
    }
    else if (data_type == 0) // I16
    {
      I16 elev;
      if (fread((void*)&elev, 2, 1, file) != 1)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        npoints = p_count;
        return FALSE;
      }
      elevation = (F32)elev;
    }
    else if (data_type == 3) // F64
    {
      F64 elev;
      if (fread((void*)&elev, 8, 1, file) != 1)
      {
#ifdef _WIN32
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %I64d points\n", row, nrows, col, ncols, p_count);
#else
        fprintf(stderr,"WARNING: end-of-file after %d of %d rows and %d of %d cols. read %lld points\n", row, nrows, col, ncols, p_count);
#endif
        npoints = p_count;
        return FALSE;
      }
      elevation = (F32)elev;
    }
    else
    {
      fprintf(stderr,"ERROR: unknown data type %d\n", (I32)data_type);
      return FALSE;
    }

    if (elevation != nodata)
    {
      point.set_x(ll_x + col * xdim);
      point.set_y(ll_y + row * ydim);
      point.set_z(elevation);
      p_count++;
      row++;
      return TRUE;
    }
    else
    {
      row++;
    }
  }
  return FALSE;
}

ByteStreamIn* LASreaderDTM::get_stream() const
{
  return 0;
}

void LASreaderDTM::close(BOOL close_stream)
{
  if (file)
  {
    fclose(file);
    file = 0;
  }
}

BOOL LASreaderDTM::reopen(const CHAR* file_name)
{
  if (file_name == 0)
  {
    fprintf(stderr,"ERROR: file name pointer is zero\n");
    return FALSE;
  }

  // close the old file

  if (file)
  {
    fclose(file);
    file = 0;
  }

  file = fopen(file_name, "rb");
  if (file == 0)
  {
    fprintf(stderr, "ERROR: cannot reopen file '%s'\n", file_name);
    return FALSE;
  }

  if (setvbuf(file, NULL, _IOFBF, 2*LAS_TOOLS_IO_IBUFFER_SIZE) != 0)
  {
    fprintf(stderr, "WARNING: setvbuf() failed with buffer size %d\n", 2*LAS_TOOLS_IO_IBUFFER_SIZE);
  }

  col = 0;
  row = 0;
  p_count = 0;

  // skip 200 bytes of header

  I32 i;
  for (i = 0; i < 200; i++)
  {
    fgetc(file);
  }

  return TRUE;
}

void LASreaderDTM::clean()
{
  if (file)
  {
    fclose(file);
    file = 0;
  }
  col = 0;
  row = 0;
  ncols = 0;
  nrows = 0;
  nodata = -9999.0f;
  data_type = -1;
  ll_x = 0.0;
  ll_y = 0.0;
  xdim = 0;
  ydim = 0;
}

LASreaderDTM::LASreaderDTM()
{
  file = 0;
  scale_factor = 0;
  offset = 0;
  clean();
}

LASreaderDTM::~LASreaderDTM()
{
  clean();
  if (scale_factor)
  {
    delete [] scale_factor;
    scale_factor = 0;
  }
  if (offset)
  {
    delete [] offset;
    offset = 0;
  }
}

void LASreaderDTM::populate_scale_and_offset()
{
  // if not specified in the command line, set a reasonable scale_factor
  if (scale_factor)
  {
    header.x_scale_factor = scale_factor[0];
    header.y_scale_factor = scale_factor[1];
    header.z_scale_factor = scale_factor[2];
  }
  else
  {
    if (-360 < header.min_x  && -360 < header.min_y && header.max_x < 360 && header.max_y < 360) // do we have longitude / latitude coordinates
    {
      header.x_scale_factor = 1e-7;
      header.y_scale_factor = 1e-7;
    }
    else // then we assume utm or mercator / lambertian projections
    {
      header.x_scale_factor = 0.01;
      header.y_scale_factor = 0.01;
    }
    header.z_scale_factor = 0.01;
  }

  // if not specified in the command line, set a reasonable offset
  if (offset)
  {
    header.x_offset = offset[0];
    header.y_offset = offset[1];
    header.z_offset = offset[2];
  }
  else
  {
    if (F64_IS_FINITE(header.min_x) && F64_IS_FINITE(header.max_x))
      header.x_offset = ((I64)((header.min_x + header.max_x)/header.x_scale_factor/20000000))*10000000*header.x_scale_factor;
    else
      header.x_offset = 0;

    if (F64_IS_FINITE(header.min_y) && F64_IS_FINITE(header.max_y))
      header.y_offset = ((I64)((header.min_y + header.max_y)/header.y_scale_factor/20000000))*10000000*header.y_scale_factor;
    else
      header.y_offset = 0;

    if (F64_IS_FINITE(header.min_z) && F64_IS_FINITE(header.max_z))
      header.z_offset = ((I64)((header.min_z + header.max_z)/header.z_scale_factor/20000000))*10000000*header.z_scale_factor;
    else
      header.z_offset = 0;
  }
}

void LASreaderDTM::populate_bounding_box()
{
  // compute quantized and then unquantized bounding box

  F64 dequant_min_x = header.get_x(header.get_X(header.min_x));
  F64 dequant_max_x = header.get_x(header.get_X(header.max_x));
  F64 dequant_min_y = header.get_y(header.get_Y(header.min_y));
  F64 dequant_max_y = header.get_y(header.get_Y(header.max_y));
  F64 dequant_min_z = header.get_z(header.get_Z(header.min_z));
  F64 dequant_max_z = header.get_z(header.get_Z(header.max_z));

  // make sure there is not sign flip

  if ((header.min_x > 0) != (dequant_min_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_x from %g to %g.\n", header.min_x, dequant_min_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.min_x = dequant_min_x;
  }
  if ((header.max_x > 0) != (dequant_max_x > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_x from %g to %g.\n", header.max_x, dequant_max_x);
    fprintf(stderr, "         set scale factor for x coarser than %g with '-rescale'\n", header.x_scale_factor);
  }
  else
  {
    header.max_x = dequant_max_x;
  }
  if ((header.min_y > 0) != (dequant_min_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_y from %g to %g.\n", header.min_y, dequant_min_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.min_y = dequant_min_y;
  }
  if ((header.max_y > 0) != (dequant_max_y > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_y from %g to %g.\n", header.max_y, dequant_max_y);
    fprintf(stderr, "         set scale factor for y coarser than %g with '-rescale'\n", header.y_scale_factor);
  }
  else
  {
    header.max_y = dequant_max_y;
  }
  if ((header.min_z > 0) != (dequant_min_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for min_z from %g to %g.\n", header.min_z, dequant_min_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.min_z = dequant_min_z;
  }
  if ((header.max_z > 0) != (dequant_max_z > 0))
  {
    fprintf(stderr, "WARNING: quantization sign flip for max_z from %g to %g.\n", header.max_z, dequant_max_z);
    fprintf(stderr, "         set scale factor for z coarser than %g with '-rescale'\n", header.z_scale_factor);
  }
  else
  {
    header.max_z = dequant_max_z;
  }
}

LASreaderDTMrescale::LASreaderDTMrescale(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor) : LASreaderDTM()
{
  scale_factor[0] = x_scale_factor;
  scale_factor[1] = y_scale_factor;
  scale_factor[2] = z_scale_factor;
}

BOOL LASreaderDTMrescale::open(const CHAR* file_name)
{
  LASreaderDTM::set_scale_factor(scale_factor);
  if (!LASreaderDTM::open(file_name)) return FALSE;
  return TRUE;
}

LASreaderDTMreoffset::LASreaderDTMreoffset(F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderDTM()
{
  this->offset[0] = x_offset;
  this->offset[1] = y_offset;
  this->offset[2] = z_offset;
}

BOOL LASreaderDTMreoffset::open(const CHAR* file_name)
{
  LASreaderDTM::set_offset(offset);
  if (!LASreaderDTM::open(file_name)) return FALSE;
  return TRUE;
}

LASreaderDTMrescalereoffset::LASreaderDTMrescalereoffset(F64 x_scale_factor, F64 y_scale_factor, F64 z_scale_factor, F64 x_offset, F64 y_offset, F64 z_offset) : LASreaderDTMrescale(x_scale_factor, y_scale_factor, z_scale_factor), LASreaderDTMreoffset(x_offset, y_offset, z_offset)
{
}

BOOL LASreaderDTMrescalereoffset::open(const CHAR* file_name)
{
  LASreaderDTM::set_scale_factor(scale_factor);
  LASreaderDTM::set_offset(offset);
  if (!LASreaderDTM::open(file_name)) return FALSE;
  return TRUE;
}
