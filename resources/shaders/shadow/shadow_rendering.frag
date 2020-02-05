#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool    smooth_shading = true;

uniform bool    is_background = false;

uniform sampler2DShadow  shadowMap;
uniform float darkness;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
} DataIn;


out vec4 FragColor;	// Ouput data


vec3 shade(vec3 worldPos)
{
    if (is_background)
        return DataIn.color;

    else {
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
        float df = abs(dot(light_dir, normal));

        vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
        float sf = abs(dot(half_vector, normal));
        sf = pow(sf, shininess);

        vec3 color = DataIn.color * df + specular * sf;
        return color;
    }
}



void main(void) {
        vec3 color = shade(DataIn.position);

        vec3 coords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
        // to avoid shadow acne: See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
        coords.z -= 0.005;
        float visibility = texture(shadowMap, coords);

        // give control over darkness;
        visibility = (visibility < 0.9) ? (1.0 - darkness) : 1.0f;

        if (is_background)
            FragColor = vec4(color * visibility, 1.0);
        else
            FragColor = vec4(color * visibility + ambient, 1.0);
}

