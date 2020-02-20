#version 330   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;// point position
in  vec3 vtx_color;// point color
in  vec3 vtx_normal;// point normal

uniform vec4    default_color;
uniform bool    per_vertex_color;

out VertexData
{
    vec4  color;
    vec3  normal;
} vertexOut;


void main()
{
    gl_Position = vec4(vtx_position, 1.0);

    if (per_vertex_color)
        vertexOut.color = vec4(vtx_color, 1.0f);
    else
        vertexOut.color = default_color;

    vertexOut.normal = vtx_normal;
}
