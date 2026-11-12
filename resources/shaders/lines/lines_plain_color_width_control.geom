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


// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;


uniform mat4 PROJ;
uniform mat4 invMV;
uniform bool perspective;
uniform float radius;

// Screen-space expansion: the stroke keeps a constant width in framebuffer
// pixels, so the segment is expanded in screen space (the same rule as the
// native Vulkan ribbon). The camera-space expansion above uses a world-space
// radius, so its width on screen changes with the distance to the camera.
uniform bool screen_space_width;
uniform vec2 viewport_size;   // framebuffer size in pixels
uniform float line_width;     // full stroke width in framebuffer pixels

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in Data {
    vec4 color;
    float clipped;
} DataIn[];

out vec4  gOutColor;
out float gOutClipped;
flat out int line_id;

void emit_vertex(vec3 point, vec4 position, vec4 color, float clipped)
{
    gl_Position = position;
    gOutColor = color;
    gOutClipped = clipped;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(point, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(point, 1.0), clippingPlane1);
    }
    EmitVertex();
}

void main()
{
    // macOS ignores GS-written gl_PrimitiveID; pass the line index explicitly.
    line_id = gl_PrimitiveIDIn;
    vec3  base = gl_in[0].gl_Position.xyz;// in camera space
    vec3  top = gl_in[1].gl_Position.xyz;// in camera space

    if (screen_space_width) {
        // Clip the centerline against the eye plane before the perspective division.
        vec4 clip_base = PROJ * vec4(base, 1.0);
        vec4 clip_top = PROJ * vec4(top, 1.0);
        float eye_epsilon = 1e-6 * max(max(abs(clip_base.w), abs(clip_top.w)), 1e-20);
        float lo = 0.0, hi = 1.0;
        if (clip_base.w < eye_epsilon)
            lo = (eye_epsilon - clip_base.w) / (clip_top.w - clip_base.w);
        if (clip_top.w < eye_epsilon)
            hi = (eye_epsilon - clip_top.w) / (clip_base.w - clip_top.w);
        if (lo > hi) {
            // The whole segment is behind the eye.
            for (int i = 0; i < 4; ++i) {
                gOutColor = vec4(0);
                gOutClipped = 0.0;
                gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
                EmitVertex();
            }
            EndPrimitive();
            return;
        }
        vec3 start_point = mix(base, top, lo);
        vec3 end_point = mix(base, top, hi);
        vec4 start = mix(clip_base, clip_top, lo);
        vec4 end = mix(clip_base, clip_top, hi);
        vec2 ndc_start = start.xy / start.w;
        vec2 ndc_end = end.xy / end.w;

        // Perpendicular half-width, in normalized device coordinates.
        vec2 delta = (ndc_end - ndc_start) * viewport_size;
        float length_pixels = length(delta);
        if (length_pixels <= 0.0) {
            for (int i = 0; i < 4; ++i) {
                gOutColor = vec4(0);
                gOutClipped = 0.0;
                gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
                EmitVertex();
            }
            EndPrimitive();
            return;
        }
        vec2 offset = vec2(-delta.y, delta.x) / length_pixels * line_width / viewport_size;

        emit_vertex(start_point, vec4((ndc_start - offset) * start.w, start.z, start.w), DataIn[0].color, DataIn[0].clipped);
        emit_vertex(end_point, vec4((ndc_end - offset) * end.w, end.z, end.w), DataIn[1].color, DataIn[1].clipped);
        emit_vertex(start_point, vec4((ndc_start + offset) * start.w, start.z, start.w), DataIn[0].color, DataIn[0].clipped);
        emit_vertex(end_point, vec4((ndc_end + offset) * end.w, end.z, end.w), DataIn[1].color, DataIn[1].clipped);
        EndPrimitive();
        return;
    }

    // Camera-space expansion: a world-space radius, so the projected width
    // follows the perspective foreshortening.
    vec3  axes = top - base;
    vec3  view_dir = vec3(0, 0, 1);// this is view direction for orthographic mode in camera space
    if (perspective)
        view_dir = normalize(vec3(0) - 0.5*(base + top));// camera pos is (0, 0, 0) in camera space

    vec3 offset = cross(view_dir, axes);
    offset = radius * normalize(offset);

    // Vertex 1
    emit_vertex(base - offset, PROJ * vec4(base - offset, 1.0), DataIn[0].color, DataIn[0].clipped);

    // Vertex 2
    emit_vertex(top - offset, PROJ * vec4(top - offset, 1.0), DataIn[1].color, DataIn[1].clipped);

    // Vertex 3
    emit_vertex(base + offset, PROJ * vec4(base + offset, 1.0), DataIn[0].color, DataIn[0].clipped);

    // Vertex 4
    emit_vertex(top + offset, PROJ * vec4(top + offset, 1.0), DataIn[1].color, DataIn[1].clipped);

    EndPrimitive();
}
