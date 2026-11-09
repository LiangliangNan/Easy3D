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

#ifndef EASY3D_TUTORIAL_POINT_SELECTION_H
#define EASY3D_TUTORIAL_POINT_SELECTION_H

#include <easy3d/viewer/viewer.h>

// Forward declaration only: the header just needs the PointCloud name, so we avoid pulling
// its full definition into every includer.
namespace easy3d {
    class PointCloud;
}

/**
 * \brief A Viewer that selects/deselects point-cloud points inside a sketched rectangle or lasso.
 *
 * The mouse overrides implement the sketch gesture: press starts it (Shift + left/right
 * button), drag extends the region, release runs the GPU selection query and colors the
 * result. key_press_event() switches between lasso and rectangle modes and cancels with
 * Escape; focus_event() and post_resize() abort a gesture when the window state changes
 * (the sketched pixel coordinates would no longer be valid). pre_draw() rebuilds the
 * screen-space overlay that visualizes the region while it is being drawn.
 */
class TutorialPointSelection : public easy3d::Viewer {
public:
    // Installs the usage manual and the on-screen hint.
    explicit TutorialPointSelection(const std::string &title = "PointSelection");

protected:
    // Starts a selection gesture on Shift + left/right press over the current point cloud.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    // On release, runs the GPU region selection and applies the resulting colors.
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    // Extends the sketched region while the gesture is active.
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
    // 'L'/'R' switch lasso/rectangle mode; Escape cancels an ongoing gesture.
    bool key_press_event(int key, int modifiers) override;
    // Losing window focus aborts the gesture (the mouse may be released elsewhere).
    bool focus_event(bool focused) override;
    // A resize invalidates the framebuffer-pixel polygon, so abort the gesture.
    void post_resize(int width, int height) override;
    // Builds/refreshes the overlay visualizing the region being sketched.
    void pre_draw() override;

private:
    // Appends the cursor position to the region (or recomputes the rectangle corners).
    void extend(int x, int y);
    // Resets all gesture state and clears the overlay.
    void cancel();
    // Gesture state: active_ = a gesture is in progress; rectangle_ = rectangle mode ('R')
    // vs. lasso mode ('L', default); dirty_ = the region changed and the overlay must be rebuilt.
    bool active_ = false, rectangle_ = false, dirty_ = true;
    int button_ = -1; //!< Which button started the gesture: left selects, right deselects.
    // The sketched region in framebuffer pixels (top-left origin), sent to the GPU query.
    std::vector<easy3d::vec2> polygon_;
    // Non-owning handle to the cloud being selected; weak so a deleted model cannot dangle.
    std::weak_ptr<easy3d::Model> target_;
    // Cached overlay geometry: the translucent interior fill and the opaque outline.
    std::shared_ptr<const easy3d::graphics::Geometry> fill_, edge_;
};

#endif // EASY3D_TUTORIAL_POINT_SELECTION_H
