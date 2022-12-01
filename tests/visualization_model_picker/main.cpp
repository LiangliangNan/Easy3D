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

using namespace easy3d;


int test_model_picker() {
    ModelPickerViewer viewer("ModelPicker");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_e.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_a.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_s.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_y.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_3.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_d.ply");

    if (viewer.models().empty()) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    viewer.set_usage("testing model picker...");

    return viewer.run();
}

