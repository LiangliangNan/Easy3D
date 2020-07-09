#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool clippingPlaneEnabled;
	bool crossSectionEnabled;
	vec4 clippingPlane0;
	vec4 clippingPlane1;
};

uniform bool	generateShadowMap;

uniform float	sphere_radius;

in Data{
	smooth	vec2	tex;
	flat	vec4	position;
} DataIn;


out vec4 FragColor;	// Ouput data

void main()
{
	if (generateShadowMap) {
		// always orthographic projection
		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
		float x = DataIn.tex.x;
		float y = DataIn.tex.y;
		float zz = 1.0 - x*x - y*y;

		if (zz <= 0.0)
			discard;

		float z = sqrt(zz);
		vec4 pos = DataIn.position;
		pos.z += sphere_radius*z;

		// compute the depth. I can do it easier
		//vec4 pos4 = PROJ * vec4(pos, 1.0);
		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
		float depth = 0.5 * clipZW.x / clipZW.y + 0.5;

		gl_FragDepth = depth;
		FragColor = vec4(depth, 0, 0, 1.0);
	}
	else {
	}
}
