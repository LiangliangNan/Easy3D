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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialFacePicker::TutorialFacePicker(const std::string &title) : Viewer(title) {
    // Look straight at the front of the mesh so faces are easy to aim at.
    camera()->set_up_vector(vec3(0, 1, 0));
    camera()->set_view_direction(vec3(0, 0, -1));

    // hint_ is shown in the viewer HUD; it tells the user how to interact.
    hint_ = "Click the left button on a face to pick it";
}

// A left click without modifiers picks a face; every other event is forwarded to the base
// Viewer so camera navigation still works.
bool TutorialFacePicker::mouse_press_event(int x, int y, int button, int modifiers) {
    if (button == BUTTON_LEFT && modifiers == 0) {
        // Mouse hooks and pick() both use logical window coordinates on both backends
        // (DPI-independent), so the cursor can be passed straight through.
        // GPU primitive-ID pick: returns the nearest model/drawable and the hit face index.
        const auto hit = pick(x, y);
        // Clear any previous highlight so only the newly picked face stays lit.
        for (const auto &m : models())
            for (const auto &d : m->renderer()->triangles_drawables())
                d->set_highlight(false);
        // We can only highlight a face if the hit model is a surface mesh rendered as triangles.
        auto *model = dynamic_cast<SurfaceMesh *>(hit.model.get());
        auto *drawable = dynamic_cast<TrianglesDrawable *>(hit.drawable.get());
        if (model && drawable && hit.face_index >= 0) {
            // Non-triangular faces are internally triangulated for rendering. This property
            // maps each original face to the [first, last] range of triangles it produced.
            const auto ranges = model->get_face_property<std::pair<int, int>>("f:triangle_range");
            if (ranges) {
                // Highlight the whole triangle range so the entire polygon lights up, not
                // just the single triangle that happened to be under the cursor.
                drawable->set_highlight_range(ranges[SurfaceMesh::Face(hit.face_index)]);
                drawable->set_highlight(true);
                std::cout << "picked face f" << hit.face_index << std::endl;
            }
        }
        // Request a redraw so the highlight change appears immediately.
        update();
    }
    return Viewer::mouse_press_event(x, y, button, modifiers);
}

// Load the model through the base class, then tweak its appearance for face picking.
Model *TutorialFacePicker::add_model(const std::string &file_name, bool create, bool fit_scene) {
    auto model = Viewer::add_model(file_name, create, fit_scene);
    if (!model)
        return nullptr;

    // make sure the edges are visible
    // The second argument (false) suppresses a warning if the drawable is absent. Showing
    // the edges draws the face boundaries, which makes individual faces easy to see.
    if (auto edges = model->renderer()->get_lines_drawable("edges", false))
        edges->set_visible(true);

    return model;
}

// \endcond