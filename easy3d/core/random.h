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

#include <random>

#include <easy3d/core/types.h>


namespace easy3d {


    /** \brief Random real in [0, 1]. */
    inline float random_float() {
        return float(std::rand()) / float(RAND_MAX);
    }

    /** \brief Random real number in the range [min, max]. */
    inline float random_float(float min, float max) {
        return min + random_float() * (max - min);
    }

    /** \brief Generates a random color. The parameter \p allow_dark controls if too dark colors are allowed. */
    inline vec3 random_color(bool allow_dark = false) {
        static float min_rgb = 0.3f;
        min_rgb = allow_dark ? 0.0f : min_rgb;
        return vec3(
            random_float(min_rgb, 1.0f),
            random_float(min_rgb, 1.0f),
            random_float(min_rgb, 1.0f)
        );
    }

}


#endif // EASY3D_CORE_RANDOM_H
