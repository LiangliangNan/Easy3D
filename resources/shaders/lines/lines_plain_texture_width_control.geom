#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;


uniform mat4    PROJ;
uniform mat4    invMV;
uniform bool    perspective;
uniform float   radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

in  vec2 vOutTexcoord[];

out vec3 gPoint;
out vec2 gOutTexcoord;


void main()
{
    vec3  base = gl_in[0].gl_Position.xyz;// in camera space
    vec3  top = gl_in[1].gl_Position.xyz;// in camera space
    vec3  axes = top - base;
    vec3  view_dir = vec3(0, 0, 1);// this is view direction for orthographic mode in camera space
    if (perspective)
        view_dir = normalize(vec3(0) - 0.5*(base + top));// camera pos is (0, 0, 0) in camera space

    vec3 offset = cross(view_dir, axes);
    offset = radius * normalize(offset) * 0.5;

    // Vertex 1
    gPoint = (base - offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[0];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 2
    gPoint = (top - offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[1];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 3
    gPoint = (base + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[0];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    // Vertex 4
    gPoint = (top + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutTexcoord = vOutTexcoord[1];
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(invMV * vec4(gPoint, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(invMV *vec4(gPoint, 1.0), clippingPlane1);
    }
    // In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in
    // the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in
    // the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value
    // of gl_PrimitiveID in the fragment shader is undefined.
    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex();

    EndPrimitive();
}
