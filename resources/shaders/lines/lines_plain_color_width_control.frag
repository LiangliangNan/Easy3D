#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform mat4 PROJ;

in  vec3 gPoint;
in  vec4 gOutColor;

uniform bool selected = false;

out vec4 outputF;


void main()
{
    outputF = gOutColor;
    if (selected)
        outputF = mix(outputF, vec4(1.0, 0.0, 0.0, 1.0), 0.6);

    // compute the depth
    vec4 pos = PROJ * vec4(gPoint, 1.0);
    gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
}
