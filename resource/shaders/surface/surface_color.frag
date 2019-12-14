#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient = vec3(0.05f, 0.05f, 0.05f);
uniform vec3	specular = vec3(0.4f, 0.4f, 0.4f);
uniform float	shininess = 64.0f;
uniform vec3	defaultColor = vec3(0.4f, 0.8f, 0.8f);

// SSAO
uniform sampler2D   ssaoTexture;
uniform bool        ssaoEnabled = false;


// two sides
uniform bool        two_sides_lighting = false;

in Data{
    vec3 color;
    vec3 position;
    vec3 normal;
} DataIn;

out vec4 outputF;

void main(void) {
    vec3 color = DataIn.color;
    vec3 normal = normalize(DataIn.normal);

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
        outputF = vec4(vec3(color * df + specular * sf + ambient) * coeff, 1.0);
    }
    else
         outputF = vec4(color * df + specular * sf + ambient, 1.0);
}
