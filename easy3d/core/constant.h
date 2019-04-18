/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_CONSTANT_H
#define EASY3D_CONSTANT_H

#include <cfloat>
#include <climits>

namespace easy3d {

#ifndef M_PI
#define M_PI			3.14159265358979323846264338327950288419716939937510582   // pi
#endif

#define half_pi			M_PI * 0.5
#define quarter_pi		M_PI * 0.25
#define two_pi			M_PI * 2.0          
#define rad2deg(a)		((a) * 180.0 / M_PI)
#define deg2rad(a)		((a) * M_PI / 180.0)

#define int_max			INT_MAX
#define int_min			INT_MIN

#define float_max		FLT_MAX
#define float_min		FLT_MIN

#define double_max		DBL_MAX
#define double_min		DBL_MIN


    // standard epsilon values
    const float  FLOAT_EPS      = 1.0e-7f;
    const float  FLOAT_EPS_SQR  = 1.0e-14f;
    const double DOUBLE_EPS     = 1.0e-14;
    const double DOUBLE_EPS_SQR = 1.0e-28;

    // Function returning epsilon for corresponding type
    template <typename FT> inline FT epsilon();
    template <typename FT> inline FT epsilon_sqr();

    // Template specializations for float and double
    template <> inline float  epsilon<float>() { return FLOAT_EPS; }
    template <> inline float  epsilon_sqr<float>() { return FLOAT_EPS_SQR; }
    template <> inline double epsilon<double>() { return DOUBLE_EPS; }
    template <> inline double epsilon_sqr<double>() { return DOUBLE_EPS_SQR; }
}


#endif // EASY3D_CONSTANT_H
