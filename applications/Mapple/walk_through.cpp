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

#include "walk_through.h"

#include <cmath>

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

WalkThrough::WalkThrough(Camera *camera, KeyFrameInterpolator *interpolator)
        : camera_(camera), kfi_(interpolator), status_(STOPPED), ground_plane_normal_(0, 0, 1),
          follow_up_(true), height_factor_(0.2f), third_person_forward_factor_(1.8f),
          rotate_zoom_out_factor_(0.0f), rotate_vertical_offset_factor_(0.0f), rotate_pitch_angle_(0.0f),
          rotate_keyframe_samples_(10), rotate_num_loops_(2), current_frame_idx_(-1) {
}

void WalkThrough::set_scene(const std::vector<std::shared_ptr<Model>> &scene) {
    if (scene.empty())
        return;
    Box3 box;
    for (const auto &m : scene) {
        if (m->renderer() && m->renderer()->is_visible())
            box += m->bounding_box();
    }
    scene_box_ = box;
    camera_->set_scene_radius(box.radius() * 1.1f);
}

void WalkThrough::walk_to(const vec3 &ground_point) {
    if (status_ != WALKING_MODE) {
        LOG(WARNING) << "wrong status: walk_to() is for the walking mode only";
        return;
    }

    const vec3 head = character_head(ground_point);
    vec3 view_dir = head - camera_->position(); // approximate (when no previous keyframes exist)
    if (kfi_->number_of_keyframes() > 0) {
        const std::size_t prev_idx = kfi_->number_of_keyframes() - 1;
        view_dir = head - kfi_->keyframe_position(prev_idx);
    }
    view_dir.z = 0;
    view_dir.normalize();

    vec3 cam_pos = head;    // if not follow up, we want to get what we see
    if (follow_up_)
        cam_pos = cam_pos - view_dir * third_person_forward_distance(); // move back

    // adjust the previous view direction to be pointing to the new one
    if (kfi_->number_of_keyframes() > 0) {
        const std::size_t last_idx = kfi_->number_of_keyframes() - 1;
        vec3 dir = cam_pos - kfi_->keyframe_position(last_idx);
        dir.z = 0;
        dir.normalize();
        kfi_->set_keyframe_orientation(last_idx, to_orientation(dir));
    }

    add_keyframe(cam_pos, view_dir);

    // set the pivot point ahead of the character.
    camera_->set_pivot_point(ground_point + view_dir * third_person_forward_distance() * 2.0f);
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

void WalkThrough::generate_camera_path(const Line3 &axis) {
    camera_->set_scene_radius(scene_box_.radius() * 1.1f);
    rotate_axis_ = axis;

    const float object_height = scene_box_.max_coord(2);

    const auto up = -rotate_axis_.direction(); // picking line points inside screen
    const auto vertical_offset = rotate_vertical_offset_factor_ * object_height * up;
    const auto at = rotate_axis_.projection(camera_->scene_center() + vertical_offset);
    const auto dist_to_axis = scene_box_.diagonal_length() + rotate_zoom_out_factor_ * object_height;
    const auto relative_cam0 = geom::orthogonal(up).normalize() * dist_to_axis;

    const auto pitch_angle = geom::to_radians(rotate_pitch_angle_);
    const auto pitch_offset = dist_to_axis * std::tan(pitch_angle);

    const auto angle_step = static_cast<float>(2.0 * M_PI / rotate_keyframe_samples_);
    for (int i = 0; i < rotate_keyframe_samples_ * rotate_num_loops_; ++i) {
        quat q(up, -angle_step * static_cast<float>(i)); // "-" for counterclockwise rotation
        const auto relative_cam = q.rotate(relative_cam0);
        Camera cam;
        cam.set_position(at + relative_cam);
        cam.look_at(at + up * pitch_offset);

        // the up vector changes w.r.t. the pitch angle
        const auto ortho = cross(relative_cam, up).normalize(); // the rotation axis for camera pitch
        q.set_axis_angle(ortho, -pitch_angle);
        cam.set_up_vector(q.rotate(up));

        add_keyframe(*cam.frame());
    }

    LOG(INFO) << kfi_->number_of_keyframes() << " key frames added to camera path";
}

void WalkThrough::set_zoom_out_factor(float v, bool re_generate) {
    rotate_zoom_out_factor_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void WalkThrough::set_vertical_offset_factor(float v, bool re_generate) {
    rotate_vertical_offset_factor_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void WalkThrough::set_pitch_angle(float v, bool re_generate) {
    rotate_pitch_angle_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void WalkThrough::set_keyframe_samples(int v, bool re_generate) {
    rotate_keyframe_samples_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void WalkThrough::set_num_loops(int v, bool re_generate) {
    rotate_num_loops_ = v;
    if (re_generate && status_ == ROTATE_AROUND_AXIS && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void WalkThrough::delete_last_keyframe() {
    if (kfi_->number_of_keyframes() == 0) {
        current_frame_idx_ = -1;
        return;
    }

    kfi_->delete_last_keyframe();

    const int last = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    current_frame_idx_ = last;
    if (follow_up_ && last >= 0)
        move_to(last);
}

void WalkThrough::clear() {
    kfi_->delete_path();
    current_frame_idx_ = -1;
}

int WalkThrough::move_to(int idx) {
    if (kfi_->number_of_keyframes() == 0 || idx < 0 || idx >= kfi_->number_of_keyframes())
        return current_frame_idx_;

    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(idx));
    camera_->frame()->set_position_and_orientation(frame.position(), frame.orientation());

    current_frame_idx_ = idx;

    vec3 view_dir = frame.inverse_transform_of(vec3(0.0, 0.0, -1.0));
    view_dir.z = 0;
    view_dir.normalize();

    const vec3 foot = frame.position() + view_dir * third_person_forward_distance() * 2.0f -
                      vec3(0, 0, 1) * character_height();
    camera_->set_pivot_point(foot);

    return current_frame_idx_;
}

void WalkThrough::add_keyframe(const Frame &frame) {
    if (kfi_->add_keyframe(frame)) {
        // keep the new keyframe inside the view frustum
        const float dist = distance(camera_->scene_center(), frame.position());
        if (dist > camera_->scene_radius())
            camera_->set_scene_radius(dist);
    }
}

void WalkThrough::add_keyframe(const vec3 &cam_pos, const vec3 &view_dir) {
    vec3 dir = view_dir;
    dir.z = 0; // force looking in a horizontal direction
    dir.normalize();

    add_keyframe(Frame(cam_pos, to_orientation(dir)));

    if (follow_up_)
        move_to(static_cast<int>(kfi_->number_of_keyframes()) - 1); // move to the new view point
}

float WalkThrough::character_height() const {
    if (scene_box_.is_valid())
        return scene_box_.range(2) * height_factor_;
    return camera_->scene_radius() * height_factor_;
}

float WalkThrough::third_person_forward_distance() const {
    return character_height() * third_person_forward_factor_;
}

vec3 WalkThrough::character_head(const vec3 &pos) const {
    return pos + ground_plane_normal_ * character_height();
}

quat WalkThrough::to_orientation(const vec3 &view_dir) const {
    const vec3 &up_dir = ground_plane_normal_;
    vec3 x_axis = cross(view_dir, up_dir);
    if (x_axis.length2() < epsilon_sqr<float>()) // the view is aligned with the up vector
        x_axis = camera_->right_vector();

    quat orient;
    orient.set_from_rotated_basis(x_axis, up_dir, -view_dir);
    return orient;
}
