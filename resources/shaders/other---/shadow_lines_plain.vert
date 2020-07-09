#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

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

in  vec3 vtx_position;	// point position

void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	gl_Position = MVP * new_position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}