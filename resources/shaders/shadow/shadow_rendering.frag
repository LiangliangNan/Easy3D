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

// smooth shading
uniform bool  smooth_shading = true;

// backside color
uniform bool  distinct_back_color = true;
uniform vec3  backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform bool  is_background = false;

uniform sampler2DShadow  shadowMap;
uniform float darkness;

uniform bool selected = false;
uniform vec4 	highlight_color;

in Data{
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
    float clipped;
} DataIn;


out vec4 FragColor;	// Ouput data


vec4 shade(vec3 worldPos)
{
    if (DataIn.clipped > 0.0)
        discard;

    vec4 color = vec4(DataIn.color, 1.0f);

    if (is_background)
        return color;

    else {
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
//          // Instead of using the vertex normal to verify the normal direction, we can use gl_FrontFacing.
//          if (dot(normal, DataIn.normal) < 0)
            if ((gl_FrontFacing == false) /*&& (two_sides_lighting == false)*/)
                normal = -normal;
        }

        vec3 view_dir = normalize(wCamPos - DataIn.position);
        vec3 light_dir = normalize(wLightPos);
        float df = abs(dot(light_dir, normal));

        vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
        float sf = abs(dot(half_vector, normal));
        sf = pow(sf, shininess);

        return vec4(DataIn.color * df + specular * sf, color.a);
    }
}



void main(void) {
        vec4 color = shade(DataIn.position);
        if (selected && !is_background)
            color = mix(color, highlight_color, 0.6);

        vec3 coords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
        // to avoid shadow acne: See: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
        coords.z -= 0.005;
        float visibility = texture(shadowMap, coords);

        // give control over darkness;
        visibility = (visibility < 0.9) ? (1.0 - darkness) : 1.0f;

        if (is_background)
            FragColor = color * visibility;
        else
            FragColor = color * visibility + vec4(ambient, 1.0);
}

