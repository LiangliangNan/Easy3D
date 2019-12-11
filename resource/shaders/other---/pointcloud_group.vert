#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;		// point color
in	vec3 vtx_normal;	// point normal

uniform bool lighting;		// true if lighting is on
uniform bool has_normal;	// true if data has information
	
layout(std140) uniform Matrices{
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
};

uniform bool    clippingPlaneEnabled;
uniform bool    crossSectionEnabled;
uniform vec4	clippingPlane0;
uniform vec4	clippingPlane1;

layout(std140) uniform Lighting{
	vec3	light_pos;		// model space
	vec3	cam_pos;		// model space
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

uniform bool two_sides_lighting;

// the data to be sent to the fragment shader
out vec3 vsColor;

void main(void) {

	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	if (lighting && has_normal) {
		vec3 normal = normalize(NORMAL * vtx_normal);
		vec3 light_dir = normalize(light_pos);
		float df = 0.0;	// diffuse factor
		if (two_sides_lighting)
			df = abs(dot(light_dir, normal));
		else
			df = max(dot(light_dir, normal), 0);

		float sf = 0.0;	// specular factor
		if (df > 0.0) {	// if the vertex is lit compute the specular color
			vec3 view_dir = normalize(cam_pos - new_position.xyz);	// compute view direction and normalize it 
			vec3 half_vector = normalize(light_dir + view_dir);				// compute the half vector

			if (two_sides_lighting)
				sf = abs(dot(half_vector, normal));
			else
				sf = max(dot(half_vector, normal), 0.0);

			sf = pow(sf, shininess);
		}

		vsColor = vtx_color * df + specular * sf + ambient;
	}
	else {
		vsColor = vtx_color;
	}

	gl_Position = MVP * new_position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}