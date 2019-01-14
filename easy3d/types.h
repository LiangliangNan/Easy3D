/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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


#ifndef EASY3D_TYPES_H
#define EASY3D_TYPES_H

#include <cstdint>

#include <easy3d/vec.h>
#include <easy3d/mat.h>
#include <easy3d/box.h>
#include <easy3d/quat.h>


/**
* Gathers different basic types for geometric operations.
* Types defined here are points/vectors, lines, segments,
* planes, boxes and matrices in 2D and 3D.
*/

namespace easy3d {

    //____________________ default types___________________

    typedef Vec<2, float>				vec2;
    typedef Vec<3, float>				vec3;
    typedef Vec<4, float>				vec4;

    typedef Mat2<float>					mat2;
    typedef Mat3<float>					mat3;
    typedef Mat4<float>					mat4;
    typedef Mat<3, 4, float>			mat34;
    typedef Mat<4, 3, float>			mat43;

    typedef Quat<float>					quat;

    typedef GenericBox2<float>			Box2;
    typedef GenericBox3<float>			Box3;

}

#endif  // EASY3D_TYPES_H


