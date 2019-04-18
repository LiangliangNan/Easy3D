#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;		// point color
in	vec3 vtx_normal;	// point normal

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
out vec3 vsPosition;
out vec3 vsColor;
out vec3 vsNormal;

void main(void) {

	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	vsPosition = new_position.xyz;
	vsNormal = normalize(NORMAL * vtx_normal);

	vsColor = default_color;
	if (per_vertex_color)
		vsColor = vtx_color;

	gl_Position = MVP * new_position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}
