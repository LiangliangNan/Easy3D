#version 150
precision highp float;

in	vec4 vtx_position;

uniform mat4 MVP;

void main(void)
{
     gl_Position = MVP * vtx_position;
}
