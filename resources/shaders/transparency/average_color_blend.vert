#version 150


in vec4 vtx_position;
in vec2 vtx_texcoord;

uniform mat4 MANIP = mat4(1.0);
uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out vec2 texCoord;

void main()
{
	vec4 new_position = MANIP * vtx_position;

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

	gl_Position = new_position;
	texCoord = vtx_texcoord;
}
