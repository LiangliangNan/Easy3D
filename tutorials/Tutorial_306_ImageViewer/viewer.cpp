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
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace easy3d;

// The implementation is reproduced on the example page via \include in main.cpp, so we hide
// it from Doxygen's separate symbol documentation to avoid documenting it twice.
// \cond

TutorialImageViewer::TutorialImageViewer(const std::string &title, const std::string &file) : Viewer(title) {
    // This is a 2D image viewer, so suppress the default 3D-navigation manual text and show
    // only the image-specific shortcuts in the HUD.
    manual_ = "";
    hint_ =
        "Ctrl + o: open an image\n"
        "Mouse wheel: zoom in/out\n"
        "f: reset the view";
    // Load the initial image; failing here is fatal, so report it as an exception.
    if (!load_image(file))
        throw std::runtime_error("could not load image: " + file);
    // Start with the whole image visible, scaled to fit the window.
    fit_screen();
}

TutorialImageViewer::~TutorialImageViewer() = default;

// Keyboard shortcuts. Returning true marks the key as handled; unrecognized keys are
// forwarded to the base Viewer.
bool TutorialImageViewer::key_press_event(int key, int modifiers) {
    if (key == KEY_F) {
        // 'f': reset zoom/position so the entire image fits the window again.
        fit_screen();
        return true;
    }
    if (key == KEY_O && (modifiers & MODIF_CTRL)) {
        // Ctrl+O: pick another image with a native file dialog (filtered to common formats).
        const auto file =
            dialog::open("Please choose an image file", resource::directory() + "/data/",
                         {"Image Files (*.png *.jpg *.bmp *.ppm *.tga)", "*.png *.jpg *.bmp *.ppm *.tga"});
        if (!file.empty() && load_image(file)) {
            fit_screen();
            return true;
        }
        return false;
    }
    return Viewer::key_press_event(key, modifiers);
}

// Wheel zoom: each scroll step multiplies the scale by 1.1 (up) or divides it (down), giving
// smooth exponential zoom. The scale is clamped to a sane range to avoid degenerate sizes.
bool TutorialImageViewer::mouse_scroll_event(int, int, int, int dy) {
    const float next = scale_ * std::pow(1.1f, float(dy));
    scale_ = std::max(1e-6f, std::min(1e6f, next));
    // The quad must be rebuilt with the new size, and a repaint must be scheduled.
    dirty_ = true;
    update();
    return true;
}

// Drag-and-drop support: take the first dropped file and display it if it is a valid image.
bool TutorialImageViewer::drop_event(const std::vector<std::string> &files) {
    if (files.empty() || !load_image(files.front()))
        return false;
    fit_screen();
    return true;
}

// The window was resized: the image stays centered only if the quad is recomputed.
void TutorialImageViewer::post_resize(int, int) {
    dirty_ = true;
}

// Called before each frame. Rebuilds the screen-space quad only when something changed
// (dirty_), so idle frames cost nothing beyond the base Viewer's own pre_draw() work.
void TutorialImageViewer::pre_draw() {
    Viewer::pre_draw();
    if (!dirty_)
        return;
    // Displayed size in framebuffer pixels = image size times the current zoom scale.
    const float w = image_->width() * scale_;
    const float h = image_->height() * scale_;
    // Center the quad in the window (pixel coordinates, origin at the top-left).
    const float x = static_cast<float>(framebuffer_width() - w) * 0.5f;
    const float y = static_cast<float>(framebuffer_height() - h) * 0.5f;
    // Four corners of the quad; UVs (0,0)-(1,1) map the entire image onto it.
    std::vector<graphics::Vertex> vertices(4);
    vertices[0].position = vec3(x, y, 0);
    vertices[0].texcoord = vec2(0, 0);
    vertices[1].position = vec3(x + w, y, 0);
    vertices[1].texcoord = vec2(1, 0);
    vertices[2].position = vec3(x + w, y + h, 0);
    vertices[2].texcoord = vec2(1, 1);
    vertices[3].position = vec3(x, y + h, 0);
    vertices[3].texcoord = vec2(0, 1);
    // Package the quad as an overlay draw command: two triangles (0,1,2) and (0,2,3),
    // textured with the image, projected with the pixel-to-clip-space matrix so positions
    // are interpreted directly as framebuffer pixels (no 3D camera involved).
    graphics::DrawCommand overlay;
    overlay.geometry = std::make_shared<graphics::Geometry>(vertices, std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
    overlay.texture = image_;
    overlay.model_view_projection = graphics::screen_space_projection(framebuffer_width(), framebuffer_height());
    set_overlays({overlay});
    dirty_ = false;
}

// Choose the largest scale at which the whole image still fits the window: the minimum of
// the horizontal and vertical fit ratios preserves the aspect ratio (letterboxing as needed).
void TutorialImageViewer::fit_screen() {
    scale_ = std::min(float(framebuffer_width()) / image_->width(), float(framebuffer_height()) / image_->height());
    dirty_ = true;
    update();
}

// Decode an image file into a GPU-ready graphics::Image. Returns false if decoding fails.
bool TutorialImageViewer::load_image(const std::string &file) {
    int w = 0, h = 0, channels = 0;
    std::vector<unsigned char> rgba;
    // Force 4 channels (RGBA) regardless of the file's native format; 'false' keeps the
    // top row first (no vertical flip), matching the screen-space UV convention used above.
    if (!ImageIO::load(file, rgba, w, h, channels, 4, false))
        return false;
    // Linear filtering interpolates between texels, so zoomed images look smooth instead of
    // blocky (nearest-neighbor would show individual pixels).
    graphics::Sampler sampler;
    sampler.filter = graphics::TextureFilter::linear;
    auto image = std::make_shared<graphics::Image>(w, h, std::move(rgba), sampler);
    image_ = std::move(image);
    // A new image requires rebuilding the overlay quad.
    dirty_ = true;
    return true;
}

// \endcond
