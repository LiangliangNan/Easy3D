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

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;


uniform mat4 PROJ;
uniform mat4 invMV;
uniform bool perspective;
uniform float radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in  vec2 vOutTexcoord[];
in float vOutClipped[];

out vec3  gPoint;
out vec2  gOutTexcoord;
out float gOutClipped;

void main()
{
    vec3  base = gl_in[0].gl_Position.xyz;// in camera space
    vec3  top = gl_in[1].gl_Position.xyz;// in camera space
    vec3  axes = top - base;
    vec3  view_dir = vec3(0, 0, 1);// this is view direction for orthographic mode in camera space
    if (perspective)
        view_dir = normalize(vec3(0) - 0.5*(base + top));// camera pos is (0, 0, 0) in camera space

    vec3 offset = cross(view_dir, axes);
    offset = radius * normalize(offset) * 0.5;

    // Vertex 1
    gPoint = (base - offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[0];
    gOutClipped = vOutClipped[0];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 2
    gPoint = (top - offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[1];
    gOutClipped = vOutClipped[1];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 3
    gPoint = (base + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[0];
    gOutClipped = vOutClipped[0];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 4
    gPoint = (top + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[1];
    gOutClipped = vOutClipped[1];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    EndPrimitive();
}
