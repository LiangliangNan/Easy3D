#version 450
// Per-pixel linked lists OIT, resolve pass. A full-screen triangle visits every
// pixel, walks its fragment list, sorts the fragments by depth, and composites
// them front-to-back into a premultiplied accumulation. The scene pass then
// over-blends that result onto the opaque geometry (the peel composite), so an
// empty pixel stays (0,0,0,0) and leaves the scene untouched.
#define MAX_FRAGMENTS 64
layout(std430,set=0,binding=0) buffer HeadBuffer { int head[]; };
struct FragmentNode { vec4 color; float depth; int next; };
layout(std430,set=0,binding=1) buffer NodeBuffer { FragmentNode nodes[]; };
layout(set=0,binding=3) uniform ListsParams { uint max_nodes; uint width; uint height; uint pad; } params;
layout(location=0) in vec2 uv;
layout(location=0) out vec4 out_color;
void main() {
    uint pixel_index = uint(gl_FragCoord.y) * params.width + uint(gl_FragCoord.x);
    vec4 colors[MAX_FRAGMENTS];
    float depths[MAX_FRAGMENTS];
    int count = 0;
    for (int idx = head[pixel_index]; idx != -1 && count < MAX_FRAGMENTS; idx = nodes[idx].next) {
        colors[count] = nodes[idx].color;
        depths[count] = nodes[idx].depth;
        ++count;
    }
    if (count == 0) { out_color = vec4(0.0); return; }
    // Insertion sort ascending depth (nearest first); lists are short.
    for (int i = 1; i < count; ++i) {
        vec4 c = colors[i]; float d = depths[i];
        int j = i - 1;
        while (j >= 0 && depths[j] > d) {
            colors[j + 1] = colors[j]; depths[j + 1] = depths[j]; --j;
        }
        colors[j + 1] = c; depths[j + 1] = d;
    }
    // Front-to-back premultiplied accumulation: accum.a tracks 1 - transmittance,
    // so each fragment contributes its premultiplied color scaled by the light
    // still reaching it. The result is premultiplied (rgb, 1 - T).
    vec4 accum = vec4(0.0);
    for (int i = 0; i < count; ++i) {
        accum.rgb += colors[i].rgb * (1.0 - accum.a);
        accum.a += colors[i].a * (1.0 - accum.a);
    }
    out_color = accum;
}