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
#include <easy3d/renderer/ambient_occlusion.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialAmbientOcclusion::TutorialAmbientOcclusion(const std::string &title) : Viewer(title) {
    // Create the effect and bind it to this viewer's camera: the occlusion sampling uses the
    // camera's projection to reason about the depth buffer in screen space.
    ao_ = new AmbientOcclusion(camera());
    // Start with the effect visible so the depth cues are obvious on launch.
    ao_enabled_ = true;

    // hint_ is shown in the viewer HUD; it tells the user which keys control the effect.
    hint_ = "Press 'space' to switch on/off ambient occlusion\n" "Press 'up/down' to increase/decrease the radius";
}

TutorialAmbientOcclusion::~TutorialAmbientOcclusion() {
    // We created ao_ with new, so we must delete it here.
    delete ao_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}

// Interactive control of the effect. Returning true marks the key as handled; any key we do
// not recognize is forwarded to the base Viewer so navigation and other shortcuts still work.
bool TutorialAmbientOcclusion::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
        // Toggle the effect on/off and request a redraw.
        ao_enabled_ = !ao_enabled_;
        update();
        return true;
    } else if (key == KEY_DOWN) {
        // Decrease the sampling radius (finer, tighter occlusion), clamped to a small positive value.
        if (ao_enabled_) {
            float r = ao_->radius();
            if (r > 0)
                ao_->set_radius(r - 0.1f);
            // make sure it is valid
            if (ao_->radius() <= 0)
                ao_->set_radius(0.1f);
            std::cout << "radius: " << ao_->radius() << std::endl;
            update();
        }
        return true;
    } else if (key == KEY_UP) {
        // Increase the sampling radius (broader, softer occlusion), clamped to 1.0.
        if (ao_enabled_) {
            float r = ao_->radius();
            if (r > 0)
                ao_->set_radius(r + 0.1f);
            // make sure it is valid
            if (ao_->radius() >= 1.0f)
                ao_->set_radius(1.0f);
            std::cout << "radius: " << ao_->radius() << std::endl;
            update();
        }
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}

// The heart of the effect: a post-processing pass wrapped around the normal scene render.
void TutorialAmbientOcclusion::draw() const {
    // Nothing to shade until a model has been loaded.
    if (!current_model()) {
        return;
    }

    if (ao_enabled_) {
        // Both backends render the scene inside begin()/end(): OpenGL into the
        // effect's offscreen target, Vulkan into the viewer frame that the effect
        // shades before presentation.
        ao_->begin(models_); // prepare the effect's render target for this frame
        Viewer::draw();      // render the scene normally (into the effect's target)
        ao_->end();          // estimate occlusion from the rendered depth and composite the result
    } else
        Viewer::draw(); // effect disabled: plain rendering, for comparison
}

// \endcond