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

in Data{
    vec3 position;
    vec3 normal;
    float clipped;
} DataIn;

// smooth shading
uniform bool        smooth_shading = true;

out vec3 gPosition;
out vec3 gNormal;

void main()
{
    if (DataIn.clipped > 0.0)
        discard;

    // store the fragment position vector in the first gbuffer texture
    gPosition = DataIn.position;

    if (smooth_shading)
        gNormal = normalize(DataIn.normal);
    else {
#if 0	// Workaround for Adreno GPUs not able to do dFdx( vViewPosition )
        vec3 fdx = vec3( dFdx( DataIn.position.x ), dFdx( DataIn.position.y ), dFdx( DataIn.position.z ) );
        vec3 fdy = vec3( dFdy( DataIn.position.x ), dFdy( DataIn.position.y ), dFdy( DataIn.position.z ) );
        gNormal = normalize(cross(fdx, fdy));
#else
        gNormal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
#endif
//      // Instead of using the vertex normal to verify the normal direction, we can use gl_FrontFacing
//      if (dot(gNormal, DataIn.normal) < 0)
        if ((gl_FrontFacing == false) /*&& (two_sides_lighting == false)*/)
            gNormal = -gNormal;
    }

    // to handle two sided lighting
    if (dot(gNormal, vec3(0, 0, 1)) < 0)
        gNormal = -gNormal;
}
