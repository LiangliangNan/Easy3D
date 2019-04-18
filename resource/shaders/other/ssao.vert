#version 150


in	vec3	vertexMC;
in	vec2	tcoordMC;

out vec2 TexCoords;

void main()
{
    TexCoords = tcoordMC;
    gl_Position = vec4(vertexMC, 1.0);
}