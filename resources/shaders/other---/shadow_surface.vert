#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;	// vertex position
in	vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;		// vertex color
in  vec2 vtx_texcoord;

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

uniform vec3 default_color;
uniform bool per_vertex_color;

// the data to be sent to the fragment shader
out Data{
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 texcoord;
	vec4 shadowCoord;
} DataOut;


void main() {
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
	DataOut.position = new_position.xyz;
	DataOut.normal = NORMAL * vtx_normal;
	DataOut.shadowCoord = SHADOW * new_position;

	DataOut.texcoord = vtx_texcoord;
	if (per_vertex_color)	
		DataOut.color = vtx_color;
	else
		DataOut.color = default_color;
	
	gl_Position = MVP * new_position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}