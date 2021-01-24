#version 150

in  vec3 vtx_position;	// vertex position

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;


out float clipped;

void main() {
        vec4 new_position = MANIP * vec4(vtx_position, 1.0);

        clipped = 0.0;
        if (clippingPlaneEnabled) {
                gl_ClipDistance[0] = dot(new_position, clippingPlane0);
                if (planeClippingDiscard && gl_ClipDistance[0] < 0)
                        clipped = 1.0;
                if (crossSectionEnabled) {
                        gl_ClipDistance[1] = dot(new_position, clippingPlane1);
                        if (planeClippingDiscard && gl_ClipDistance[1] < 0)
                                clipped = 1.0;
                }
        }

        gl_Position = MVP * new_position;
}
