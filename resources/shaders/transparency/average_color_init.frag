/** \cond LICENSE
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
 \endcond */

#version 150

// the output of this shader
out vec4 fragOutput0;
out vec4 fragOutput1;

uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;

uniform float	Alpha;

uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform bool	lighting = true;
uniform bool	two_sides_lighting = true;
uniform bool	distinct_back_color = true;
uniform vec3	backside_color = vec3(0.8f, 0.4f, 0.4f);
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool    smooth_shading = true;
uniform bool    use_texture = false;
uniform sampler2D textureID;
uniform float   texture_repeat = 1.0f;
uniform float   fractional_repeat = 0.0f;

uniform bool 	selected = false;
uniform vec4 	highlight_color;

in Data{
    vec2 texcoord;
	vec3 color;
	vec3 normal;
	vec3 position;
    float clipped;
} DataIn;


vec4 ShadeFragment(vec3 flat_normal)
{
    vec4 color;
    if (use_texture) {
        float repeat = texture_repeat + fractional_repeat / 100.0;
        color = texture(textureID, DataIn.texcoord * repeat);
    }
    else
        color = vec4(DataIn.color, 1.0);

    if ((gl_FrontFacing == false) && distinct_back_color)
        color.rgb = backside_color;

    if (!lighting)
        return vec4(color.rgb, Alpha * (use_texture ? color.a : 1.0));

    vec3 normal = smooth_shading ? normalize(DataIn.normal) : flat_normal;
    if (!smooth_shading && (gl_FrontFacing == false) && !two_sides_lighting)
        normal = -normal;

    vec3 view_dir = normalize(wCamPos - DataIn.position);
    vec3 light_dir = normalize(wLightPos);
    float df = two_sides_lighting ? abs(dot(light_dir, normal)) : max(dot(light_dir, normal), 0.0);
    float sf = 0.0;
    if (df > 0.0) {
        vec3 half_vector = normalize(light_dir + view_dir);
        sf = two_sides_lighting ? abs(dot(half_vector, normal)) : max(dot(half_vector, normal), 0.0);
        sf = pow(sf, shininess);
    }

    return vec4(color.rgb * df + specular * sf + ambient, Alpha * (use_texture ? color.a : 1.0));
}

void main(void)
{
    vec3 flat_normal = vec3(0.0);
    if (!smooth_shading)
        flat_normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
    if (DataIn.clipped > 0.0)
        discard;

    vec4 color = ShadeFragment(flat_normal);
    if (selected)
        color.rgb = mix(color.rgb, highlight_color.rgb, 0.6);

    float alpha = clamp(color.a, 0.0, 1.0);
    fragOutput0 = vec4(color.rgb * alpha, alpha);
    fragOutput1 = vec4(1.0);
}
