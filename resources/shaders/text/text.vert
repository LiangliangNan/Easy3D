#version 150

in	vec4	coords;
out	vec2	tcoordVC;

void main()
{
	tcoordVC = coords.zw;
	gl_Position = vec4(coords.xy, 0.0, 1.0);
}
