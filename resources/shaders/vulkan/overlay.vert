#version 450
layout(location=0) in vec3 position;
layout(location=2) in vec4 color;
layout(location=3) in vec2 texcoord;
layout(push_constant) uniform Draw {
    mat4 mvp; vec4 tint; uint object_id; uint lighting; float normal_matrix[9];
} draw;
layout(location=0) out vec4 vertex_color;
layout(location=1) out vec2 uv;
void main() {
    vec4 p = draw.mvp*vec4(position,1);
    gl_Position = vec4(p.x,-p.y,0.5*(p.z+p.w),p.w);
    vertex_color = color*draw.tint;
    uv = texcoord;
}
