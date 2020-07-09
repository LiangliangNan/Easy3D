#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

uniform mat4    MVP;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

// the data to be sent to the fragment shader
out Data{
    vec3 position;
    vec2 texcoord;
    vec3 normal;
} DataOut;


void main() {
    DataOut.position = vtx_position;
    DataOut.texcoord = vtx_texcoord;
    DataOut.normal = vtx_normal;

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * vec4(vtx_position, 1.0);

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(vec4(vtx_position, 1.0), clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(vec4(vtx_position, 1.0), clippingPlane1);
    }
}
