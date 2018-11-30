#version 330

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient;		
uniform vec3	specular;
uniform float	shininess;
uniform vec3	SurfaceColor;

in  vec3	position;

out vec4	outputF;

void main(void) {
	vec3 color = DataIn.SurfaceColor;
	vec3 normal = normalize(cross(dFdx(position), dFdy(position)));

	vec3 view_dir = normalize(wCamPos - DataIn.position);
	vec3 light_dir = normalize(wLightPos);

	// diffuse factor
	float df = abs(dot(light_dir, normal));

	// specular factor
	vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
	float sf = abs(dot(half_vector, normal));
	sf = pow(sf, shininess);

	outputF = vec4(color * df + specular * sf + ambient, 1.0);
}