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

#version 430
precision highp float;

// Maximum number of fragments we can sort per pixel. Fragments beyond this
// limit are silently dropped. 64 is generous for typical scenes; the common
// case is 2-8 fragments per pixel.
#define MAX_FRAGMENTS 64

// Must match the build shader's SSBO layout.
layout(std430, binding = 0) buffer HeadBuffer {
    int head[];
};

struct FragmentNode {
    vec4 color;
    float depth;
    int next;
};

layout(std430, binding = 1) buffer NodeBuffer {
    FragmentNode nodes[];
};

uniform int viewport_width;
uniform int viewport_height;
uniform vec4 background_color;

in vec2 texCoord;

layout(location = 0) out vec4 fragOutput;


void main(void)
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    if (pixel.x < 0 || pixel.x >= viewport_width || pixel.y < 0 || pixel.y >= viewport_height) {
        fragOutput = background_color;
        return;
    }
    int pixel_index = pixel.y * viewport_width + pixel.x;

    // Traverse the linked list for this pixel, collecting fragments.
    vec4 colors[MAX_FRAGMENTS];
    float depths[MAX_FRAGMENTS];
    int count = 0;

    int node_index = head[pixel_index];
    while (node_index != -1 && count < MAX_FRAGMENTS) {
        colors[count] = nodes[node_index].color;
        depths[count] = nodes[node_index].depth;
        node_index = nodes[node_index].next;
        ++count;
    }

    // Empty list: output background.
    if (count == 0) {
        fragOutput = background_color;
        return;
    }

    // Insertion sort by depth (ascending = front-to-back).
    // O(N^2) but efficient for the small N values typical in OIT.
    for (int i = 1; i < count; ++i) {
        vec4 key_color = colors[i];
        float key_depth = depths[i];
        int j = i - 1;
        while (j >= 0 && depths[j] > key_depth) {
            colors[j + 1] = colors[j];
            depths[j + 1] = depths[j];
            --j;
        }
        colors[j + 1] = key_color;
        depths[j + 1] = key_depth;
    }

    // Front-to-back compositing with premultiplied alpha.
    // Each node stores (color.rgb * alpha, alpha). The composite over the
    // background is:
    //   result = frag + result * (1 - frag.a)
    vec4 result = background_color;
    for (int i = 0; i < count; ++i) {
        vec4 frag = colors[i];
        result.rgb = frag.rgb + result.rgb * (1.0 - frag.a);
        result.a = frag.a + result.a * (1.0 - frag.a);
    }

    fragOutput = result;
}
