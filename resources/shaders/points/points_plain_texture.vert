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


in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

// the data to be sent to the fragment shader
out Data{
    vec3 position;
    vec2 texcoord;
    vec3 normal;
} DataOut;


void main() {
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.position = vtx_position;
    DataOut.texcoord = vtx_texcoord;
    DataOut.normal = NORMAL * vtx_normal;

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (crossSectionEnabled) {
            gl_ClipDistance[1] = dot(new_position, clippingPlane1);
        }
    }

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * new_position;
}
