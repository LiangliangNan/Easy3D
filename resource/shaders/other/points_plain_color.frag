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

uniform bool lighting;		// true if lighting is on
uniform bool two_sides_lighting;

uniform int	 hightlight_id;
uniform bool selected;

uniform bool distinct_back_color;
uniform vec3 back_color;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


in  vec3 vsPosition;
in  vec3 vsColor;
in  vec3 vsNormal;
out vec4 outputF;


void main(void) {
	vec3 view_dir = normalize(wCamPos - vsPosition);	// compute view direction and normalize it 
	vec3 normal = vsNormal;

	vec3 color = vsColor;
	if (lighting && distinct_back_color) {
		if (dot(normal, view_dir) < 0)
			color = back_color;
	}

	//uint addr = gl_PrimitiveID / 32;
	//uint offs = gl_PrimitiveID % 32;
	uint addr = gl_PrimitiveID >> 5;
	uint offs = gl_PrimitiveID & 31;
	if ((selection.data[addr] & (1 << offs)) != 0)
		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
	else if (gl_PrimitiveID == hightlight_id)
		color = vec3(1.0, 0.0, 0.0);

	if (selected)
		color = vec3(1.0, 0, 0);

	if (lighting) {
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

		color = color * df + specular * sf + ambient;
	}
	else 
		color = color + ambient;

	outputF = vec4(color, 1.0f);
}