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

in  vec3 vtx_position;	// vertex position
in  vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;		// vertex color
in  vec2 vtx_texcoord;

uniform mat4 SHADOW;
uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

uniform vec3 default_color = vec3(0.3, 0.3, 0.7);
uniform bool per_vertex_color = false;

// the data to be sent to the fragment shader
out Data {
        vec2 texcoord;
        vec3 color;
        vec3 normal;
        vec3 position;
        vec4 shadowCoord;
        float clipped;
} DataOut;


void main() {
        vec4 new_position = MANIP * vec4(vtx_position, 1.0);

        DataOut.clipped = 0.0;
        if (clippingPlaneEnabled) {
                gl_ClipDistance[0] = dot(new_position, clippingPlane0);
                if (planeClippingDiscard && gl_ClipDistance[0] < 0)
                DataOut.clipped = 1.0;
                if (crossSectionEnabled) {
                        gl_ClipDistance[1] = dot(new_position, clippingPlane1);
                        if (planeClippingDiscard && gl_ClipDistance[1] < 0)
                        DataOut.clipped = 1.0;
                }
        }

        if (per_vertex_color)
                DataOut.color = vtx_color;
        else
                DataOut.color = default_color;

        DataOut.normal = NORMAL * vtx_normal;
        DataOut.position = new_position.xyz;
        DataOut.shadowCoord = SHADOW * new_position;
        DataOut.texcoord = vtx_texcoord;

        // the position of the vertex as seen from the current camera, in clip space
        gl_Position = MVP * new_position;
}
