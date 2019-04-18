#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient = vec3(0.05f, 0.05f, 0.05f);
uniform vec3	specular = vec3(0.4f, 0.4f, 0.4f);
uniform float	shininess = 64.0f;
uniform bool	lighting = true;

// SSAO
uniform sampler2D   ssaoTexture;
uniform bool        ssaoEnabled = false;

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
        if (ssaoEnabled) {
            vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
            float coeff = texture(ssaoTexture, texCoord).r;
            color = color * coeff;
        }
    }
    else
        color = color + ambient;

    outputF = vec4(color, 1.0f);
}
