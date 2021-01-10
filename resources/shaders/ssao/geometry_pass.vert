#version 150

in vec3 vtx_position;
in vec3 vtx_normal;

uniform mat4 MV;
uniform mat3 invMV;
uniform mat4 PROJ;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out Data{
    vec3 position;
    vec3 normal;
} DataOut;


void main()
{
    vec4 new_position = MANIP * vec4(vtx_position, 1.0);

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

    vec4 viewPos = MV * new_position;
    DataOut.position = viewPos.xyz;

    DataOut.normal = invMV * NORMAL * vtx_normal;
    
    gl_Position = PROJ * viewPos;
}
