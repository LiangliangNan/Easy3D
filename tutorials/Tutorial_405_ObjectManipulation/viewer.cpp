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

namespace {
const char* const usage_manual = "-------------------- Manipulator Viewer usage ---------------------\n"
            "Press the left button to pick/unpick a model.                      \n"
            "When a model is picked, using the mouse to manipulate it:          \n"
            "    - Alt + left button: rotate the model                          \n"
            "    - Alt + right button: translate the model                      \n"
            "------------------------------------------------------------------ \n";
const char* const usage_hint = "Press left button to pick/unpick a model\n"
        "When picked:\n"
        "  - Alt + left button: rotate the model\n"
        "  - Alt + right button: translate the model";
}

#if !EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/gui/picker_model.h>
#endif
#include <easy3d/renderer/camera.h>
#include <algorithm>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>


using namespace easy3d;

// \cond

#if EASY3D_TUTORIAL_USES_VULKAN
TutorialObjectManipulation::TutorialObjectManipulation(const std::string& title,const VulkanViewerOptions& options)
    : TutorialViewer(title,options) {
    camera()->setViewDirection(vec3(0,0,-1));camera()->setUpVector(vec3(0,1,0));
    manual_=usage_manual;hint_=usage_hint;
}
std::shared_ptr<Model> TutorialObjectManipulation::selected_model() const {
    auto model=selected_model_.lock();
    return model && model->renderer() && model->manipulator() &&
           std::find(models().begin(),models().end(),model)!=models().end()?model:nullptr;
}
void TutorialObjectManipulation::end_model_drag() {
    if (auto model=drag_model_.lock()) if (model->manipulator()) model->manipulator()->frame()->action_end();
    drag_model_.reset();
}
bool TutorialObjectManipulation::mouse_press_event(int x,int y,int button,int modifiers) {
    end_model_drag();
    if (modifiers==MODIF_ALT && (button==BUTTON_LEFT || button==BUTTON_RIGHT)) {
        if (auto model=selected_model()) {
            model->manipulator()->frame()->action_start();drag_model_=model;
        }
        return true;
    }
    if (modifiers==0 && button==BUTTON_LEFT) {
        const auto hit=pick(x,y);mark(hit.model.get());
    }
    return TutorialViewer::mouse_press_event(x,y,button,modifiers);
}
bool TutorialObjectManipulation::mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) {
    const auto model=selected_model();
    if (modifiers==MODIF_ALT && model && drag_model_.lock()==model) {
        auto axis=ManipulatedFrame::NONE;
        if (pressed_key_==KEY_X) axis=ManipulatedFrame::HORIZONTAL;
        else if (pressed_key_==KEY_Y) axis=ManipulatedFrame::VERTICAL;
        else if (pressed_key_==KEY_O) axis=ManipulatedFrame::ORTHOGONAL;
        auto* frame=model->manipulator()->frame();
        if (button==BUTTON_LEFT) frame->action_rotate(x,y,dx,dy,camera(),axis);
        else if (button==BUTTON_RIGHT) frame->action_translate(x,y,dx,dy,camera(),axis);
        update();return true;
    }
    if (!model) end_model_drag();
    return TutorialViewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
}
bool TutorialObjectManipulation::mouse_release_event(int x,int y,int button,int modifiers) {
    end_model_drag();return TutorialViewer::mouse_release_event(x,y,button,modifiers);
}
bool TutorialObjectManipulation::focus_event(bool focused) {
    if (!focused) end_model_drag();
    return false;
}
void TutorialObjectManipulation::post_resize(int,int) { end_model_drag(); }
void TutorialObjectManipulation::mark(Model* target) {
    if (selected_model().get()==target) target=nullptr;
    end_model_drag();selected_model_.reset();
    for (const auto& model:models()) if (model->renderer()) {
        const bool selected=model.get()==target;model->renderer()->set_selected(selected);
        if (selected) {
            selected_model_=model;
            if (!model->manipulator()) model->set_manipulator(std::make_shared<Manipulator>(model.get()));
        }
    }
    update();
}
void TutorialObjectManipulation::pre_draw() {
    const auto model=selected_model();
    if (model && model->renderer()->is_selected() && model->manipulator()) {
        auto command=guide_.draw_frame(*model,*camera());
        set_overlays(command.geometry?std::vector<graphics::DrawCommand>{command}:std::vector<graphics::DrawCommand>{});
    } else {end_model_drag();selected_model_.reset();set_overlays({});}
}
#else
TutorialObjectManipulation::TutorialObjectManipulation(const std::string &title)
        : Viewer(title), selected_model_(nullptr)
{
    // We always want to look at the front of the easy3d logo.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));

    manual_ = usage_manual;

    hint_ = usage_hint;
}


bool TutorialObjectManipulation::mouse_press_event(int x, int y, int button, int modifiers) {
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


bool TutorialObjectManipulation::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
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


void TutorialObjectManipulation::mark(easy3d::Model *model) {
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
                static_cast<void (TutorialObjectManipulation::*)(void) const>(&TutorialObjectManipulation::update));
    }
    update();
}


void TutorialObjectManipulation::draw() const {
    Viewer::draw();
    for (auto m : models()) {
        if (m->renderer()->is_selected() && m->manipulator())
            m->manipulator()->draw_frame(camera());
    }
}

// \endcond
#endif // EASY3D_TUTORIAL_USES_VULKAN
