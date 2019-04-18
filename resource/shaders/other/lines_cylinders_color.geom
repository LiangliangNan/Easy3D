#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

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

uniform bool	perspective;
uniform float	radius;

in  vec3 vOutcolor[];

out vec3 gOutColor;

out Data{
	vec3 point;// camera space
	vec3 axis;
	vec3 base;
	vec3 end;
	vec3 U;
	vec3 V;
} DataOut;



void main()
{
	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
	vec3  len_dir = top - base;
	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
	if (perspective) {
		//view_dir = normalize(vec3(0) - 0.5*(base + top));	// camera pos is (0, 0, 0) in camera space
		view_dir = normalize(vec3(0) - base);	// camera pos is (0, 0, 0) in camera space
	}

	vec3 base_color = vOutcolor[0];
	vec3 top_color = vOutcolor[1];

	float b = dot(view_dir, len_dir);
	if (b < 0.0) {// direction vector looks away, so flip
		len_dir = -len_dir;
		//swap(base, top);
		vec3 tmp = base;
		base = top;
		top = tmp;
		
		base_color = vOutcolor[1];
		top_color = vOutcolor[0];
	}

	vec3 left = cross(view_dir, len_dir);
	vec3 up = cross(left, len_dir);
	DataOut.axis = normalize(len_dir);
	DataOut.U = normalize(up);
	DataOut.V = normalize(left);
	left = radius * DataOut.V;
	up = radius * DataOut.U;

	DataOut.base = base;
	DataOut.end = top;

	// Vertex 1
	DataOut.point = (base + left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = base_color;
	EmitVertex();

	// Vertex 2
	DataOut.point = (base - left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = base_color;
	EmitVertex();

	// Vertex 3
	DataOut.point = (top + left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = top_color;
	EmitVertex();

	// Vertex 4
	DataOut.point = (top - left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = top_color;
	EmitVertex();

	// Vertex 5
	DataOut.point = (top + left + up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = top_color;
	EmitVertex();

	// Vertex 6
	DataOut.point = (top - left + up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	gOutColor = top_color;
	EmitVertex();

	EndPrimitive();
}