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

#include "keyframe_gizmo.h"

#include <cmath>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>

namespace easy3d {

namespace {

    // Distance, in logical pixels, from p to the segment [a, b].
    float screen_segment_distance(const vec2 &p, const vec2 &a, const vec2 &b) {
        const vec2 ab = b - a;
        const float length2 = ab.length2();
        if (length2 < 1e-6f)
            return distance(p, a);
        const float t = clamp(dot(p - a, ab) / length2, 0.0f, 1.0f);
        return distance(p, a + ab * t);
    }

    // An orthonormal basis of the plane orthogonal to axis, chosen the same way for
    // every call so press and drag angles are comparable.
    void plane_basis(const vec3 &axis, vec3 &u, vec3 &v) {
        const vec3 helper = std::abs(axis.z) < 0.9f ? vec3(0, 0, 1) : vec3(1, 0, 0);
        u = normalize(cross(axis, helper));
        v = cross(axis, u);
    }

} // namespace

bool KeyframeGizmo::project_to_screen(const Camera *camera, const vec3 &point, vec2 &screen) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
        return false;
    if (camera->type() == Camera::PERSPECTIVE) {
        const float depth = dot(point - camera->position(), camera->view_direction());
        if (!(depth > 0.5f * camera->z_near()))
            return false;
    }
    const vec3 p = camera->projected_coordinates_of(point);
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || p.z < 0.0f || p.z > 1.0f)
        return false;
    screen = vec2(p.x, p.y);
    return true;
}

float KeyframeGizmo::pixel_size_at(const Camera *camera, const vec3 &point) {
    vec2 origin, offset;
    if (!project_to_screen(camera, point, origin) ||
        !project_to_screen(camera, point + camera->right_vector(), offset))
        return 0.0f;
    const float pixels_per_unit = distance(origin, offset);
    return pixels_per_unit > 1e-6f ? 1.0f / pixels_per_unit : 0.0f;
}

KeyframeGizmo::Geometry KeyframeGizmo::geometry(const Frame &frame, float glyph_width) {
    Geometry geometry;
    const quat &orientation = frame.orientation();
    geometry.axes[0] = normalize(orientation.rotate(vec3(1, 0, 0)));
    geometry.axes[1] = normalize(orientation.rotate(vec3(0, 1, 0)));
    geometry.axes[2] = normalize(orientation.rotate(vec3(0, 0, 1)));
    geometry.arrow_length = 1.25f * glyph_width;
    geometry.ring_radius = 0.75f * glyph_width;
    return geometry;
}

bool KeyframeGizmo::is_turn(Handle handle) {
    return handle >= Handle::turn_x && handle <= Handle::turn_z;
}

int KeyframeGizmo::axis_of(Handle handle) {
    if (handle >= Handle::move_x && handle <= Handle::move_z)
        return static_cast<int>(handle) - static_cast<int>(Handle::move_x);
    if (is_turn(handle))
        return static_cast<int>(handle) - static_cast<int>(Handle::turn_x);
    return -1;
}

bool KeyframeGizmo::axis_parameter_at(const Camera *camera, float x, float y, const vec3 &origin,
                                      const vec3 &axis, float &parameter) {
    vec3 ray_origin, ray_direction;
    camera->convert_click_to_line(static_cast<int>(x), static_cast<int>(y), ray_origin, ray_direction);
    const float b = dot(axis, ray_direction);
    const float denom = 1.0f - b * b; // axis and ray direction are unit vectors
    if (denom < 1e-4f)
        return false;
    const vec3 r = origin - ray_origin;
    parameter = (b * dot(ray_direction, r) - dot(axis, r)) / denom;
    return true;
}

bool KeyframeGizmo::ring_angle_at(const Camera *camera, float x, float y, const vec3 &center, const vec3 &axis,
                                  float &angle) {
    vec3 ray_origin, ray_direction;
    camera->convert_click_to_line(static_cast<int>(x), static_cast<int>(y), ray_origin, ray_direction);
    const float denom = dot(ray_direction, axis);
    if (std::abs(denom) < 1e-5f)
        return false;
    const float t = dot(center - ray_origin, axis) / denom;
    if (t <= 0.0f)
        return false;
    const vec3 hit = ray_origin + ray_direction * t;
    vec3 u, v;
    plane_basis(axis, u, v);
    angle = std::atan2(dot(hit - center, v), dot(hit - center, u));
    return true;
}

KeyframeGizmo::Handle KeyframeGizmo::handle_at(const Camera *camera, const Frame &frame, float glyph_width,
                                               float x, float y) {
    const Geometry g = geometry(frame, glyph_width);
    if (!(g.arrow_length > 0.0f) || !(g.ring_radius > 0.0f))
        return Handle::none;
    const vec3 center = frame.position();
    const vec3 view = camera->view_direction();
    const vec2 cursor(x, y);
    const float tolerance = 7.0f; // logical pixels

    // With the camera on the station the gizmo is not drawn either, so nothing of it
    // can be grabbed.
    vec2 center_screen;
    if (!project_to_screen(camera, center, center_screen))
        return Handle::none;

    float best = tolerance;
    Handle result = Handle::none;

    // Arrows first: at the ring's radius the shaft passes close to the ring, and the
    // straight handle is the more predictable grab there.
    for (int i = 0; i < 3; ++i) {
        const vec3 &axis = g.axes[i];
        if (std::abs(dot(axis, view)) > 0.97f) // seen end-on: no stable parameter
            continue;
        vec2 start, end;
        if (!project_to_screen(camera, center, start) ||
            !project_to_screen(camera, center + axis * g.arrow_length, end))
            continue;
        if (distance(start, end) < 10.0f) // too short to grab, even off the end-on case
            continue;
        const float d = screen_segment_distance(cursor, start, end);
        if (d <= best) {
            best = d;
            result = static_cast<Handle>(static_cast<int>(Handle::move_x) + i);
        }
    }

    // Then the rings, accepted only when their plane is not seen edge-on.
    for (int i = 0; i < 3; ++i) {
        const vec3 &axis = g.axes[i];
        if (std::abs(dot(axis, view)) < 0.2f) // edge-on: a flat line, ambiguous to grab
            continue;
        vec3 u, v;
        plane_basis(axis, u, v);
        const int segments = 64;
        vec2 previous;
        bool valid = project_to_screen(camera, center + (u * g.ring_radius), previous);
        for (int s = 1; valid && s <= segments; ++s) {
            const float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(s) / segments;
            vec2 current;
            valid = project_to_screen(
                    camera, center + (u * std::cos(angle) + v * std::sin(angle)) * g.ring_radius, current);
            if (!valid)
                break;
            const float d = screen_segment_distance(cursor, previous, current);
            if (d < best) {
                best = d;
                result = static_cast<Handle>(static_cast<int>(Handle::turn_x) + i);
            }
            previous = current;
        }
    }
    return result;
}

void KeyframeGizmo::append_overlay(std::vector<graphics::DrawCommand> &commands, const Camera *camera,
                                   const Frame &frame, float glyph_width, Handle active,
                                   unsigned fb_width, unsigned fb_height, float dpi) {
    if (fb_width == 0 || fb_height == 0)
        return;

    const Geometry g = geometry(frame, glyph_width);
    if (!(g.arrow_length > 0.0f) || !(g.ring_radius > 0.0f))
        return;

    const vec3 center = frame.position();
    const vec3 view = camera->view_direction();
    // Standing on the station (e.g. after Prev/Next) leaves no screen position for
    // the center: the handles cannot be drawn meaningfully from inside the glyph.
    vec2 center_screen;
    if (!project_to_screen(camera, center, center_screen))
        return;
    const mat4 mvp = camera->model_view_projection_matrix();
    // Axis colors, so an arrow and its ring read as the same axis.
    const vec4 colors[3] = {vec4(1.0f, 0.0f, 0.0f, 1.0f),
                            vec4(0.0f, 1.0f, 0.0f, 1.0f),
                            vec4(0.0f, 0.0f, 1.0f, 1.0f)};

    auto handle_for = [](bool turn, int axis) {
        return turn ? static_cast<Handle>(static_cast<int>(Handle::turn_x) + axis)
                    : static_cast<Handle>(static_cast<int>(Handle::move_x) + axis);
    };
    // The handles are world-space ribbons depth tested against the scene, so the parts
    // behind the camera glyph or other geometry are hidden instead of floating over it.
    // The grabbed or hovered element is drawn full and thick; the rest dims, so the
    // handle a gesture will act on stays unmistakable.
    auto emit = [&](bool turn, int axis, const std::vector<vec3> &segments) {
        if (segments.size() < 2)
            return;
        const bool hot = handle_for(turn, axis) == active;
        const bool dim = active != Handle::none && !hot;
        vec4 color = colors[axis];
        color.w = dim ? 0.25f : hot ? 1.0f : 0.8f;
        graphics::DrawCommand command = graphics::shape::draw_lines_3d(
                segments, std::vector<vec3>(segments.size(), vec3(1.0f)), mvp, fb_width, fb_height,
                (hot ? 4.5f : 3.0f) * dpi);
        command.color = color;
        commands.push_back(command);
    };

    // Arrows: the shaft plus a camera-facing head drawn at the tip.
    for (int i = 0; i < 3; ++i) {
        vec2 start, tip_screen;
        const vec3 tip = center + g.axes[i] * g.arrow_length;
        if (!project_to_screen(camera, center, start) ||
            !project_to_screen(camera, tip, tip_screen))
            continue;
        if (distance(start, tip_screen) < 10.0f) // seen end-on: no stable grab and no readable arrow
            continue;
        // The head plane spans the axis and the screen-space side of the axis, so the
        // arrow stays readable from any direction.
        vec3 side = cross(g.axes[i], view);
        if (side.length2() < 1e-8f)
            continue;
        side.normalize();
        const float pixel = pixel_size_at(camera, tip);
        const float head = pixel > 0.0f ? std::min(0.4f * g.arrow_length, 12.0f * pixel)
                                        : 0.4f * g.arrow_length;
        const vec3 back = tip - g.axes[i] * head;
        const vec3 wing = side * (head * 0.45f);
        emit(false, i, {center, tip, tip, back + wing, tip, back - wing});
    }

    // Rings: only when their plane is not seen edge-on.
    for (int i = 0; i < 3; ++i) {
        if (std::abs(dot(g.axes[i], view)) < 0.2f)
            continue;
        vec3 u, v;
        plane_basis(g.axes[i], u, v);
        const int sample_count = 64;
        std::vector<vec3> segments;
        segments.reserve(sample_count * 2);
        vec3 previous = center + u * g.ring_radius;
        for (int s = 1; s <= sample_count; ++s) {
            const float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(s) /
                                static_cast<float>(sample_count);
            const vec3 current = center + (u * std::cos(angle) + v * std::sin(angle)) * g.ring_radius;
            segments.push_back(previous);
            segments.push_back(current);
            previous = current;
        }
        emit(true, i, segments);
    }
}

} // namespace easy3d
