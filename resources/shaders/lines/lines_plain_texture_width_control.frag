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

#version 330 core

// It uses geometry shader for vertex generation.
// The code does not cover round caps.

uniform sampler2D	textureID;

uniform bool selected = false;
uniform vec4 	highlight_color;

uniform bool has_highlight_mask = false;
uniform sampler2D highlight_mask;
uniform int highlight_mask_width;

in  vec2 gOutTexcoord;
in float gOutClipped;
flat in int line_id;

out vec4 outputF;

void main()
{
    if (gOutClipped > 0.0)
        discard;

    outputF = texture(textureID, gOutTexcoord);
    if (has_highlight_mask) {
        ivec2 coord = ivec2(line_id % highlight_mask_width, line_id / highlight_mask_width);
        outputF = mix(outputF, highlight_color, texelFetch(highlight_mask, coord, 0).r);
    }
    if (selected)
        outputF = mix(outputF, highlight_color, 0.6);

    // The depth comes from the rasterizer, as in the colored variant: the geometry
    // shader places the ribbon at the depth of the stroke's own geometry, so the
    // interpolated depth is already the depth of the stroke. This is also what the
    // native Vulkan ribbon does.
}
