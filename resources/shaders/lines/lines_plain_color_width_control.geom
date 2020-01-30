#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

//#extension GL_EXT_geometry_shader4 : enable

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;


uniform mat4    PROJ;
uniform bool    perspective;
uniform float   radius;

in  vec4 vOutcolor[];

out vec3 gPoint;
out vec4 gOutColor;


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
    gOutColor = vOutcolor[0];
    EmitVertex();

    // Vertex 2
    gPoint = (top - offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutColor = vOutcolor[1];
    EmitVertex();

    // Vertex 3
    gPoint = (base + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutColor = vOutcolor[0];
    EmitVertex();

    // Vertex 4
    gPoint = (top + offset);
    gl_Position = PROJ  * vec4(gPoint, 1.0);
    gOutColor = vOutcolor[1];
    EmitVertex();

    EndPrimitive();
}
