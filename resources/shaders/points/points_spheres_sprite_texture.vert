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


layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool clippingPlaneEnabled;
	bool crossSectionEnabled;
	vec4 clippingPlane0;
	vec4 clippingPlane1;
};

in vec3  vtx_position;
in vec2  vtx_texcoord;
//in float sphere_radius;

uniform int		screen_width;   // scale to calculate size in pixels
uniform float	sphere_radius;


out Data{
	vec4	position; // eye space
	vec2	texcoord;
	//float	sphere_radius;
} DataOut;

void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	DataOut.texcoord = vtx_texcoord;

	// Output vertex position
	DataOut.position = MV * new_position; // eye space

	//DataOut.sphere_radius = sphere_radius;

	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, DataOut.position.z, DataOut.position.w);
	gl_PointSize = screen_width * projCorner.x / projCorner.w;
	// you can also compute the point size using both screen width and height information.
	// http://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
	//vec2 projSize = screen_size * projCorner.xy / projCorner.w;  // screen_size = vec2(screen_width, screen_height);
	//gl_PointSize = 0.5 * (projSize.x + projSize.y);

	gl_Position = PROJ * DataOut.position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}