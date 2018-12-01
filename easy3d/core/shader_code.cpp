#include "shader_code.h"



std::string surface_color_frag = R"(
#version 330

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient;		
uniform vec3	specular;
uniform float	shininess;
uniform vec3	color;

in  vec3	position;

out vec4	outputF;

void main(void) {
	vec3 normal = normalize(cross(dFdx(position), dFdy(position)));

	vec3 view_dir = normalize(wCamPos - position);
	vec3 light_dir = normalize(wLightPos);

	// diffuse factor
	float df = abs(dot(light_dir, normal));

	// specular factor
	vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
	float sf = abs(dot(half_vector, normal));
	sf = pow(sf, shininess);

	outputF = vec4(color * df + specular * sf + ambient, 1.0);
}
)";



std::string surface_color_vert = R"(
#version 330
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;	// vertex position

uniform mat4 MVP;

out vec3 position;

void main() {
	position = vtx_position;
	gl_Position = MVP * vec4(vtx_position, 1.0);
}
)";