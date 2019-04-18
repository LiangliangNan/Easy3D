#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

layout(std140) uniform Lighting{
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

uniform bool distinct_back_color;
uniform vec3 back_color;

uniform int  hightlight_id_min;
uniform int  hightlight_id_max;

uniform bool selected;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
} DataIn;

// SSAO
//------------------------------
uniform sampler2D	ssaoTexture;
uniform bool        ssaoEnabled;

out vec4 outputF;

void main(void) {
	vec3 color = DataIn.color;
	if (!gl_FrontFacing && distinct_back_color)
		color = back_color;

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
			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector

			if (two_sides_lighting)
				sf = abs(dot(half_vector, normal));
			else
				sf = max(dot(half_vector, normal), 0.0);

			sf = pow(sf, shininess);
		}

		if (ssaoEnabled) {
			vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
			float coeff = texture(ssaoTexture, texCoord).r;
			outputF = vec4(vec3(color * df + specular * sf + ambient) * coeff, 1.0);
		}
		else
			outputF = vec4(color * df + specular * sf + ambient, 1.0);
	}
	else {
		outputF = vec4(color, 1.0);
	}

	// the following line is required for fxaa (will not work with blending!)
	//outputF.a = dot(outputF.rgb, vec3(0.299, 0.587, 0.114));
}