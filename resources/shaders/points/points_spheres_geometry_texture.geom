#version 330 core
// please send comments or report bug to: liangliang.nan@gmail.com

//#extension GL_EXT_geometry_shader4 : enable

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4    MV;
uniform mat4    PROJ;
uniform float	sphere_radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in		vec2	texcoord[];
//in float	sphere_radius_in;

out Data{
	flat	vec2	texcoord;
	smooth	vec2	tex;
	flat	vec4	position;
	//flat	float	sphere_radius;
} DataOut;


void main()
{
	// Output vertex position
	DataOut.position = MV * gl_in[0].gl_Position;
	DataOut.texcoord = texcoord[0];
	//sphere_radius = sphere_radius_in;

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
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
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
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
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
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
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
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	EndPrimitive();
}