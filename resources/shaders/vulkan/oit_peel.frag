#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// One front-to-back depth-peeling layer. Fragments at or in front of the
// previous layer are rejected; the depth test then keeps only the nearest
// surviving fragment, which is written to the layer target without blending.
// A full-screen pass over-blends that premultiplied layer into the result.
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(set=0,binding=3) uniform sampler2D previous_depth;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
layout(location=0) out vec4 out_color;
void main() {
    // Normalized lookup so the 1x1 sentinel image of the first layer clamps to
    // its single cleared texel (0.0) instead of reading out of bounds.
    vec2 previous_uv = gl_FragCoord.xy / vec2(textureSize(previous_depth, 0));
    float previous = texture(previous_depth, previous_uv).r;
    // The same epsilon as the OpenGL peel shaders absorbs rasterization
    // differences between passes of one surface.
    if (gl_FragCoord.z <= previous + 0.00001) discard;
    gl_FragDepth = gl_FragCoord.z;
    vec4 color = vertex_color * texture(image_texture, uv);
    if (material.shading.x != 0 && world_normal_out.w != 0)
        color = shade_material(color, world_normal_out.xyz, world_position, !gl_FrontFacing);
    color.rgb = mix(color.rgb, selection_tint.rgb, selection_tint.a);
    float alpha = clamp(color.a, 0.0, 1.0);
    out_color = vec4(color.rgb * alpha, alpha);
}
