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

#include <easy3d/viewer/offscreen.h>
#include <easy3d/util/resource.h>


using namespace easy3d;


int offscreen() {
    const std::string file_name = resource::directory() + "/data/bunny.ply";
    OffScreen os;
    if (!os.add_model(file_name)) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    bool failed(false);
    if (!os.render("bunny-0.png"))
        failed = true;
    if (!os.render("bunny-1.png", 2.0f, 0))
        failed = true;
    if (!os.render("bunny-2.png", 2.0f, 4, 0))
        failed = true;
    if (!os.render("bunny-3.png", 2.0f, 4, 1))
        failed = true;
    if (!os.render("bunny-4.png", 2.0f, 4, 2))
        failed = true;

    LOG_IF(failed, ERROR) << "offscreen renderer failed to render the scene";
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}