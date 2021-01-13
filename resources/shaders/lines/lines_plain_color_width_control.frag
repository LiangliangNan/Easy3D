#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform mat4 PROJ;

uniform bool selected = false;

in vec3  gOutPoint;
in vec4  gOutColor;
in float gOutClipped;

out vec4 outputF;

void main()
{
    if (gOutClipped > 0.0)
        discard;

    outputF = gOutColor;
    if (selected)
        outputF = mix(outputF, vec4(1.0, 0.0, 0.0, 1.0), 0.6);

    // compute the depth
    vec4 pos = PROJ * vec4(gOutPoint, 1.0);
    gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
}
