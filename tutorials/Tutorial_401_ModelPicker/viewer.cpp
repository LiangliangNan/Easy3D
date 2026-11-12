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
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialModelPicker::TutorialModelPicker(const std::string &title) : Viewer(title) {
    // We always want to look at the front of the easy3d logo.
    camera()->set_view_direction(vec3(0, 0, -1));
    camera()->set_up_vector(vec3(0, 1, 0));

    // hint_ is shown in the viewer HUD; it tells the user how to interact.
    hint_ = "Press the left button to pick/unpick a model";
}

// A left click without modifiers is interpreted as a pick; every other event (including
// modifier-clicks and drags) is forwarded to the base Viewer so navigation still works.
bool TutorialModelPicker::mouse_press_event(int x, int y, int button, int modifiers) {
    // Mouse hooks and pick() both use logical window coordinates on both backends
    // (DPI-independent), so the cursor can be passed straight through.
    // pick() re-rasterizes the scene off screen with primitive IDs and reads back the pixel
    // under the cursor. hit.model is null when the click landed on the background.
    const auto hit = (button == BUTTON_LEFT && modifiers == 0) ? pick(x, y) : PickResult{};
    auto *model = hit.model.get();
    if (model)
        mark(model);

    return Viewer::mouse_press_event(x, y, button, modifiers);
}

// Visual feedback for the pick: toggle the clicked model's selection flag, then recolor
// every model according to its (possibly updated) selection state.
void TutorialModelPicker::mark(easy3d::Model *model) {
    for (auto m : models()) {
        // Only the clicked model changes state: selected becomes unselected and vice versa.
        if (m.get() == model)
            m->renderer()->set_selected(!m->renderer()->is_selected());

        // "faces" is the standard triangles drawable of a surface mesh. Point clouds or
        // graphs may not have one, so skip those.
        auto faces = m->renderer()->get_triangles_drawable("faces");
        if (!faces)
            continue;
        // Selected models are painted solid red; the others get the default surface color.
        if (m->renderer()->is_selected())
            faces->set_uniform_coloring(vec4(1, 0, 0, 1.0f));
        else
            faces->set_uniform_coloring(setting::surface_mesh_faces_color);
    }
    std::cout << "picked model: " << file_system::simple_name(model->name()) << std::endl;
    // Request a redraw so the new colors appear immediately.
    update();
}

// \endcond