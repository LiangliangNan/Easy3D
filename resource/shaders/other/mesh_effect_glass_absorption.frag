#version 150
// please send comments or report bug to: liangliang.nan@gmail.com


uniform sampler2D textureID;
uniform vec3 color;

in vec2 tcoordVC;

out vec4 FragColor;

void main()
{
	float thickness = abs(texture2D(textureID, tcoordVC).r);
    if (thickness <= 0.0)
        discard;

    float sigma = 10.0;
	float fresnel = 1.0 - texture2D(textureID, tcoordVC).g;
    float intensity = fresnel * exp(-sigma * thickness);
	FragColor = vec4(intensity * color, 1.0);
}