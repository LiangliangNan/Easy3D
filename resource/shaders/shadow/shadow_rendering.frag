#version 150

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient = vec3(0.05f, 0.05f, 0.05f);
uniform vec3	specular = vec3(0.4f, 0.4f, 0.4f);
uniform float	shininess = 64.0f;

uniform sampler2DShadow  shadowMap;
uniform float darkness;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
} DataIn;


out vec4 FragColor;	// Ouput data


void main(void) {
        vec3 normal = normalize(DataIn.normal);

        vec3 view_dir = normalize(wCamPos - DataIn.position);
        vec3 light_dir = normalize(wLightPos);
        float df = abs(dot(light_dir, normal));

        vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
        float sf = abs(dot(half_vector, normal));
        sf = pow(sf, shininess);

        vec3 color = DataIn.color;
        color = color * df + specular * sf;

        vec3 coords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
        // to avoid shadow acne: See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
        coords.z -= 0.005;
        float visibility = texture(shadowMap, coords);

        // give control over darkness;
        visibility = (visibility < 0.9) ? (1.0 - darkness) : 1.0f;

        FragColor = vec4(color * visibility + ambient, 1.0);
}

