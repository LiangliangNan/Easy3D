/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */

#version 150
precision highp float;

out vec4 fragOutput;

uniform sampler2D PreviousDepthTex;

#ifndef USE_PROVIDED_NORMALS
uniform bool	smooth_shading;
#endif // !USE_PROVIDED_NORMALS

uniform float	Alpha;

uniform vec3 wLightPos;
uniform vec3 wCamPos;
layout(std140) uniform Material {
	vec3	ambient;
	vec3	specular;
	float	shininess;
};

uniform bool use_texture = false;
uniform sampler2D textureID;
uniform float texture_repeat = 1.0f;
uniform float fractional_repeat = 0.0f;
//#define ENABLE_ALPHA

uniform bool        lighting = true;
// two sides
uniform bool        two_sides_lighting = true;
// backside color
uniform bool        distinct_back_color = true;
uniform vec3        backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool selected = false;
uniform vec4 	highlight_color;

in Data{
	vec2 texcoord;
	vec3 color;
	vec3 normal;
	vec3 position;
	float clipped;
} DataIn;


// Derivatives must be evaluated before the previous-depth discard.
vec4 ShadeFragment(vec3 flat_normal)
{
	vec4 color;
	if (use_texture) {
		float repeat = texture_repeat + fractional_repeat / 100.0f;
		color = texture(textureID, DataIn.texcoord * repeat);

#ifndef ENABLE_ALPHA
		color.a = 1.0f;
#else
		if (color.a <= 0)
			discard;
#endif
	}
	else {
		color = vec4(DataIn.color, 1.0f);
	}

	if ((gl_FrontFacing == false) && distinct_back_color)
		color = vec4(backside_color, color.a);

	if (selected)
		color = mix(color, highlight_color, 0.6);

	if (!lighting) {
		return vec4(color.xyz, Alpha);
	}

	vec3 normal;
	if (smooth_shading)
		normal = normalize(DataIn.normal);
	else {
		normal = flat_normal;
		if ((gl_FrontFacing == false) && (two_sides_lighting == false))
			normal = -normal;
	}

	vec3 view_dir = normalize(wCamPos - DataIn.position);
	vec3 light_dir = normalize(wLightPos);

	float df = 0.0;	// diffuse factor
	if (two_sides_lighting)
		df = abs(dot(light_dir, normal));
	else
		df = max(dot(light_dir, normal), 0.0);

	// specular factor
	float sf = 0.0;	// specular factor
	if (df > 0.0) {
		vec3 half_vector = normalize(light_dir + view_dir);
		if (two_sides_lighting)
			sf = abs(dot(half_vector, normal));
		else
			sf = max(dot(half_vector, normal), 0.0);
		sf = pow(sf, shininess);
	}

	return vec4(color.xyz * df + specular * sf + ambient, Alpha);
}


void main(void)
{
	vec3 flat_normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
	if (DataIn.clipped > 0.0)
		discard;

	// Reject the previous layer before shading; the depth write keeps late testing
	// so rejected nearer fragments cannot replace this layer's nearest survivor.
	ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
	float previousDepth = texelFetch(PreviousDepthTex, pixelCoord, 0).r;
	if (gl_FragCoord.z <= previousDepth)
		discard;

	// Fully transparent fragments must not occlude a farther layer.
	vec4 color = ShadeFragment(flat_normal);
	float alpha = clamp(Alpha, 0.0, 1.0);
	if (alpha == 0.0)
		discard;
	gl_FragDepth = gl_FragCoord.z;
	fragOutput = vec4(color.rgb * alpha, alpha);
}
