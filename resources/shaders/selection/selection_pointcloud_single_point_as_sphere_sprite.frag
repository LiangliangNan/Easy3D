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

uniform mat4 PROJ;
uniform bool sphere;
uniform bool perspective;
uniform float sphere_radius;

flat in int source_id;
flat in vec4 position;
out vec4 outputF;

void main() {
    gl_FragDepth = gl_FragCoord.z;
    if (sphere) {
        vec2 tex = (gl_PointCoord * 2.0 - 1.0) * vec2(1, -1);
        vec3 hit;
        if (perspective) {
            vec3 ray = normalize(position.xyz + vec3(tex * (1.5 * sphere_radius), 0));
            float along = dot(ray, position.xyz);
            float discriminant = along * along - dot(position.xyz, position.xyz) + sphere_radius * sphere_radius;
            if (discriminant < 0.0)
                discard;
            hit = ray * (along - sqrt(discriminant));
        } else {
            float z2 = 1.0 - dot(tex, tex);
            if (z2 < 0.0)
                discard;
            hit = position.xyz + vec3(tex, sqrt(z2)) * sphere_radius;
        }
        vec4 clip = PROJ * vec4(hit, 1.0);
        gl_FragDepth = 0.5 * clip.z / clip.w + 0.5;
    }

    // Preserve the VBO slot in both indexed and unindexed draws.
    outputF = vec4((source_id >> 16) & 255, (source_id >> 8) & 255,
                   source_id & 255, (source_id >> 24) & 255) / 255.0;
}
