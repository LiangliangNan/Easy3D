#version 150
precision highp float;

in  float clipped;
out vec4 fragOutput;

void main(void)
{
	if (clipped > 0.0)
		discard;
	fragOutput.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
}
