#version 150

in	vec3	vertexMC;
in  vec3    vertexColor;

uniform vec3	default_color = vec3(0.0f, 0.0f, 0.0f);
uniform bool	per_vertex_color = false;


out vec3 vOutColor;

void main()
{
	if (per_vertex_color)
		vOutColor = vertexColor;
	else
		vOutColor = default_color;

	gl_Position = vec4(vertexMC, 1.0);
}
