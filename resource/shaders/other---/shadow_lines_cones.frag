#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool clippingPlaneEnabled;
	bool crossSectionEnabled;
	vec4 clippingPlane0;
	vec4 clippingPlane1;
};

uniform bool	generateShadowMap;

uniform float	radius;

layout(std140) uniform Lighting {
	vec3	wLightPos;
	vec3	eLightPos;
	vec3	wCamPos;
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

in Data{
	vec3 point;// camera space
	vec3 axis;
	vec3 base;
	vec3 end;
	vec3 U;
	vec3 V;
} DataIn;


out vec4 FragColor;	// Ouput data

void main()
{
	if (generateShadowMap) {

		// First of all, I need the correct point that we're pointing at

		vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space

		// basis = local coordinate system of cone 
		mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
		vec3 diff = DataIn.point - DataIn.base;
		vec3 P = diff * basis;

		float radius2 = radius*radius;
		float length2 = dot(DataIn.end - DataIn.base, DataIn.end - DataIn.base);
		float coeff = radius2 / length2;
		float h = dot(DataIn.end - DataIn.base, DataIn.axis);

		// Now calculate distance to the intersecting point from ray origin
		// See Ray tracing cones here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray

		// Direction of the ray in cone space
		vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
		// Now the intersection is between z-axis aligned cone and a ray
		float a = D.x * D.x + D.y * D.y - D.z * D.z * coeff;
		float b = 2.0 * (P.x * D.x + P.y * D.y - coeff * (P.z * D.z - h * D.z));
		float c = P.x * P.x + P.y * P.y - coeff * (P.z - h) * (P.z - h);
		// calculate a discriminant of the above quadratic equation
		float d = b*b - 4 * a*c;
		if (d < 0.0)		// outside of the cone
			discard;

		// point of intersection on cone surface
		float dist = (-b + sqrt(d)) / (2 * a);
		vec3 new_point = DataIn.point + dist * view_dir;

		// now compute the normal of the new point (for computing the lighting).
		vec3 tangent = cross(new_point - DataIn.base, new_point - DataIn.end);
		vec3 normal = cross(new_point - DataIn.end, tangent);
		normal = normalize(normal);

		// to calculate caps, simply check the angle between the following two vectors:
		//		- (point of intersection) - (the cone's base center)
		//		- cap plane normal (which is the cone axis)
		// if the angle < 0, the point is outside of cone
		float cap_test = dot((new_point - DataIn.base), DataIn.axis);


		// Now I need to determine if the projection of the cone's peak (onto the base plane) is within the base disc
		float tt = dot(DataIn.base - DataIn.end, -DataIn.axis) / dot(view_dir, -DataIn.axis);
		vec3 projection = DataIn.end + view_dir * tt;
		bool within_base = dot(projection - DataIn.base, projection - DataIn.base) < radius2;


		if (cap_test < 0.0 || (within_base && dot(view_dir, DataIn.axis) < 0.0))
		{
			normal = -DataIn.axis;

			// ray-plane intersection
			float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
			new_point = DataIn.point + view_dir * t;

			// within the cap radius?
			if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
				discard;
		}

		// compute the depth. I can do it easier
		//vec4 pos = PROJ * vec4(new_point, 1.0);
		//float depth = 0.5*(pos.z / pos.w) + 0.5;
		vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
		float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;

		// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
		if (depth <= 0.0 || depth >= 1.0)
			discard;

		gl_FragDepth = depth;
		FragColor = vec4(depth, 0, 0, 1.0);
	}
}