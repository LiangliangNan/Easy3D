/** \cond LICENSE
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
 \endcond */

#include "viewer.h"
#include <easy3d/renderer/camera.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialObjectManipulation::TutorialObjectManipulation(const std::string &title) : Viewer(title) {

    // Look straight at the front of the logo so the letters are easy to click.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));

    // manual_ is the full help text (shown on demand); hint_ is the short HUD reminder.
    manual_ =
        "-------------------- Manipulator Viewer usage ---------------------\n"
        "Press the left button to pick/unpick a model.                      \n"
        "When a model is picked, using the mouse to manipulate it:          \n"
        "    - Alt + left button: rotate the model                          \n"
        "    - Alt + right button: translate the model                      \n"
        "------------------------------------------------------------------ \n";
    hint_ =
        "Press left button to pick/unpick a model\n"
        "When picked:\n"
        "  - Alt + left button: rotate the model\n"
        "  - Alt + right button: translate the model";
}
// Validates the weak selection handle: the model must still exist, still be part of the
// scene, and have both a renderer and a manipulator before we act on it.
std::shared_ptr<Model> TutorialObjectManipulation::selected_model() const {
    auto model = selected_model_.lock();
    return model && model->renderer() && model->manipulator() &&
                   std::find(models().begin(), models().end(), model) != models().end()
               ? model
               : nullptr;
}
// Closes an in-flight drag: action_end() lets the frame finalize the incremental motion.
void TutorialObjectManipulation::end_model_drag() {
    if (auto model = drag_model_.lock())
        if (model->manipulator())
            model->manipulator()->frame()->action_end();
    drag_model_.reset();
}
// Two interaction modes: Alt + button manipulates the selected model; a plain left click
// picks a model. Everything else is forwarded to the base Viewer (camera navigation).
bool TutorialObjectManipulation::mouse_press_event(int x, int y, int button, int modifiers) {
    end_model_drag();
    if (modifiers == MODIF_ALT && (button == BUTTON_LEFT || button == BUTTON_RIGHT)) {
        if (auto model = selected_model()) {
            // Bracket the drag: the frame snapshots its state so subsequent motion events
            // are applied incrementally and consistently.
            model->manipulator()->frame()->action_start();
            drag_model_ = model;
        }
        return true;
    }
    if (modifiers == 0 && button == BUTTON_LEFT) {
        // GPU pick at the cursor. Mouse hooks and pick() both use logical window
        // coordinates on both backends (DPI-independent). hit.model is null when the
        // click landed on the background.
        const auto hit = pick(x, y);
        mark(hit.model.get());
    }
    return Viewer::mouse_press_event(x, y, button, modifiers);
}
// While Alt-dragging the selected model, mouse motion drives its manipulated frame instead
// of the camera. Left button rotates (trackball), right button translates in the screen plane.
bool TutorialObjectManipulation::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    const auto model = selected_model();
    if (modifiers == MODIF_ALT && model && drag_model_.lock() == model) {
        // Optional axis constraint: holding 'X', 'Y', or 'O' during the drag restricts the
        // motion to a horizontal, vertical, or view-orthogonal screen axis.
        auto axis = ManipulatedFrame::NONE;
        if (input_.key == KEY_X)
            axis = ManipulatedFrame::HORIZONTAL;
        else if (input_.key == KEY_Y)
            axis = ManipulatedFrame::VERTICAL;
        else if (input_.key == KEY_O)
            axis = ManipulatedFrame::ORTHOGONAL;
        auto *frame = model->manipulator()->frame();
        if (button == BUTTON_LEFT)
            frame->action_rotate(x, y, dx, dy, camera(), axis);
        else if (button == BUTTON_RIGHT)
            frame->action_translate(x, y, dx, dy, camera(), axis);
        update();
        return true;
    }
    // The selection disappeared mid-drag (e.g. the model was deleted): abort cleanly.
    if (!model)
        end_model_drag();
    return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}
// Any button release finishes the drag; other release handling stays with the base Viewer.
bool TutorialObjectManipulation::mouse_release_event(int x, int y, int button, int modifiers) {
    end_model_drag();
    return Viewer::mouse_release_event(x, y, button, modifiers);
}
// If focus is lost mid-drag, the release may arrive elsewhere; abort so the drag cannot stick.
bool TutorialObjectManipulation::focus_event(bool focused) {
    if (!focused)
        end_model_drag();
    return false;
}
// Screen-space drag coordinates become invalid after a resize; abort the drag.
void TutorialObjectManipulation::post_resize(int, int) {
    end_model_drag();
}
// Selects the clicked model (clicking the already-selected model deselects it). Selection is
// exclusive: all other models are unmarked, and the newly selected one gets a Manipulator.
void TutorialObjectManipulation::mark(Model *target) {
    // Toggle: clicking the current selection clears it.
    if (selected_model().get() == target)
        target = nullptr;
    end_model_drag();
    selected_model_.reset();
    for (const auto &model : models())
        if (model->renderer()) {
            const bool selected = model.get() == target;
            model->renderer()->set_selected(selected);
            if (selected) {
                selected_model_ = model;
                // Create the manipulator lazily, on first selection. Its frame origin sits
                // at the object center, so rotations pivot around the object itself.
                if (!model->manipulator())
                    model->set_manipulator(std::make_shared<Manipulator>(model.get()));
            }
        }
    update();
}
// Every frame, show the manipulation gizmo (the frame's axes) of the selected model as a
// depth-tested overlay, so the user sees the pivot and orientation being manipulated.
void TutorialObjectManipulation::pre_draw() {
    Viewer::pre_draw();
    const auto model = selected_model();
    if (model && model->renderer()->is_selected() && model->manipulator()) {
        // draw_frame() returns an immutable draw command in framebuffer pixels, ready for
        // set_overlays(); an empty command means there is nothing to draw.
        auto command = model->manipulator()->draw_frame(camera(), framebuffer_width(), framebuffer_height());
        set_overlays(command.geometry ? std::vector<graphics::DrawCommand>{command}
                                      : std::vector<graphics::DrawCommand>{});
    } else {
        // No valid selection: drop any stale drag/selection state and clear the overlay.
        end_model_drag();
        selected_model_.reset();
        set_overlays({});
    }
}

// \endcond
