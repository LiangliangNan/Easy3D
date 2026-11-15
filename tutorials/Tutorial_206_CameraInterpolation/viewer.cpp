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
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/shape.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>

using namespace easy3d;

// \cond

namespace {

    // Window (logical pixel) distances within which a gizmo line can be grabbed. Rings are drawn a bit
    // wider apart than the arrows, so they get a slightly smaller tolerance to keep the two separable.
    constexpr float k_arrow_tolerance = 10.0f;
    constexpr float k_ring_tolerance = 9.0f;
    // Radius of the "drag the camera itself" zone around the glyph apex, and the ring size limits
    // (logical pixels) for extreme zoom levels.
    constexpr float k_free_tolerance = 10.0f;
    constexpr float k_min_ring_radius = 14.0f;
    constexpr float k_max_ring_radius = 140.0f;
    // Ring radius relative to the keyframe camera glyph, and relative to the arrow length. The rings sit
    // outside the arrows so a shaft cannot be crossed by another axis' ring.
    constexpr float k_ring_scale = 1.4f;
    constexpr float k_ring_arrow_ratio = 1.4f;
    constexpr float k_two_pi = 6.283185307179586f;

    float distance_to_segment(const vec2 &p, const vec2 &a, const vec2 &b) {
        const vec2 ab = b - a;
        const float len2 = dot(ab, ab);
        float t = 0.0f;
        if (len2 > 1e-6f)
            t = std::max(0.0f, std::min(1.0f, dot(p - a, ab) / len2));
        return length(p - (a + t * ab));
    }

}

// \endcond

TutorialCameraInterpolation::TutorialCameraInterpolation(const std::string &title) : Viewer(title) {
    // Draw the recorded path and keyframe cameras in the scene so the user can see the route.
    show_camera_path_ = true;
    // hint_ is shown in the viewer HUD; it lists the keys this tutorial adds.
    hint_ =
        "Click a green keyframe camera to edit it\n"
        "Drag the camera itself to move it freely\n"
        "Drag an arrow to move along one axis, a ring to rotate\n"
        "Press 'k' to add key frames\n"
        "Press 'space' to start/stop the animation\n"
        "Press 'd' to delete the camera path\n"
        "Press 'Esc' to stop editing";
}

// Friendly key bindings layered on top of the Viewer's built-in camera-path machinery.
bool TutorialCameraInterpolation::key_press_event(int key, int modifiers) {
    if (key == KEY_ESCAPE && modifiers == 0 && selected_ >= 0) {
        // End editing; the keyframe keeps its edited pose.
        clear_selection();
        return true;
    }

    if (key == KEY_K && modifiers == 0) {
        // Record the camera's current pose (position + orientation) as a new keyframe.
        easy3d::Frame *frame = camera()->frame();
        if (kfi_->add_keyframe(*frame)) {
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
        // 'space' is an alias for the Viewer's Ctrl+K shortcut, which starts/stops playback
        // of the interpolated camera path. Playback hides the keyframe cameras, so end editing.
        if (selected_ >= 0)
            clear_selection();
        Viewer::key_press_event(KEY_K, MODIF_CTRL);
        return true;
    } else if (key == KEY_D && modifiers == 0) {
        clear_selection();
        // Clear all keyframes...
        kfi_->delete_path();
        // ...and shrink the scene bounding box back to the models (it may have been grown to
        // enclose keyframes that no longer exist).
        // update scene bounding box
        Box3 box;
        for (auto m : models_)
            box.grow(m->bounding_box());
        if (box.is_valid())
            camera_->set_scene_bounding_box(box.min_point(), box.max_point());
        std::cout << "path deleted" << std::endl;
        return true;
    } else
        // Unhandled keys fall through to the base Viewer (navigation, snapshots, ...).
        return Viewer::key_press_event(key, modifiers);
}

bool TutorialCameraInterpolation::mouse_press_event(int x, int y, int button, int modifiers) {
    // The keyframe cameras and their gizmo are only on screen when the path is shown and not playing.
    if (button == BUTTON_LEFT && show_camera_path_ && !kfi_->is_interpolation_started()) {
        if (selected_ >= 0) {
            Handle handle = pick_handle(x, y);
            // Pressing the selected camera away from a gizmo handle drags the camera itself.
            if (handle == Handle::NONE && pick_keyframe(x, y) == selected_)
                handle = Handle::MOVE_FREE;
            if (handle != Handle::NONE) {
                begin_drag(handle, x, y);
                if (dragged_ != Handle::NONE) {
                    rebuild_gizmo();
                    update();
                    return true;
                }
            }
        }
        const int hit = pick_keyframe(x, y);
        if (hit >= 0) {
            select_keyframe(hit);
            return true;
        }
        // A click on empty space ends editing and falls through to the base camera controls.
        if (selected_ >= 0)
            clear_selection();
    }
    return Viewer::mouse_press_event(x, y, button, modifiers);
}

bool TutorialCameraInterpolation::mouse_release_event(int x, int y, int button, int modifiers) {
    if (dragged_ != Handle::NONE) {
        // The press was consumed, so the base viewer never started a camera gesture.
        dragged_ = Handle::NONE;
        hovered_ = pick_handle(x, y);
        rebuild_gizmo();
        update();
        return true;
    }
    return Viewer::mouse_release_event(x, y, button, modifiers);
}

bool TutorialCameraInterpolation::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (dragged_ != Handle::NONE && button == BUTTON_LEFT && selected_ >= 0) {
        if (dragged_ == Handle::MOVE_FREE) {
            // Free translation: the keyframe keeps following the cursor in the screen plane captured at
            // the press, so it lands exactly where the mouse points, whatever the viewing angle.
            vec3 hit;
            if (screen_plane_hit(drag_start_position_, drag_plane_normal_, x, y, hit)) {
                move_keyframe(drag_start_position_ + (hit - drag_start_hit_));
                rebuild_gizmo();
                update();
            }
            return true;
        }

        const int axis = axis_index(dragged_);
        const vec3 direction = axis_direction(axis);

        if (is_translation(dragged_)) {
            // Move along the axis: follow the point of the axis line closest to the mouse ray.
            float t = 0.0f;
            if (axis_parameter(drag_start_position_, direction, x, y, t)) {
                move_keyframe(drag_start_position_ + (t - drag_axis_parameter_) * direction);
                rebuild_gizmo(); // the whole gizmo moves with the keyframe
                update();
            }
        } else {
            // Rotate around the axis: accumulate the angle swept on the ring plane and apply it to
            // the pose captured at the press (world-space rotation, so the keyframe position is fixed).
            float angle = 0.0f;
            if (ring_angle(drag_start_position_, axis, x, y, angle)) {
                float delta = angle - drag_last_angle_;
                delta = std::atan2(std::sin(delta), std::cos(delta)); // unwrap to (-pi, pi]
                drag_last_angle_ = angle;
                drag_total_angle_ += delta;
                const quat rotation = quat(direction, drag_total_angle_);
                kfi_->set_keyframe_orientation(selected_, rotation * drag_start_orientation_);
                update();
            }
        }
        return true;
    }
    return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}

bool TutorialCameraInterpolation::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    // Highlight the handle under the cursor so the available drag directions are discoverable.
    if (dragged_ == Handle::NONE && selected_ >= 0) {
        const Handle handle = pick_handle(x, y);
        if (handle != hovered_) {
            hovered_ = handle;
            rebuild_gizmo();
            update();
        }
    }
    return Viewer::mouse_free_move_event(x, y, dx, dy, modifiers);
}

void TutorialCameraInterpolation::pre_draw() {
    Viewer::pre_draw();

    if (selected_ >= static_cast<int>(kfi_->number_of_keyframes())) {
        // The path disappeared behind our back (e.g., the base viewer's Ctrl+Alt+D shortcut).
        selected_ = -1;
        hovered_ = dragged_ = Handle::NONE;
    }

    if (selected_ < 0)
        return;

    if (!show_camera_path_ || kfi_->is_interpolation_started()) {
        if (gizmo_)
            gizmo_->set_visible(false);
        return;
    }

    // The gizmo size follows the projected keyframe camera glyph, so it must be rebuilt when the view
    // camera moves or zooms enough to change the glyph's apparent size.
    const float length = gizmo_length();
    if (gizmo_length_ <= 0.0f || std::fabs(length - gizmo_length_) > 0.02f * gizmo_length_)
        rebuild_gizmo();
    else if (gizmo_)
        gizmo_->set_visible(true);
}

void TutorialCameraInterpolation::end_gesture() {
    // Only reset the drag state here: this also runs while the viewer tears down (its drawables are
    // already cleared then), so touching the gizmo or the camera would be unsafe.
    dragged_ = Handle::NONE;
    Viewer::end_gesture();
}

void TutorialCameraInterpolation::select_keyframe(int index) {
    if (index < 0 || index >= static_cast<int>(kfi_->number_of_keyframes()))
        return;
    selected_ = index;
    hovered_ = Handle::NONE;
    dragged_ = Handle::NONE;
    gizmo_length_ = 0.0f;
    rebuild_gizmo();
    update();
    std::cout << "editing keyframe " << (index + 1) << " of " << kfi_->number_of_keyframes() << std::endl;
}

void TutorialCameraInterpolation::clear_selection() {
    if (selected_ < 0)
        return;
    selected_ = -1;
    hovered_ = dragged_ = Handle::NONE;
    gizmo_length_ = 0.0f;
    if (gizmo_)
        gizmo_->set_visible(false);
    update();
}

void TutorialCameraInterpolation::move_keyframe(const vec3 &position) {
    kfi_->set_keyframe_position(selected_, position);
    // Keep the moved keyframe inside the scene sphere: its radius sets clipping and the glyph size.
    const float dist = distance(camera_->scene_center(), position);
    if (dist > camera_->scene_radius())
        camera_->set_scene_radius(dist);
}

int TutorialCameraInterpolation::pick_keyframe(int x, int y) const {
    if (!show_camera_path_ || kfi_->is_interpolation_started() || kfi_->number_of_keyframes() == 0)
        return -1;

    // Reproduce what the base viewer draws: one wireframe camera per keyframe, sized relative to
    // the scene and using the current camera's field of view and aspect.
    std::vector<vec3> glyph;
    shape::create_camera(glyph, camera()->scene_radius() * 0.05f, camera()->field_of_view(),
                         static_cast<float>(camera()->screen_height()) /
                         static_cast<float>(camera()->screen_width()));

    const vec2 mouse(static_cast<float>(x), static_cast<float>(y));
    int best = -1;
    float best_distance = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < kfi_->number_of_keyframes(); ++i) {
        const vec3 position = kfi_->keyframe_position(i);
        vec2 apex;
        if (!project(position, apex))
            continue; // the camera glyph's apex is behind the eye

        const mat4 transform = Frame(position, kfi_->keyframe_orientation(i)).matrix();
        float xmin = std::numeric_limits<float>::max();
        float ymin = xmin;
        float xmax = -xmin;
        float ymax = -xmin;
        bool visible = false;
        for (const vec3 &p : glyph) {
            vec2 screen;
            if (!project(transform * p, screen))
                continue;
            visible = true;
            xmin = std::min(xmin, screen.x);
            xmax = std::max(xmax, screen.x);
            ymin = std::min(ymin, screen.y);
            ymax = std::max(ymax, screen.y);
        }
        if (!visible)
            continue;

        const float pad = 3.0f; // the frustum is a wireframe; let the click land slightly outside it
        if (mouse.x < xmin - pad || mouse.x > xmax + pad || mouse.y < ymin - pad || mouse.y > ymax + pad)
            continue;

        const float distance = length(mouse - apex);
        if (distance < best_distance) {
            best_distance = distance;
            best = static_cast<int>(i);
        }
    }
    return best;
}

TutorialCameraInterpolation::Handle TutorialCameraInterpolation::pick_handle(int x, int y) const {
    if (selected_ < 0)
        return Handle::NONE;

    const vec2 mouse(static_cast<float>(x), static_cast<float>(y));
    const vec3 center = kfi_->keyframe_position(selected_);
    // At the glyph apex the arrows all meet; that spot grabs the camera itself so a plain drag moves
    // it in the screen plane instead of picking one axis arbitrarily. Keep the zone smaller than the
    // arrows when the gizmo is very small on screen.
    const float free_radius = std::min(k_free_tolerance, 0.4f * (ring_screen_radius() / k_ring_arrow_ratio));
    vec2 apex;
    if (project(center, apex) && length(mouse - apex) <= free_radius)
        return Handle::MOVE_FREE;

    // Ignore handles that cannot be followed from the current view (a shaft seen end-on or a ring
    // seen edge-on): highlighting them would promise a drag that does nothing.
    bool can_move[3], can_rotate[3];
    for (int axis = 0; axis < 3; ++axis) {
        float unused;
        can_move[axis] = axis_parameter(center, axis_direction(axis), x, y, unused);
        can_rotate[axis] = ring_angle(center, axis, x, y, unused);
    }

    Handle best = Handle::NONE;
    float best_score = 1.0f; // distance / tolerance; below 1 means the cursor is on the handle
    for (const Segment &segment : gizmo_segments()) {
        const int axis = axis_index(segment.handle);
        if (segment.ring ? !can_rotate[axis] : !can_move[axis])
            continue;
        vec2 a, b;
        if (!project(segment.a, a) || !project(segment.b, b))
            continue;
        const float tolerance = segment.ring ? k_ring_tolerance : k_arrow_tolerance;
        const float score = distance_to_segment(mouse, a, b) / tolerance;
        if (score < best_score) {
            best_score = score;
            best = segment.handle;
        }
    }
    return best;
}

bool TutorialCameraInterpolation::project(const vec3 &world, vec2 &screen) const {
    if (camera()->camera_coordinates_of(world).z >= 0.0f) // the camera looks along -Z
        return false;
    const vec3 projected = camera()->projected_coordinates_of(world);
    screen = vec2(projected.x, projected.y);
    return true;
}

float TutorialCameraInterpolation::glyph_screen_radius(int index) const {
    if (index < 0 || index >= static_cast<int>(kfi_->number_of_keyframes()))
        return 0.0f;

    const vec3 position = kfi_->keyframe_position(index);
    vec2 apex;
    if (!project(position, apex))
        return 0.0f;

    std::vector<vec3> glyph;
    shape::create_camera(glyph, camera()->scene_radius() * 0.05f, camera()->field_of_view(),
                         static_cast<float>(camera()->screen_height()) /
                         static_cast<float>(camera()->screen_width()));
    const mat4 transform = Frame(position, kfi_->keyframe_orientation(index)).matrix();
    float radius = 0.0f;
    for (const vec3 &p : glyph) {
        vec2 screen;
        if (project(transform * p, screen))
            radius = std::max(radius, length(screen - apex));
    }
    return radius;
}

float TutorialCameraInterpolation::ring_screen_radius() const {
    if (selected_ < 0)
        return 0.0f;
    // The rings sit just outside the drawn keyframe camera, so the gizmo reads as a frame around that
    // glyph instead of an unrelated overlay. Clamp for extreme zoom levels.
    return std::min(std::max(k_ring_scale * glyph_screen_radius(selected_), k_min_ring_radius),
                    k_max_ring_radius);
}

float TutorialCameraInterpolation::gizmo_length() const {
    if (selected_ < 0)
        return 0.0f;
    const float arrow = ring_screen_radius() / k_ring_arrow_ratio;
    return arrow * camera()->world_pixel_ratio(kfi_->keyframe_position(selected_));
}

std::vector<TutorialCameraInterpolation::Segment> TutorialCameraInterpolation::gizmo_segments() const {
    std::vector<Segment> segments;
    if (selected_ < 0)
        return segments;

    const vec3 center = kfi_->keyframe_position(selected_);
    const float length = (gizmo_length_ > 0.0f) ? gizmo_length_ : gizmo_length();
    // The rings sit outside the arrow tips: a ring inside the arrows would be crossed by the other
    // axes' shafts, and a click there could not be attributed to one handle.
    const float radius = k_ring_arrow_ratio * length;
    const Handle move_handles[3] = {Handle::MOVE_X, Handle::MOVE_Y, Handle::MOVE_Z};
    const Handle rotate_handles[3] = {Handle::ROTATE_X, Handle::ROTATE_Y, Handle::ROTATE_Z};

    for (int axis = 0; axis < 3; ++axis) {
        const vec3 direction = axis_direction(axis);
        const vec3 u = axis_direction((axis + 1) % 3);
        const vec3 v = axis_direction((axis + 2) % 3);

        // Translation arrow: the shaft crosses the rotation rings, the four head lines make the tip.
        const vec3 tip = center + length * direction;
        segments.push_back({center, tip, move_handles[axis], false});
        const vec3 back = tip - 0.17f * length * direction;
        const float half_width = 0.07f * length;
        segments.push_back({tip, back + half_width * u, move_handles[axis], false});
        segments.push_back({tip, back - half_width * u, move_handles[axis], false});
        segments.push_back({tip, back + half_width * v, move_handles[axis], false});
        segments.push_back({tip, back - half_width * v, move_handles[axis], false});

        // Rotation ring in the plane perpendicular to the axis.
        const int samples = 64;
        vec3 previous = center + radius * u;
        for (int i = 1; i <= samples; ++i) {
            const float angle = k_two_pi * static_cast<float>(i) / static_cast<float>(samples);
            const vec3 current = center + radius * (std::cos(angle) * u + std::sin(angle) * v);
            segments.push_back({previous, current, rotate_handles[axis], true});
            previous = current;
        }
    }
    return segments;
}

void TutorialCameraInterpolation::rebuild_gizmo() {
    if (!gizmo_) {
        gizmo_ = std::make_shared<LinesDrawable>("camera_keyframe_gizmo");
        gizmo_->set_line_width(3.0f);
        gizmo_->set_impostor_type(LinesDrawable::CYLINDER);
        // The buffer holds one color per vertex (axis color, dimmed for rings, bright for the active
        // handle), so the state must read vertex colors instead of its uniform color.
        gizmo_->set_property_coloring(State::VERTEX);
        add_drawable(gizmo_);
    }

    if (selected_ < 0) {
        gizmo_length_ = 0.0f;
        gizmo_->set_visible(false);
        return;
    }

    gizmo_length_ = gizmo_length();
    const std::vector<Segment> segments = gizmo_segments();
    const vec3 axis_colors[3] = {vec3(0.90f, 0.25f, 0.25f), vec3(0.25f, 0.85f, 0.30f), vec3(0.30f, 0.45f, 0.95f)};
    const Handle active = (dragged_ != Handle::NONE) ? dragged_ : hovered_;

    std::vector<vec3> vertices;
    std::vector<vec3> colors;
    vertices.reserve(segments.size() * 2);
    colors.reserve(segments.size() * 2);
    for (const Segment &segment : segments) {
        vec3 color = axis_colors[axis_index(segment.handle)];
        if (segment.ring)
            color *= 0.80f; // keep the rings visually behind the arrows
        if (segment.handle == active)
            color = vec3(1.0f, 0.90f, 0.15f); // the handle under the cursor or being dragged
        vertices.push_back(segment.a);
        colors.push_back(color);
        vertices.push_back(segment.b);
        colors.push_back(color);
    }

    gizmo_->update_vertex_buffer(vertices, true);
    gizmo_->update_color_buffer(colors, true);
    gizmo_->set_visible(show_camera_path_ && !kfi_->is_interpolation_started());
}

void TutorialCameraInterpolation::begin_drag(Handle handle, int x, int y) {
    const int axis = axis_index(handle);
    drag_start_position_ = kfi_->keyframe_position(selected_);
    drag_start_orientation_ = kfi_->keyframe_orientation(selected_);
    drag_total_angle_ = 0.0f;
    dragged_ = Handle::NONE;

    if (handle == Handle::MOVE_FREE) {
        // The screen plane through the keyframe; fixed at the press so the mapping stays consistent.
        drag_plane_normal_ = camera()->view_direction();
        if (screen_plane_hit(drag_start_position_, drag_plane_normal_, x, y, drag_start_hit_))
            dragged_ = handle;
    } else if (is_translation(handle)) {
        if (axis_parameter(drag_start_position_, axis_direction(axis), x, y, drag_axis_parameter_))
            dragged_ = handle;
    } else {
        if (ring_angle(drag_start_position_, axis, x, y, drag_last_angle_))
            dragged_ = handle;
    }
    hovered_ = (dragged_ != Handle::NONE) ? handle : Handle::NONE;
}

bool TutorialCameraInterpolation::screen_plane_hit(const vec3 &center, const vec3 &normal,
                                                   int x, int y, vec3 &hit) const {
    vec3 eye, direction;
    camera()->convert_click_to_line(x, y, eye, direction);
    // The plane faces the eye, so the ray is never parallel to it unless the click is degenerate.
    const float denominator = dot(direction, normal);
    if (std::fabs(denominator) < 1e-5f)
        return false;
    hit = eye + dot(center - eye, normal) / denominator * direction;
    return true;
}

bool TutorialCameraInterpolation::axis_parameter(const vec3 &origin, const vec3 &axis,
                                                 int x, int y, float &t) const {
    vec3 eye, direction;
    camera()->convert_click_to_line(x, y, eye, direction);

    // Closest point between the axis line (origin + t * axis) and the mouse ray (eye + s * direction).
    // The axis is a unit vector; when the view looks along it the intersection is ill-conditioned and
    // the handle should not be dragged along an arbitrary screen direction.
    const float b = dot(axis, direction);
    const float denominator = 1.0f - b * b;
    if (denominator < 1e-5f)
        return false;
    const vec3 offset = origin - eye;
    const float d = dot(axis, offset);
    const float e = dot(direction, offset);
    t = (b * e - d) / denominator;
    return true;
}

bool TutorialCameraInterpolation::ring_angle(const vec3 &center, int axis, int x, int y, float &angle) const {
    vec3 eye, direction;
    camera()->convert_click_to_line(x, y, eye, direction);

    // Intersect the mouse ray with the ring plane (through the keyframe, normal to the axis).
    const vec3 normal = axis_direction(axis);
    const float denominator = dot(direction, normal);
    if (std::fabs(denominator) < 1e-5f) // the ray runs parallel to the ring plane
        return false;
    const float s = dot(center - eye, normal) / denominator;
    const vec3 hit = eye + s * direction;
    const vec3 offset = hit - center;
    // Near edge-on, the ray grazes the ring plane and the hit lands far away; the angle would jump
    // wildly, so such a ring is not grabbable until the view is turned toward it.
    const float distance = length(offset);
    if (distance < 0.2f * gizmo_length() || distance > 2.0f * gizmo_length())
        return false;

    // The ring is spanned by the two other world axes, matching gizmo_segments().
    const vec3 u = axis_direction((axis + 1) % 3);
    const vec3 v = axis_direction((axis + 2) % 3);
    angle = std::atan2(dot(offset, v), dot(offset, u));
    return true;
}

int TutorialCameraInterpolation::axis_index(Handle handle) {
    switch (handle) {
        case Handle::MOVE_X:
        case Handle::ROTATE_X:
            return 0;
        case Handle::MOVE_Y:
        case Handle::ROTATE_Y:
            return 1;
        case Handle::MOVE_Z:
        case Handle::ROTATE_Z:
            return 2;
        default:
            return -1;
    }
}

bool TutorialCameraInterpolation::is_translation(Handle handle) {
    return handle == Handle::MOVE_X || handle == Handle::MOVE_Y || handle == Handle::MOVE_Z;
}

vec3 TutorialCameraInterpolation::axis_direction(int axis) {
    switch (axis) {
        case 0:
            return vec3(1.0f, 0.0f, 0.0f);
        case 1:
            return vec3(0.0f, 1.0f, 0.0f);
        default:
            return vec3(0.0f, 0.0f, 1.0f);
    }
}

// \endcond
