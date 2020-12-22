#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);

in  vec3 vtx_position;	// point position
in  vec2 vtx_texcoord;

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out vec2 vOutTexcoord;

void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (planeClippingDiscard && gl_ClipDistance[0] < 0)
		return;
		if (crossSectionEnabled) {
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
			if (planeClippingDiscard && gl_ClipDistance[1] < 0)
			return;
		}
	}

	vOutTexcoord = vtx_texcoord;

	gl_Position = MVP * new_position;
}