//#version 330 core   // for geometry shader to work
#version 430 core	  // for shader storage buffer
// please send comments or report bug to: liangliang.nan@gmail.com

//#extension GL_EXT_gpu_shader4 : enable

layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool clippingPlaneEnabled;
	bool crossSectionEnabled;
	vec4 clippingPlane0;
	vec4 clippingPlane1;
};

in vec3  vtx_position;
in vec3  vtx_color;
//in float sphere_radius;

uniform vec3	default_color;
uniform bool	per_vertex_color;
uniform int		hightlight_id;
uniform bool	selected;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;

out vec3	sphere_color_in;
//out float	sphere_radius_in;

void main()
{  
	sphere_color_in = default_color;
	if (per_vertex_color)
		sphere_color_in = vtx_color;
	
	//uint addr = gl_VertexID / 32;
	//uint offs = gl_VertexID % 32;
	uint addr = gl_VertexID >> 5;
	uint offs = gl_VertexID & 31;
	if ((selection.data[addr] & (1 << offs)) != 0)
		sphere_color_in = mix(sphere_color_in, vec3(1.0, 0.0, 0.0), 0.8);
	else if (gl_VertexID == hightlight_id)
		sphere_color_in = vec3(1.0, 0.0, 0.0);

	if (selected)
		sphere_color_in = vec3(1.0, 0.0, 0.0);

	//sphere_radius_in = sphere_radius;

	gl_Position = MANIP * vec4(vtx_position, 1.0);
}