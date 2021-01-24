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

uniform bool use_texture = false;
uniform sampler2D textureID;
uniform float texture_repeat = 1.0f;
uniform float fractional_repeat = 0.0f;
//#define ENABLE_ALPHA

uniform bool selected = false;

in Data{
    vec2 texcoord;
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
    float clipped;
} DataIn;


out vec4 FragColor;	// Ouput data


vec3 shade(vec3 worldPos)
{
    if (DataIn.clipped > 0.0)
        discard;

    vec4 color;
    if (use_texture) {
        float repeat = texture_repeat + fractional_repeat / 100.0f;
        color = texture(textureID, DataIn.texcoord * repeat);

        #ifndef ENABLE_ALPHA
        color.a = 1.0f;
        #else
        if (color.a <= 0)
        discard;
        #endif
    }
    else {
        color = DataIn.color;
    }

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
        if (selected && !is_background)
            color = mix(color, vec3(1.0, 0.0, 0.0), 0.6);

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

