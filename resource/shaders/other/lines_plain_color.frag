#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vOutColor;

out vec4 outputF;

void main()
{
	outputF = vec4(vOutColor, 1.0);

	// the following line is required for fxaa (will not work with blending!)
	//outputF.a = dot(outputF.rgb, vec3(0.299, 0.587, 0.114));
}