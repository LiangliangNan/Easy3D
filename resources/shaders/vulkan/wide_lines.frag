#version 450
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(location=0) in vec4 ribbon_color;
layout(location=1) in vec2 ribbon_uv;
layout(location=2) flat in uvec2 id;
layout(location=0) out vec4 out_color;
layout(location=1) out uvec2 out_id;
void main() {
    out_color = ribbon_color * texture(image_texture,ribbon_uv);
    out_id = id;
    // Keep the rasterizer's per-sample depth; no explicit depth output or discard.
}
