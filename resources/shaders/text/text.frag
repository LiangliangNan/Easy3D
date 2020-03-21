#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec2		tcoordVC;
out		vec4		fragOutput;	// Ouput data

uniform sampler2D	textureID;
uniform vec3        font_color;


void main() {
    float alpha = texture(textureID, tcoordVC).r;
    fragOutput = vec4(font_color, alpha);
}
