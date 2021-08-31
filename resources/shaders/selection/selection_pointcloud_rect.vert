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

#version 430


in  vec3	vtx_position;	// vertex position

uniform mat4 MVP;		// model-view-projection matrix
uniform mat4 MANIP = mat4(1.0);

uniform ivec4 viewport;		// view port
uniform vec4  rect;			// the rectangle region
uniform bool  deselect;		// select or deselect

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


void main()
{
	vec4 p = MVP * MANIP * vec4(vtx_position, 1.0);
	float x = p.x / p.w * 0.5 + 0.5;
	float y = p.y / p.w * 0.5 + 0.5;
	x = x * viewport[2] + viewport[0];
	y = y * viewport[3] + viewport[1];
	y = viewport[3] - 1 - y;
	if (x >= rect[0] && x <= rect[1] && y >= rect[2] && y <= rect[3]) 
	{
		//int addr = gl_VertexID / 32;
		//int offs = gl_VertexID % 32;
		uint addr = gl_VertexID >> 5;
		uint offs = gl_VertexID & 31;
		if (deselect) {
			atomicAnd(selection.data[addr], ~(1 << offs));
			//selection.data[addr] &= ~(1 << offs);
		}
		else {
			atomicOr(selection.data[addr], (1 << offs));
			//selection.data[addr] |= (1 << offs);
		}
	}

	gl_Position = p;
}
