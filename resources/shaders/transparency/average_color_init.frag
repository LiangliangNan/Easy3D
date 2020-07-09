#version 150

// the output of this shader
out vec4 fragOutput0;
out vec4 fragOutput1;

uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;

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


vec4 ShadeFragment()
{
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
    float df = abs(dot(light_dir, normal));				// light up both sides

    // specular factor
    vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
    float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
    sf = pow(sf, shininess);

    return vec4(DataIn.color * df + specular * sf + ambient, Alpha);
}

void main(void)
{
	vec4 color = ShadeFragment();
	fragOutput0 = vec4(color.rgb * color.a, color.a);
	fragOutput1 = vec4(1.0);
}
