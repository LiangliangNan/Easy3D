#version 330 core   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3  vtx_position;
in vec3  vtx_color;
//in float sphere_radius;

uniform vec4 default_color;
uniform bool per_vertex_color;

uniform mat4 MANIP = mat4(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out vec4    sphere_color_in;

void main()
{
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    if (clippingPlaneEnabled) {
        if (planeClippingDiscard && dot(new_position, clippingPlane0) < 0)
            return;
        if (crossSectionEnabled) {
            if (planeClippingDiscard && dot(new_position, clippingPlane1) < 0)
                return;
        }
    }

    gl_Position = new_position;

	if (per_vertex_color)
        sphere_color_in = vec4(vtx_color, 1.0);
    else
        sphere_color_in = default_color;
}
