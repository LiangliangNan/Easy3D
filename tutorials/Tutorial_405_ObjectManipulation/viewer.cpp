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
#include <easy3d/renderer/camera.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>


using namespace easy3d;

// \cond

TutorialObjectManipulation::TutorialObjectManipulation(const std::string& title)
    : Viewer(title) {

    camera()->setViewDirection(vec3(0,0,-1));
    camera()->setUpVector(vec3(0,1,0));

    manual_="-------------------- Manipulator Viewer usage ---------------------\n"
            "Press the left button to pick/unpick a model.                      \n"
            "When a model is picked, using the mouse to manipulate it:          \n"
            "    - Alt + left button: rotate the model                          \n"
            "    - Alt + right button: translate the model                      \n"
            "------------------------------------------------------------------ \n";
    hint_="Press left button to pick/unpick a model\n"
        "When picked:\n"
        "  - Alt + left button: rotate the model\n"
        "  - Alt + right button: translate the model";
}
std::shared_ptr<Model> TutorialObjectManipulation::selected_model() const {
    auto model=selected_model_.lock();
    return model && model->renderer() && model->manipulator() &&
           std::find(models().begin(),models().end(),model)!=models().end()?model:nullptr;
}
void TutorialObjectManipulation::end_model_drag() {
    if (auto model=drag_model_.lock())
        if (model->manipulator())
            model->manipulator()->frame()->action_end();
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
        const int px=int(double(x)*framebuffer_width()/camera()->screenWidth());
        const int py=int(double(y)*framebuffer_height()/camera()->screenHeight());
        const auto hit=pick(px,py);mark(hit.model.get());
    }
    return Viewer::mouse_press_event(x,y,button,modifiers);
}
bool TutorialObjectManipulation::mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) {
    const auto model=selected_model();
    if (modifiers==MODIF_ALT && model && drag_model_.lock()==model) {
        auto axis=ManipulatedFrame::NONE;
        if (pressed_key_==KEY_X)
            axis=ManipulatedFrame::HORIZONTAL;
        else if (pressed_key_==KEY_Y)
            axis=ManipulatedFrame::VERTICAL;
        else if (pressed_key_==KEY_O)
            axis=ManipulatedFrame::ORTHOGONAL;
        auto* frame=model->manipulator()->frame();
        if (button==BUTTON_LEFT)
            frame->action_rotate(x,y,dx,dy,camera(),axis);
        else if (button==BUTTON_RIGHT)
            frame->action_translate(x,y,dx,dy,camera(),axis);
        update();return true;
    }
    if (!model)
        end_model_drag();
    return Viewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
}
bool TutorialObjectManipulation::mouse_release_event(int x,int y,int button,int modifiers) {
    end_model_drag();
    return Viewer::mouse_release_event(x,y,button,modifiers);
}
bool TutorialObjectManipulation::focus_event(bool focused) {
    if (!focused)
        end_model_drag();
    return false;
}
void TutorialObjectManipulation::post_resize(int,int) { end_model_drag(); }
void TutorialObjectManipulation::mark(Model* target) {
    if (selected_model().get()==target)
        target=nullptr;
    end_model_drag();
    selected_model_.reset();
    for (const auto& model:models())
        if (model->renderer()) {
            const bool selected=model.get()==target;model->renderer()->set_selected(selected);
            if (selected) {
                selected_model_=model;
                if (!model->manipulator())
                    model->set_manipulator(std::make_shared<Manipulator>(model.get()));
            }
    }
    update();
}
void TutorialObjectManipulation::pre_draw() {
    Viewer::pre_draw();
    const auto model=selected_model();
    if (model && model->renderer()->is_selected() && model->manipulator()) {
        auto command=model->manipulator()->draw_frame(camera(),framebuffer_width(),framebuffer_height());
        set_overlays(command.geometry?std::vector<graphics::DrawCommand>{command}:std::vector<graphics::DrawCommand>{});
    } else {
        end_model_drag();
        selected_model_.reset();
        set_overlays({});
    }
}

// \endcond
