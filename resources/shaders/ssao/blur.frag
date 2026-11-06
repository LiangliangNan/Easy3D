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

// Edge-aware denoising of the ground-truth AO term. Taps that lie on a
// different plane than the center pixel (measured against the view-space
// position and normal) contribute almost nothing, so silhouettes and depth
// discontinuities keep their contrast.

in vec2 TexCoords;

uniform sampler2D ssaoInput;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform float radius;

out float FragColor;

const int RADIUS = 2;

void main() {
    vec3 normal = texture(gNormal, TexCoords).rgb;
    if (dot(normal, normal) < 1e-8) {
        FragColor = 1.0;
        return;
    }
    normal = normalize(normal);
    vec3 center = texture(gPosition, TexCoords).xyz;
    vec2 texel = 1.0 / vec2(textureSize(ssaoInput, 0));

    float sum = 0.0, weight = 0.0;
    for (int y = -RADIUS; y <= RADIUS; ++y) {
        for (int x = -RADIUS; x <= RADIUS; ++x) {
            vec2 coord = TexCoords + vec2(x, y) * texel;
            if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
                continue;
            vec3 neighbor = texture(gNormal, coord).rgb;
            if (dot(neighbor, neighbor) < 1e-8)
                continue; // background
            float plane = abs(dot(texture(gPosition, coord).xyz - center, normal));
            float w = exp(-float(x * x + y * y) / 8.0) * exp(-plane / (0.05 * radius));
            sum += texture(ssaoInput, coord).r * w;
            weight += w;
        }
    }

    FragColor = weight > 0.0 ? sum / weight : 1.0;
}
