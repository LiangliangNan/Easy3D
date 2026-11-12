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

#ifndef EASY3D_CORE_RANDOM_H
#define EASY3D_CORE_RANDOM_H

#include <cstdlib>
#include <random>

#include <easy3d/core/types.h>


namespace easy3d {


    /**
     * \brief Generate a pseudo-random float in [0,1].
     * \details Uses the C library rand() generator and its global state.
     * \return Uniformly scaled value in the inclusive range [0,1].
     */
    inline float random_float() {
        return float(std::rand()) / float(RAND_MAX);
    }

    /**
     * \brief Generate a pseudo-random float in a specified interval.
     * \param min Lower endpoint.
     * \param max Upper endpoint.
     * \return Value in [min,max] when min <= max.
     */
    inline float random_float(float min, float max) {
        return min + random_float() * (max - min);
    }

    /**
     * \brief Generate a pseudo-random vivid RGB color.
     * \details Sampling red, green, and blue independently is simple, but does
     * not control the resulting hue, saturation, or brightness; it can produce
     * dull or very dark colors, and its hue distribution is not uniform. This
     * function instead samples hue uniformly around the HSV color wheel,
     * restricts saturation to [0.75, 1], then converts the result to RGB.
     * Unless \p allow_dark is true, HSV value is restricted to [0.8, 1], so
     * the strongest RGB channel stays bright. HSV value is not perceptual
     * luminance, so some hues (notably blue) can still appear darker than
     * others. Independently generated colors are not guaranteed to be distinct.
     * \param allow_dark If true, allow HSV value to range from 0 to 1; hue and
     *        saturation ranges are unchanged.
     * \return RGB components in [0, 1].
     */
    inline vec3 random_color(bool allow_dark = false) {
        const float hue = random_float() * 6.0f;
        const int hue_sector = static_cast<int>(hue);
        const int sector = hue_sector % 6;
        const float fraction = hue - static_cast<float>(hue_sector);
        const float saturation = random_float(0.75f, 1.0f);
        const float value = random_float(allow_dark ? 0.0f : 0.8f, 1.0f);
        const float chroma = value * saturation;
        const float x = chroma * (sector % 2 == 0 ? fraction : 1.0f - fraction);
        const float offset = value - chroma;

        float r, g, b;
        switch (sector) {
            case 0: r = chroma; g = x;      b = 0.0f;    break;
            case 1: r = x;      g = chroma; b = 0.0f;    break;
            case 2: r = 0.0f;   g = chroma; b = x;       break;
            case 3: r = 0.0f;   g = x;      b = chroma; break;
            case 4: r = x;      g = 0.0f;   b = chroma; break;
            default:r = chroma; g = 0.0f;   b = x;       break;
        }
        return vec3(r + offset, g + offset, b + offset);
    }

}


#endif // EASY3D_CORE_RANDOM_H
