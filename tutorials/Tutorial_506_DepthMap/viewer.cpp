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
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/renderer.h>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialDepthMap::TutorialDepthMap(const std::string &title) : Viewer(title) {
    // A conventional viewing setup: y up, looking down -z, framed on the whole scene.
    camera()->set_up_vector(vec3(0, 1, 0));
    camera()->set_view_direction(vec3(0, 0, -1));
    camera()->show_entire_scene();
}

TutorialDepthMap::~TutorialDepthMap() = default;

// pre_draw() is called by the Viewer before the main scene render each frame. We keep the
// base-class behavior and add our depth inset registration on top.
void TutorialDepthMap::pre_draw() {
    Viewer::pre_draw();
    draw_depth();
}

// Builds the overlay that visualizes the frame's depth buffer in a corner quad.
void TutorialDepthMap::draw_depth() const {
    // High-DPI displays scale logical pixels to framebuffer pixels; all screen-space
    // geometry must use the same scaling to keep a constant apparent size and margin.
    const float dpi = dpi_scaling();
    // The device generates and samples current scene depth in its render passes.
    // Keep only the inset's immutable geometry here; presentation stays GPU-only.
    if (framebuffer_width() != depth_width_ || framebuffer_height() != depth_height_ || dpi != depth_dpi_) {
        // The inset rectangle: a quarter of the framebuffer in each dimension, offset by a
        // small DPI-scaled margin from the lower-left corner. Rect takes (xmin, xmax, ymin, ymax).
        const Rect quad(20 * dpi, 20 * dpi + framebuffer_width() / 4.0f, 40 * dpi,
                        40 * dpi + framebuffer_height() / 4.0f);
        // Two immutable overlay commands: one samples the frame's scene depth texture into
        // the quad (as a grayscale range image, in top-left image order, entirely on the
        // GPU), and one draws a red wire frame around it so the inset is easy to spot.
        depth_commands_ = {
            graphics::shape::draw_depth_texture(quad, framebuffer_width(), framebuffer_height()),
            graphics::shape::draw_quad_wire(quad, vec4(1, 0, 0, 1), framebuffer_width(), framebuffer_height(), dpi)};
        // Remember the configuration the cached geometry was built for; it stays valid
        // until the window is resized or moved to a display with different DPI scaling.
        depth_width_ = framebuffer_width();
        depth_height_ = framebuffer_height();
        depth_dpi_ = dpi;
    }
    // draw_depth() is logically const (it only prepares what the frame will show), so we
    // cast away constness to hand the cached commands to the non-const Viewer API. The
    // viewer draws them on top of the scene later in this frame.
    const_cast<TutorialDepthMap *>(this)->set_overlays(depth_commands_);
}

// \endcond
