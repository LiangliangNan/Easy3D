#version 150

in	vec3	vertexMC;
in	vec2	tcoordMC;

out	vec2	tcoordVC;

void main()
{
	tcoordVC = tcoordMC;
	gl_Position = vec4(vertexMC, 1.0);
}
