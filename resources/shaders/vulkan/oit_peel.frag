#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// One front-to-back depth-peeling layer. Fragments at or in front of the
// previous layer are rejected; the depth test then keeps only the nearest
// surviving fragment, which is written to the layer target without blending.
// A full-screen pass under-blends that premultiplied layer into the result, so
// each farther layer ends up behind the ones already accumulated.
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(set=0,binding=3) uniform sampler2D previous_depth;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
layout(location=0) out vec4 out_color;
layout(push_constant) uniform PushConstants {
    mat4 mvp; vec4 color; uint object_id; uint lighting; float normal_matrix[9];
    uint image_uv_flip; vec2 depth_range;
} push;
void main() {
    // Normalized lookup so the 1x1 sentinel image of the first layer clamps to
    // its single cleared texel (0.0) instead of reading out of bounds.
    vec2 previous_uv = gl_FragCoord.xy / vec2(textureSize(previous_depth, 0));
    float previous = texture(previous_depth, previous_uv).r;
    float depth = push.depth_range.y > push.depth_range.x
                      ? push.depth_range.x + gl_FragCoord.z * (push.depth_range.y - push.depth_range.x)
                      : push.depth_range.x;
    if (depth <= previous) discard;
    // Write the mapped command depth deliberately. Any fragment-shader depth write
    // keeps late depth testing, so the previous-depth rejection and zero-alpha discard
    // happen before a fragment can win the layer. Otherwise a rejected nearer fragment
    // could hide this layer's own fragment and make the result depend on draw order.
    vec4 color = vertex_color * texture(image_texture, uv);
    if (material.shading.x != 0 && world_normal_out.w != 0)
        color = shade_material(color, world_normal_out.xyz, world_position, !gl_FrontFacing);
    color.rgb = mix(color.rgb, selection_tint.rgb, selection_tint.a);
    float alpha = clamp(color.a, 0.0, 1.0);
    if (alpha == 0.0) discard;
    gl_FragDepth = depth;
    out_color = vec4(color.rgb * alpha, alpha);
}
