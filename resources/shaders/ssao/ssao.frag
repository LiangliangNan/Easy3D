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

// Ground-truth ambient occlusion from the scene depth of the projection
// buffer. View-space positions and normals are reconstructed from depth, then
// each pixel marches a set of azimuthal slices along the projected sample
// radius to find the horizon of the surrounding depth. The cosine-weighted
// visibility of the two half-planes around that horizon gives the AO term.
// The same algorithm runs on the Vulkan backend (ao_gtao.comp).

in vec2 TexCoords;

uniform sampler2D scene_depth;
uniform mat4 invPROJ;
uniform mat4 PROJ;

uniform float radius;
uniform float bias;

out float FragColor;

const int SLICE_COUNT = 8;
const int STEP_COUNT = 8;

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// View-space position from a depth sample.
vec3 view_position(vec2 coord, float depth) {
    vec4 p = invPROJ * vec4(coord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return p.xyz / p.w;
}

// Position of a depth neighbor. Background neighbors are reported invalid so
// the normal reconstruction never mixes separate surfaces across a silhouette.
vec3 neighbor_position(vec2 coord, vec3 center, out bool valid) {
    float depth = texture(scene_depth, coord).r;
    valid = depth < 1.0;
    return valid ? view_position(coord, depth) : center;
}

// View-space normal from the closest surviving neighbor on each axis.
vec3 surface_normal(vec2 coord, vec3 center, vec2 texel) {
    bool valid_left, valid_right, valid_up, valid_down;
    vec3 left = neighbor_position(coord - vec2(texel.x, 0.0), center, valid_left);
    vec3 right = neighbor_position(coord + vec2(texel.x, 0.0), center, valid_right);
    vec3 up = neighbor_position(coord - vec2(0.0, texel.y), center, valid_up);
    vec3 down = neighbor_position(coord + vec2(0.0, texel.y), center, valid_down);
    const float far_away = 1e30;
    float left_distance = valid_left ? abs(left.z - center.z) : far_away;
    float right_distance = valid_right ? abs(right.z - center.z) : far_away;
    float up_distance = valid_up ? abs(up.z - center.z) : far_away;
    float down_distance = valid_down ? abs(down.z - center.z) : far_away;
    vec3 dx = left_distance < right_distance ? center - left : right - center;
    vec3 dy = up_distance < down_distance ? up - center : center - down;
    vec3 normal = cross(dx, dy);
    normal = dot(normal, normal) < 1e-20 ? -normalize(center) : normalize(normal);
    return dot(normal, center) > 0.0 ? -normal : normal; // face the camera
}

// Texture coordinates of a view-space position in this framebuffer.
vec2 project(vec3 position) {
    vec4 p = PROJ * vec4(position, 1.0);
    return p.xy / p.w * 0.5 + 0.5;
}

// Sine of the elevation of an occluder above the tangent plane, or zero when
// the sample cannot occlude the center pixel.
float horizon_sine(vec2 coord, vec3 center, vec3 normal) {
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return 0.0;
    float depth = texture(scene_depth, coord).r;
    if (depth >= 1.0) // background
        return 0.0;
    vec3 delta = view_position(coord, depth) - center;
    float distance_squared = dot(delta, delta);
    // The kernel is a sphere: geometry beyond the radius belongs to a different
    // surface across a depth jump and must not occlude.
    if (distance_squared >= radius * radius)
        return 0.0;
    float distance = sqrt(distance_squared);
    if (distance < 1e-5)
        return 0.0;
    float elevation = dot(delta, normal);
    if (elevation <= bias) // behind (or on) the tangent plane
        return 0.0;
    return clamp(elevation / distance, 0.0, 1.0);
}

void main() {
    float depth = texture(scene_depth, TexCoords).r;
    if (depth >= 1.0) { // the background is never darkened
        FragColor = 1.0;
        return;
    }
    vec3 center = view_position(TexCoords, depth);
    vec2 size = vec2(textureSize(scene_depth, 0));
    vec2 texel = 1.0 / size;
    vec3 normal = surface_normal(TexCoords, center, texel);

    vec3 tangent = cross(normal, abs(normal.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0));
    if (dot(tangent, tangent) < 1e-12)
        tangent = cross(normal, vec3(0.0, 0.0, 1.0));
    tangent = normalize(tangent);
    vec3 bitangent = cross(normal, tangent);

    float jitter = hash12(gl_FragCoord.xy);
    vec2 center_uv = project(center);
    float visibility = 0.0;
    for (int slice = 0; slice < SLICE_COUNT; ++slice) {
        float phi = (float(slice) + jitter) * (6.28318530718 / float(SLICE_COUNT));
        vec3 omega = tangent * cos(phi) + bitangent * sin(phi);

        // Screen-space extent of the half-slice: the radius projected along omega.
        vec2 span = (project(center + omega * radius) - center_uv) * size;
        float distance = length(span);
        if (distance < 1.0)
            continue;
        vec2 direction = span / distance / size;

        float horizon_positive = 0.0, horizon_negative = 0.0;
        for (int step_index = 0; step_index < STEP_COUNT; ++step_index) {
            float offset = distance * (float(step_index) + 0.5) / float(STEP_COUNT);
            horizon_positive = max(horizon_positive, horizon_sine(TexCoords + direction * offset, center, normal));
            horizon_negative = max(horizon_negative, horizon_sine(TexCoords - direction * offset, center, normal));
        }
        // Cosine-weighted visibility of the two half-planes around the horizon.
        visibility += 1.0 - 0.5 * (horizon_positive * horizon_positive + horizon_negative * horizon_negative);
    }

    FragColor = clamp(visibility / float(SLICE_COUNT), 0.0, 1.0);
}
