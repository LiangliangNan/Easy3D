#version 150

in  vec3 vtx_position;
in  vec3 vtx_normal;
in  vec3 vtx_color;

uniform mat4	MVP;
uniform mat4    MANIP = mat4(1.0);
uniform vec3	default_color = vec3(0.4f, 0.8f, 0.8f);
uniform bool	per_vertex_color = false;

out Data{
    vec3 color;
    vec3 position;
    vec3 normal;
} DataOut;

void main() {
    if (per_vertex_color)
        DataOut.color = vtx_color;
    else
        DataOut.color = default_color;
    DataOut.position = vtx_position;
    DataOut.normal = vtx_normal;
    gl_Position = MVP * MANIP * vec4(vtx_position, 1.0);
}
