#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
layout(set=0,binding=0) uniform sampler2D image_texture;

// Plain points. A plain point has no impostor surface, so the rasterized depth is
// already correct and this shader never writes gl_FragDepth. That keeps early
// depth testing enabled, which matters for dense clouds: the OpenGL plain-point
// shader does the same. The impostor path (sphere/surfel) needs the depth
// write and lives in the separate impostor shader.
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=2) flat in uvec2 id;
layout(location=4) flat in uint lighting;
layout(location=6) flat in vec4 selection_tint;
layout(location=7) in vec3 world_position;
layout(location=8) in vec3 world_normal_out;
layout(location=0) out vec4 out_color;
layout(location=1) out uvec2 out_id;
void main() {
    out_color = vertex_color * texture(image_texture,uv);
    out_id = id;
    if (material.shading.x!=0 && lighting!=0)
        out_color=shade_material(out_color,world_normal_out,world_position,
            dot(world_normal_out,material.eye.xyz-world_position)<0);
    out_color.rgb=mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
}
