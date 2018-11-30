/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of easy3d: software for processing and rendering
*   meshes and point clouds.
*
*	easy3d is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	easy3d is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _EASY3D_MATH_TYPES_H_
#define _EASY3D_MATH_TYPES_H_

#include <cstdint>


#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif



#include "vec.h"
#include "mat.h"
#include "rect.h"
#include "line.h"
#include "segment.h"
#include "plane.h"
#include "box.h"
#include "quat.h"

/**
* Gathers different basic types for geometric operations.
* Types defined here are points/vectors, lines, segments,
* planes, boxes and matrices in 2D and 3D.
*/

//____________________ default types___________________

typedef Vec<2, float>				vec2;
typedef Vec<3, float>				vec3;
typedef Vec<4, float>				vec4;

typedef Vec<2, int32_t>				ivec2;
typedef Vec<3, int32_t>				ivec3;
typedef Vec<4, int32_t>				ivec4;

typedef Mat2<float>					mat2;
typedef Mat3<float>					mat3;
typedef Mat4<float>					mat4;
typedef Mat<3, 4, float>			mat34;
typedef Mat<4, 3, float>			mat43;

typedef Quat<float>					quat;

typedef GenericLine<2, float>		Line2;
typedef GenericLine<3, float>		Line3;

typedef GenericSegment<2, float>	Segment2;
typedef GenericSegment<3, float>	Segment3;

typedef GenericPlane3<float>		Plane3;

typedef GenericBox2<float>			Box2;
typedef GenericBox3<float>			Box3;

typedef GenericRectangle<float>		Rect;
typedef GenericRectangle<int32_t>	iRect;


#endif


