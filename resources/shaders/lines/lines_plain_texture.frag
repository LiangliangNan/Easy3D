#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

uniform sampler2D	textureID;

in  vec2 vOutTexcoord;

out vec4 outputF;

void main()
{
	outputF = texture(textureID, vOutTexcoord);

	// the following line is required for fxaa (will not work with blending!)
	//outputF.a = dot(outputF.rgb, vec3(0.299, 0.587, 0.114));
}