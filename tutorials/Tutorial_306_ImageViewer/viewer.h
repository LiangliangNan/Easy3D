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

#ifndef EASY3D_TUTORIAL_IMAGE_VIEWER_H
#define EASY3D_TUTORIAL_IMAGE_VIEWER_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer subclass that displays a 2D image in screen space on either backend.
 *
 * Instead of rendering a 3D scene, this viewer draws a single textured quad positioned in
 * framebuffer pixel coordinates (see pre_draw() in viewer.cpp). It subclasses Viewer to
 * reuse the window, event loop, and overlay machinery, overriding only the hooks needed for
 * image viewing: keyboard shortcuts, wheel zoom, file drop, resize, and the per-frame overlay
 * update. The image itself is an immutable graphics::Image shared with the GPU draw command.
 */
class TutorialImageViewer : public easy3d::Viewer {
public:
    // Loads the initial image (throws std::runtime_error on failure) and fits it to the window.
    TutorialImageViewer(const std::string &title, const std::string &image_file);
    ~TutorialImageViewer() override;

protected:
    // Handles 'f' (fit to screen) and Ctrl+O (open file dialog); other keys go to Viewer.
    bool key_press_event(int key, int modifiers) override;
    // Mouse wheel zooms by scaling the displayed image size multiplicatively.
    bool mouse_scroll_event(int x, int y, int dx, int dy) override;
    // Recomputes scale_ so the whole image fits inside the current window.
    void fit_screen();
    // Drag-and-drop: load the first dropped file as the new image.
    bool drop_event(const std::vector<std::string> &files) override;
    // Window size changed: the centered quad must be recomputed on the next frame.
    void post_resize(int width, int height) override;
    // Rebuilds the screen-space textured quad whenever the dirty_ flag is set.
    void pre_draw() override;

private:
    // Decodes the file into an RGBA graphics::Image; returns false on failure.
    bool load_image(const std::string &file);
    std::shared_ptr<const easy3d::graphics::Image> image_; //!< The currently displayed image (GPU-ready).
    float scale_ = 1;   //!< Display scale: image pixels -> framebuffer pixels (changed by zoom/fit).
    bool dirty_ = true; //!< True when the overlay quad must be rebuilt (zoom, resize, or new image).
};

#endif // EASY3D_TUTORIAL_IMAGE_VIEWER_H
