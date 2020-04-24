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

#include "polygonization.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_triangles.h>

#include "polycube/polycube.h"

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // for the KEYs


using namespace easy3d;

TutorialPolygonization::TutorialPolygonization(const std::string &title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();
}


std::string TutorialPolygonization::usage() const {
    return ("----------- Polygonization usage ------------ \n"
            "Press key 'r' to run 1-iteration of the polygonization algorithm\n"
            "press key 'p' to compute and visualize the planarity\n"
            "------------------------------------------------ \n");
}


bool TutorialPolygonization::key_press_event(int key, int modifiers) {
    auto *mesh = dynamic_cast<SurfaceMesh *>(current_model());
    if (key == GLFW_KEY_P) {
        if (!mesh) {
            LOG(ERROR) << "model does not exist" << std::endl;
            return false;
        }
        Polygonizer poly(mesh);
        poly.compute_planarity(2);
        auto faces = mesh->get_triangles_drawable("faces");
        faces->color_scheme().source = ColorScheme::SCALAR_FIELD;
        faces->color_scheme().name = "v:planarity";
        mesh->update();
        return false;
    } else if (key == GLFW_KEY_R) {
        if (!mesh) {
            LOG(ERROR) << "model does not exist" << std::endl;
            return false;
        }
        Polygonizer poly(mesh);
        poly.polygonize(1000);
        mesh->update();
        return false;
    } else
        return Viewer::key_press_event(key, modifiers);
}