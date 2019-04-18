#version 150

in vec2 texCoord;

uniform sampler2D FrontBlenderTex;
uniform sampler2D BackBlenderTex;

out vec4 fragOutput;

void main(void)
{
	vec4 frontColor = texture(FrontBlenderTex, texCoord);
	vec3 backColor = texture(BackBlenderTex, texCoord).rgb;
	float alphaMultiplier = 1.0 - frontColor.w;

	// front + back
	fragOutput.rgb = frontColor.rgb + backColor * alphaMultiplier;

	// Liangliang: I don't want transparent images
	fragOutput.a = 1.0f;
}



