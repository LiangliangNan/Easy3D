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

uniform bool lightingEnabled = true;
uniform bool two_sides_lighting;
uniform bool smooth_shading;

uniform int  hightlight_id_min;
uniform int  hightlight_id_max;

uniform bool selected;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;

in Data{
	vec2 texcoord;
	vec3 normal;
	vec3 position;
} DataIn;

uniform sampler2D	textureID;

uniform float texture_repeat;
uniform float fractional_repeat;

// SSAO
//------------------------------
uniform sampler2D	ssaoTexture;
uniform bool        ssaoEnabled;

out vec4 outputF;	// Ouput data


//#define ENABLE_ALPHA

void main() {
	float repeat = texture_repeat + fractional_repeat / 100.0f;

#ifndef ENABLE_ALPHA
	vec3 color = texture(textureID, DataIn.texcoord * repeat).rgb;
	float alpha = 1.0f;
#else
	vec4 tmp = texture(textureID, DataIn.texcoord * repeat);
	if (tmp.a <= 0)
		discard;
	vec3 color = tmp.rgb;
	float alpha = tmp.a;
#endif

	//uint addr = gl_PrimitiveID / 32;
	//uint offs = gl_PrimitiveID % 32;
	uint addr = gl_PrimitiveID >> 5;
	uint offs = gl_PrimitiveID & 31;
	if ((selection.data[addr] & (1 << offs)) != 0)
		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
	else if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
		color = vec3(1.0, 0.0, 0.0);

	if (selected) {
		color = vec3(1.0, 0.0, 0.0);
	}

	if (lightingEnabled) {
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

		float df = 0.0;	// diffuse factor
		if (two_sides_lighting)
			df = abs(dot(light_dir, normal));
		else
			df = max(dot(light_dir, normal), 0);

		float sf = 0.0;	// specular factor
		if (df > 0.0) {	// if the vertex is lit compute the specular color
			vec3 half_vector = normalize(light_dir + view_dir);		// compute the half vector

			if (two_sides_lighting)
				sf = abs(dot(half_vector, normal));
			else
				sf = max(dot(half_vector, normal), 0.0);

			sf = pow(sf, shininess);
		}

		if (ssaoEnabled) {
			vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
			float coeff = texture(ssaoTexture, texCoord).r;
			outputF = vec4(vec3(color * df + specular * sf + ambient) * coeff, alpha);
		}
		else
			outputF = vec4(color * df + specular * sf + ambient, alpha);
	}
	else {
		outputF = vec4(color, alpha);
	}
}
