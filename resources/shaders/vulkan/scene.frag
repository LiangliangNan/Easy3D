#version 450
layout(set = 0, binding = 0) uniform sampler2D image_texture;
layout(location = 0) in vec4 vertex_color;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in uvec2 id;
layout(location = 0) out vec4 out_color;
layout(location = 1) out uvec2 out_id;
void main() {
    out_color = vertex_color * texture(image_texture, uv);
    out_id = id;
}
