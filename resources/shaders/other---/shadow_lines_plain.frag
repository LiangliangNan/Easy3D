#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

uniform bool	generateShadowMap;

out vec4 FragColor;	// Ouput data

void main()
{
	if (generateShadowMap) {
		// write the depth into the RED component
		FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
	}
	else {
		// not needed, just supress the "generateShadowMap not active" warning
		discard; 
	}
}