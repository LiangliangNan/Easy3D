#version 150

uniform vec3    wLightPos;
uniform vec3    wCamPos;

layout(std140) uniform Material {
    vec3    ambient;// in [0, 1], r==g==b;
    vec3    specular;// in [0, 1], r==g==b;
    float   shininess;
};

uniform bool    lighting = true;
uniform bool    two_sides_lighting = true;

// backside color
uniform bool        distinct_back_color = true;
uniform vec3        backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool highlight;
uniform int  hightlight_id_min;
uniform int  hightlight_id_max;

in Data{
    vec4 color;
    vec3 position;
    vec3 normal;
} DataIn;

out vec4 outputF;

void main(void) {
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

    vec3 view_dir = normalize(wCamPos - DataIn.position);// compute view direction and normalize it
    vec3 normal = DataIn.normal;
    vec3 light_dir = normalize(wLightPos);
    if (distinct_back_color) {
        if (dot(normal, view_dir) < 0)
            color = backside_color;
    }

    float df = 0.0;	// diffuse factor
    if (two_sides_lighting)
        df = abs(dot(light_dir, normal));
    else
        df = max(dot(light_dir, normal), 0);

    float sf = 0.0;// specular factor
    if (df > 0.0) { // if the vertex is lit compute the specular color
        vec3 half_vector = normalize(light_dir + view_dir);// compute the half vector
        if (two_sides_lighting)
            sf = abs(dot(half_vector, normal));
        else
            sf = max(dot(half_vector, normal), 0.0);
        sf = pow(sf, shininess);
    }

    color = color * df + specular * sf + ambient;
    outputF = vec4(color, DataIn.color.a);
}
