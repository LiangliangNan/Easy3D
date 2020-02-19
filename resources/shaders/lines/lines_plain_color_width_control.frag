#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform mat4 PROJ;

in  vec3 gPoint;
in  vec4 gOutColor;

out vec4 outputF;


void main()
{
    outputF = gOutColor;
    // compute the depth
    vec4 pos = PROJ * vec4(gPoint, 1.0);
    gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
}
