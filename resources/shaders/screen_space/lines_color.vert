#version 150

in  vec3    vertexMC;
in  vec3    vertexColor;

uniform vec4	default_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
uniform bool	per_vertex_color = false;


out vec4 vOutColor;

void main()
{
	if (per_vertex_color)
		vOutColor = vec4(vertexColor, 1.0f);
	else
		vOutColor = default_color;

	gl_Position = vec4(vertexMC, 1.0);
}
