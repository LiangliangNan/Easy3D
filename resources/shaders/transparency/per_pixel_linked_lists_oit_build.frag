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

// Per-pixel head pointers: one int per pixel, initialized to -1 (empty list).
// Each pixel's linked list starts at head[pixel_index] and chains through
// node_next[] until a -1 sentinel.
layout(std430, binding = 0) buffer HeadBuffer {
    int head[];
};

// Fragment node pool. Each node stores premultiplied color, depth, and a
// pointer to the next node in the list.
// std430 struct layout:
//   color:  vec4  offset  0, size 16, align 16
//   depth:  float offset 16, size  4, align  4
//   next:   int   offset 20, size  4, align  4
//   (8 bytes implicit padding to reach struct alignment of 16)
//   Total stride: 32 bytes per node.
struct FragmentNode {
    vec4 color;
    float depth;
    int next;
};

layout(std430, binding = 1) buffer NodeBuffer {
    FragmentNode nodes[];
};

// Atomic counter for allocating node indices. Reset to 0 each frame.
layout(binding = 0) uniform atomic_uint node_counter;

uniform int viewport_width;
uniform int viewport_height;
uniform int max_nodes;

// --- Lighting uniforms (matching the peel rendering shader) ---

#ifndef USE_PROVIDED_NORMALS
uniform bool smooth_shading;
#endif

uniform float Alpha;

uniform vec3 wLightPos;
uniform vec3 wCamPos;
layout(std140) uniform Material {
    vec3 ambient;
    vec3 specular;
    float shininess;
};

uniform bool lighting = true;
uniform bool two_sides_lighting = true;
uniform bool distinct_back_color = true;
uniform vec3 backside_color = vec3(0.8, 0.4, 0.4);

uniform bool selected = false;
uniform vec4 highlight_color;

uniform bool use_texture = false;
uniform sampler2D textureID;
uniform float texture_repeat = 1.0;
uniform float fractional_repeat = 0.0;

in Data{
    vec2 texcoord;
    vec3 color;
    vec3 normal;
    vec3 position;
    float clipped;
} DataIn;

// Dummy output: the build pass writes fragment data to SSBOs, not to the
// framebuffer color. A minimal output keeps the FBO attachment valid.
layout(location = 0) out vec4 fragOutput;


// Compute the shaded fragment color. Same lighting model as the dual depth
// peeling peel rendering shader.
vec4 ShadeFragment(vec3 flat_normal)
{
    vec4 color;
    if (use_texture) {
        float repeat = texture_repeat + fractional_repeat / 100.0;
        color = texture(textureID, DataIn.texcoord * repeat);
        color.a = 1.0;
    } else {
        color = vec4(DataIn.color, 1.0);
    }

    if ((gl_FrontFacing == false) && distinct_back_color)
        color = vec4(backside_color, color.a);

    if (selected)
        color = mix(color, highlight_color, 0.6);

    if (!lighting)
        return vec4(color.xyz, Alpha);

    vec3 normal;
    if (smooth_shading)
        normal = normalize(DataIn.normal);
    else {
        normal = flat_normal;
        if ((gl_FrontFacing == false) && (two_sides_lighting == false))
            normal = -normal;
    }

    vec3 view_dir = normalize(wCamPos - DataIn.position);
    vec3 light_dir = normalize(wLightPos);

    float df = 0.0;
    if (two_sides_lighting)
        df = abs(dot(light_dir, normal));
    else
        df = max(dot(light_dir, normal), 0.0);

    float sf = 0.0;
    if (df > 0.0) {
        vec3 half_vector = normalize(light_dir + view_dir);
        if (two_sides_lighting)
            sf = abs(dot(half_vector, normal));
        else
            sf = max(dot(half_vector, normal), 0.0);
        sf = pow(sf, shininess);
    }

    return vec4(color.xyz * df + specular * sf + ambient, Alpha);
}


void main(void)
{
    // Compute the flat (face) normal in uniform control flow, before any
    // discard. dFdx/dFdy are undefined after non-uniform returns (see #219).
    vec3 flat_normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));

    if (DataIn.clipped > 0.0)
        discard;

    vec4 color = ShadeFragment(flat_normal);

    // Pixel coordinates from the window-space fragment position.
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    if (pixel.x < 0 || pixel.x >= viewport_width || pixel.y < 0 || pixel.y >= viewport_height) {
        fragOutput = vec4(0.0);
        return;
    }
    int pixel_index = pixel.y * viewport_width + pixel.x;

    // Allocate a node index from the atomic counter.
    uint node_index = atomicCounterIncrement(node_counter);
    if (int(node_index) >= max_nodes) {
        fragOutput = vec4(0.0);
        return;
    }

    // Store the fragment data with premultiplied alpha.
    float a = color.a;
    nodes[node_index].color = vec4(color.rgb * a, a);
    nodes[node_index].depth = gl_FragCoord.z;

    // Insert at the head of this pixel's linked list using atomic compare-and-swap.
    // The loop retries if another fragment modified the head between our read and write.
    int old_head;
    do {
        old_head = head[pixel_index];
        nodes[node_index].next = old_head;
    } while (atomicCompSwap(head[pixel_index], old_head, int(node_index)) != old_head);

    fragOutput = vec4(0.0);
}
