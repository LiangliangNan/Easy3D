#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
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
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} DataOut;


void main() {

	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	DataOut.texcoord = vtx_texcoord;
	DataOut.normal = NORMAL * vtx_normal;

	// Output position of the vertex, in clip space : MVP * position
	gl_Position = MVP * new_position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}
