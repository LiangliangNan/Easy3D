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


in  vec3		vtx_position;	// vertex position

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);

uniform ivec4 viewport;
uniform bool  deselect;


layout(std430, binding = 0) buffer polygon_t {
	vec2 data[];
} polygon;


layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;



bool point_in_polygon(vec2 p)
{
	bool inside = false;
	uint n = polygon.data.length();
	for (uint i = 0, j = n - 1; i < n; j = i, ++i) {
		vec2 u0 = polygon.data[i];
		vec2 u1 = polygon.data[j];  // current edge

		if (((u0.y <= p.y) && (p.y < u1.y)) ||  // U1 is above the ray, U0 is on or below the ray
			((u1.y <= p.y) && (p.y < u0.y)))    // U0 is above the ray, U1 is on or below the ray
		{
			// find x-intersection of current edge with the ray. 
			// Only consider edge crossings on the ray to the right of P.
			float x = u0.x + (p.y - u0.y) * (u1.x - u0.x) / (u1.y - u0.y);
			if (x > p.x)
				inside = !inside;
		}
	}

	return inside;
}


void main()
{
	vec4 p = MVP * MANIP * vec4(vtx_position, 1.0);
	float x = p.x / p.w * 0.5 + 0.5;
	float y = p.y / p.w * 0.5 + 0.5;
	x = x * viewport[2] + viewport[0];
	y = y * viewport[3] + viewport[1];

	vec2 v = vec2(x, viewport[3] - 1 - y);
	if (point_in_polygon(v)) {
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
