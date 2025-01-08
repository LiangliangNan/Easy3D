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
#include <easy3d/gui/picker_model.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>


using namespace easy3d;


ManipulationViewer::ManipulationViewer(const std::string &title)
        : Viewer(title), selected_model_(nullptr)
{
    // We always want to look at the front of the easy3d logo.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));

    manual_ =
            "-------------------- Manipulator Viewer usage ---------------------\n"
            "Press the left button to pick/unpick a model.                      \n"
            "When a model is picked, using the mouse to manipulate it:          \n"
            "    - Alt + left button: rotate the model                          \n"
            "    - Alt + right button: translate the model                      \n"
            "------------------------------------------------------------------ \n";

    hint_ = "Press left button to pick/unpick a model\n"
        "When picked:\n"
        "  - Alt + left button: rotate bunny model\n"
        "  - Alt + right button: translate bunny model";
}


bool ManipulationViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    if (modifiers != MODIF_ALT) {// this is reserved for manipulation
        ModelPicker picker(camera());
        auto model = picker.pick(models(), x, y);
        if (model)
            mark(model);
        return true;
    }
    else
        return Viewer::mouse_press_event(x, y, button, modifiers);
}


bool ManipulationViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    // control modifier is reserved for zooming on region
    if (modifiers == MODIF_ALT && selected_model_) {
        ManipulatedFrame *frame = selected_model_->manipulator()->frame();

        auto axis = ManipulatedFrame::NONE;
        if (pressed_key_ == KEY_X) axis = ManipulatedFrame::HORIZONTAL;
        else if (pressed_key_ == KEY_Y) axis = ManipulatedFrame::VERTICAL;
        else if (pressed_key_ == KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
        switch (button) {
            case BUTTON_LEFT:
                frame->action_rotate(x, y, dx, dy, camera(), axis);
                break;
            case BUTTON_RIGHT:
                frame->action_translate(x, y, dx, dy, camera(), axis);
                break;
            default:
                break;
        }
        return true;
    }
    else
        return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}


void ManipulationViewer::mark(easy3d::Model *model) {
    for (auto m : models()) {
        m->renderer()->set_selected(m.get() == model);
        auto faces = m->renderer()->get_triangles_drawable("faces");
        if (m.get() == model)
            faces->set_uniform_coloring(vec4(1, 0, 0, 1.0f));
        else
            faces->set_uniform_coloring(setting::surface_mesh_faces_color);
    }
    std::cout << "picked model: " << file_system::simple_name(model->name()) << std::endl;

    selected_model_ = model;
    if (!model->manipulator()) {    // create manipulator if it doesn't exist
        model->set_manipulator(std::make_shared<Manipulator>(model));
        model->manipulator()->frame()->modified.connect(this,
                static_cast<void (ManipulationViewer::*)(void) const>(&ManipulationViewer::update));
    }
    update();
}


void ManipulationViewer::draw() const {
    Viewer::draw();
    for (auto m : models()) {
        if (m->renderer()->is_selected() && m->manipulator())
            m->manipulator()->draw_frame(camera());
    }
}
