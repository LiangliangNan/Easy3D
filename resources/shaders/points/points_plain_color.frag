#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;

layout(std140) uniform Material {
        vec3	ambient;		// in [0, 1], r==g==b;
        vec3	specular;		// in [0, 1], r==g==b;
        float	shininess;
};

uniform bool	lighting = true;


in  vec3 vsPosition;
in  vec3 vsColor;
in  vec3 vsNormal;

out vec4 outputF;

void main(void) {
    vec3 view_dir = normalize(wCamPos - vsPosition);	// compute view direction and normalize it
    vec3 normal = vsNormal;
    vec3 color = vsColor;
    if (lighting) {
        vec3 light_dir = normalize(wLightPos);
        float df = max(dot(light_dir, normal), 0);
        float sf = 0.0;	// specular factor
        if (df > 0.0) {	// if the vertex is lit compute the specular color
            vec3 half_vector = normalize(light_dir + view_dir);		// compute the half vector
            sf = max(dot(half_vector, normal), 0.0);
            sf = pow(sf, shininess);
        }

        color = color * df + specular * sf + ambient;
    }
    else
        color = color + ambient;

    outputF = vec4(color, 1.0f);
}
