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

#include <easy3d/util/initializer.h>

#include <cstdlib> // For srand() and rand()
#include <ctime>   // For time() to seed the random number generator

#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/resource.h>


namespace easy3d {

    void initialize(bool info_to_stdout, bool use_log_file, bool use_setting_file, const std::string& resource_dir) {
        // initialize random number generator
        std::srand(std::time(0)); // Use the current time as the seed

        // initialize the logger
        logging::initialize(info_to_stdout, true, true, false, use_log_file ? "default" : "", 9);
        // initialize the setting
        setting::initialize(use_setting_file ? "default" : "");
        // initialize the resource directory
        resource::initialize(resource_dir);
    }

} // namespace easy3d

