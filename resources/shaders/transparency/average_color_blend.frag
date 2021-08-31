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

in vec2  texCoord;
in float clipped;

uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;
uniform vec3 BackgroundColor;

out vec4 fragOutput;

void main(void)
{
	if (clipped > 0.0)
		discard;

	vec4 SumColor = texture(ColorTex0, texCoord);
	float n = texture(ColorTex1, texCoord).r;

	if (n == 0.0) {
		fragOutput.rgb = BackgroundColor;
		// Liangliang: I don't want transparent images
		fragOutput.a = 1.0f;
		return;
	}

	vec3 AvgColor = SumColor.rgb / SumColor.a;
	float AvgAlpha = SumColor.a / n;

	float T = pow(1.0-AvgAlpha, n);
	fragOutput.rgb = AvgColor * (1 - T) + BackgroundColor * T;

	// Liangliang: I don't want transparent images
	fragOutput.a = 1.0f;
}
