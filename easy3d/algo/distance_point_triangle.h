/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef EASY3D_ALGO_SURFACE_DISTANCE_POINT_TRIANGLE_H
#define EASY3D_ALGO_SURFACE_DISTANCE_POINT_TRIANGLE_H

#include <easy3d/core/types.h>


namespace easy3d {

//=============================================================================

//! Compute the distance of a vec3 p to a line segment given by vec3s (v0,v1).
float dist_point_line_segment(const vec3& p, const vec3& v0, const vec3& v1,
                               vec3& nearest_vec3);

//! Compute the distance of a vec3 p to the triangle given by vec3s (v0, v1, v2).
float dist_point_triangle(const vec3& p, const vec3& v0, const vec3& v1,
                           const vec3& v2, vec3& nearest_vec3);

//=============================================================================
} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_DISTANCE_POINT_TRIANGLE_H