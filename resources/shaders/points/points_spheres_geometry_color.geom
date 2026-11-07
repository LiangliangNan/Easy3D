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

#version 330 core


//#extension GL_EXT_geometry_shader4 : enable

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

uniform mat4 MV;
uniform mat4 PROJ;
uniform float sphere_radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in vec4 sphere_color_in[];

out Data {
    flat    vec4    sphere_color;
    smooth  vec2    tex;
    flat    vec4    position;
    flat    int     point_id;
} DataOut;


void main()
{
    // Output vertex position
    DataOut.position = MV * gl_in[0].gl_Position;
    DataOut.sphere_color = sphere_color_in[0];
    // macOS ignores GS-written gl_PrimitiveID; pass the point index explicitly.
    DataOut.point_id = gl_PrimitiveIDIn;

    // Vertex 1
    DataOut.tex = vec2(-1.0, -1.0);
    gl_Position = DataOut.position;
    gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
    gl_Position = PROJ  * gl_Position;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
          gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    // Vertex 2
    DataOut.tex = vec2(-1.0, 1.0);
    gl_Position = DataOut.position;
    gl_Position.xy += vec2(-sphere_radius, sphere_radius);
    gl_Position = PROJ  * gl_Position;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
          gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    // Vertex 3
    DataOut.tex = vec2(1.0, -1.0);
    gl_Position = DataOut.position;
    gl_Position.xy += vec2(sphere_radius, -sphere_radius);
    gl_Position = PROJ  * gl_Position;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
          gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    // Vertex 4
    DataOut.tex = vec2(1.0, 1.0);
    gl_Position = DataOut.position;
    gl_Position.xy += vec2(sphere_radius, sphere_radius);
    gl_Position = PROJ  * gl_Position;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
          gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    EndPrimitive();
}
