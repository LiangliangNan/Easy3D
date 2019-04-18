#version 150

out vec4 fragOutput;

void main(void)
{
	fragOutput.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
}
