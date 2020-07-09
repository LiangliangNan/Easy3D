#version 150

in	vec2	ndc_position;
in	vec2	tex_coord;

uniform float   depth;

out	vec2	tcoordVC;

void main()
{
	tcoordVC = tex_coord;
	gl_Position = vec4(ndc_position, depth, 1.0);
}
