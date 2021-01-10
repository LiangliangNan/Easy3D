#version 150

in  vec4 vOutColor;
out vec4 outputF;

uniform bool selected = false;

void main()
{
    outputF = vOutColor;
    if (selected)
        outputF = mix(outputF, vec4(1.0, 0.0, 0.0, 1.0), 0.6);
}
