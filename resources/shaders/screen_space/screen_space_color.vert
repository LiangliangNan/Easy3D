#version 150

in	vec2	ndc_position; // NDC format

uniform float   depth;


void main()
{
  gl_Position = vec4(ndc_position, depth, 1.0);
}


