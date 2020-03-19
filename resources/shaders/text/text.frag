#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec2		tcoordVC;
out		vec4		fragOutput;	// Ouput data

uniform sampler2D	textureID;


void main() {
    float a = texture(textureID, tcoordVC).r;
    fragOutput = vec4(0, 0, 0, a);
}
