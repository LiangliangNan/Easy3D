/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include "real_camera.h"
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>


// This example shows how to
//        - render a model from the view point given the camera's intrinsic and
//        extrinsic parameters. The camera parameters were recovered using
//        commonly used computer vison techniques (i.e., calibration, or SfM).

using namespace easy3d;

int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    // the bunder file (We use only the camera instric and extrinsic parameters).
    const std::string bundler_file = resource::directory() + "/data/fountain/bundle.out";
    // the point cloud file.
    const std::string cloud_file = resource::directory() + "/data/fountain/pointcloud.ply";

    try {
        RealCamera viewer("Tutorial_304_RealCamera", bundler_file, cloud_file);

        // Run the viewer
        viewer.run();
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
