/********************************************************************
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
 ********************************************************************/

#version 150
precision highp float;

#define MAX_DEPTH 1.0

// the output of this shader
out vec4 fragOutput0;
out vec4 fragOutput1;
out vec4 fragOutput2;

uniform sampler2D DepthBlenderTex;
uniform sampler2D FrontBlenderTex;

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

uniform int  highlight_id_min;
uniform int  highlight_id_max;

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


// Recent OSX/ATI drivers perform some out-of-order execution that's causing
// the dFdx/dFdy calls to be conditionally executed. Specifically, it looks
// like the early returns when the depth is not on a current peel layer are 
// moved before the dFdx/dFdy calls used to compute normals. Disable the 
// early returns on apple for now, I don't think most GPUs really benefit 
// from them anyway at this point.
#ifdef __APPLE__
#define NO_PRECOLOR_EARLY_RETURN
#endif

vec4 ShadeFragment()
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
#if 0	// Workaround for Adreno GPUs not able to do dFdx( vViewPosition )
		vec3 fdx = vec3( dFdx( DataIn.position.x ), dFdx( DataIn.position.y ), dFdx( DataIn.position.z ) );
		vec3 fdy = vec3( dFdy( DataIn.position.x ), dFdy( DataIn.position.y ), dFdy( DataIn.position.z ) );
		normal = normalize(cross(fdx, fdy));
#else
		normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
#endif
//		// Instead of using the vertex normal to verify the normal direction, we can use gl_FrontFacing.
//		if (dot(normal, DataIn.normal) < 0)
		if ((gl_FrontFacing == false) && (two_sides_lighting == false))
			normal = -normal;
	}

	vec3 view_dir = normalize(wCamPos - DataIn.position);
	vec3 light_dir = normalize(wLightPos);

	float df = 0.0;	// diffuse factor
	if (two_sides_lighting)
		df = abs(dot(light_dir, normal));
	else
		df = max(dot(light_dir, normal), 0);

	// specular factor
	float sf = 0.0;	// specular factor
	if (df > 0.0) {	// if the vertex is lit compute the specular color
		vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector

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
	if (DataIn.clipped > 0.0)
		discard;

	// Set gl_FragDepth here (gl_FragCoord.z by default)
	// window-space depth interpolated linearly in screen space
	gl_FragDepth = gl_FragCoord.z;

	// Liangliang: gl_FragCoord contains the window-relative coordinates (non-normalized) 
	//			   of the current fragment, so texture() cannot be used. Direct accessing
	//			   of a texel value with non-normalized coordinates can be done through 
	//             the texelFetch() function. See https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
	ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
	vec2 depthBlender = texelFetch(DepthBlenderTex, pixelCoord, 0).xy;
	vec4 forwardTemp = texelFetch(FrontBlenderTex, pixelCoord, 0);
	
	// Depths and 1.0-alphaMult always increase
	// so we can use pass-through by default with MAX blending
	fragOutput0.xy = vec2(-MAX_DEPTH);
	
	// Front colors always increase (DST += SRC*ALPHA_MULT)
	// so we can use pass-through by default with MAX blending
	fragOutput1 = forwardTemp;
	
	// Because over blending makes color increase or decrease,
	// we cannot pass-through by default.
	// Each pass, only one fragment writes a color greater than 0
	fragOutput2 = vec4(0.0);

	float nearestDepth = -depthBlender.x;
	float farthestDepth = depthBlender.y;

	// the .0000001 below is an epsilon.  It turns out that
	// graphics cards can render the same polygon two times
	// in a row with different z values. I suspect it has to
	// do with how rasterization of the polygon is broken up.
	// A different breakup across fragment shaders can result in
	// very slightly different z values for some of the pixels.
	// The end result is that with depth peeling, you can end up
	// counting/accumulating pixels of the same surface twice
	// simply due to this randomness in z values. So we introduce
	// an epsilon into the transparent test to require some
	// minimal z seperation between pixels. This value was determined
	// through trial-and-error -- it may need to be increased at
	// some point. See also the comment in vtkDepthPeelingPass's shader.
//	float epsilon = 0.000001; // VTK uses this value
	float epsilon = 0.00001;

	// Is this fragment outside the current peels?
	if (gl_FragDepth < nearestDepth - epsilon || gl_FragDepth > farthestDepth + epsilon) {
#ifndef NO_PRECOLOR_EARLY_RETURN
		return;
#endif
	}

	// Is this fragment inside the current peels?
	if (gl_FragDepth > nearestDepth + epsilon && gl_FragDepth < farthestDepth - epsilon) {
		// Write out depth so this frag will be peeled later:
		fragOutput0.xy = vec2(-gl_FragDepth, gl_FragDepth);

#ifndef NO_PRECOLOR_EARLY_RETURN
		return;
#endif
	}
	
	// If we made it here, this fragment is on the peeled layer from last pass
	// therefore, we need to shade it, and make sure it is not peeled any farther
	vec4 color = ShadeFragment();
	fragOutput0.xy = vec2(-MAX_DEPTH);
	
	// Check if this fragment is on a current peel:
	if (gl_FragDepth >= nearestDepth - epsilon && gl_FragDepth <= nearestDepth + epsilon)
	{ // Front peel:
		// We store the front alpha value as (1-alpha) to allow MAX
		// blending. This also means it is really initialized to 1,
		// as it should be for under-blending.
		float alphaMultiplier = 1.0 - forwardTemp.w;

		// Use under-blending to combine fragment with front color:
		fragOutput1.xyz += color.rgb * color.a * alphaMultiplier;

		// Write out (1-alpha):
		fragOutput1.w = 1.0 - alphaMultiplier * (1.0 - color.a);
	} 
	else  // Back peel:
	{	
		// Dump premultiplied fragment, it will be blended later:
		fragOutput2 += color;
	}
}
