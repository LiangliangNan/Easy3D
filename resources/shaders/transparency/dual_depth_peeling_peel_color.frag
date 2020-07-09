#version 150
precision highp float;

#define MAX_DEPTH 1.0

// the output of this shader
out vec4 fragOutput0;
out vec4 fragOutput1;
out vec4 fragOutput2;

uniform sampler2D DepthBlenderTex;
uniform sampler2D FrontBlenderTex;

uniform float	Alpha;

uniform vec3	wLightPos;
uniform vec3	wCamPos;

layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool    smooth_shading = true;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
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
	vec3 color = DataIn.color;

        vec3 normal;
        if (smooth_shading)
            normal = normalize(DataIn.normal);
        else {
            normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
            if (dot(normal, DataIn.normal) < 0)
                normal = -normal;
        }

        vec3 view_dir = normalize(wCamPos - DataIn.position);
        vec3 light_dir = normalize(wLightPos);

        // diffuse factor
        float df = abs(dot(light_dir, normal));			// light up both sides

        // specular factor
        vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
        float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
        sf = pow(sf, shininess);

        return vec4(color * df + specular * sf + ambient, Alpha);
}


void main(void)
{
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
	//float epsilon = 0.000001; // VTK uses this value
	float epsilon = 0.0000001;

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
