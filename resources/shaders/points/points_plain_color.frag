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

uniform vec3    wLightPos;
uniform vec3    wCamPos;

layout(std140) uniform Material {
    vec3    ambient;// in [0, 1], r==g==b;
    vec3    specular;// in [0, 1], r==g==b;
    float   shininess;
};

uniform bool    lighting = true;
uniform bool    two_sides_lighting = true;

// backside color
uniform bool        distinct_back_color = true;
uniform vec3        backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool highlight;
uniform int  highlight_id_min;
uniform int  highlight_id_max;

uniform bool selected = false;
uniform vec4 	highlight_color;


in Data{
    vec4 color;
    vec3 position;
    vec3 normal;
} DataIn;

out vec4 outputF;

void main(void) {
    if (!lighting) {
        outputF = DataIn.color;
        if (selected)
            outputF = mix(outputF, highlight_color, 0.6);
        if (highlight) {
            if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
                outputF = mix(outputF, highlight_color, 0.8);
        }
        return;
    }

    vec3 color = DataIn.color.xyz;
    if ((gl_FrontFacing == false) && distinct_back_color)
        color = backside_color;

    if (highlight) {
        if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
            color = mix(color, highlight_color.xyz, 0.8);
    }

    vec3 view_dir = normalize(wCamPos - DataIn.position);// compute view direction and normalize it
    vec3 normal = DataIn.normal;
    vec3 light_dir = normalize(wLightPos);
    if (distinct_back_color) {
        if (dot(normal, view_dir) < 0)
            color = backside_color;
    }

    if (selected)
        color = mix(color, highlight_color.xyz, 0.6);

    float df = 0.0;	// diffuse factor
    if (two_sides_lighting)
        df = abs(dot(light_dir, normal));
    else
        df = max(dot(light_dir, normal), 0);

    float sf = 0.0;// specular factor
    if (df > 0.0) { // if the vertex is lit compute the specular color
        vec3 half_vector = normalize(light_dir + view_dir);// compute the half vector
        if (two_sides_lighting)
            sf = abs(dot(half_vector, normal));
        else
            sf = max(dot(half_vector, normal), 0.0);
        sf = pow(sf, shininess);
    }

    color = color * df + specular * sf + ambient;
    outputF = vec4(color, DataIn.color.a);
}
