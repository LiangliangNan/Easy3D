#version 430
// please send comments or report bug to: liangliang.nan@gmail.com


layout(std140) uniform Lighting {
	vec3	wLightPos;
	vec3	eLightPos;
	vec3	wCamPos;
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


uniform bool lighting;	// true if lighting is on
uniform bool two_sides_lighting;

uniform int	 hightlight_id;
uniform bool selected;

uniform sampler2D	textureID;

///-----------------------------------------

uniform bool generateShadowMap;


#define USE_RandomSampling

// Liangliang: Random Sampling dosn't produce very good shadow effect on the model, 
//			   but OK on ground plane. So I use a smaller blur radius for models.
#define MODEL_BLUR_RADIUS_FACTOR  0.5  

uniform sampler2DShadow  ShadowMap;

uniform bool selfShadow;

uniform bool isGroundPlane;
uniform vec3 backgroundColor;

uniform sampler3D	OffsetTex;
uniform vec3		OffsetTexSize; // (width, height, depth)
uniform int			BlurRadius;

///-----------------------------------------

in Data{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec4 shadowCoord;
} DataIn;

out vec4 FragColor;	// Ouput data


// We only shade if the current fragment¡¯s depth (in light space) is really less (by a threshold) 
// than lightmap value. But this has the 'Peter Panning' problem, see 
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
#define EPSILON 0.01


float ShadowFactor(vec4 LightSpacePos, float threshold, int blur_radius)
{
	// transforming from clip space to NDC space
	vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w; 
	// Liangliang: To avoid sample values out of the light's view frustum.
	if ((ProjCoords.x <= -1 || ProjCoords.x >= 1) || (ProjCoords.y <= -1 || ProjCoords.y >= 1) || (ProjCoords.z <= -1 || ProjCoords.z >= 1))
		return 1.0;
	
	// transforming X from NDC space to texture space
	ProjCoords.x = 0.5 * ProjCoords.x + 0.5;	
	ProjCoords.y = 0.5 * ProjCoords.y + 0.5;
	ProjCoords.z = 0.5 * ProjCoords.z + 0.5 - threshold;

#ifdef USE_RandomSampling

	ivec2 ShadowMapSize = textureSize(ShadowMap, 0);
	float radius = float(blur_radius) / ShadowMapSize.x;

	float Factor = 0.0;

	ivec3 offsetCoord;
	offsetCoord.xy = ivec2(mod(gl_FragCoord.xy, OffsetTexSize.xy));

	int samplesDiv2 = int(OffsetTexSize.z);
	vec4 sc = vec4(ProjCoords, LightSpacePos.w);

	for (int i = 0; i < 4; i++) {
		offsetCoord.z = i;
		vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * radius * LightSpacePos.w;
		sc.xy = ProjCoords.xy + offsets.xy;
		Factor += textureProj(ShadowMap, sc);
		sc.xy = ProjCoords.xy + offsets.zw;
		Factor += textureProj(ShadowMap, sc);
	}
	float shadow = Factor / 8.0;

	// The first loop tests the first eight samples. If the first eight samples are all 0.0 or 
	// 1.0, then we assume that all of the samples will be the same (the sample area is completely 
	// in or out of the shadow). In that case, we skip the evaluation of the rest of the samples. 
	// Otherwise, we evaluate the following samples and compute the overall average.
	if (shadow != 1.0 && shadow != 0.0) {
		for (int i = 4; i < samplesDiv2; i++) {
			offsetCoord.z = i;
			vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * radius * LightSpacePos.w;
			sc.xy = ProjCoords.xy + offsets.xy;
			Factor += textureProj(ShadowMap, sc);
			sc.xy = ProjCoords.xy + offsets.zw;
			Factor += textureProj(ShadowMap, sc);
		}
		shadow = Factor / float(samplesDiv2 * 2.0);
	}
	//return shadow;			  // in [0, 1], too dark   
	return 0.5 + shadow / 2.0;	  // in [0.5, 1.0], not too dark

#else // Standard PCF

	float Factor = 0.0;
	float sum = (blur_radius * 2 + 1) * (blur_radius * 2 + 1);
	for (int y = -blur_radius; y <= blur_radius; y++) {
		for (int x = -blur_radius; x <= blur_radius; x++) {
			Factor += textureProjOffset(ShadowMap, vec4(ProjCoords, LightSpacePos.w), ivec2(x, y));
		}
	}
	//return (Factor / sum);			// in [0, 1], too dark   
	return 0.5 + Factor / (sum * 2.0);  // in [0.5, 1.0], not too dark

#endif
}


void main()
{
	if (generateShadowMap) {
	}
	else {
		vec3 color = texture(textureID, DataIn.texcoord).rgb;

		vec3 normal = normalize(DataIn.normal);
		vec3 light_dir = normalize(wLightPos);

		//------------------------------------------------------------
		// compupte the scalable bias according to the slope
		float cosTheta = clamp(dot(normal, light_dir), 0.0, 1.0);
		float threshold = EPSILON * tan(acos(cosTheta));
		threshold = clamp(threshold, 0.0, EPSILON * 2.0);
		//------------------------------------------------------------

		if (isGroundPlane) {
			float shadow = ShadowFactor(DataIn.shadowCoord, threshold, BlurRadius);
			if (shadow < 0.9999999) { // the background is in shadow
				// Consider ambient as a portion of the backgroundColor. Otherwise an 
				// undersired border may appear in penumbra when changing ambient.
				//FragColor = vec4(backgroundColor * shadow + ambient, 1.0);
				float factor = clamp(shadow + ambient.x, 0, 1);
				FragColor = vec4(backgroundColor * factor, 1.0);
			}
			else { // the background is lit
				discard;
				//FragColor = vec4(vec3(0.9, 0.9, 0.9) * shadow, 1.0);
			}
		}
		else {
			if (lighting) {
				float df = 0.0;	// diffuse factor
				if (two_sides_lighting)
					df = abs(dot(light_dir, normal));
				else
					df = max(dot(light_dir, normal), 0);

				float sf = 0.0;	// specular factor
				if (df > 0.0) {	// if the vertex is lit compute the specular color
					vec3 view_dir = normalize(wCamPos - DataIn.position);	// compute view direction and normalize it 
					vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector

					if (two_sides_lighting)
						sf = abs(dot(half_vector, normal));
					else
						sf = max(dot(half_vector, normal), 0.0);

					sf = pow(sf, shininess);
				}

				color = color * df + specular * sf;
			}

			//uint addr = gl_PrimitiveID / 32;
			//uint offs = gl_PrimitiveID % 32;
			uint addr = gl_PrimitiveID >> 5;
			uint offs = gl_PrimitiveID & 31;
			if ((selection.data[addr] & (1 << offs)) != 0)
				color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
			else if (gl_PrimitiveID == hightlight_id)
				color = vec3(1.0, 0.0, 0.0);

			if (selected) {
				color = vec3(1.0, 0.0, 0.0);
			}

			// If the fragment is in shadow, use ambient light only.
			if (selfShadow) {
				float shadow = ShadowFactor(DataIn.shadowCoord, threshold, int(BlurRadius * MODEL_BLUR_RADIUS_FACTOR));
				FragColor = vec4(color * shadow + ambient, 1.0);
			}
			else
				FragColor = vec4(color + ambient, 1.0);
		}
	}
}