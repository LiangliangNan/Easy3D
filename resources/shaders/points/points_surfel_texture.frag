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

#version 330   // for geometry shader to work


in FragmentData
{
    vec2 texcoord;  // the point's
    vec2 tex;       // the local
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
uniform int  highlight_id_min;
uniform int  highlight_id_max;

layout(std140) uniform Material {
        vec3	ambient;		// in [0, 1], r==g==b;
        vec3	specular;		// in [0, 1], r==g==b;
        float	shininess;
};

uniform sampler2D	textureID;

uniform bool selected = false;
uniform vec4 	highlight_color;

out vec4 fragmentColor;

void main()
{
    // calculate whether this fragment is inside or outside the splat circle
    // tex coords from -1.0 to 1.0
    if (pow(FragmentIn.tex.x, 2.0) + pow(FragmentIn.tex.y, 2.0) > 1.0)
        discard;

    vec3 color = texture(textureID, FragmentIn.texcoord).rgb;
    if (!lighting) {
        fragmentColor = vec4(color, 1.0);
        if (highlight) {
            if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
                fragmentColor = mix(fragmentColor, highlight_color, 0.8);
        }
        if (selected)
            fragmentColor = mix(fragmentColor, highlight_color, 0.6);
        return;
    }

    if (highlight) {
        if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
            color = mix(color, highlight_color.xyz, 0.8);
    }

    if (selected)
        color = mix(color, highlight_color.xyz, 0.6);

    vec3 view_dir = normalize(wCamPos - FragmentIn.point);// compute view direction and normalize it
    vec3 normal = FragmentIn.normal;
    vec3 light_dir = normalize(wLightPos);
    if (distinct_back_color) {
        if (dot(normal, view_dir) < 0)
            color = backside_color;
    }

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
