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
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <iostream>

using namespace easy3d;

// \cond

TutorialCameraInterpolation::TutorialCameraInterpolation(const std::string &title) : Viewer(title) {
    interpolator_ = new KeyFrameInterpolator(camera_->frame());
    // The interpolator runs on a timer thread; wake the viewer for every sample so the
    // animation is drawn at the keyframe rate (the device backend has no other wake-up).
    easy3d::connect(&interpolator_->frame_interpolated, (Viewer *) this, &Viewer::update);
    // Update the viewer when the interpolation finishes (the frame is restored on the path's last keyframe).
    easy3d::connect(&interpolator_->interpolation_stopped, (Viewer *) this, &Viewer::update);

    // hint_ is shown in the viewer HUD; it lists the keys this tutorial adds.
    hint_ = "Press 'k' to add key frames\n"
            "Press 'space' to start/stop the animation\n"
            "Press 'd' to delete the camera path";
}


TutorialCameraInterpolation::~TutorialCameraInterpolation() {
    interpolator_->stop_interpolation();
    delete interpolator_;
}


bool TutorialCameraInterpolation::key_press_event(int key, int modifiers) {
    if (key == KEY_K && modifiers == 0) {
        easy3d::Frame *frame = camera()->frame();
        if (interpolator_->add_keyframe(*frame)) {
            // Grow the scene radius if this keyframe lies outside it, so the whole path stays
            // visible (the radius drives clipping, navigation speed, and path drawing).
            float dist = distance(camera_->scene_center(), frame->position());
            if (dist > camera_->scene_radius())
                camera_->set_scene_radius(dist);
            std::cout << "Key frame added" << std::endl;
            return true;
        } else
            return false;
    } else if (key == KEY_SPACE && modifiers == 0) {
        if (interpolator_->is_interpolation_started()) {
            interpolator_->stop_interpolation();
            std::cout << "Animation stopped" << std::endl;
        } else {
            interpolator_->start_interpolation();
            if (interpolator_->is_interpolation_started())
                std::cout << "Animation started" << std::endl;
        }
        return true;
    } else if (key == KEY_D && modifiers == 0) {
        interpolator_->delete_path();
        // update scene bounding box (it may have been grown to enclose keyframes that no longer exist)
        Box3 box;
        for (auto m : models_)
            box.grow(m->bounding_box());
        if (box.is_valid())
            camera_->set_scene_bounding_box(box.min_point(), box.max_point());
        std::cout << "path deleted" << std::endl;
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialCameraInterpolation::pre_draw() {
    Viewer::pre_draw();

    // Draw the keyframe cameras and the path as scene geometry: the scene pass hides the back
    // side of each glyph behind its front side, and the models occlude the guides.
    const std::size_t count = interpolator_->number_of_keyframes();
    if (count == 0) {
        // The path was deleted; forget what the guides were built from, so a newly recorded
        // path with the same number of keyframes rebuilds them.
        cameras_keyframes_ = path_keyframes_ = 0;
    }
    if (interpolator_->is_interpolation_started() || count == 0) {
        if (cameras_)
            cameras_->set_visible(false);
        if (path_)
            path_->set_visible(false);
        return;
    }

    if (!cameras_) {
        cameras_ = std::make_shared<TrianglesDrawable>("keyframe cameras");
        add_drawable(cameras_);
    }
    if (!path_) {
        path_ = std::make_shared<LinesDrawable>("camera path");
        add_drawable(path_);
    }

    const float width = camera()->scene_radius() * 0.05f;
    const float fov = camera()->field_of_view();
    const float aspect = static_cast<float>(camera()->screen_height()) / static_cast<float>(camera()->screen_width());
    if (count != cameras_keyframes_ || width != cameras_width_ ||
        fov != cameras_field_of_view_ || aspect != cameras_aspect_) {
        if (interpolator_->update_cameras_drawable(*cameras_, camera(), width)) {
            cameras_keyframes_ = count;
            cameras_width_ = width;
            cameras_field_of_view_ = fov;
            cameras_aspect_ = aspect;
        }
    }
    cameras_->set_visible(true);

    // A single keyframe has no path to interpolate; only the camera glyph is shown then.
    if (count != path_keyframes_) {
        if (interpolator_->update_path_drawable(*path_))
            path_keyframes_ = count;
    }
    path_->set_visible(count > 1 && count == path_keyframes_);
}

// \endcond
