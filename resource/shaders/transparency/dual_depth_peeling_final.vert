#version 150

in vec4 vtx_position;
in vec2 vtx_texcoord;

out vec2 texCoord;

void main()
{
	gl_Position = vtx_position;
	texCoord = vtx_texcoord;
}
