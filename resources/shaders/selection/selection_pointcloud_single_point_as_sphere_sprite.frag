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


uniform bool	perspective;

uniform mat4	PROJ;
uniform float	sphere_radius;

in  vec4 position; // in eye space

out vec4 outputF;


void main()
{
	/*  with perspective correction
	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
	**/
	if (perspective) {
		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
		tex = vec2(tex.x, -tex.y) * 1.5; // 1.5 times larger ensure the quad is big enought in perspective view

		vec3 planePos = vec3(tex * sphere_radius, 0.0) + position.xyz;
		vec3 view_dir = normalize(planePos);
		float B = 2.0 * dot(view_dir, -position.xyz);
		float C = dot(position.xyz, position.xyz) - (sphere_radius * sphere_radius);
		float det = (B * B) - (4 * C);
		if (det < 0.0)
			discard;

		float sqrtDet = sqrt(det);
		float posT = (-B + sqrtDet) / 2;
		float negT = (-B - sqrtDet) / 2;
		float intersectT = min(posT, negT);
		vec3 pos = view_dir * intersectT;

		// compute the depth. I can do it easier
		//vec4 pos4 = PROJ * vec4(pos, 1.0);
		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;


		// NOTE: For points, the gl_PrimitiveID represent the ID of the input points. 
		//		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles 
		//		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
		//		 Since we're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
		int id = gl_PrimitiveID;
		outputF.r = ((id >> 16) & 0xff) / 255.0;
		outputF.g = ((id >> 8) & 0xff) / 255.0;
		outputF.b = (id & 0xff) / 255.0;
		outputF.a = (id >> 24) / 255.0;
	}

	// without perspective correction
	else {
		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
		float x = tex.x;
		float y = -tex.y;
		float zz = 1.0 - x*x - y*y;

		if (zz < 0.0)
			discard;

		float z = sqrt(zz);
		vec4 pos = position;
		pos.z += sphere_radius*z;

		// compute the depth. I can do it easier
		//pos = PROJ * pos;
		//gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;

		// NOTE: For points, the gl_PrimitiveID represent the ID of the input points. 
		//		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles 
		//		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
		//		 Since we're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
		int id = gl_PrimitiveID;
		outputF.r = ((id >> 16) & 0xff) / 255.0;
		outputF.g = ((id >> 8) & 0xff) / 255.0;
		outputF.b = (id & 0xff) / 255.0;
		outputF.a = (id >> 24) / 255.0;
	}
}

