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

#include "viewer.h"
#include <easy3d/util/resource.h>
#include <easy3d/util/timer.h>

using namespace easy3d;


int test_real_camera() {
    const std::string bundler_file = resource::directory() + "/data/fountain/bundle.out";
    const std::string cloud_file = resource::directory() + "/data/fountain/pointcloud.ply";
    RealCamera viewer("RealCamera", bundler_file, cloud_file);
    viewer.set_usage("testing real camera...");

    Timer<> timer;
    timer.set_interval(500, &viewer, &RealCamera::change_view);

    // stop the timer before exit
    Timer<>::single_shot(4900, [&]() -> void { timer.stop(); });
    // exit
    Timer<>::single_shot(5000, (Viewer*)&viewer, &Viewer::exit);

    return viewer.run();
}
