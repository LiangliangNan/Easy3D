#version 430 core
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
in vec2  vtx_texcoord;
//in float sphere_radius;
 
out		vec2	texcoord;
//out float	sphere_radius_in;

void main()
{  
	texcoord = vtx_texcoord;

	//sphere_radius_in = sphere_radius;

	gl_Position = MANIP * vec4(vtx_position, 1.0);
}