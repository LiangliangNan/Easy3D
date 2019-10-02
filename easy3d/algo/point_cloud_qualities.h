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


#ifndef _ALGORITHM_POINT_SET_QUALITIES_H_
#define _ALGORITHM_POINT_SET_QUALITIES_H_

#include <algorithm/algorithm_common.h>
#include <math/math_types.h>
#include <string>


class PointCloud;

class ALGO_API PointSetQualities {
public:
	// Evaluates the quality of fitting a local tangent plane and the local sampling uniformity at each point.
	// A value of 1 indicates a perfectly fitted plane and a uniform disc distribution.
	// Typical neighbor_sizes are: 6, 16, 32
	static void compupte_planarities(PointCloud* cloud, const std::vector<unsigned int>& neighbor_sizes);

};

#endif

