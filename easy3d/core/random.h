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

#ifndef EASY3D_CORE_RANDOM_H
#define EASY3D_CORE_RANDOM_H

#include <cmath>
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
     * \brief Convert a color from HSV to RGB.
     * \details The hue is a position on the HSV color wheel rather than an angle,
     *      and the wheel is divided into the six sectors whose RGB axes the color
     *      mixes between. HSV value is not perceptual luminance, so a hue near blue
     *      still looks darker than a hue near green at the same value.
     * \param hue Hue as a fraction of a full turn: 0 is red, 1/3 green, 2/3 blue.
     *      Values outside [0,1] wrap around the wheel, so 1.2 and 0.2 are the same
     *      hue, as are -0.2 and 0.8.
     * \param saturation Saturation in [0,1]: 0 gives the gray of that brightness, 1
     *      the fully colored one.
     * \param value Value in [0,1]: 0 is black, 1 the brightest the hue can be.
     * \return The RGB components in [0,1].
     */
    inline vec3 hsv_to_rgb(float hue, float saturation, float value) {
        float turn = std::fmod(hue, 1.0f);
        if (turn < 0.0f)
            turn += 1.0f;   // fmod keeps the sign of the dividend: wrap the negatives
        const float wheel = turn * 6.0f;
        const int wheel_index = static_cast<int>(wheel);
        const int sector = wheel_index % 6;
        const float fraction = wheel - static_cast<float>(wheel_index);
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
     * others. Independently generated colors are not guaranteed to be distinct,
     * and are not reproducible from run to run: colors that must differ and must
     * come back the same for the same thing are the job of color_from_index().
     * \param allow_dark If true, allow HSV value to range from 0 to 1; hue and
     *        saturation ranges are unchanged.
     * \return RGB components in [0, 1].
     */
    inline vec3 random_color(bool allow_dark = false) {
        // The three draws are sequenced here instead of written as arguments to the
        // conversion: they share the global generator state, and the order in which
        // arguments are evaluated is the compiler's.
        const float hue = random_float();
        const float saturation = random_float(0.75f, 1.0f);
        const float value = random_float(allow_dark ? 0.0f : 0.8f, 1.0f);
        return hsv_to_rgb(hue, saturation, value);
    }

    /**
     * \brief Generate a color by an index. Colors from different indices have visual
     *      distinction.
     * \details The hue of an index steps by the golden angle (137.50776 degrees, the
     *      turn whose whole-number steps never land back on a hue already used) around
     *      the color wheel, so the colors of successive indices keep falling as far from
     *      each other as a fixed step can: dividing the wheel by the number of colors
     *      instead would put neighbors side by side and repaint every color when one
     *      more is added. Saturation and value are fixed at the vivid end of the ranges
     *      random_color() samples ([0.75, 1] and [0.8, 1]), so every color is bright and
     *      two of them differ in hue rather than in shade. Unlike random_color() the
     *      result depends only on the index: the same thing keeps its color across a
     *      redraw, a rerun and a second viewer.
     *      How far apart is measured as the RGB distance between the closest pair of the
     *      colors of the indices 0, 1, ...: 0.26 for the first 8, 0.14 for 16, 0.06 for
     *      32 and 0.03 for 64. The step does come back near hues it already used, so the
     *      distinction is a claim about a handful to a few dozen colors, not hundreds.
     * \param index Which color of the sequence. Negative indices wrap the wheel like any
     *      other hue, and the zero value is the wheel's red.
     * \return RGB components in [0, 1].
     */
    inline vec3 color_from_index(int index) {
        // The product is a double: at float precision a large index would lose the
        // fraction of the turn that carries the distinction.
        const double turn = std::fmod(static_cast<double>(index) * 0.38196601125010515, 1.0);
        return hsv_to_rgb(static_cast<float>(turn), 0.75f, 0.9f);
    }

    /**
     * \brief Generate a color table consisting of \p num distinct colors
     * \details The table holds color_from_index() of the indices 0, 1, ... \p num - 1,
     *      i.e. the color at entry i belongs to index i, so an entry can be asked for on
     *      its own and the two agree. Build one table for a whole set of things (segments,
     *      groups, components) rather than calling color_from_index() with an unrelated
     *      counter: the table is what makes the indices of the set the indices of the
     *      colors. See color_from_index() for how long the colors stay distinct from
     *      each other.
     * \param num Number of colors, and the size of the table. It is signed because the
     *      callers hold signed index ranges; zero gives an empty table, and so does a
     *      negative count rather than a huge allocation.
     * \return A vector of \p num RGB colors, each channel in [0, 1].
     */
    inline std::vector<vec3> color_table(std::size_t num) {
        std::vector<vec3> colors(num);
        for (int idx = 0; idx < num; ++idx)
            colors[idx] = color_from_index(idx);
        return colors;
    }

}


#endif // EASY3D_CORE_RANDOM_H
