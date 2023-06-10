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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;

// This example shows how to render a surface mesh with transparency effect using the following techniques
//		  - Average Color Blending
//		  - Dual Depth Peeling


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();
    const std::string file_name = resource::directory() + "/data/torusknot.obj";

    // create the viewer.
    TutorialTransparency viewer(EXAMPLE_TITLE);

    // Load a mesh model and create a drawable for the faces.
    if (!viewer.add_model(file_name, true)) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    auto drawable = viewer.current_model()->renderer()->get_triangles_drawable("faces");
    drawable->set_smooth_shading(true);
    drawable->set_distinct_back_color(false);

    // run the viewer
    return viewer.run();
}

