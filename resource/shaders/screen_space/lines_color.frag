#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in		vec3		vOutColor;
out		vec4		outputF;

void main() {
	outputF = vec4(vOutColor, 1.0f);
}