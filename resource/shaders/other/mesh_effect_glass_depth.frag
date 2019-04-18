#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

uniform float DepthScale;

in vec3 vNormal;
in vec3 vPosition;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 P = vPosition;
    vec3 I = normalize(P);
    float cosTheta = abs(dot(I, N));
    float fresnel = pow(1.0 - cosTheta, 4.0);

    float depth = DepthScale * gl_FragCoord.z;

    FragColor = vec4(depth, fresnel, 0, 0);
}