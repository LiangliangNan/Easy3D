#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// Order-independent average-color accumulation, matching the OpenGL
// transparency/average_color_init shader: premultiplied color and alpha are
// summed, and the fragment count is accumulated separately.
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
layout(location=0) out vec4 out_color;
layout(location=1) out vec4 out_count;
void main() {
    vec4 color = vertex_color * texture(image_texture, uv);
    if (material.shading.x != 0 && world_normal_out.w != 0)
        color = shade_material(color, world_normal_out.xyz, world_position, !gl_FrontFacing);
    color.rgb = mix(color.rgb, selection_tint.rgb, selection_tint.a);
    float alpha = clamp(color.a, 0.0, 1.0);
    out_color = vec4(color.rgb * alpha, alpha);
    out_count = vec4(1.0);
}
