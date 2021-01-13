#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// SSAO
uniform sampler2D   ssaoTexture;
uniform bool        ssaoEnabled = false;

uniform bool        lighting = true;
// two sides
uniform bool        two_sides_lighting = true;

// smooth shading
uniform bool        smooth_shading = true;

// backside color
uniform bool        distinct_back_color = true;
uniform vec3        backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool highlight;
uniform int  hightlight_id_min;
uniform int  hightlight_id_max;

uniform bool selected = false;

in Data{
    vec4 color;
    vec3 position;
    vec3 normal;
    float clipped;
} DataIn;

out vec4 outputF;

void main(void) {
    if (DataIn.clipped > 0.0)
        discard;

    if (!lighting) {
        outputF = DataIn.color;
        return;
    }

    vec3 color = DataIn.color.xyz;
    if (!gl_FrontFacing && distinct_back_color)
        color = backside_color;

    if (highlight) {
        if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
            color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
    }

    if (selected)
        color = mix(color, vec3(1.0, 0.0, 0.0), 0.6);

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

    if (ssaoEnabled) {
        vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
        float coeff = texture(ssaoTexture, texCoord).r;
        outputF = vec4((color * df + specular * sf + ambient) * coeff, DataIn.color.a);
    }
    else
         outputF = vec4(color * df + specular * sf + ambient, DataIn.color.a);
}
