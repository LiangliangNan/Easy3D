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

    vec3 normal;
    if (smooth_shading)
        gNormal = normalize(DataIn.normal);
    else {
        gNormal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
        if (dot(gNormal, DataIn.normal) < 0)
            gNormal = -gNormal;
    }

    // to handle two sided lighting
    if (dot(gNormal, vec3(0, 0, 1)) < 0)
        gNormal = -gNormal;
}
