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

uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// SSAO
uniform sampler2D   ssaoTexture;
uniform bool        ssaoEnabled = false;

uniform bool        lighting = true;
// two sides
uniform bool        two_sides_lighting = true;
// backside color
uniform bool        distinct_back_color = true;
uniform vec3        backside_color = vec3(0.8f, 0.4f, 0.4f);

// smooth shading
uniform bool        smooth_shading = true;

uniform bool highlight;
uniform int  highlight_id_min;
uniform int  highlight_id_max;

uniform bool use_texture = false;
uniform sampler2D textureID;
uniform float texture_repeat = 1.0f;
uniform float fractional_repeat = 0.0f;

//#define ENABLE_ALPHA

uniform bool selected = false;
uniform vec4 	highlight_color;

in Data{
    vec2 texcoord;
    vec4 color;
    vec3 position;
    vec3 normal;
    float clipped;
} DataIn;

out vec4 outputF;

void main(void) {
    if (DataIn.clipped > 0.0)
        discard;

    vec4 color;
    if (use_texture) {
        float repeat = texture_repeat + fractional_repeat / 100.0f;
        color = texture(textureID, DataIn.texcoord * repeat);

#ifndef ENABLE_ALPHA
        color.a = 1.0f;
#else
        if (color.a <= 0)
            discard;
#endif
    }
    else {
        color = DataIn.color;
    }

    if (!lighting) {
        outputF = color;
        if (selected)
            outputF = mix(outputF, highlight_color, 0.6);
        return;
    }

    if ((gl_FrontFacing == false) && distinct_back_color)
        color = vec4(backside_color, color.a);

    if (highlight) {
        if (gl_PrimitiveID >= highlight_id_min && gl_PrimitiveID <= highlight_id_max)
            color = mix(color, highlight_color, 0.8);
    }

    if (selected)
        color = mix(color, highlight_color, 0.6);

    vec3 normal;
    if (smooth_shading)
        normal = normalize(DataIn.normal);
    else {
#if 0	// Workaround for Adreno GPUs not able to do dFdx( vViewPosition )
        vec3 fdx = vec3( dFdx( DataIn.position.x ), dFdx( DataIn.position.y ), dFdx( DataIn.position.z ) );
        vec3 fdy = vec3( dFdy( DataIn.position.x ), dFdy( DataIn.position.y ), dFdy( DataIn.position.z ) );
        normal = normalize(cross(fdx, fdy));
#else
        normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
#endif
//      // Instead of using the vertex normal to verify the normal direction, we can use gl_FrontFacing.
//      if (dot(normal, DataIn.normal) < 0)
        if ((gl_FrontFacing == false) && (two_sides_lighting == false))
            normal = -normal;
    }

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

    if (ssaoEnabled) {
        vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
        float coeff = texture(ssaoTexture, texCoord).r;
        outputF = vec4((color.xyz * df + specular * sf + ambient) * coeff, color.a);
    }
    else
         outputF = vec4(color.xyz * df + specular * sf + ambient, color.a);
}
