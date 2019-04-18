#version 150

in vec2 texCoord;

uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;
uniform vec3 BackgroundColor;

out vec4 fragOutput;

void main(void)
{
	vec4 SumColor = texture(ColorTex0, texCoord);
	float n = texture(ColorTex1, texCoord).r;

	if (n == 0.0) {
		fragOutput.rgb = BackgroundColor;
		// Liangliang: I don't want transparent images
		fragOutput.a = 1.0f;
		return;
	}

	vec3 AvgColor = SumColor.rgb / SumColor.a;
	float AvgAlpha = SumColor.a / n;

	float T = pow(1.0-AvgAlpha, n);
	fragOutput.rgb = AvgColor * (1 - T) + BackgroundColor * T;

	// Liangliang: I don't want transparent images
	fragOutput.a = 1.0f;
}
