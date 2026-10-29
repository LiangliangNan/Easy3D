#version 450
layout(set=0,binding=0) uniform sampler2D scene_depth;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=0) out vec4 out_color;
void main() {
    float depth = texture(scene_depth,uv).r;
    out_color = vec4(vec3(depth),1)*vertex_color;
}
