#version 330   // for geometry shader to work
// please send comments or report bug to: liangliang.nan@gmail.com

in FragmentData
{
    vec2 texcoord;
    vec3 color;
    vec3 point;
    vec3 normal;
} FragmentIn;


uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform bool    lighting = true;

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
    if (pow(FragmentIn.texcoord.x, 2.0) + pow( FragmentIn.texcoord.y, 2.0) > 1.0)
      discard;

    vec3 view_dir = normalize(wCamPos - FragmentIn.point);	// compute view direction and normalize it
    vec3 normal = FragmentIn.normal;
    vec3 color = FragmentIn.color;
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

    fragmentColor = vec4(color, 1.0);
}
