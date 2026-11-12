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
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/eye_dome_lighting.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialEyeDomeLighting::TutorialEyeDomeLighting(const std::string &title) : Viewer(title) {
    // A conventional viewing setup for the bunny: y up, looking down -z, framed on the scene.
    camera()->set_up_vector(vec3(0, 1, 0));
    camera()->set_view_direction(vec3(0, 0, -1));
    camera_->show_entire_scene();

    // Create the EDL effect bound to this viewer's camera: the neighbor-depth comparisons
    // are done in screen space, using the near/far range of this camera's projection.
    edl_ = new EyeDomeLighting(camera());
    // Start with the effect visible so the depth cues are obvious on launch.
    edl_enabled_ = true;

    // hint_ is shown in the viewer HUD; it tells the user which key controls the effect.
    hint_ = "Press 'space' to toggle Eye Dome Lighting";
}

TutorialEyeDomeLighting::~TutorialEyeDomeLighting() {
    // We created edl_ with new, so we must delete it here.
    delete edl_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}

// Interactive control of the effect. Returning true marks the key as handled; any key we do
// not recognize is forwarded to the base Viewer so navigation and other shortcuts still work.
bool TutorialEyeDomeLighting::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
        // Toggle the effect on/off, report the new state, and request a redraw.
        edl_enabled_ = !edl_enabled_;
        std::cout << "Eye Dome Lighting " << (edl_enabled_ ? "enabled" : "disabled") << std::endl;
        update();
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}

// The heart of the effect: a depth-based shading pass wrapped around the normal scene render.
void TutorialEyeDomeLighting::draw() const {
    // Nothing to shade until a model has been loaded.
    if (!current_model()) {
        return;
    }

    if (edl_enabled_) {
        // Both backends render the scene inside begin()/end(): OpenGL into the
        // effect's offscreen target, Vulkan into the viewer frame that the effect
        // shades before presentation.
        edl_->begin();  // prepare the effect's render target for this frame
        Viewer::draw(); // render the point cloud normally (fills color and depth)
        edl_->end();    // darken pixels on the far side of depth discontinuities
    } else
        Viewer::draw(); // effect disabled: flat-colored cloud, for comparison
}

// \endcond