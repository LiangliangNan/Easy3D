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

#ifndef EASY3D_TUTORIAL_DEPTH_MAP_H
#define EASY3D_TUTORIAL_DEPTH_MAP_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer that visualizes the scene's depth buffer as an inset depth image.
 *
 * Instead of wrapping draw() with an effect object (as the other 5xx tutorials do), this
 * class overrides pre_draw() — the hook called before the main scene render — to register a
 * screen-space overlay that samples the current frame's scene depth directly on the GPU.
 * The overlay geometry (a textured quad plus a wire frame) is immutable and cached, and is
 * rebuilt only when the framebuffer size or DPI scaling changes.
 */
class TutorialDepthMap : public easy3d::Viewer {
public:
    // Sets up a conventional camera framing the whole scene.
    explicit TutorialDepthMap(const std::string &title = "");
    // Nothing extra to release: the overlay commands are plain values owned by the viewer.
    ~TutorialDepthMap() override;

protected:
    // Per-frame hook: runs the base-class pre_draw(), then refreshes the depth inset overlay.
    void pre_draw() override;
    // Builds (or reuses) the overlay commands that show the frame's depth in a corner quad.
    void draw_depth() const;

private:
    // The cache is an implementation detail of the logically const draw_depth(), hence
    // mutable: these members remember the framebuffer size / DPI the cached geometry was
    // built for, so it can be reused until the window is resized.
    mutable int depth_width_ = 0, depth_height_ = 0; //!< Framebuffer size the cached overlay was built for.
    mutable float depth_dpi_ = 0;                    //!< DPI scaling the cached overlay was built for.
    mutable std::vector<easy3d::graphics::DrawCommand>
        depth_commands_; //!< Cached inset: depth-texture quad + red wire frame.
};

#endif // EASY3D_TUTORIAL_DEPTH_MAP_H
