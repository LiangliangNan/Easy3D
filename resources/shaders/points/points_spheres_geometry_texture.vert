#version 330 core   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3  vtx_position;
in vec2  vtx_texcoord;
//in float sphere_radius;

out		vec2	texcoord;
//out float	sphere_radius_in;

void main()
{  
	texcoord = vtx_texcoord;

	//sphere_radius_in = sphere_radius;

	gl_Position = vec4(vtx_position, 1.0);
}