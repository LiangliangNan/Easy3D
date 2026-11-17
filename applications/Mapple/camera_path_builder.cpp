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

#include "camera_path_builder.h"

#include <cmath>

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

CameraPathBuilder::CameraPathBuilder(Camera *camera, KeyFrameInterpolator *interpolator)
        : camera_(camera), kfi_(interpolator), mode_(Mode::Off), ground_plane_normal_(0, 0, 1),
          follow_up_(true), eye_height_in_scene_heights_(0.2f), eye_distance_in_character_heights_(1.8f),
          orbit_radius_offset_in_object_heights_(0.0f), orbit_vertical_offset_in_object_heights_(0.0f),
          pitch_degrees_(0.0f), keyframes_per_loop_(10), num_loops_(2), current_frame_idx_(-1) {
}

void CameraPathBuilder::set_scene(const std::vector<std::shared_ptr<Model>> &scene) {
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

void CameraPathBuilder::walk_to(const vec3 &ground_point) {
    if (mode_ != Mode::GroundWalk) {
        LOG(WARNING) << "wrong status: walk_to() is for the walking mode only";
        return;
    }

    const vec3 head = eye_position(ground_point);
    vec3 view_dir = head - camera_->position(); // approximate (when no previous keyframes exist)
    if (kfi_->number_of_keyframes() > 0) {
        const std::size_t prev_idx = kfi_->number_of_keyframes() - 1;
        view_dir = head - kfi_->keyframe_position(prev_idx);
    }
    view_dir.z = 0;
    view_dir.normalize();

    vec3 cam_pos = head;    // if not follow up, we want to get what we see
    if (follow_up_)
        cam_pos = cam_pos - view_dir * eye_distance(); // move back

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
    camera_->set_pivot_point(ground_point + view_dir * eye_distance() * 2.0f);
}

void CameraPathBuilder::set_eye_height_in_scene_heights(float height_in_scene_heights) {
    eye_height_in_scene_heights_ = height_in_scene_heights;
    if (follow_up_)
        move_to(current_frame_idx_);
}

void CameraPathBuilder::set_eye_distance_in_character_heights(float distance_in_character_heights) {
    eye_distance_in_character_heights_ = distance_in_character_heights;
    if (follow_up_)
        move_to(current_frame_idx_);
}

void CameraPathBuilder::generate_camera_path(const Line3 &axis) {
    camera_->set_scene_radius(scene_box_.radius() * 1.1f);
    rotate_axis_ = axis;

    for (const auto &frame : orbit_frames(axis, camera_->scene_center()))
        add_keyframe(frame);

    LOG(INFO) << kfi_->number_of_keyframes() << " key frames added to camera path";
}

std::vector<Frame> CameraPathBuilder::orbit_frames(const Line3 &axis, const vec3 &center) const {
    std::vector<Frame> frames;
    if (!scene_box_.is_valid() || keyframes_per_loop_ <= 0 || num_loops_ <= 0)
        return frames;

    const auto up = -axis.direction(); // picking line points inside screen
    // The orbit offsets are relative to the object height: the extent of the
    // scene box along the axis (z range for an upright scene, like the walking mode).
    const auto extent = scene_box_.max_point() - scene_box_.min_point();
    const float object_height =
            std::abs(up.x) * extent.x + std::abs(up.y) * extent.y + std::abs(up.z) * extent.z;

    const auto vertical_offset = orbit_vertical_offset_in_object_heights_ * object_height * up;
    const auto at = axis.projection(center + vertical_offset);
    const auto dist_to_axis =
            scene_box_.diagonal_length() + orbit_radius_offset_in_object_heights_ * object_height;
    const auto relative_cam0 = geom::orthogonal(up).normalize() * dist_to_axis;

    const auto pitch_angle = geom::to_radians(pitch_degrees_);
    const auto pitch_offset = dist_to_axis * std::tan(pitch_angle);

    const int steps = keyframes_per_loop_ * num_loops_;
    const auto angle_step = static_cast<float>(2.0 * M_PI / keyframes_per_loop_);
    frames.reserve(static_cast<std::size_t>(steps) + 1);
    // The last pose repeats the first: the path closes the requested number of
    // loops instead of stopping one step short of the start.
    for (int i = 0; i <= steps; ++i) {
        quat q(up, -angle_step * static_cast<float>(i)); // "-" for counterclockwise rotation
        const auto relative_cam = q.rotate(relative_cam0);
        Camera cam;
        cam.set_position(at + relative_cam);
        cam.look_at(at + up * pitch_offset);

        // the up vector changes w.r.t. the pitch angle
        const auto ortho = cross(relative_cam, up).normalize(); // the rotation axis for camera pitch
        q.set_axis_angle(ortho, -pitch_angle);
        cam.set_up_vector(q.rotate(up));

        frames.emplace_back(*cam.frame());
    }
    return frames;
}

std::vector<Frame> CameraPathBuilder::ground_frames(const std::vector<vec3> &feet, const vec3 &up,
                                              float eye_height) const {
    std::vector<Frame> frames;
    if (feet.size() < 2)
        return frames;

    vec3 normal = up;
    if (normal.length2() < epsilon_sqr<float>())
        return frames;
    normal.normalize();

    // In-plane heading that stays continuous: the central difference of the
    // neighbours, with repeated waypoints skipped and an arbitrary in-plane
    // heading as the last resort.
    auto heading = [&](size_t i) -> vec3 {
        const vec3 &a = feet[i > 0 ? i - 1 : i];
        const vec3 &b = feet[i + 1 < feet.size() ? i + 1 : i];
        vec3 d = b - a;
        if (d.length2() > epsilon_sqr<float>())
            return d;
        for (size_t j = i + 1; j < feet.size(); ++j) {
            d = feet[j] - feet[i];
            if (d.length2() > epsilon_sqr<float>())
                return d;
        }
        for (size_t j = i; j-- > 0;) {
            d = feet[i] - feet[j];
            if (d.length2() > epsilon_sqr<float>())
                return d;
        }
        return geom::orthogonal(normal);
    };

    frames.reserve(feet.size());
    for (size_t i = 0; i < feet.size(); ++i) {
        vec3 dir = heading(i);
        dir -= normal * dot(dir, normal);
        dir.normalize();
        frames.emplace_back(feet[i] + normal * eye_height, to_orientation(dir, normal));
    }
    return frames;
}

std::vector<Frame> CameraPathBuilder::flight_frames(const std::vector<vec3> &positions,
                                              const std::vector<vec3> &targets, const vec3 &up) const {
    std::vector<Frame> frames;
    if (positions.size() < 2 || positions.size() != targets.size())
        return frames;

    frames.reserve(positions.size());
    for (size_t i = 0; i < positions.size(); ++i)
        frames.emplace_back(positions[i], look_at_orientation(positions[i], targets[i], up));
    return frames;
}

void CameraPathBuilder::set_orbit_radius_offset_in_object_heights(float offset_in_object_heights,
                                                                  bool re_generate) {
    orbit_radius_offset_in_object_heights_ = offset_in_object_heights;
    if (re_generate && mode_ == Mode::Orbit && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void CameraPathBuilder::set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights,
                                                                    bool re_generate) {
    orbit_vertical_offset_in_object_heights_ = offset_in_object_heights;
    if (re_generate && mode_ == Mode::Orbit && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void CameraPathBuilder::set_pitch_degrees(float degrees, bool re_generate) {
    pitch_degrees_ = degrees;
    if (re_generate && mode_ == Mode::Orbit && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void CameraPathBuilder::set_keyframes_per_loop(int count, bool re_generate) {
    keyframes_per_loop_ = count;
    if (re_generate && mode_ == Mode::Orbit && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void CameraPathBuilder::set_num_loops(int count, bool re_generate) {
    num_loops_ = count;
    if (re_generate && mode_ == Mode::Orbit && kfi_->number_of_keyframes() > 0) {
        kfi_->delete_path();
        generate_camera_path(rotate_axis_);
    }
}

void CameraPathBuilder::delete_last_keyframe() {
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

void CameraPathBuilder::clear() {
    kfi_->delete_path();
    current_frame_idx_ = -1;
}

int CameraPathBuilder::move_to(int idx) {
    if (kfi_->number_of_keyframes() == 0 || idx < 0 || idx >= kfi_->number_of_keyframes())
        return current_frame_idx_;

    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(idx));
    camera_->frame()->set_position_and_orientation(frame.position(), frame.orientation());

    current_frame_idx_ = idx;

    vec3 view_dir = frame.inverse_transform_of(vec3(0.0, 0.0, -1.0));
    view_dir.z = 0;
    view_dir.normalize();

    const vec3 foot = frame.position() + view_dir * eye_distance() * 2.0f -
                      vec3(0, 0, 1) * eye_height();
    camera_->set_pivot_point(foot);

    return current_frame_idx_;
}

void CameraPathBuilder::add_keyframe(const Frame &frame) {
    if (kfi_->add_keyframe(frame)) {
        // keep the new keyframe inside the view frustum
        const float dist = distance(camera_->scene_center(), frame.position());
        if (dist > camera_->scene_radius())
            camera_->set_scene_radius(dist);
    }
}

void CameraPathBuilder::add_keyframe(const vec3 &cam_pos, const vec3 &view_dir) {
    vec3 dir = view_dir;
    dir.z = 0; // force looking in a horizontal direction
    dir.normalize();

    add_keyframe(Frame(cam_pos, to_orientation(dir)));

    if (follow_up_)
        move_to(static_cast<int>(kfi_->number_of_keyframes()) - 1); // move to the new view point
}

float CameraPathBuilder::eye_height() const {
    if (scene_box_.is_valid())
        return scene_box_.range(2) * eye_height_in_scene_heights_;
    return camera_->scene_radius() * eye_height_in_scene_heights_;
}

float CameraPathBuilder::eye_distance() const {
    return eye_height() * eye_distance_in_character_heights_;
}

vec3 CameraPathBuilder::eye_position(const vec3 &pos) const {
    return pos + ground_plane_normal_ * eye_height();
}

quat CameraPathBuilder::to_orientation(const vec3 &view_dir) const {
    return to_orientation(view_dir, ground_plane_normal_);
}

quat CameraPathBuilder::to_orientation(const vec3 &view_dir, const vec3 &up) {
    vec3 up_dir = up;
    up_dir.normalize();
    vec3 dir = view_dir;
    dir.normalize();

    vec3 x_axis = cross(dir, up_dir);
    if (x_axis.length2() < epsilon_sqr<float>()) // the view is aligned with the up vector
        x_axis = geom::orthogonal(up_dir);

    quat orient;
    orient.set_from_rotated_basis(x_axis, up_dir, -dir);
    return orient;
}

quat CameraPathBuilder::look_at_orientation(const vec3 &position, const vec3 &target, const vec3 &up) {
    vec3 up_dir = up;
    if (up_dir.length2() < epsilon_sqr<float>())
        up_dir = vec3(0, 0, 1);
    up_dir.normalize();

    vec3 view_dir = target - position;
    if (view_dir.length2() < epsilon_sqr<float>()) // no aim direction: look down the up reference
        view_dir = -up_dir;
    view_dir.normalize();

    // Roll is referenced to `up`, so the camera up vector is the up reference
    // orthogonalized against the view direction.
    vec3 right = cross(view_dir, up_dir);
    if (right.length2() < epsilon_sqr<float>()) // looking along the up reference
        right = geom::orthogonal(view_dir);
    right.normalize();
    const vec3 cam_up = cross(right, view_dir).normalize();

    quat orient;
    orient.set_from_rotated_basis(right, cam_up, -view_dir);
    return orient;
}
