#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec2		tcoordVC;
out		vec4		fragOutput;	// Ouput data

uniform sampler2D	textureID;
uniform vec3        font_color;


void main() {
    float a = texture(textureID, tcoordVC).r;
    if (a == 0)
        discard;
    fragOutput = vec4(font_color, a);
}
