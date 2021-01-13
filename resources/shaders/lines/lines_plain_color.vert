#version 150

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;	// point color

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);

uniform vec4 default_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
uniform bool per_vertex_color = false;

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out Data{
    vec4 color;
    float clipped;
} DataOut;

void main()
{
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    DataOut.clipped = 0.0;
    if (clippingPlaneEnabled) {
        gl_ClipDistance[0] = dot(new_position, clippingPlane0);
        if (planeClippingDiscard && gl_ClipDistance[0] < 0)
        DataOut.clipped = 1.0;
        if (crossSectionEnabled) {
            gl_ClipDistance[1] = dot(new_position, clippingPlane1);
            if (planeClippingDiscard && gl_ClipDistance[1] < 0)
            DataOut.clipped = 1.0;
        }
    }

    if (per_vertex_color)
        DataOut.color = vec4(vtx_color, 1.0);
    else
        DataOut.color = default_color;

    gl_Position = MVP * new_position;
}
