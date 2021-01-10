#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
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
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.position = vtx_position;
    DataOut.texcoord = vtx_texcoord;
    DataOut.normal = NORMAL * vtx_normal;

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (planeClippingDiscard && gl_ClipDistance[0] < 0)
        return;
        if (crossSectionEnabled) {
            gl_ClipDistance[1] = dot(new_position, clippingPlane1);
            if (planeClippingDiscard && gl_ClipDistance[1] < 0)
            return;
        }
    }

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * new_position;
}
