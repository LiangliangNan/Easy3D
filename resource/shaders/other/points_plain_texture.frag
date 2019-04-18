#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

uniform sampler2D	textureID;	
uniform bool lighting;	// true if lighting is on

layout(std140) uniform Lighting {
	vec3	wLightPos;
	vec3	eLightPos;
	vec3	wCamPos;
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

uniform bool two_sides_lighting;

uniform int	 hightlight_id;
uniform bool selected;

uniform bool distinct_back_color;
uniform vec3 back_color;

in Data{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
} DataIn;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


out vec4 outputF;	// Ouput data


void main() 
{
	vec3 color = texture(textureID, DataIn.texcoord).rgb;

	if (lighting) {
		vec3 normal = normalize(DataIn.normal);
		vec3 light_dir = normalize(wLightPos);
		vec3 view_dir = normalize(wCamPos - DataIn.position);	// compute view direction and normalize it 

		if (distinct_back_color) {
			if (dot(normal, view_dir) < 0)
				color = back_color;
		}

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

		color = color * df + specular * sf + ambient;
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

	outputF = vec4(color, 1.0);
}