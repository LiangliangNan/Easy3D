#version 450
// Composites the blurred ground-truth ambient occlusion term over the scene.
// The AO value is the fraction of the cosine-weighted hemisphere that remains
// visible, so multiplying the destination color by it darkens only creases.
layout(set = 0, binding = 2) uniform sampler2D ao_texture;
layout(location = 0) in vec4 vertex_color;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main() {
    float ao = clamp(texture(ao_texture, uv).r, 0.0, 1.0);
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - ao);
}
