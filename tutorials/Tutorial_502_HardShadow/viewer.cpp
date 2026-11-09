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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialHardShadow::TutorialHardShadow(const std::string &title) : Viewer(title) {
    // Choose a natural "walk into the room" viewpoint: y is up and we look along +z.
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, 1));

    // Create the shadow effect bound to this viewer's camera: the shadow test re-projects
    // each visible surface point into the light's frame, which is derived from the scene
    // as seen by this camera.
    shadow_ = new Shadow(camera());
    // Enable a virtual background plane (colored like the viewer background) that receives
    // shadows, so the scene casts a ground shadow even without explicit floor geometry.
    shadow_->set_virtual_background_color(background_color());
    // Start with shadows visible so the effect is obvious on launch.
    shadow_enabled_ = true;

    // hint_ is shown in the viewer HUD; it tells the user which key controls the effect.
    hint_ = "Press 'space' to switch on/off shadowing";
}

TutorialHardShadow::~TutorialHardShadow() {
    // We created shadow_ with new, so we must delete it here.
    delete shadow_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}

// Interactive control of the effect. Returning true marks the key as handled; any key we do
// not recognize is forwarded to the base Viewer so navigation and other shortcuts still work.
bool TutorialHardShadow::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
        // Toggle shadowing on/off and request a redraw.
        shadow_enabled_ = !shadow_enabled_;
        update();
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}

// The heart of the effect: the shadow pass wrapped around the normal scene render.
void TutorialHardShadow::draw() const {
    // Nothing to shade until a model has been loaded.
    if (!current_model()) {
        return;
    }

    // Collect every triangles drawable of every model: these are the surfaces that both
    // cast shadows (they are rendered into the shadow map) and receive them.
    std::vector<TrianglesDrawable *> surfaces;
    for (auto m : models_) {
        for (auto d : m->renderer()->triangles_drawables()) {
            surfaces.push_back(d.get());
        }
    }
    if (shadow_enabled_) {
        // Both backends shadow the scene inside begin()/end(): begin() sets up the light
        // frame and shadow map for the registered surfaces, and end() applies the shadow
        // test to the scene rendered by Viewer::draw().
        shadow_->begin(surfaces); // prepare the light-space depth pass for this frame
        Viewer::draw();           // render the scene normally
        shadow_->end();           // darken points the light cannot see
    } else
        Viewer::draw(); // effect disabled: plain rendering, for comparison
}

// \endcond