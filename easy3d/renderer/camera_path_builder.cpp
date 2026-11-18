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

#include <algorithm>
#include <cmath>

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>

namespace easy3d {

    namespace {
        // A point along a polyline: \p segment - 1 to \p segment, \p t of the way;
        // segment 0 with t 0 is the first point.
        struct ArcSample {
            std::size_t segment;
            float t;
        };

        // Samples a polyline at constant arc length. The first point, every \p spacing
        // sample and the stroke end are kept; a sample closer than a quarter spacing to
        // the end is dropped so the last keyframes do not crowd.
        std::vector<ArcSample> arc_length_samples(const std::vector<vec3> &points, float spacing) {
            std::vector<ArcSample> samples;
            if (points.empty())
                return samples;
            samples.push_back({0, 0.0f});
            if (points.size() < 2)
                return samples;

            std::vector<float> cumulative(points.size(), 0.0f);
            for (std::size_t i = 1; i < points.size(); ++i)
                cumulative[i] = cumulative[i - 1] + distance(points[i], points[i - 1]);
            const float total = cumulative.back();
            if (total <= 1e-6f)
                return samples;

            std::size_t segment = 1;
            for (float s = spacing; s < total - 0.25f * spacing; s += spacing) {
                while (segment < points.size() && cumulative[segment] < s)
                    ++segment;
                if (segment >= points.size())
                    break;
                const float length = cumulative[segment] - cumulative[segment - 1];
                const float t = length > 0.0f ? (s - cumulative[segment - 1]) / length : 0.0f;
                samples.push_back({segment, t});
            }
            samples.push_back({points.size() - 1, 1.0f});
            return samples;
        }

        // The value of a per-stroke-point quantity at a sample: linearly interpolated,
        // so positions and the walker's up directions share the resampling.
        vec3 sample_value(const std::vector<vec3> &values, const ArcSample &sample) {
            if (sample.segment == 0)
                return values.front();
            return values[sample.segment - 1] + (values[sample.segment] - values[sample.segment - 1]) * sample.t;
        }
    }

    CameraPathBuilder::CameraPathBuilder(Camera *camera, KeyFrameInterpolator *interpolator)
            : camera_(camera), kfi_(interpolator),
              eye_height_in_scene_heights_(0.2f), keyframe_spacing_in_character_heights_(0.5f),
              flight_height_in_scene_heights_(0.5f),
              orbit_radius_offset_in_object_heights_(0.0f), orbit_vertical_offset_in_object_heights_(0.0f),
              pitch_degrees_(0.0f), keyframes_per_loop_(10), num_loops_(2) {
    }

    void CameraPathBuilder::set_scene(const std::vector<std::shared_ptr<Model>> &scene) {
        Box3 box;
        for (const auto &m : scene) {
            if (m->renderer() && m->renderer()->is_visible())
                box += m->bounding_box();
        }
        scene_box_ = box;
    }

    void CameraPathBuilder::set_eye_height_in_scene_heights(float height_in_scene_heights) {
        eye_height_in_scene_heights_ = height_in_scene_heights;
    }

    void CameraPathBuilder::set_keyframe_spacing_in_character_heights(float spacing_in_character_heights) {
        keyframe_spacing_in_character_heights_ = spacing_in_character_heights;
    }

    vec3 CameraPathBuilder::ground_up_direction() const {
        // The fallback for poses whose surface supplies no normal: the view's up
        // direction, which is also what the eye height is measured against.
        vec3 up = camera_->up_vector();
        if (up.length2() < epsilon_sqr<float>())
            return vec3(0.0f, 0.0f, 1.0f);
        return up.normalize();
    }

    float CameraPathBuilder::scene_height_in_world_units() const {
        if (!scene_box_.is_valid())
            return camera_->scene_radius();

        // The scene "height" is its extent along the route's fallback up direction
        // (the view's up), so heights keep the same ratio in any view.
        const vec3 up = ground_up_direction();
        return std::abs(up.x) * scene_box_.range(0) +
               std::abs(up.y) * scene_box_.range(1) +
               std::abs(up.z) * scene_box_.range(2);
    }

    float CameraPathBuilder::eye_height_in_world_units() const {
        return scene_height_in_world_units() * eye_height_in_scene_heights_;
    }

    float CameraPathBuilder::flight_height_in_world_units() const {
        return scene_height_in_world_units() * flight_height_in_scene_heights_;
    }

    void CameraPathBuilder::set_flight_height_in_scene_heights(float height_in_scene_heights) {
        flight_height_in_scene_heights_ = height_in_scene_heights;
    }

    bool CameraPathBuilder::resample_stroke(const std::vector<vec3> &stroke,
                                            const std::vector<vec3> &surface_up_directions,
                                            float spacing, std::vector<vec3> &feet, std::vector<vec3> &ups) const {
        feet.clear();
        ups.clear();
        if (stroke.size() < 2)
            return false;
        if (!surface_up_directions.empty() && surface_up_directions.size() != stroke.size())
            return false;

        const std::vector<ArcSample> samples = arc_length_samples(stroke, spacing);
        const vec3 fallback_up = ground_up_direction();
        const vec3 view_dir = camera_->view_direction();

        // The face winding decides a normal's sign, but not the side of the surface the
        // walker stands on: that is the side facing the camera, so every up must oppose
        // the view direction. Sounding the eye offset off the far side would put the
        // camera centers behind the surface. The input is oriented before resampling so
        // arbitrary winding cannot cancel out in the interpolation.
        std::vector<vec3> directions = surface_up_directions;
        for (auto &up : directions) {
            if (up.length2() < epsilon_sqr<float>())
                up = fallback_up;
            up.normalize();
            if (dot(up, view_dir) > 0.0f)
                up = -up;
        }

        feet.reserve(samples.size());
        ups.reserve(samples.size());
        for (const auto &sample : samples) {
            feet.push_back(sample_value(stroke, sample));
            vec3 up = directions.empty() ? fallback_up : sample_value(directions, sample);
            if (up.length2() < epsilon_sqr<float>())
                up = fallback_up;
            up.normalize();
            ups.push_back(up);
        }

        // Picked facet normals step at facet edges; smoothing along the route keeps the
        // walker from tipping on a single keyframe.
        for (int pass = 0; pass < 2; ++pass) {
            std::vector<vec3> smoothed = ups;
            for (std::size_t i = 1; i + 1 < ups.size(); ++i) {
                vec3 n = ups[i - 1] + ups[i] * 2.0f + ups[i + 1];
                if (n.length2() > epsilon_sqr<float>())
                    smoothed[i] = n.normalize();
            }
            ups.swap(smoothed);
        }
        return true;
    }

    std::size_t CameraPathBuilder::generate_ground_route(const std::vector<vec3> &stroke,
                                                         const std::vector<vec3> &foot_up_directions) {
        const float eye = eye_height_in_world_units();
        if (eye <= 1e-4f)
            return 0;
        const float spacing = std::max(1e-4f, keyframe_spacing_in_character_heights_ * eye);

        std::vector<vec3> feet, ups;
        if (!resample_stroke(stroke, foot_up_directions, spacing, feet, ups))
            return 0;

        const std::vector<Frame> frames = ground_frames(feet, ups, eye);
        for (const auto &frame : frames)
            add_keyframe(frame);
        return frames.size();
    }

    std::vector<CameraPathBuilder::FlightStation>
    CameraPathBuilder::flight_stations(const std::vector<vec3> &stroke,
                                       const std::vector<vec3> &surface_up_directions) const {
        std::vector<FlightStation> stations;
        const float height = flight_height_in_world_units();
        if (height <= 1e-4f)
            return stations;
        const float spacing = std::max(1e-4f, keyframe_spacing_in_character_heights_ * height);

        std::vector<vec3> feet, ups;
        if (!resample_stroke(stroke, surface_up_directions, spacing, feet, ups))
            return stations;

        stations.reserve(feet.size());
        for (std::size_t i = 0; i < feet.size(); ++i)
            stations.push_back(FlightStation{feet[i] + ups[i] * height, feet[i]});
        return stations;
    }

    std::vector<vec3> CameraPathBuilder::aim_targets(const std::vector<vec3> &stroke, std::size_t count) const {
        std::vector<vec3> targets;
        if (stroke.empty() || count == 0)
            return targets;
        targets.assign(count, stroke.front());

        std::vector<float> cumulative(stroke.size(), 0.0f);
        for (std::size_t i = 1; i < stroke.size(); ++i)
            cumulative[i] = cumulative[i - 1] + distance(stroke[i], stroke[i - 1]);
        const float total = cumulative.back();
        if (stroke.size() < 2 || total <= 1e-6f)
            return targets;

        std::size_t segment = 1;
        for (std::size_t i = 0; i < count; ++i) {
            const float fraction = count == 1 ? 0.5f : static_cast<float>(i) / static_cast<float>(count - 1);
            const float s = fraction * total;
            while (segment < stroke.size() && cumulative[segment] < s)
                ++segment;
            if (segment >= stroke.size())
                segment = stroke.size() - 1;
            const float length = cumulative[segment] - cumulative[segment - 1];
            const float t = length > 0.0f ? (s - cumulative[segment - 1]) / length : 0.0f;
            targets[i] = stroke[segment - 1] + (stroke[segment] - stroke[segment - 1]) * t;
        }
        return targets;
    }

    std::size_t CameraPathBuilder::generate_flight(const std::vector<FlightStation> &stations,
                                                   const std::vector<vec3> &targets) {
        const std::vector<Frame> frames = flight_frames(stations, targets, ground_up_direction());
        for (const auto &frame : frames)
            add_keyframe(frame);
        return frames.size();
    }

    std::size_t CameraPathBuilder::generate_orbit(const Line3 &axis, const vec3 &center) {
        const std::vector<Frame> frames = orbit_frames(axis, center);
        for (const auto &frame : frames)
            add_keyframe(frame);
        return frames.size();
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

    std::vector<Frame> CameraPathBuilder::ground_frames(const std::vector<vec3> &feet,
                                                        const std::vector<vec3> &up_directions,
                                                        float eye_height) const {
        std::vector<Frame> frames;
        if (feet.size() < 2 || up_directions.size() != feet.size())
            return frames;
        for (const auto &up : up_directions) {
            if (up.length2() < epsilon_sqr<float>())
                return frames;
        }

        // In-plane heading that stays continuous: the central difference of the
        // neighbours, with repeated waypoints skipped and an arbitrary in-plane
        // heading as the last resort.
        auto heading = [&](size_t i, const vec3 &up) -> vec3 {
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
            return geom::orthogonal(up);
        };

        frames.reserve(feet.size());
        for (size_t i = 0; i < feet.size(); ++i) {
            vec3 up = up_directions[i];
            up.normalize();
            vec3 dir = heading(i, up);
            dir -= up * dot(dir, up);
            // A near-vertical step (steep terrain) leaves no in-plane heading.
            if (dir.length2() < epsilon_sqr<float>())
                dir = geom::orthogonal(up);
            dir.normalize();
            frames.emplace_back(feet[i] + up * eye_height, to_orientation(dir, up));
        }
        return frames;
    }

    std::vector<Frame> CameraPathBuilder::flight_frames(const std::vector<FlightStation> &stations,
                                                        const std::vector<vec3> &targets, const vec3 &up) const {
        std::vector<Frame> frames;
        if (stations.size() < 2 || (!targets.empty() && stations.size() != targets.size()))
            return frames;

        frames.reserve(stations.size());
        for (size_t i = 0; i < stations.size(); ++i) {
            // No targets: the default aim, back down at the foot the station rose from.
            const vec3 target = targets.empty() ? stations[i].foot : targets[i];
            frames.emplace_back(stations[i].position, look_at_orientation(stations[i].position, target, up));
        }
        return frames;
    }

    std::vector<Frame> CameraPathBuilder::aimed_frames(const std::vector<Frame> &frames,
                                                       const std::vector<vec3> &targets,
                                                       const std::vector<vec3> &roll_ups) const {
        std::vector<Frame> aimed;
        if (frames.empty() || frames.size() != targets.size())
            return aimed;
        if (!roll_ups.empty() && roll_ups.size() != frames.size())
            return aimed;

        aimed.reserve(frames.size());
        for (size_t i = 0; i < frames.size(); ++i) {
            // Keep each pose's roll unless the caller supplies the mode's reference.
            const vec3 up = roll_ups.empty()
                                ? frames[i].orientation().rotate(vec3(0.0f, 1.0f, 0.0f))
                                : roll_ups[i];
            aimed.emplace_back(frames[i].position(),
                               look_at_orientation(frames[i].position(), targets[i], up));
        }
        return aimed;
    }

    void CameraPathBuilder::set_orbit_radius_offset_in_object_heights(float offset_in_object_heights) {
        orbit_radius_offset_in_object_heights_ = offset_in_object_heights;
    }

    void CameraPathBuilder::set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights) {
        orbit_vertical_offset_in_object_heights_ = offset_in_object_heights;
    }

    void CameraPathBuilder::set_pitch_degrees(float degrees) {
        pitch_degrees_ = degrees;
    }

    void CameraPathBuilder::set_keyframes_per_loop(int count) {
        keyframes_per_loop_ = count;
    }

    void CameraPathBuilder::set_num_loops(int count) {
        num_loops_ = count;
    }

    void CameraPathBuilder::add_keyframe(const Frame &frame) {
        if (kfi_->add_keyframe(frame)) {
            // keep the new keyframe inside the view frustum
            const float dist = distance(camera_->scene_center(), frame.position());
            if (dist > camera_->scene_radius())
                camera_->set_scene_radius(dist);
        }
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

} // namespace easy3d
