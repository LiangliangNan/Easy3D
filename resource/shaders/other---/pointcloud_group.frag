#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in	vec3 vsColor;
out vec4 outputF;


void main(void) {
	outputF = vec4(vsColor, 1.0f);
}