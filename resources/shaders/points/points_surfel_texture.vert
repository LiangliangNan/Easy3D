#version 330   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;  // point position
in  vec2 vtx_texcoord;  // texture coordinate
in  vec3 vtx_normal;    // point normal

out VertexData
{
    vec2  texcoord;
    vec3  normal;
} vertexOut;


void main()
{
    gl_Position = vec4(vtx_position, 1.0);

    vertexOut.texcoord = vtx_texcoord;
    vertexOut.normal = vtx_normal;
}
