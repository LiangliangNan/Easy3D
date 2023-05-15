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
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


PickerViewer::PickerViewer(const std::string &title)
        : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));

    usage_string_ =
            "------------ Picker Viewer usage ---------- \n"
            "Press the mouse to pick a face              \n"
            "------------------------------------------- \n";
}


bool PickerViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    auto model = dynamic_cast<SurfaceMesh *>(current_model());
    if (model) {
        SurfaceMeshPicker picker(camera());
        auto face = picker.pick_face(model, x, y);
        if (face.is_valid())  // picked a face
            std::cout << "picked face " << face << std::endl;

        // highlight the picked face in the model 
        // always treat the model as a general polygonal mesh.
        auto drawable = model->renderer()->get_triangles_drawable("faces");
        auto triangle_range = model->get_face_property<std::pair<int, int> >("f:triangle_range");
        if (triangle_range && face.is_valid()) {
            const auto& range = triangle_range[face];
            drawable->set_highlight_range(range);
            drawable->set_highlight(true);
        }
        else {
            drawable->set_highlight_range(std::make_pair(-1, -1));
            drawable->set_highlight(false);
        }

        LOG_IF(!triangle_range, ERROR) << "face property 'f:triangle_range' not defined";
    }

    return Viewer::mouse_press_event(x, y, button, modifiers);
}


Model* PickerViewer::add_model(const std::string& file_name) {
    Model* model = Viewer::add_model(file_name, true);

    // make sure the edges are visible
    model->renderer()->get_lines_drawable("edges")->set_visible(true);

    return model;
}
