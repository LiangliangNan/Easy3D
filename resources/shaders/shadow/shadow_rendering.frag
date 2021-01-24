#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool  smooth_shading = true;

// backside color
uniform bool  distinct_back_color = true;
uniform vec3  backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool  is_background = false;

uniform sampler2DShadow  shadowMap;
uniform float darkness;

uniform bool selected = false;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
    float clipped;
} DataIn;


out vec4 FragColor;	// Ouput data


vec4 shade(vec3 worldPos)
{
    if (DataIn.clipped > 0.0)
        discard;

    vec4 color = vec4(DataIn.color, 1.0f);

    if (is_background)
        return color;

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

        return vec4(DataIn.color * df + specular * sf, color.a);
    }
}



void main(void) {
        vec4 color = shade(DataIn.position);
        if (selected && !is_background)
            color = mix(color, vec4(1.0, 0.0, 0.0, 1.0), 0.6);

        vec3 coords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
        // to avoid shadow acne: See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
        coords.z -= 0.005;
        float visibility = texture(shadowMap, coords);

        // give control over darkness;
        visibility = (visibility < 0.9) ? (1.0 - darkness) : 1.0f;

        if (is_background)
            FragColor = color * visibility;
        else
            FragColor = color * visibility + vec4(ambient, 1.0);
}

