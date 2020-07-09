#version 330   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in FragmentData
{
    vec2 texcoord;
    vec4 color;
    vec3 point;
    vec3 normal;
} FragmentIn;


uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform bool    lighting = true;

// two sides
uniform bool    two_sides_lighting = true;

// backside color
uniform bool    distinct_back_color = true;
uniform vec3    backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool highlight;
uniform int  hightlight_id_min;
uniform int  hightlight_id_max;

layout(std140) uniform Material {
        vec3	ambient;		// in [0, 1], r==g==b;
        vec3	specular;		// in [0, 1], r==g==b;
        float	shininess;
};


out vec4 fragmentColor;

void main()
{
    // calculate whether this fragment is inside or outside the splat circle
    // tex coords from -1.0 to 1.0
    if (pow(FragmentIn.texcoord.x, 2.0) + pow(FragmentIn.texcoord.y, 2.0) > 1.0)
        discard;

    if (!lighting) {
        fragmentColor = FragmentIn.color;
        return;
    }

    vec3 color = FragmentIn.color.xyz;
    if (highlight) {
        if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
            color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
    }

    vec3 view_dir = normalize(wCamPos - FragmentIn.point);// compute view direction and normalize it
    vec3 normal = FragmentIn.normal;

    if (distinct_back_color) {
        if (dot(normal, view_dir) < 0)
            color = backside_color;
    }

    vec3 light_dir = normalize(wLightPos);

    float df = 0.0;// diffuse factor
    if (two_sides_lighting)
        df = abs(dot(light_dir, normal));
    else
        df = max(dot(light_dir, normal), 0);

    float sf = 0.0;// specular factor
    if (df > 0.0) { // if the vertex is lit compute the specular color
        vec3 half_vector = normalize(light_dir + view_dir);// compute the half vector
        sf = max(dot(half_vector, normal), 0.0);
        sf = pow(sf, shininess);
    }

    color = color * df + specular * sf + ambient;

    fragmentColor = vec4(color, 1.0);
}
