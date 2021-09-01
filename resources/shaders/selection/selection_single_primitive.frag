/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#version 150


out  vec4    outputF;

void main()
{
    // NOTE: For points, the gl_PrimitiveID represent the ID of the input points.
    //		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles
    //		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
    //		 If you're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
    int id = gl_PrimitiveID;
    outputF.r = ((id >> 16) & 0xff) / 255.0;
    outputF.g = ((id >> 8) & 0xff) / 255.0;
    outputF.b = (id & 0xff) / 255.0;
    outputF.a = (id >> 24) / 255.0;
} 