#version 150

in Data {
    vec4 color;
    float clipped;
} DataIn;

out vec4 outputF;

uniform bool selected = false;

void main()
{
    if (DataIn.clipped > 0.0)
        discard;

    outputF = DataIn.color;
    if (selected)
        outputF = mix(outputF, vec4(1.0, 0.0, 0.0, 1.0), 0.6);
}
