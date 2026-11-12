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


// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform bool highlight;
uniform int  highlight_id_min;
uniform int  highlight_id_max;

uniform bool selected = false;
uniform vec4 	highlight_color;

in vec4  gOutColor;
in float gOutClipped;
flat in int line_id;

out vec4 outputF;

void main()
{
    if (gOutClipped > 0.0)
        discard;

    outputF = gOutColor;
    // Selection and primitive-range highlighting tint the final color.
    if (highlight) {
        if (line_id >= highlight_id_min && line_id <= highlight_id_max)
            outputF = mix(outputF, highlight_color, 0.8);
    }
    if (selected)
        outputF = mix(outputF, highlight_color, 0.6);

    // The depth comes from the rasterizer: the geometry shader places the ribbon at
    // the depth of the stroke's own geometry, so the interpolated depth is already
    // the depth of the stroke. Writing it again per fragment would only cost a
    // matrix multiplication and disable early depth testing. This is also what the
    // native Vulkan ribbon does.
}
