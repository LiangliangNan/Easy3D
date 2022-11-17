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

#ifndef EASY3D_UTIL_INITIALIZER_H
#define EASY3D_UTIL_INITIALIZER_H

#include <string>


namespace easy3d {

    /**
     * @brief Initialization of Easy3D.
     * @details This function initializes logging, setting, and resources. Internally it calls (and is thus identical
     *    to calling) logging::initialize(), setting::initialize(), and resource::initialize().
     *    For more fine-grained initializations, please refer to the documentation of these functions.
     * @param use_log_file \c True to create a ".log" file (which will be created next to the executable program).
     * @param use_setting_file \c True to create an ".ini" file (which will be created next to the executable program).
     *    This setting file stores the default rendering parameters. Users can modify this file to change the default
     *    rendering parameters, then the changes will be effective for the future.
     * @param resource_dir The resource directory containing color maps, shaders, textures, fonts, etc.
     *    \c Easy3D_RESOURCE_DIR is the default value, which is the directory coming with the Easy3D distribution.
     *    In most cases you should use the default resource directory (unless you want to use different resources).
     *
     * @sa logging::initialize(), setting::initialize(), and resource::initialize().
     */
    void initialize(bool use_log_file = false, bool use_setting_file = false, const std::string& resource_dir = Easy3D_RESOURCE_DIR);


} // namespace easy3d


#endif // EASY3D_UTIL_INITIALIZER_H
