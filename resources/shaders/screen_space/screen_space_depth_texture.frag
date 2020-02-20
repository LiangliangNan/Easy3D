#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec2		tcoordVC;
out		vec4		fragOutput;	// Ouput data

uniform sampler2D	textureID;


void main() {
    vec4 c = texture(textureID, tcoordVC);
    fragOutput = vec4(c.r, c.r, c.r, c.a);
}
