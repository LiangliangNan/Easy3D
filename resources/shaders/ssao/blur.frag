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

// Edge-aware denoising of the ground-truth AO term. Positions and normals are
// reconstructed from the scene depth, exactly as in the GTAO pass. Taps that
// lie on a different plane than the center pixel contribute almost nothing, so
// silhouettes and depth discontinuities keep their contrast.
// The same filter runs on the Vulkan backend (ao_blur.comp).

in vec2 TexCoords;

uniform sampler2D scene_depth;
uniform sampler2D ssaoInput;
uniform mat4 invPROJ;

uniform float radius;

out float FragColor;

const int RADIUS = 2;

vec3 view_position(vec2 coord, float depth) {
    vec4 p = invPROJ * vec4(coord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return p.xyz / p.w;
}

vec3 neighbor_position(vec2 coord, vec3 center, out bool valid) {
    float depth = texture(scene_depth, coord).r;
    valid = depth < 1.0;
    return valid ? view_position(coord, depth) : center;
}

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

    float plane_tolerance = 0.05 * radius;
    float sum = 0.0, weight = 0.0;
    for (int y = -RADIUS; y <= RADIUS; ++y) {
        for (int x = -RADIUS; x <= RADIUS; ++x) {
            vec2 coord = TexCoords + vec2(x, y) * texel;
            if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
                continue;
            float neighbor_depth = texture(scene_depth, coord).r;
            if (neighbor_depth >= 1.0)
                continue; // background
            vec3 neighbor = view_position(coord, neighbor_depth);
            float plane = exp(-abs(dot(neighbor - center, normal)) / plane_tolerance);
            float w = exp(-float(x * x + y * y) / 8.0) * plane;
            sum += texture(ssaoInput, coord).r * w;
            weight += w;
        }
    }

    FragColor = weight > 0.0 ? sum / weight : 1.0;
}
