#version 330 core   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3  vtx_position;
in vec3  vtx_color;
//in float sphere_radius;

uniform vec4    default_color;
uniform bool    per_vertex_color;

out vec4    sphere_color_in;
//out float	sphere_radius_in;

void main()
{  
	if (per_vertex_color)
        sphere_color_in = vec4(vtx_color, 1.0);
    else
        sphere_color_in = default_color;

	//sphere_radius_in = sphere_radius;

    gl_Position = vec4(vtx_position, 1.0);
}
