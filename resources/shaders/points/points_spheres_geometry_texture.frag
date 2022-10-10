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

#version 330 core


uniform mat4 PROJ;
uniform bool	perspective;

uniform vec3    eLightPos;
uniform bool    lighting = true;

uniform sampler2D	textureID;

layout(std140) uniform Material {
	vec3	ambient;		// in [0, 1], r==g==b;
	vec3	specular;		// in [0, 1], r==g==b;
	float	shininess;
};

uniform float	sphere_radius;

uniform bool highlight;
uniform int  highlight_id_min;
uniform int  highlight_id_max;

uniform bool selected = false;
uniform vec4 	highlight_color;

in Data{
	flat	vec2	texcoord;
	smooth	vec2	tex;
	flat	vec4	position;
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

		// camera pos is (0, 0, 0) in the camera coordinate system
		vec3 light_dir = normalize(eLightPos);
		float df = max(dot(normal, light_dir), 0);

		float sf = 0.0;	// specular factor
		if (df > 0.0) {	// if the vertex is lit compute the specular color
			view_dir = -view_dir; // '-' because I used the reversed direction 
			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
			sf = max(dot(half_vector, normal), 0.0);
			sf = pow(sf, shininess);
		}

		vec3 color = texture(textureID, DataIn.texcoord).rgb;
		if (!lighting) {
			outputF = vec4(color, 1.0);
			if (highlight) {
				if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
					outputF = mix(outputF, highlight_color, 0.8);
			}
			if (selected)
				outputF = mix(outputF, highlight_color, 0.6);
			return;
		}

		if (highlight) {
			if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
			color = mix(color, highlight_color.xyz, 0.8);
		}

//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, highlight_color.xyz, 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = highlight_color.xyz;
//
//		if (selected)
//			color = highlight_color.xyz;

		outputF = vec4(color * df + specular * sf + ambient, 1.0);
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
		pos = PROJ * pos;
		gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;

		// camera pos is (0, 0, 0) in the camera coordinate system
		vec3 view_dir = vec3(0, 0, 1);
		vec3 light_dir = normalize(eLightPos);

		vec3 normal = vec3(x, y, z); // sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
		float df = max(dot(normal, light_dir), 0);

		float sf = 0.0;	// specular factor
		if (df > 0.0) {	// if the vertex is lit compute the specular color
			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
			sf = max(dot(half_vector, normal), 0.0);
			sf = pow(sf, shininess);
		}

		vec3 color = texture(textureID, DataIn.texcoord).rgb;
		if (!lighting) {
			outputF = vec4(color, 1.0);
			if (highlight) {
				if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
					outputF = mix(outputF, highlight_color, 0.8);
			}
			if (selected)
				outputF = mix(outputF, highlight_color, 0.6);
			return;
		}

		if (highlight) {
			if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
				color = mix(color, highlight_color.xyz, 0.8);
		}

//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, highlight_color.xyz, 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = highlight_color.xyz;
//
//		if (selected)
//			color = highlight_color.xyz;

		outputF = vec4(color * df + specular * sf + ambient, 1.0);
	}

	if (selected)
		outputF = mix(outputF, highlight_color, 0.6);
}
