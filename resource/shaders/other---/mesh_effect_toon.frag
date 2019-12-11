#version 150
// please send comments or report bug to: liangliang.nan@gmail.com


layout(std140) uniform Lighting {
	vec3	wLightPos;
	vec3	eLightPos;
	vec3	wCamPos;
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

uniform vec3 color;
uniform bool two_sides_lighting;

in Data{
	vec3 normal;
	vec3 position;
} DataIn;


out vec4 outputF;


float stepmix(float edge0, float edge1, float E, float x)
{
    float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
    return mix(edge0, edge1, T);
}

void main()
{
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

    const float A = 0.1;
    const float B = 0.3;
    const float C = 0.6;
    const float D = 1.0;
    float E = fwidth(df);

    if      (df > A - E && df < A + E) df = stepmix(A, B, E, df);
    else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
    else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
    else if (df < A) df = 0.0;
    else if (df < B) df = B;
    else if (df < C) df = C;
    else df = D;

    E = fwidth(sf);
    if (sf > 0.5 - E && sf < 0.5 + E) {
		// Hermite interpolation between two values
        sf = smoothstep(0.5 - E, 0.5 + E, sf);
    }
    else
        sf = step(0.5, sf);

	outputF = vec4(df * color + sf * specular + ambient, 1.0);
}