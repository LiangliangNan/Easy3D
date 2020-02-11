#version 330 core
// please send comments or report bug to: liangliang.nan@gmail.com

uniform mat4    PROJ;
uniform bool	perspective;

uniform vec3    eLightPos;
uniform bool    lighting = true;

layout(std140) uniform Material {
        vec3	ambient;		// in [0, 1], r==g==b;
        vec3	specular;		// in [0, 1], r==g==b;
        float	shininess;
};

uniform float	sphere_radius;

in Data{
	flat    vec4    sphere_color;
	smooth  vec2    tex;
	flat    vec4    position;
//flat	float	sphere_radius;
} DataIn;

out vec4 outputF;


void main()
{
	/*  with perspective correction
	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
	**/
	if (perspective) {
		//vec2 tex = DataIn.tex;
		vec2 tex = DataIn.tex * 1.5; // 1.5 times larger ensuring the quad is big enought in perspective view

		vec3 planePos = vec3(tex * sphere_radius, 0.0) + DataIn.position.xyz;
		vec3 view_dir = normalize(planePos);
		float B = 2.0 * dot(view_dir, -DataIn.position.xyz);
		float C = dot(DataIn.position.xyz, DataIn.position.xyz) - (sphere_radius * sphere_radius);
		float det = (B * B) - (4 * C);
		if (det < 0.0)
			discard;

		float sqrtDet = sqrt(det);
		float posT = (-B + sqrtDet) / 2;
		float negT = (-B - sqrtDet) / 2;
		float intersectT = min(posT, negT);
		vec3 pos = view_dir * intersectT;
		vec3 normal = normalize(pos - DataIn.position.xyz);

		// compute the depth. I can do it easier
		//vec4 pos4 = PROJ * vec4(pos, 1.0);
		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;

		if (!lighting) {
			outputF = DataIn.sphere_color;
			return;
		}

		// camera pos is (0, 0, 0) in the camera coordinate system
		vec3 light_dir = normalize(eLightPos);
		float df = max(dot(normal, light_dir), 0);

		float sf = 0.0;// specular factor
		if (df > 0.0) { // if the vertex is lit compute the specular color
			view_dir = -view_dir;// '-' because I used the reversed direction
			vec3 half_vector = normalize(light_dir + view_dir);// compute the half vector
			sf = max(dot(half_vector, normal), 0.0);
			sf = pow(sf, shininess);
		}

		outputF = vec4(DataIn.sphere_color.xyz * df + specular * sf + ambient, DataIn.sphere_color.a);
	}

	// without perspective correction
	else {
		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
		float x = DataIn.tex.x;
		float y = DataIn.tex.y;
		float zz = 1.0 - x*x - y*y;

		if (zz <= 0.0)
			discard;

		float z = sqrt(zz);
		vec4 pos = DataIn.position;
		pos.z += sphere_radius*z;

		// compute the depth. I can do it easier
		//vec4 pos4 = PROJ * vec4(pos, 1.0);
		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;

		if (!lighting) {
			outputF = DataIn.sphere_color;
			return;
		}

		// camera pos is (0, 0, 0) in the camera coordinate system
		vec3 view_dir = vec3(0, 0, 1);
		vec3 light_dir = normalize(eLightPos);

		vec3 normal = vec3(x, y, z);// sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
		float df = max(dot(normal, light_dir), 0);

		float sf = 0.0;// specular factor
		if (df > 0.0) { // if the vertex is lit compute the specular color
			vec3 half_vector = normalize(light_dir + view_dir);// compute the half vector
			sf = max(dot(half_vector, normal), 0.0);
			sf = pow(sf, shininess);
		}

		outputF = vec4(DataIn.sphere_color.xyz * df + specular * sf + ambient, DataIn.sphere_color.a);
	}
}
