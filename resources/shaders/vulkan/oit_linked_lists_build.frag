#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// Per-pixel linked lists OIT, build pass (Yang et al., NVIDIA 2010). Every
// transparent fragment is shaded exactly like the average/peel passes, then
// appended to a per-pixel singly linked list stored in storage buffers. An
// atomic counter hands out a unique node slot; an atomic compare-and-swap on
// the pixel's head pointer pushes the node onto the list. No depth test or
// blending runs here, so all fragments survive regardless of order.
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(std430,set=2,binding=0) buffer HeadBuffer { int head[]; };
struct FragmentNode { vec4 color; float depth; int next; };
layout(std430,set=2,binding=1) buffer NodeBuffer { FragmentNode nodes[]; };
layout(std430,set=2,binding=2) buffer CounterBuffer { uint node_counter; };
layout(set=2,binding=3) uniform ListsParams { uint max_nodes; uint width; uint height; uint pad; } params;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
void main() {
    vec4 color = vertex_color * texture(image_texture, uv);
    if (material.shading.x != 0 && world_normal_out.w != 0)
        color = shade_material(color, world_normal_out.xyz, world_position, !gl_FrontFacing);
    color.rgb = mix(color.rgb, selection_tint.rgb, selection_tint.a);
    float alpha = clamp(color.a, 0.0, 1.0);
    // Reserve a node. Once the buffer is full, drop the fragment (the resolve
    // pass still composites everything that fit, matching the OpenGL build).
    uint node_index = atomicAdd(node_counter, 1u);
    if (node_index >= params.max_nodes) return;
    uint pixel_index = uint(gl_FragCoord.y) * params.width + uint(gl_FragCoord.x);
    // std430 stride is 32 bytes: vec4(16) + float(4) + int(4) + padding(8).
    nodes[node_index].color = vec4(color.rgb * alpha, alpha);
    nodes[node_index].depth = gl_FragCoord.z;
    // Push onto the pixel list. The loop retries until this invocation wins the
    // head slot, so concurrent fragments on one pixel form a correct chain.
    int old_head;
    do {
        old_head = head[pixel_index];
        nodes[node_index].next = old_head;
    } while (atomicCompSwap(head[pixel_index], old_head, int(node_index)) != old_head);
}