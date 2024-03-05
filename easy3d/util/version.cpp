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


#include <easy3d/util/version.h>
#include <cassert>

namespace easy3d {

    int version_major() {
        return Easy3D_VERSION_MAJOR;
    }

    int version_minor() {
        return Easy3D_VERSION_MINOR;
    }

    int version_patch() {
        return Easy3D_VERSION_PATCH;
    }

    std::string version_string() {
        assert(std::string(EASY3D_VERSION_STR) == std::string(Easy3D_VERSION_STRING));
        return std::string(EASY3D_VERSION_STR);
    }

    std::size_t version_number() {
        assert(EASY3D_VERSION_NR == Easy3D_VERSION_NUMBER);
        return EASY3D_VERSION_NR;
    }

    std::size_t release_date() {
        return EASY3D_RELEASE_DATE;
    }
}
