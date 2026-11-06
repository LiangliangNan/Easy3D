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

// Ground-truth ambient occlusion from the view-space position and normal
// buffers. Each pixel marches a set of azimuthal slices along the projected
// sample radius to find the horizon of the surrounding depth; the cosine-
// weighted visibility of the two half-planes around that horizon gives the AO
// term. The same algorithm runs on the Vulkan backend (ao_gtao.comp).

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

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

// View-space position to texture coordinates of this framebuffer.
vec2 project(vec3 position) {
    vec4 clip = PROJ * vec4(position, 1.0);
    return clip.xy / clip.w * 0.5 + 0.5;
}

// Sine of the elevation of an occluder above the tangent plane, or zero when
// the sample cannot occlude the center pixel.
float horizon_sine(vec2 coord, vec3 center, vec3 normal) {
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return 0.0;
    vec3 neighbor = texture(gNormal, coord).rgb;
    if (dot(neighbor, neighbor) < 1e-8)
        return 0.0; // background
    vec3 delta = texture(gPosition, coord).xyz - center;
    float distance = length(delta);
    if (distance < 1e-5)
        return 0.0;
    float elevation = dot(delta, normal);
    if (elevation <= bias) // behind (or on) the tangent plane
        return 0.0;
    return clamp(elevation / distance, 0.0, 1.0) * clamp(radius / distance, 0.0, 1.0);
}

void main() {
    vec3 normal = texture(gNormal, TexCoords).rgb;
    if (dot(normal, normal) < 1e-8) { // the background is never darkened
        FragColor = 1.0;
        return;
    }
    normal = normalize(normal);
    vec3 center = texture(gPosition, TexCoords).xyz;

    vec2 size = vec2(textureSize(gPosition, 0));
    float jitter = hash12(gl_FragCoord.xy);

    vec3 tangent = cross(normal, abs(normal.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0));
    if (dot(tangent, tangent) < 1e-12)
        tangent = cross(normal, vec3(0.0, 0.0, 1.0));
    tangent = normalize(tangent);
    vec3 bitangent = cross(normal, tangent);
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
