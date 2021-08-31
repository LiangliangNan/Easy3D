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


// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_gpu_shader4 : enable

uniform mat4 MANIP = mat4(1.0);
uniform mat4 MV;
uniform mat4 PROJ;
uniform vec4 default_color;
uniform bool per_vertex_color;

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in  vec3 vtx_position;// point position
in  vec3 vtx_color;// point color

out vec4  vOutColor;
out float vOutClipped;

void main()
{
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    vOutClipped = 0.0;
    if (clippingPlaneEnabled) {
        if (planeClippingDiscard && dot(new_position, clippingPlane0) < 0)
            vOutClipped = 1.0;
        if (crossSectionEnabled) {
            if (planeClippingDiscard && dot(new_position, clippingPlane1) < 0)
                vOutClipped = 1.0;
        }
    }
    
    gl_Position = MV * new_position;

    if (per_vertex_color)
        vOutColor = vec4(vtx_color, 1.0);
    else
        vOutColor = default_color;
}
