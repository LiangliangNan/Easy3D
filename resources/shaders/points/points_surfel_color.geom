#version 330   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

// we accept points and render quads
layout (points) in;
layout (triangle_strip, max_vertices=4) out;

uniform mat4    MVP;
uniform float   radius;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;


in VertexData {
    vec4  color;
    vec3  normal;
} VertexIn[];

out FragmentData {
    vec2 texcoord;
    vec4 color;
    vec3 point;
    vec3 normal;
} VertexOut;


void main()
{
    vec4 center = gl_in[0].gl_Position;
    vec3 normal = VertexIn[0].normal;

    // create tangent space. Helper vectors dependent on major orientation of normal
    vec3 u, v;
    if (abs(normal.y) > abs(normal.x)) {
        v = cross(normal, vec3(1.0, 0.0, 0.0));
        u = cross(normal, v);
    }
    else {
        v = cross(vec3(0.0, 1.0, 0.0), normal);
        u = cross(normal, v);
    }

    // Scale the splat
    u *= radius;
    v *= radius;

    // Calculate the four corner vertices of the quad
    vec4 a = center + vec4(-u-v, 0.0);
    vec4 b = center + vec4(-u+v, 0.0);
    vec4 c = center + vec4(+u+v, 0.0);
    vec4 d = center + vec4(+u-v, 0.0);

    VertexOut.point = center.xyz;
    VertexOut.normal = normal;
    VertexOut.color = VertexIn[0].color;

    // transform the four points. Note the order of output
    gl_Position = MVP * b;
    VertexOut.texcoord = vec2(-1.0, 1.0);
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

    gl_Position = MVP * a;
    VertexOut.texcoord = vec2(-1.0, -1.0);
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

    gl_Position = MVP * c;
    VertexOut.texcoord = vec2(1.0, 1.0);
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

    gl_Position = MVP * d;
    VertexOut.texcoord = vec2(1.0, -1.0);
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
