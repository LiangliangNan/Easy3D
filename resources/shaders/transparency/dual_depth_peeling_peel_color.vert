#version 150
precision highp float;

in  vec3 vtx_position;	// vertex position
in  vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;	// vertex color

uniform vec3 default_color;
uniform bool per_vertex_color;

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

// the data to be sent to the fragment shader
out Data{
	vec3 color;
	vec3 normal;
	vec3 position; 
} DataOut;


void main(void)
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

	if (per_vertex_color)
		DataOut.color = vtx_color;
	else
		DataOut.color = default_color;

	DataOut.normal = NORMAL * vtx_normal;
	DataOut.position = new_position.xyz;

	gl_Position = MVP * new_position;
}
