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

#include "picker_viewer.h"
#include <easy3d/gui/picker_model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // for the mouse buttons


using namespace easy3d;


PickerViewer::PickerViewer(const std::string &title)
        : Viewer(title) {
}


std::string PickerViewer::usage() const {
    return ("------------ Picker Viewer usage ---------- \n"
            "Press the left button to pick a model\n"
            "Press the right button to unpick a model\n"
            "------------------------------------------ \n");
}


bool PickerViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    ModelPicker picker(camera());
    auto model = picker.pick(models(), x, y);
    if (model) {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            change_color(model, vec3(1, 0, 0));
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            change_color(model, vec3(0.8f, 0.8f, 0.8f));
        std::cout << "picked model: " << model->name() << std::endl;
    }

    return Viewer::mouse_press_event(x, y, button, modifiers);
}


void PickerViewer::change_color(Model *model, const vec3 &color) const {
    // make sure the vertex buffer holds the right data.
    if (dynamic_cast<SurfaceMesh *>(model)) {
        Drawable *drawable = model->triangles_drawable("faces");
        drawable->set_per_vertex_color(false);
        drawable->set_default_color(color);
    } else if (dynamic_cast<PointCloud *>(model)) {
        Drawable *drawable = model->points_drawable("vertices");
        drawable->set_per_vertex_color(false);
        drawable->set_default_color(color);
    } else if (dynamic_cast<Graph *>(model)) {
        Drawable *drawable = model->points_drawable("vertices");
        drawable->set_per_vertex_color(false);
        drawable->set_default_color(color);

        drawable = model->lines_drawable("edges");
        drawable->set_per_vertex_color(false);
        drawable->set_default_color(color);
    }
}