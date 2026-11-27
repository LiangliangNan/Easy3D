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
precision highp float;

uniform sampler2D TempTex;

in vec2 texCoord;

out vec4 fragOutput;

void main(void)
{	
    // Resolve the 2x peel target before under-blending at output resolution.
    ivec2 pixel = ivec2(gl_FragCoord.xy) * 2;
    fragOutput = 0.25 * (texelFetch(TempTex, pixel, 0) +
                        texelFetch(TempTex, pixel + ivec2(1, 0), 0) +
                        texelFetch(TempTex, pixel + ivec2(0, 1), 0) +
                        texelFetch(TempTex, pixel + ivec2(1, 1), 0));

	// for occlusion query
	if (fragOutput.a == 0) 
		discard;
}
