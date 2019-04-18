#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

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

uniform float	sphere_radius;

out Data{
	smooth	vec2	tex;
	flat	vec4	position;
} DataOut;


void main()
{
	// Output vertex position
	DataOut.position = MV * gl_in[0].gl_Position;

	// Vertex 1
	DataOut.tex = vec2(-1.0, -1.0);
	gl_Position = DataOut.position;
	gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
	gl_Position = PROJ  * gl_Position;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
	}
	EmitVertex();

	// Vertex 2
	DataOut.tex = vec2(-1.0, 1.0);
	gl_Position = DataOut.position;
	gl_Position.xy += vec2(-sphere_radius, sphere_radius);
	gl_Position = PROJ  * gl_Position;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
	}
	EmitVertex();

	// Vertex 3
	DataOut.tex = vec2(1.0, -1.0);
	gl_Position = DataOut.position;
	gl_Position.xy += vec2(sphere_radius, -sphere_radius);
	gl_Position = PROJ  * gl_Position;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
	}
	EmitVertex();

	// Vertex 4
	DataOut.tex = vec2(1.0, 1.0);
	gl_Position = DataOut.position;
	gl_Position.xy += vec2(sphere_radius, sphere_radius);
	gl_Position = PROJ  * gl_Position;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
	}
	EmitVertex();

	EndPrimitive();
}