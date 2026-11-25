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

#include "keyframe_editor.h"

#include <algorithm>
#include <cmath>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/keyframe_interpolator.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/util/setting.h>

using namespace easy3d;

namespace mapple {

KeyframeEditor::KeyframeEditor(Camera *camera, KeyframeInterpolator *interpolator)
        : camera_(camera), kfi_(interpolator) {
}

void KeyframeEditor::set_glyph_width(float width) {
    glyph_width_ = width;
}

void KeyframeEditor::set_editing(bool editing) {
    editing_ = editing;
    if (!editing_)
        reset_drag();
}

int KeyframeEditor::selected() const {
    if (!kfi_ || selected_ < 0)
        return -1;
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    return std::min(selected_, count - 1);
}

void KeyframeEditor::set_selected(int index) {
    if (!kfi_ || index < 0) {
        selected_ = -1;
        return;
    }
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    selected_ = count > 0 ? std::min(index, count - 1) : -1;
}

bool KeyframeEditor::editable() const {
    return editing_ && kfi_ && !kfi_->is_interpolation_started() && selected() >= 0;
}

int KeyframeEditor::editable_keyframe() const {
    return editable() ? selected() : -1;
}

KeyframeEditor::Handle KeyframeEditor::handle_at(int x, int y) const {
    const int index = editable_keyframe();
    if (index < 0 || !(glyph_width_ > 0.0f))
        return Handle::none;
    return KeyframeGizmo::handle_at(camera_, kfi_->keyframe(static_cast<std::size_t>(index)), glyph_width_,
                                    static_cast<float>(x), static_cast<float>(y));
}

bool KeyframeEditor::update_hover(int x, int y) {
    Handle hovered = Handle::none;
    if (x >= 0 && y >= 0 && editable() && dragged_keyframe_ < 0)
        hovered = handle_at(x, y);
    if (hovered == hovered_handle_)
        return false;
    hovered_handle_ = hovered;
    return true;
}

bool KeyframeEditor::press(int x, int y, bool upright_plane, const std::function<int(int, int)> &keyframe_at) {
    if (!editable())
        return false;

    drag_moved_ = false;
    hovered_handle_ = Handle::none;

    // A gizmo handle owns the press before the glyph's picking: the handles are
    // screen-space, so the test is cheap, takes precedence where they overlap the
    // glyph, and needs the keyframe to be selected already.
    Handle handle = handle_at(x, y);
    int index = selected();
    if (handle != Handle::none) {
        const Frame start = kfi_->keyframe(static_cast<std::size_t>(index));
        drag_start_position_ = start.position();
        drag_start_orientation_ = start.orientation();
        drag_plane_point_ = start.position();
        const bool turn = KeyframeGizmo::is_turn(handle);
        const int axis_index = KeyframeGizmo::axis_of(handle);
        drag_axis_ = normalize(start.orientation().rotate(
                axis_index == 0 ? vec3(1, 0, 0) : axis_index == 1 ? vec3(0, 1, 0) : vec3(0, 0, 1)));
        if (turn) {
            drag_angle_ = 0.0f;
            if (!KeyframeGizmo::ring_angle_at(camera_, static_cast<float>(x), static_cast<float>(y),
                                              drag_plane_point_, drag_axis_, drag_last_angle_))
                return false;
        } else {
            if (!KeyframeGizmo::axis_parameter_at(camera_, static_cast<float>(x), static_cast<float>(y),
                                                  drag_plane_point_, drag_axis_, drag_start_scalar_))
                return false;
        }
        dragged_keyframe_ = index;
        handle_ = handle;
        return true;
    }

    const int glyph = keyframe_at ? keyframe_at(x, y) : -1;
    if (glyph < 0 || glyph >= static_cast<int>(kfi_->number_of_keyframes()))
        return false;

    set_selected(glyph);
    dragged_keyframe_ = glyph;
    handle_ = Handle::free;
    const Frame start = kfi_->keyframe(static_cast<std::size_t>(glyph));
    drag_start_position_ = start.position();
    drag_start_orientation_ = start.orientation();
    drag_plane_point_ = drag_start_position_;
    if (upright_plane) {
        // The pose's own up: the terrain plane for a walking pose, the orbit plane
        // for a turntable pose.
        drag_plane_normal_ = start.orientation().rotate(vec3(0.0f, 1.0f, 0.0f));
        if (drag_plane_normal_.length2() < epsilon_sqr<float>()) {
            reset_drag();
            return false;
        }
        drag_plane_normal_.normalize();
    } else // the screen plane through the pose: the drag follows the cursor
        drag_plane_normal_ = camera_->view_direction();

    // Keep the station where it was under the cursor: the vector from the press's
    // plane point to the station is added back to every drag intersection.
    drag_grab_offset_ = vec3(0.0f);
    {
        vec3 orig, dir;
        camera_->convert_click_to_line(x, y, orig, dir);
        const float denom = dot(dir, drag_plane_normal_);
        if (std::abs(denom) > 1e-5f) {
            const float t = dot(drag_plane_point_ - orig, drag_plane_normal_) / denom;
            if (t > 0.0f)
                drag_grab_offset_ = drag_start_position_ - (orig + dir * t);
        }
    }
    return true;
}

void KeyframeEditor::drag(int x, int y) {
    if (dragged_keyframe_ < 0 || dragged_keyframe_ >= static_cast<int>(kfi_->number_of_keyframes()))
        return;
    const auto index = static_cast<std::size_t>(dragged_keyframe_);

    if (handle_ == Handle::none) // nothing grabbed
        return;

    // An arrow moves the pose along its axis: the cursor ray selects the parameter on
    // the axis line, so the pose follows the cursor without drifting off the axis.
    if (handle_ >= Handle::move_x && handle_ <= Handle::move_z) {
        float parameter;
        if (!KeyframeGizmo::axis_parameter_at(camera_, static_cast<float>(x), static_cast<float>(y),
                                              drag_plane_point_, drag_axis_, parameter))
            return;
        const vec3 position = drag_start_position_ + drag_axis_ * (parameter - drag_start_scalar_);
        if (distance(position, drag_start_position_) < epsilon<float>())
            return;
        if (!drag_moved_) {
            // The gesture becomes an edit - and the undo point - only once it moves.
            if (begin_edit)
                begin_edit();
            drag_moved_ = true;
        }
        kfi_->set_keyframe_position(index, position);
        const float dist = distance(camera_->scene_center(), position);
        if (dist > camera_->scene_radius())
            camera_->set_scene_radius(dist);
        return;
    }

    // A ring turns the pose around its axis: the accumulated cursor angle around the
    // ring's center is applied to the orientation captured at the press, so the pose
    // does not drift when the cursor crosses the ring's seam.
    if (KeyframeGizmo::is_turn(handle_)) {
        float angle;
        if (!KeyframeGizmo::ring_angle_at(camera_, static_cast<float>(x), static_cast<float>(y),
                                          drag_plane_point_, drag_axis_, angle))
            return;
        const float delta = std::remainder(angle - drag_last_angle_, 2.0f * static_cast<float>(M_PI));
        drag_last_angle_ = angle;
        drag_angle_ += delta;
        if (std::abs(drag_angle_) < 1e-6f)
            return;
        if (!drag_moved_) {
            if (begin_edit)
                begin_edit();
            drag_moved_ = true;
        }
        const quat turn(drag_axis_, drag_angle_);
        kfi_->set_keyframe_orientation(index, turn * drag_start_orientation_);
        return;
    }

    // The glyph itself: intersection with the plane fixed at the press.
    vec3 orig, dir;
    camera_->convert_click_to_line(x, y, orig, dir);
    const float denom = dot(dir, drag_plane_normal_);
    // The plane is seen edge-on (the pose's up points at the camera): no stable
    // intersection, so keep the last position.
    if (std::abs(denom) < 1e-5f)
        return;
    const float t = dot(drag_plane_point_ - orig, drag_plane_normal_) / denom;
    if (t <= 0.0f)
        return;
    // The station follows the cursor from where the glyph was grabbed, not from the
    // cursor itself: without the offset the pose jumps to the clicked point.
    const vec3 position = orig + dir * t + drag_grab_offset_;
    if (distance(position, drag_start_position_) < epsilon<float>())
        return;

    if (!drag_moved_) {
        // The gesture becomes an edit - and the undo point - only once it moves.
        if (begin_edit)
            begin_edit();
        drag_moved_ = true;
    }
    kfi_->set_keyframe_position(index, position);
    // Keep the moved pose inside the view frustum, as adding a keyframe does.
    const float dist = distance(camera_->scene_center(), position);
    if (dist > camera_->scene_radius())
        camera_->set_scene_radius(dist);
}

KeyframeEditor::DragResult KeyframeEditor::release() {
    DragResult result;
    result.keyframe = dragged_keyframe_;
    result.moved = drag_moved_;
    result.turned = KeyframeGizmo::is_turn(handle_);
    reset_drag();
    return result;
}

void KeyframeEditor::cancel() {
    if (dragged_keyframe_ >= 0 && drag_moved_ && kfi_ &&
        dragged_keyframe_ < static_cast<int>(kfi_->number_of_keyframes())) {
        const auto index = static_cast<std::size_t>(dragged_keyframe_);
        kfi_->set_keyframe_position(index, drag_start_position_);
        kfi_->set_keyframe_orientation(index, drag_start_orientation_);
    }
    reset_drag();
}

void KeyframeEditor::reset_drag() {
    dragged_keyframe_ = -1;
    handle_ = Handle::none;
    hovered_handle_ = Handle::none;
    drag_moved_ = false;
}

bool KeyframeEditor::aim_at(const vec3 &target) {
    const int index = editable_keyframe();
    if (index < 0)
        return false;

    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(index));
    vec3 direction = target - frame.position();
    if (direction.length2() <= epsilon_sqr<float>())
        return false;
    direction.normalize();

    // The least turn that points the pose's -Z (what Camera::view_direction() is for
    // the live camera) at the target. The pose's own up preserves its roll; a target
    // straight along that up leaves no side vector, so the pose's X axis is kept, the
    // same fallback as Camera::set_view_direction().
    vec3 right = cross(direction, frame.orientation().rotate(vec3(0.0f, 1.0f, 0.0f)));
    if (right.length2() <= epsilon_sqr<float>())
        right = frame.orientation().rotate(vec3(1.0f, 0.0f, 0.0f));
    quat orientation;
    orientation.set_from_rotated_basis(right, cross(right, direction), -direction);

    if (begin_edit)
        begin_edit();
    kfi_->set_keyframe_orientation(static_cast<std::size_t>(index), orientation);
    return true;
}

int KeyframeEditor::insert_after_selected() {
    if (!kfi_)
        return -1;
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    if (count == 0)
        return -1;
    const int after = selected() >= 0 ? selected() : count - 1;

    // Halfway to the next keyframe, so the section keeps its timing and shape
    // between the two neighbours; past the last keyframe the last segment is
    // extended, so the new glyph does not land on top of an existing one.
    Frame frame;
    float time = 0.0f;
    if (after + 1 < count) {
        const Frame a = kfi_->keyframe(static_cast<std::size_t>(after));
        const Frame b = kfi_->keyframe(static_cast<std::size_t>(after + 1));
        frame.set_position_and_orientation(0.5f * (a.position() + b.position()),
                                           quat::slerp(a.orientation(), b.orientation(), 0.5f));
        time = 0.5f * (kfi_->keyframe_time(static_cast<std::size_t>(after)) +
                       kfi_->keyframe_time(static_cast<std::size_t>(after + 1)));
    } else {
        const Frame last = kfi_->keyframe(static_cast<std::size_t>(after));
        vec3 position = last.position();
        float gap = 1.0f;
        if (after > 0) {
            position += last.position() - kfi_->keyframe_position(static_cast<std::size_t>(after - 1));
            gap = kfi_->keyframe_time(static_cast<std::size_t>(after)) -
                  kfi_->keyframe_time(static_cast<std::size_t>(after - 1));
        }
        frame.set_position_and_orientation(position, last.orientation());
        time = kfi_->keyframe_time(static_cast<std::size_t>(after)) + std::max(gap, 1e-3f);
    }

    std::vector<Frame> frames;
    std::vector<float> times;
    std::vector<float> dwells;
    frames.reserve(count + 1);
    times.reserve(count + 1);
    dwells.reserve(count + 1);
    for (int i = 0; i < after + 1; ++i) {
        frames.push_back(kfi_->keyframe(static_cast<std::size_t>(i)));
        times.push_back(kfi_->keyframe_time(static_cast<std::size_t>(i)));
        dwells.push_back(kfi_->keyframe_dwell(static_cast<std::size_t>(i)));
    }
    frames.push_back(frame);
    times.push_back(time);
    dwells.push_back(0.0f);   // a new keyframe holds nothing until it is given a dwell
    for (int i = after + 1; i < count; ++i) {
        frames.push_back(kfi_->keyframe(static_cast<std::size_t>(i)));
        times.push_back(kfi_->keyframe_time(static_cast<std::size_t>(i)));
        dwells.push_back(kfi_->keyframe_dwell(static_cast<std::size_t>(i)));
    }

    if (begin_edit)
        begin_edit();
    if (!rebuild(frames, times, dwells))
        return -1;
    set_selected(after + 1);
    return after + 1;
}

int KeyframeEditor::delete_selected() {
    if (!kfi_)
        return -1;
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    const int index = selected();
    if (count == 0 || index < 0)
        return -1;

    std::vector<Frame> frames;
    std::vector<float> times;
    std::vector<float> dwells;
    frames.reserve(count - 1);
    times.reserve(count - 1);
    dwells.reserve(count - 1);
    for (int i = 0; i < count; ++i) {
        if (i == index)
            continue;
        frames.push_back(kfi_->keyframe(static_cast<std::size_t>(i)));
        times.push_back(kfi_->keyframe_time(static_cast<std::size_t>(i)));
        dwells.push_back(kfi_->keyframe_dwell(static_cast<std::size_t>(i)));
    }

    if (begin_edit)
        begin_edit();
    if (!rebuild(frames, times, dwells))
        return -1;
    set_selected(frames.empty() ? -1 : index);
    return index;
}

bool KeyframeEditor::rebuild(const std::vector<Frame> &frames, const std::vector<float> &times,
                             const std::vector<float> &dwells) {
    if (!kfi_ || frames.size() != times.size())
        return false;
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    const bool periodic = kfi_->is_path_periodic();
    kfi_->delete_path();
    for (std::size_t i = 0; i < frames.size(); ++i) {
        if (!kfi_->add_keyframe(frames[i], times[i]))
            return false;
        // The dwell belongs to the keyframe, so a path rebuilt from kept keyframes
        // keeps their pauses; a shorter dwells list simply leaves the rest at 0.
        if (i < dwells.size())
            kfi_->set_keyframe_dwell(i, dwells[i]);
    }
    kfi_->set_path_periodic(periodic);
    return true;
}

void KeyframeEditor::append_overlay(std::vector<graphics::DrawCommand> &commands,
                                    unsigned fb_width, unsigned fb_height, float dpi) const {
    const int index = editable_keyframe();
    if (index < 0 || fb_width == 0 || fb_height == 0)
        return;

    // The handle Insert/Delete act on: a camera-facing square around the pose's
    // position (the glyph itself is the scene geometry being edited). It is real
    // world-space geometry depth tested against the scene, so the glyph and other
    // objects hide the part of it behind them. With the camera standing on this
    // pose, the station is at the eye and has no screen position, so it is skipped.
    const vec3 station = kfi_->keyframe_position(static_cast<std::size_t>(index));
    vec2 projected;
    if (!KeyframeGizmo::project_to_screen(camera_, station, projected))
        return;
    const mat4 mvp = camera_->model_view_projection_matrix();
    const float pixel = KeyframeGizmo::pixel_size_at(camera_, station);
    if (pixel > 0.0f) {
        const float r = 7.0f * pixel;
        const vec3 right = camera_->right_vector() * r;
        const vec3 up = camera_->up_vector() * r;
        const vec3 corners[4] = {station - right - up, station + right - up,
                                 station + right + up, station - right + up};
        std::vector<vec3> segments;
        segments.reserve(8);
        for (int i = 0; i < 4; ++i) {
            segments.push_back(corners[i]);
            segments.push_back(corners[(i + 1) % 4]);
        }
        graphics::DrawCommand square = graphics::shape::draw_lines_3d(
                segments, std::vector<vec3>(segments.size(), vec3(1.0f)), mvp, fb_width, fb_height, 2.0f * dpi);
        square.color = setting::highlight_color;
        commands.push_back(square);
    }

    // The pose's aim: its stored orientation applied to the camera-space -Z axis (what
    // Camera::view_direction() does for the live camera), drawn for one scene radius.
    // Dimmed so it does not compete with the square and the gizmo, and depth tested so
    // it disappears behind the scene like a real ray.
    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(index));
    const vec3 aim_direction = frame.orientation().rotate(vec3(0.0f, 0.0f, -1.0f));
    const vec4 aim(setting::highlight_color.x, setting::highlight_color.y, setting::highlight_color.z, 0.6f);
    graphics::DrawCommand ray = graphics::shape::draw_lines_3d(
            {station, station + aim_direction * camera_->scene_radius()},
            std::vector<vec3>(2, vec3(1.0f)), mvp, fb_width, fb_height, 2.0f * dpi);
    ray.color = aim;
    commands.push_back(ray);

    KeyframeGizmo::append_overlay(commands, camera_, frame, glyph_width_,
                                  dragged_keyframe_ >= 0 ? handle_ : hovered_handle_,
                                  fb_width, fb_height, dpi);
}

} // namespace mapple
