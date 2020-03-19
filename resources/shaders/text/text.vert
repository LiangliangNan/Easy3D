#version 150

in	vec3	vtx_position;
in	vec2	tex_coord;

out	vec2	tcoordVC;

void main()
{
	tcoordVC = tex_coord;
	gl_Position = vec4(vtx_position, 1.0);
}
