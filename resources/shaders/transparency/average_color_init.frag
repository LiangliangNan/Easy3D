/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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

// the output of this shader
out vec4 fragOutput0;
out vec4 fragOutput1;

uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;

uniform float	Alpha;

uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool    smooth_shading = true;

uniform bool 	selected = false;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
    float clipped;
} DataIn;


vec4 ShadeFragment()
{
    vec3 normal;
    if (smooth_shading)
        normal = normalize(DataIn.normal);
    else {
        normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
        if (dot(normal, DataIn.normal) < 0)
            normal = -normal;
    }

    vec3 view_dir = normalize(wCamPos - DataIn.position);
    vec3 light_dir = normalize(wLightPos);

    // diffuse factor
    float df = abs(dot(light_dir, normal));				// light up both sides

    // specular factor
    vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
    float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
    sf = pow(sf, shininess);

    return vec4(DataIn.color * df + specular * sf + ambient, Alpha);
}

void main(void)
{
    if (DataIn.clipped > 0.0)
        discard;

	vec4 color = ShadeFragment();
    if (selected)
        color = mix(color, vec4(1.0, 0.0, 0.0, 1.0), 0.6);

	fragOutput0 = vec4(color.rgb * color.a, color.a);
	fragOutput1 = vec4(1.0);
}
