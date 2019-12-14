#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec2		tcoordVC;
out		vec4		fragOutput;	// Ouput data

uniform sampler2D	textureID;


void main() {
	vec4 color = texture(textureID, tcoordVC);
	if (color.a <= 0)
		discard;

	fragOutput = color;
}