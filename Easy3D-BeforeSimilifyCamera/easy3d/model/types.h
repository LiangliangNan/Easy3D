//=============================================================================
// Copyright (C) 2013 Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


#ifndef _TYPES_H_
#define _TYPES_H_


//== INCLUDES =================================================================


#include <easy3d/model/vec.h>


//=============================================================================


namespace easy3d {


//=============================================================================


/// Scalar type
typedef float Scalar;

/** 2-float vector */
typedef Vec<2, Scalar>	Vec2f;
/** 3-float vector */
typedef Vec<3, Scalar>	Vec3f;
/** 4-float vector */
typedef Vec<4, Scalar>	Vec4f;


/// Point type
typedef Vec<3, Scalar> Point;

/// 3D vector type
typedef Vec<3, Scalar> Vec3;

/// Normal type
typedef Vec<3, Scalar> Normal;

/// Color type
typedef Vec<3, Scalar> Color;

/// Texture coordinate type
typedef Vec<2, Scalar> Texture_coordinate;


//=============================================================================
} // namespace easy3d
//=============================================================================
#endif // _TYPES_H_
//============================================================================
