#version 150

in  vec3 vtx_position;// point position
in  vec3 vtx_color;// point color
in  vec3 vtx_normal;// point normal

uniform vec4 default_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
uniform bool per_vertex_color = false;

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out Data{
    vec4 color;
    vec3 position;
    vec3 normal;
} DataOut;


void main(void) {
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.position = vtx_position;
    DataOut.normal = NORMAL * vtx_normal;

    if (per_vertex_color)
        DataOut.color = vec4(vtx_color, 1.0);
    else
        DataOut.color = default_color;

    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (crossSectionEnabled)
            gl_ClipDistance[1] = dot(new_position, clippingPlane1);
    }

    gl_Position = MVP * new_position;
}
