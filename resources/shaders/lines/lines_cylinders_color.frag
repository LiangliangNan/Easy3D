/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#version 150


// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform mat4 PROJ;
uniform bool	perspective;
uniform float	radius;

uniform vec3    eLightPos;
uniform bool    lighting = true;

layout(std140) uniform Material {
        vec3	ambient;		// in [0, 1], r==g==b;
        vec3	specular;		// in [0, 1], r==g==b;
        float	shininess;
};

uniform bool selected = false;

in Data{
	vec3 point;// camera space
	vec3 axis;
	vec3 base;
	vec3 end;
	vec3 U;
	vec3 V;
} DataIn;

in vec4  gOutColor;
in float gOutClipped;

out vec4 outputF;


vec3 shade(vec3 N, vec3 L, vec3 V, vec3 amb, vec3 spec, float sh, vec3 color){
        float df = max(abs(dot(N, L)), 0);

	float sf = 0.0;	// specular factor
	if (df > 0.0) {	// if the vertex is lit compute the specular color
		vec3 half_vector = normalize(L + V);	// compute the half vector
		sf = max(dot(half_vector, N), 0.0);
		sf = pow(sf, sh);
	}

    color = color * df + spec * sf + amb;
	if (selected)
		color = mix(color, vec3(1.0, 0.0, 0.0), 0.6);
	return color;
}

void main()
{
	if (gOutClipped > 0.0)
		discard;

	// First of all, I need the correct point that we're pointing at

	vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
	if (perspective) {
		view_dir = normalize(vec3(0) - DataIn.point);	// camera pos is (0, 0, 0) in camera space
	}

	// basis = local coordinate system of cylinder
	mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
	vec3 diff = DataIn.point - DataIn.base;
	vec3 P = diff * basis;

	float radius2 = radius*radius;

	// Now calculate distance to the cylinder from ray origin
	// See Ray tracing cylinders here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray

	// Direction of the ray in cylinder space
	vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
	// Now the intersection is between z-axis aligned cylinder and a ray
	float a = D.x * D.x + D.y * D.y;
	float b = 2.0 * (P.x * D.x + P.y * D.y);
	float c = P.x * P.x + P.y * P.y - radius2;
	// calculate a discriminant of the above quadratic equation
	float d = b*b - 4*a*c;
	if (d < 0.0)		// outside of the cylinder
		discard;

	float dist = (-b + sqrt(d)) / (2 * a);

	// point of intersection on cylinder surface
	vec3 new_point = DataIn.point + dist * view_dir;

	// now compute the normal of the new point (for computing the lighting).
	vec3 tmp_point = new_point - DataIn.base;
	vec3 normal = normalize(tmp_point - DataIn.axis * dot(tmp_point, DataIn.axis));

	// to calculate caps, simply check the angle between the following two vectors:
	//		- (point of intersection) - (one of the cylinder's end points)
	//		- cap plane normal (which is the cylinder axis)
	// if the angle < 0, the point is outside of cylinder
	float cap_test = dot((new_point - DataIn.base), DataIn.axis);

	// test front cap
	if (cap_test < 0.0) {
		normal = -DataIn.axis;

		// ray-plane intersection
		float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
		new_point = DataIn.point + view_dir * t;

		// within the cap radius?
		if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
			discard;
	}

	// test end cap
	cap_test = dot((new_point - DataIn.end), DataIn.axis);
	if (cap_test > 0.0) {
		normal = DataIn.axis;

		// ray-plane intersection
		float t = dot(DataIn.end - DataIn.point, normal) / dot(view_dir, normal);
		new_point = DataIn.point + view_dir * t;

		// within the cap radius?
		if (dot(new_point - DataIn.end, new_point - DataIn.end) > radius2)
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

	if (!lighting) {
		outputF = gOutColor;
	}

	else {
		vec3 light_dir = normalize(eLightPos);
		vec3 final_color = shade(normal, light_dir, view_dir, ambient, specular, shininess, gOutColor.xyz);
		outputF = vec4(final_color, gOutColor.a);
	}
}
