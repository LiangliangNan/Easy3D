#version 150
precision highp float;

uniform sampler2D TempTex;

in vec2 texCoord;

out vec4 fragOutput;

void main(void)
{	
	fragOutput = texture(TempTex, texCoord, 0);

	// for occlusion query
	if (fragOutput.a == 0) 
		discard;
}
