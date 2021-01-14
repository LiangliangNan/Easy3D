#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

uniform sampler2D	textureID;
uniform bool selected = false;

in vec2  vOutTexcoord;
in float vOutClipped;

out vec4 outputF;

void main()
{
	if (vOutClipped > 0.0)
		discard;

	outputF = texture(textureID, vOutTexcoord);
	if (selected)
		outputF = mix(outputF, vec4(1.0, 0.0, 0.0, 1.0), 0.6);

	// the following line is required for fxaa (will not work with blending!)
	//outputF.a = dot(outputF.rgb, vec3(0.299, 0.587, 0.114));
}