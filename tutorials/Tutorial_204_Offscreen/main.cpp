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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>

/**
 * \example{lineno} Tutorial_204_Offscreen/main.cpp
 * This example shows how to use the OffScreen renderer to render scene into an image without creating a viewer.
 */

int main(int argc, char** argv) {
    easy3d::initialize();
    easy3d::OffScreen renderer(800, 600);
    const std::string filename = easy3d::resource::directory() + "/data/bunny.ply";
    auto* mesh = renderer.add_model(filename);
    if (!mesh) {
        std::cerr << "model not found or failed to load: " << filename << std::endl;
        return EXIT_FAILURE;
    }
    auto* faces = mesh->renderer()->get_triangles_drawable("faces", true);
    if (faces)
        faces->set_uniform_coloring(easy3d::vec4(0.7f, 0.85f, 1, 1));
    if (renderer.render("offscreen.png")) {
        std::cout << "surface rendering saved to 'offscreen.png'" << std::endl;
        return EXIT_SUCCESS;
    }

    std::cerr << "offscreen rendering failed" << std::endl;
    return EXIT_FAILURE;
}
