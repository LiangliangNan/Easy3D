/********************************************************************
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
 ********************************************************************/


#include "walk_through.h"
#include <easy3d/core//model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


WalkThrough::WalkThrough(Camera *camera)
        : camera_(camera), status_(STOPPED), ground_plane_normal_(0, 0, 1), follow_up_(true),
          height_factor_(0.2f), third_person_forward_factor_(1.8f), rotate_zoom_out_factor_(0.0f),
          rotate_vertical_offset_factor_(0.0f), rotate_pitch_angle_(0.0f), rotate_keyframe_samples_(10),
          rotate_num_loops_(2), cameras_visible_(false), path_visible_(false), current_frame_idx_(-1)
{
    kfi_ = new KeyFrameInterpolator(camera_->frame());
}


WalkThrough::~WalkThrough() {
    delete_path();
    delete kfi_;
}


void WalkThrough::add_keyframe(const Frame &frame, bool quiet) {
    if (kfi_->add_keyframe(frame)) {
        // update scene radius to make sure the path is within the view frustum
        float dist = distance(camera_->sceneCenter(), frame.position());
        if (dist > camera_->sceneRadius())
            camera_->setSceneRadius(dist);

        LOG_IF(!quiet, INFO) << "a key frame added to camera path";
        path_modified.send();
    }
}


void WalkThrough::set_scene(const std::vector<Model *> &scene) {
    if (scene.empty())
        return;
    Box3 box;
    for (const auto &m : scene) {
        if (m->renderer()->is_visible())
        box += m->bounding_box();
    }
    scene_box_ = box;
    camera_->setSceneRadius(box.radius() * 1.1f);

    if (status_ != STOPPED)
        interpolator()->interpolate();
}


void WalkThrough::walk_to(const vec3 &ground_point) {
    if (status_ != WALKING_MODE) {
        LOG(WARNING) << "wrong status: walk_to() is for the walking mode only";
        return;
    }

    const vec3 head = character_head(ground_point);
    vec3 view_dir = head - camera_->position(); // approximate (when no previous keyframes exist)
    if (interpolator()->number_of_keyframes() > 0) {
        std::size_t prev_idx = interpolator()->number_of_keyframes() - 1;
        view_dir = head - interpolator()->keyframe_position(prev_idx);
    }
    view_dir.z = 0;
    view_dir.normalize();

    vec3 cam_pos = head;    // if not follow up, we want to get what we see
    if (follow_up_)
        cam_pos = cam_pos - view_dir * third_person_forward_distance(); // move back

    // adjust the previous view direction to be pointing to the new one
    if (interpolator()->number_of_keyframes() > 0) {
		std::size_t last_idx = interpolator()->number_of_keyframes() - 1;
        vec3 dir = cam_pos - interpolator()->keyframe_position(last_idx);
        dir.z = 0;
        dir.normalize();
        interpolator()->set_keyframe_orientation(last_idx, to_orientation(dir));
    }

    add_keyframe(cam_pos, view_dir);

    // set the pivot point ahead of the character.
    camera_->setPivotPoint(ground_point + view_dir * third_person_forward_distance() * 2.0f);
}


void WalkThrough::set_height_factor(float f) {
    height_factor_ = f;
    if (follow_up_)
        move_to(current_frame_idx_);
}


void WalkThrough::set_third_person_forward_factor(float f) {
    third_person_forward_factor_ = f;
    if (follow_up_)
        move_to(current_frame_idx_);
}


void WalkThrough::generate_camera_path(const easy3d::Line3 &axis) {
    camera_->setSceneRadius(scene_box_.radius() * 1.1f);
    rotate_axis_ = axis;

    const float object_height = scene_box_.max_coord(2);

    const auto up = -rotate_axis_.direction(); // picking line points inside screen
    const auto vertical_offset = rotate_vertical_offset_factor_ * object_height * up;
    const auto at = rotate_axis_.projection(camera_->sceneCenter() + vertical_offset);
    const auto dist_to_axis = scene_box_.diagonal_length() + rotate_zoom_out_factor_ * object_height;
    const auto relative_cam0 = geom::orthogonal(up).normalize() * dist_to_axis;

    const auto pitch_angle = geom::to_radians(rotate_pitch_angle_);
    const auto pitch_offset = dist_to_axis * std::tan(pitch_angle);

    const auto angle_step = static_cast<float>(2.0 * M_PI / rotate_keyframe_samples_);
    for (int i=0; i<rotate_keyframe_samples_ * rotate_num_loops_; ++i) {
        easy3d::quat q(up, -angle_step * static_cast<float>(i)); // "-" for counterclockwise rotation
        const auto relative_cam = q.rotate(relative_cam0);
        Camera cam;
        cam.setPosition(at + relative_cam);
        cam.lookAt(at + up * pitch_offset);

//        cam.setUpVector(up); // if no pitch

        // the up vector changes w.r.t. pitch angle
        const auto ortho = cross(relative_cam, up).normalize(); // the rotation axis for camera pitch
        q.set_axis_angle(ortho, -pitch_angle);
        cam.setUpVector(q.rotate(up));

        add_keyframe(*cam.frame(), true);
    }

    LOG(INFO) << interpolator()->number_of_keyframes() << " key frames added to camera path";
}


void WalkThrough::set_zoom_out_factor(float v, bool re_generate) {
    rotate_zoom_out_factor_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && interpolator()->number_of_keyframes() > 0) {
        interpolator()->delete_path();
        generate_camera_path(rotate_axis_);
    }
}


void WalkThrough::set_vertical_offset_factor(float v, bool re_generate) {
    rotate_vertical_offset_factor_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && interpolator()->number_of_keyframes() > 0) {
        interpolator()->delete_path();
        generate_camera_path(rotate_axis_);
    }
}


void WalkThrough::set_pitch_angle(float v, bool re_generate) {
    rotate_pitch_angle_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && interpolator()->number_of_keyframes() > 0) {
        interpolator()->delete_path();
        generate_camera_path(rotate_axis_);
    }
}


void WalkThrough::set_keyframe_samples(int v, bool re_generate) {
    rotate_keyframe_samples_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && interpolator()->number_of_keyframes() > 0) {
        interpolator()->delete_path();
        generate_camera_path(rotate_axis_);
    }
}


void WalkThrough::set_num_loops(int v, bool re_generate) {
    rotate_num_loops_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && interpolator()->number_of_keyframes() > 0) {
        interpolator()->delete_path();
        generate_camera_path(rotate_axis_);
    }
}


void WalkThrough::delete_last_keyframe() {
    if (kfi_->number_of_keyframes() == 0) {
        current_frame_idx_ = -1;
        return;
    }

    kfi_->delete_last_keyframe();

    if (follow_up_)
        move_to(static_cast<int>(kfi_->number_of_keyframes() - 1));
}


void WalkThrough::delete_path() {
    kfi_->delete_path();
    current_frame_idx_ = -1;
}


void WalkThrough::preview() {
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    else
        kfi_->start_interpolation();
}


int WalkThrough::move_to(int idx, bool animation /* = true */, float duration /* = 0.5f */) {
    if (kfi_->number_of_keyframes() == 0 || idx < 0 || idx >= kfi_->number_of_keyframes())
        return current_frame_idx_;

    const auto frame = kfi_->keyframe(idx);
    if (animation)
        camera_->interpolateTo(frame, duration);
    else
        camera_->frame()->setPositionAndOrientation(frame.position(), frame.orientation());

    current_frame_idx_ = idx;
    path_modified.send();

    vec3 view_dir = frame.inverseTransformOf(vec3(0.0, 0.0, -1.0));
    view_dir.z = 0;
    view_dir.normalize();

    vec3 foot =
            frame.position() + view_dir * third_person_forward_distance() * 2.0f - vec3(0, 0, 1) * character_height();
    camera_->setPivotPoint(foot);

    return current_frame_idx_;
}


void WalkThrough::draw() const {
    if (cameras_visible())
        kfi_->draw_cameras(camera_, character_height() * 0.2f);
    if (path_visible())
        kfi_->draw_path(camera_, 2.0f);
}


void WalkThrough::add_keyframe(const vec3 &cam_pos, const vec3 &view_dir) {
    vec3 dir = view_dir;
    dir.z = 0; // force looking at horizontal direction
    dir.normalize();

    add_keyframe(Frame(cam_pos, to_orientation(dir)));

    if (follow_up_)
        move_to(static_cast<int>(kfi_->number_of_keyframes() - 1)); // move to the new view point.
}


float WalkThrough::character_height() const {
    if (scene_box_.is_valid())
        return scene_box_.range(2) * height_factor_;
    else
        return camera_->sceneRadius() * height_factor_;
}


float WalkThrough::third_person_forward_distance() const {
    return character_height() * third_person_forward_factor_;
}


vec3 WalkThrough::character_head(const vec3 &pos) const {
    return pos + ground_plane_normal_ * character_height();
}


quat WalkThrough::to_orientation(const vec3 &view_dir) const {
    const vec3 &up_dir = ground_plane_normal_;
    vec3 xAxis = cross(view_dir, up_dir);
    if (xAxis.length2() < epsilon_sqr<float>()) {    // target is aligned with this upVector, meaning a rotation around the X axis.
        // The X axis is then unchanged, so let's keep it!
        // rightVector() == camera_->frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
        xAxis = camera_->rightVector();
    }

    quat orient;
    orient.set_from_rotated_basis(xAxis, up_dir, -view_dir);
    return orient;
}
