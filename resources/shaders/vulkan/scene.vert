#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texcoord;
layout(location = 4) in uint primitive_id;
layout(push_constant) uniform Draw {
    mat4 mvp;
    vec4 tint;
    uint object_id;
    uint lighting;
    float normal_matrix[9]; // std430 scalar-array stride 4; total push data = 124 bytes
} draw;
layout(location = 0) out vec4 vertex_color;
layout(location = 1) out vec2 uv;
layout(location = 2) flat out uvec2 id;
void main() {
    vec4 p = draw.mvp * vec4(position, 1.0);
    gl_Position = vec4(p.x, -p.y, 0.5 * (p.z + p.w), p.w);
    gl_PointSize = 1.0;
    float diffuse = 1.0;
    mat3 n = mat3(vec3(draw.normal_matrix[0], draw.normal_matrix[1], draw.normal_matrix[2]),
                  vec3(draw.normal_matrix[3], draw.normal_matrix[4], draw.normal_matrix[5]),
                  vec3(draw.normal_matrix[6], draw.normal_matrix[7], draw.normal_matrix[8]));
    vec3 world_normal = n * normal;
    if (draw.lighting != 0 && dot(world_normal, world_normal) > 0.0)
        diffuse = 0.2 + 0.8 * abs(dot(normalize(world_normal), normalize(vec3(0.3, 0.5, 1.0))));
    vertex_color = color * draw.tint;
    vertex_color.rgb *= diffuse;
    uv = texcoord;
    id = uvec2(draw.object_id, primitive_id);
}
