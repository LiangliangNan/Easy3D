#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

uniform mat4	MVP;
uniform mat4    MANIP = mat4(1.0);

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;


out Data{
    vec2 texcoord;
    vec3 normal;
    vec3 position;
} DataOut;

void main() {

    DataOut.texcoord = vtx_texcoord;
    DataOut.normal = vtx_normal;

    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.position = new_position.xyz;

    gl_Position = MVP * new_position;

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (crossSectionEnabled)
        gl_ClipDistance[1] = dot(new_position, clippingPlane1);
    }
}
