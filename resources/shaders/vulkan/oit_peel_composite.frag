#version 450
// The peel passes already accumulated premultiplied front-to-back color, so the
// composite is a premultiplied over-blend onto the scene.
layout(set=0,binding=0) uniform sampler2D accumulated;
layout(location=0) in vec2 uv;
layout(location=0) out vec4 out_color;
void main() {
    out_color = texture(accumulated, uv);
}
