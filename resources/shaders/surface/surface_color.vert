#version 150

in  vec3 vtx_position;
in  vec3 vtx_normal;
in  vec3 vtx_color;

uniform mat4	MVP;
uniform mat4    MANIP = mat4(1.0);
uniform vec4	default_color = vec4(0.4f, 0.8f, 0.8f, 1.0f);
uniform bool	per_vertex_color = false;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;


out Data{
    vec4 color;
    vec3 position;
    vec3 normal;
} DataOut;

void main() {
    if (per_vertex_color)
        DataOut.color = vec4(vtx_color, 1.0);
    else
        DataOut.color = default_color;

    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.position = new_position.xyz;
    DataOut.normal = vtx_normal;
    gl_Position = MVP * new_position;

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(new_position, clippingPlane1);
    }
}
