#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
layout(set = 0, binding = 0) uniform sampler2D image_texture;
layout(location = 0) in vec4 vertex_color;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in uvec2 id;
layout(location = 0) out vec4 out_color;
layout(location = 1) out uvec2 out_id;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
void main() {
    out_color = vertex_color * texture(image_texture, uv);
    out_id = id;
    if(material.shading.x!=0 && world_normal_out.w!=0)
        out_color=shade_material(out_color,world_normal_out.xyz,world_position,!gl_FrontFacing);
    out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
}
