#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec3 vtx_normal;

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

// the data to be sent to the fragment shader
out Data{
	vec3 normal;
	vec3 position;
} DataOut;


void main()
{
	DataOut.normal = vtx_normal;
	DataOut.position = vec4(vtx_position, 1.0).xyz;

	gl_Position = MVP * vec4(vtx_position, 1.0);
}