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
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/core/model.h>
#include <iostream>

using namespace easy3d;

// \cond

TutorialCameraInterpolation::TutorialCameraInterpolation(const std::string &title) : Viewer(title) {
    // Draw the recorded path and keyframe cameras in the scene so the user can see the route.
    show_camera_path_ = true;
    // hint_ is shown in the viewer HUD; it lists the keys this tutorial adds.
    hint_ =
        "Press 'k' to add key frames\n" "Press 'space' to start/stop the animation\n" "Press 'd' to delete the camera path";
}

// Friendly key bindings layered on top of the Viewer's built-in camera-path machinery.
bool TutorialCameraInterpolation::key_press_event(int key, int modifiers) {
    if (key == KEY_K && modifiers == 0) {
        // Record the camera's current pose (position + orientation) as a new keyframe.
        easy3d::Frame *frame = camera()->frame();
        if (kfi_->add_keyframe(*frame)) {
            // Grow the scene radius if this keyframe lies outside it, so the whole path stays
            // visible (the radius drives clipping, navigation speed, and path drawing).
            float dist = distance(camera_->sceneCenter(), frame->position());
            if (dist > camera_->sceneRadius())
                camera_->setSceneRadius(dist);
            std::cout << "Key frame added" << std::endl;
            return true;
        } else
            return false;
    } else if (key == KEY_SPACE && modifiers == 0) {
        // 'space' is an alias for the Viewer's Ctrl+K shortcut, which starts/stops playback
        // of the interpolated camera path.
        Viewer::key_press_event(KEY_K, MODIF_CTRL);
        return true;
    } else if (key == KEY_D && modifiers == 0) {
        // Clear all keyframes...
        kfi_->delete_path();
        // ...and shrink the scene bounding box back to the models (it may have been grown to
        // enclose keyframes that no longer exist).
        // update scene bounding box
        Box3 box;
        for (auto m : models_)
            box.grow(m->bounding_box());
        if (box.is_valid())
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
        std::cout << "path deleted" << std::endl;
        return true;
    } else
        // Unhandled keys fall through to the base Viewer (navigation, snapshots, ...).
        return Viewer::key_press_event(key, modifiers);
}

// \endcond
