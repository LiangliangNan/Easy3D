#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_gpu_shader4 : enable

uniform mat4    MV;
uniform vec4    default_color;
uniform bool    per_vertex_color;

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;		// point color

out vec4 vOutcolor;

void main()
{
    gl_Position = MV * vec4(vtx_position, 1.0);

    if (per_vertex_color)
        vOutcolor = vec4(vtx_color, 1.0f);
    else
        vOutcolor = default_color;
}
