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


// we accept points and render quads
layout (points) in;
layout (triangle_strip, max_vertices=4) out;

uniform mat4 MVP;
uniform float radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in VertexData {
    vec2  texcoord;
    vec3  normal;
} VertexIn[];

out FragmentData {
    vec2 texcoord;  // the point's
    vec2 tex;       // the local
    vec3 point;
    vec3 normal;
    flat int point_id;
} VertexOut;


void main()
{
    vec4 center = gl_in[0].gl_Position;
    vec3 normal = VertexIn[0].normal;

    // create tangent space. Helper vectors dependent on major orientation of normal
    vec3 u, v;
    if (abs(normal.y) > abs(normal.x)) {
        v = cross(normal, vec3(1.0, 0.0, 0.0));
        u = cross(normal, v);
    }
    else {
        v = cross(vec3(0.0, 1.0, 0.0), normal);
        u = cross(normal, v);
    }

    // Scale the splat
    u *= radius;
    v *= radius;

    // Calculate the four corner vertices of the quad
    vec4 a = center + vec4(-u-v, 0.0);
    vec4 b = center + vec4(-u+v, 0.0);
    vec4 c = center + vec4(+u+v, 0.0);
    vec4 d = center + vec4(+u-v, 0.0);

    VertexOut.point = center.xyz;
    VertexOut.normal = normal;
    VertexOut.texcoord = VertexIn[0].texcoord;
    // macOS ignores GS-written gl_PrimitiveID; pass the point index explicitly.
    VertexOut.point_id = gl_PrimitiveIDIn;

    // transform the four points. Note the order of output
    gl_Position = MVP * b;
    VertexOut.tex = vec2(-1.0, 1.0);
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    gl_Position = MVP * a;
    VertexOut.tex = vec2(-1.0, -1.0);
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    gl_Position = MVP * c;
    VertexOut.tex = vec2(1.0, 1.0);
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    gl_Position = MVP * d;
    VertexOut.tex = vec2(1.0, -1.0);
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
    }
    EmitVertex();

    EndPrimitive();
}
