#version 150

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;	// point color

uniform mat4	MVP;
uniform vec3	default_color = vec3(0.0f, 0.0f, 0.0f);
uniform bool	per_vertex_color = false;

out vec3 vOutColor;

void main()
{
    gl_Position = MVP * vec4(vtx_position, 1.0);

    if (per_vertex_color)
        vOutColor = vtx_color;
    else
        vOutColor = default_color;
}
