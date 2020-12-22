#version 330 core   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

uniform mat4 MANIP = mat4(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in vec3  vtx_position;
in vec2  vtx_texcoord;

out vec2 texcoord;

void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	if (clippingPlaneEnabled) {
		if (planeClippingDiscard && dot(new_position, clippingPlane0) < 0)
			return;
		if (crossSectionEnabled) {
			if (planeClippingDiscard && dot(new_position, clippingPlane1) < 0)
				return;
		}
	}

	texcoord = vtx_texcoord;

	gl_Position = new_position;
}