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
#include <easy3d/core/model.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/per_pixel_linked_lists_oit.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialTransparency::TutorialTransparency(const std::string &title) : Viewer(title) {
    // Start with Dual Depth Peeling: good quality on every platform (unlike per-pixel
    // linked lists, which need OpenGL 4.3+). The effect is bound to this viewer's camera.
    transparency_ = new DualDepthPeeling(camera());
    method_ = 2;
    // The usage/help text is the same for every method except for the trailing method name;
    // keep the common part so each switch only appends the new name.
    hint_string_ =
        "Press 'space' to switch between different transparency techniques\n" "Press 'up/down' to increase/decrease the transparency\n\nMethod: ";
    Viewer::set_usage("default", hint_string_ + "Dual Depth Peeling");
}

TutorialTransparency::~TutorialTransparency() {
    // We created transparency_ with new, so we must delete it here. Transparency has a
    // virtual destructor, so the concrete technique is destroyed correctly.
    delete transparency_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}

// Interactive control: 'space' cycles the technique, 'up'/'down' change the opacity.
// Returning true marks the key as handled; unrecognized keys go to the base Viewer so
// navigation and other shortcuts still work.
bool TutorialTransparency::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
        // Destroy the current technique before constructing the next one; each method
        // owns its own GPU resources (FBOs / buffers), so they cannot simply be reused.
        if (transparency_) {
            delete transparency_;
            transparency_ = nullptr;
        }

        // Cycle through the four states: 0 disabled, 1 ACB, 2 DDP, 3 per-pixel linked lists.
        method_ = (method_ + 1) % 4;
        switch (method_) {
        case 0:
            // No technique object: draw() falls back to plain Viewer rendering, where
            // semi-transparent surfaces show the usual order-dependent blending artifacts.
            set_usage("default", hint_string_ + "NULL (transparency disabled)");
            std::cout << "Method: NULL (transparency disabled)" << std::endl;
            break;
        case 1:
            // Single-pass, fast, order independent — but layers are averaged, not stacked.
            transparency_ = new AverageColorBlending(camera());
            set_usage("default", hint_string_ + "Average Color Blending");
            std::cout << "Method: Average Color Blending" << std::endl;
            break;
        case 2:
            // Multi-pass peeling: composites depth layers in correct order (near-exact
            // when the number of peels covers the scene's depth complexity).
            transparency_ = new DualDepthPeeling(camera());
            set_usage("default", hint_string_ + "Dual Depth Peeling");
            std::cout << "Method: Dual Depth Peeling" << std::endl;
            break;
        case 3:
            // Exact OIT: store all fragments per pixel in GPU linked lists, then sort and
            // composite. Requires atomic counters / storage buffers (OpenGL 4.3+).
            transparency_ = new PerPixelLinkedListsOIT(camera());
#if defined(__APPLE__) && !defined(EASY3D_HAS_VULKAN)
            // macOS caps OpenGL at 4.1, so the linked-list shaders cannot compile there.
            set_usage("default", hint_string_ + "Per Pixel Linked Lists OIT - NOT available on macOS with OpenGL");
            std::cerr
                << "PerPixelLinkedListsOIT: requires OpenGL 4.3+, current version is 4.1. macOS OpenGL is limited to 4.1. Use the Vulkan backend or DualDepthPeeling for transparency. "
                << std::endl;
            ;
#else
            set_usage("default", hint_string_ + "Per Pixel Linked Lists OIT");
            std::cout << "Method: Per Pixel Linked Lists OIT" << std::endl;
#endif
            break;
        }
        update();
        return true;
    } else if (key == KEY_DOWN) {
        // Decrease the surface opacity (more transparent), clamped to a small positive value.
        auto faces = current_model()->renderer()->get_triangles_drawable("faces");
        if (faces) {
            float o = faces->opacity();
            if (o > 0)
                faces->set_opacity(o - 0.1f);
            // make sure it is valid
            if (faces->opacity() <= 0)
                faces->set_opacity(0.1f);
            std::cout << "opacity: " << faces->opacity() << std::endl;
            update();
        }
        return true;
    } else if (key == KEY_UP) {
        // Increase the surface opacity (more opaque), clamped to fully opaque (1.0).
        auto faces = current_model()->renderer()->get_triangles_drawable("faces");
        if (faces) {
            float o = faces->opacity();
            if (o > 0)
                faces->set_opacity(o + 0.1f);
            // make sure it is valid
            if (faces->opacity() >= 1.0f)
                faces->set_opacity(1.0f);
            std::cout << "opacity: " << faces->opacity() << std::endl;
            update();
        }
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}

// Rendering with the active technique. Note the difference from the shadow/AO tutorials:
// the Transparency interface exposes draw(surfaces), which performs all the passes of the
// technique itself, instead of a begin()/end() pair wrapped around Viewer::draw().
void TutorialTransparency::draw() const {
    // Nothing to render until a model has been loaded.
    if (!current_model()) {
        return;
    }

    // Collect the triangle surfaces of all models; only actual triangle drawables can
    // participate in the OIT passes.
    std::vector<TrianglesDrawable *> surfaces;
    for (auto m : models_) {
        for (auto d : m->renderer()->triangles_drawables()) {
            if (d->type() == Drawable::DT_TRIANGLES)
                surfaces.push_back(dynamic_cast<TrianglesDrawable *>(d.get()));
        }
    }

    if (method_ != 0)
        transparency_->draw(surfaces); // the technique renders the surfaces with OIT
    else
        Viewer::draw(); // transparency disabled: plain rendering, for comparison
}

// \endcond