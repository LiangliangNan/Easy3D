/** \cond LICENSE
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
 \endcond */

#version 150



in vec3  vtx_position;

uniform mat4	MV;
uniform mat4	PROJ;
uniform mat4	MANIP;

uniform int		screen_width;   // scale to calculate size in pixels
uniform float sphere_radius;
uniform float point_size;
uniform bool sphere;
uniform bool clippingPlaneEnabled;
uniform bool crossSectionEnabled;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;


flat out vec4 position; // in eye space
flat out int source_id;

void main()
{
	vec4 world = MANIP * vec4(vtx_position, 1.0);
    position = MV * world;
    source_id = gl_VertexID;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(world, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(world, clippingPlane1);
    }

	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, position.z, position.w);
	gl_PointSize = sphere ? screen_width * projCorner.x / projCorner.w : point_size;

	gl_Position = PROJ * position;
}