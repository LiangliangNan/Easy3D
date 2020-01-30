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
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>


using namespace easy3d;


PickerViewer::PickerViewer(const std::string &title)
        : Viewer(title) {
    picker_ = new SurfaceMeshPicker(camera());
}


PickerViewer::~PickerViewer() {
    delete picker_;
}


std::string PickerViewer::usage() const {
    return ("------------ Picker Viewer usage ---------- \n"
            "Press the left button to pick a face\n"
            "------------------------------------------ \n");
}


bool PickerViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    auto model = dynamic_cast<SurfaceMesh *>(current_model());
    if (model) {
        auto face = picker_->pick_face(model, x, y);
        model->triangles_drawable("faces")->set_highlight_id(face.idx());
    }

    return Viewer::mouse_press_event(x, y, button, modifiers);
}
