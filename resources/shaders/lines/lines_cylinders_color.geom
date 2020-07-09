#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;


uniform mat4    PROJ;
uniform mat4    invMV;
uniform bool	perspective;
uniform float	radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in  vec4 vOutcolor[];

out vec4 gOutColor;

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
    vec3  base = gl_in[0].gl_Position.xyz;// in camera space
    vec3  top = gl_in[1].gl_Position.xyz;// in camera space
    vec3  len_dir = top - base;
    vec3  view_dir = vec3(0, 0, 1);// this is view direction for orthographic mode in camera space
    if (perspective) {
        //view_dir = normalize(vec3(0) - 0.5*(base + top));	// camera pos is (0, 0, 0) in camera space
        view_dir = normalize(vec3(0) - base);// camera pos is (0, 0, 0) in camera space
    }

    vec4 base_color = vOutcolor[0];
    vec4 top_color = vOutcolor[1];

    float b = dot(view_dir, len_dir);
    if (b < 0.0) { // direction vector looks away, so flip
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
	gOutColor = base_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	// Vertex 2
	DataOut.point = (base - left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	gOutColor = base_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	// Vertex 3
	DataOut.point = (top + left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	gOutColor = top_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	// Vertex 4
	DataOut.point = (top - left - up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	gOutColor = top_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	// Vertex 5
	DataOut.point = (top + left + up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	gOutColor = top_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	// Vertex 6
	DataOut.point = (top - left + up);
	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
	gOutColor = top_color;
	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
		if (crossSectionEnabled)
		gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
	}
	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
	// of gl_PrimitiveID in the fragment shader is undefined.
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();

	EndPrimitive();
}
