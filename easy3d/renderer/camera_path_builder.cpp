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
#include <utility>

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

        // Samples a polyline at constant arc length, both ends included: the requested
        // spacing decides how many intervals the stroke has (rounded to the nearest
        // whole one) and the samples share the stroke evenly. A stroke whose length is
        // not a multiple of the spacing is therefore still sampled uniformly, with no
        // cramped last interval.
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

            const int intervals = std::max(1, static_cast<int>(std::lround(total / spacing)));
            std::size_t segment = 1;
            for (int i = 1; i < intervals; ++i) {
                const float s = total * static_cast<float>(i) / static_cast<float>(intervals);
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
            : camera_(camera), kfi_(interpolator) {
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
        settings_.eye_height_in_scene_heights = height_in_scene_heights;
    }

    void CameraPathBuilder::set_keyframe_spacing_in_scene_heights(float spacing_in_scene_heights) {
        settings_.keyframe_spacing_in_scene_heights = spacing_in_scene_heights;
    }

    void CameraPathBuilder::set_follow_ground_slopes(bool follow_slopes) {
        settings_.follow_ground_slopes = follow_slopes;
    }

    void CameraPathBuilder::set_ground_up_axis(GroundUpAxis axis) {
        settings_.ground_up_axis = axis;
    }

    vec3 CameraPathBuilder::ground_up_direction() const {
        // One anti-gravity direction for the whole path. Auto takes the world axis of the
        // scene box's shortest extent: a building or a terrain stands along the direction
        // it is least deep in, so that is the way up. The view's up is the last resort, for
        // a scene whose orientation is unknown and which the user has turned upright.
        switch (settings_.ground_up_axis) {
            case GroundUpAxis::X:
                return vec3(1.0f, 0.0f, 0.0f);
            case GroundUpAxis::Y:
                return vec3(0.0f, 1.0f, 0.0f);
            case GroundUpAxis::Z:
                return vec3(0.0f, 0.0f, 1.0f);
            case GroundUpAxis::Auto:
                if (scene_box_.is_valid()) {
                    // The extents are read one axis at a time, the way Box3::min_range_axis()
                    // intends: a Vec reached through an integer index resolves to its
                    // conversion operator to const FT*, not to a component, so the extents
                    // must be taken by name.
                    const vec3 lo = scene_box_.min_point();
                    const vec3 hi = scene_box_.max_point();
                    const float extents[3] = {hi.x - lo.x, hi.y - lo.y, hi.z - lo.z};
                    unsigned int shortest = 0;
                    for (unsigned int i = 1; i < 3; ++i) {
                        if (extents[i] < extents[shortest])
                            shortest = i;
                    }
                    switch (shortest) {
                        case 0:
                            return vec3(1.0f, 0.0f, 0.0f);
                        case 1:
                            return vec3(0.0f, 1.0f, 0.0f);
                        default:
                            return vec3(0.0f, 0.0f, 1.0f);
                    }
                }
                break; // nothing to measure: stand on the view's up instead
            case GroundUpAxis::View:
                break;
        }
        vec3 up = camera_->up_vector();
        if (up.length2() < epsilon_sqr<float>())
            return vec3(0.0f, 0.0f, 1.0f);
        return up.normalize();
    }

    float CameraPathBuilder::scene_height_in_world_units() const {
        if (!scene_box_.is_valid())
            return camera_->scene_radius();

        // The scene "height" is its extent along the path's up direction, so the eye and
        // flight heights keep the same ratio whatever the view.
        const vec3 up = ground_up_direction();
        return std::abs(up.x) * scene_box_.range(0) +
               std::abs(up.y) * scene_box_.range(1) +
               std::abs(up.z) * scene_box_.range(2);
    }

    float CameraPathBuilder::eye_height_in_world_units() const {
        return scene_height_in_world_units() * settings_.eye_height_in_scene_heights;
    }

    float CameraPathBuilder::flight_height_in_world_units() const {
        return scene_height_in_world_units() * settings_.flight_height_in_scene_heights;
    }

    float CameraPathBuilder::keyframe_spacing_in_world_units() const {
        // Clamped to at least 1e-4, so a value at or below zero is accepted without
        // producing an unbounded number of keyframes.
        return std::max(1e-4f, scene_height_in_world_units() * settings_.keyframe_spacing_in_scene_heights);
    }

    void CameraPathBuilder::set_flight_height_in_scene_heights(float height_in_scene_heights) {
        settings_.flight_height_in_scene_heights = height_in_scene_heights;
    }

    std::vector<vec3> CameraPathBuilder::lifted_feet(const std::vector<vec3> &feet, float height) const {
        const vec3 up = ground_up_direction();
        if (settings_.follow_ground_slopes) {
            // Each pose stands its own height above the point under it, so the track climbs
            // and falls with the terrain.
            std::vector<vec3> lifted;
            lifted.reserve(feet.size());
            for (const vec3 &foot : feet)
                lifted.push_back(foot + up * height);
            return lifted;
        }
        // One altitude for the whole track: the first point's, so the camera flies level
        // whatever the ground does beneath it.
        const float altitude = feet.empty() ? 0.0f : dot(feet.front(), up);
        std::vector<vec3> lifted;
        lifted.reserve(feet.size());
        for (const vec3 &foot : feet)
            lifted.push_back(foot - up * dot(foot, up) + up * (altitude + height));
        return lifted;
    }

    bool CameraPathBuilder::resample_track(const std::vector<vec3> &stroke, float height, float spacing,
                                           std::vector<vec3> &feet, std::vector<vec3> &positions) const {
        feet.clear();
        positions.clear();
        if (stroke.size() < 2)
            return false;

        const std::vector<vec3> lifted = lifted_feet(stroke, height);
        const std::vector<ArcSample> samples = arc_length_samples(lifted, spacing);
        feet.reserve(samples.size());
        positions.reserve(samples.size());
        for (const auto &sample : samples) {
            feet.push_back(sample_value(stroke, sample));
            positions.push_back(sample_value(lifted, sample));
        }
        return true;
    }

    std::size_t CameraPathBuilder::generate_ground_route(const std::vector<vec3> &stroke) {
        // One stroke, one route: frames_of() carries the derivation so a route built here and
        // one rebuilt later from the same input cannot drift apart.
        const std::vector<Frame> frames = frames_of(Source::walk({stroke}));
        for (const auto &frame : frames)
            add_keyframe(frame);
        return frames.size();
    }

    std::vector<CameraPathBuilder::FlightStation>
    CameraPathBuilder::flight_stations(const std::vector<vec3> &stroke) const {
        std::vector<FlightStation> stations;
        const float height = flight_height_in_world_units();
        if (height <= 1e-4f)
            return stations;
        // The same spacing as the walking route: the flight height lifts the track, it
        // does not thin out its stations.
        const float spacing = keyframe_spacing_in_world_units();

        std::vector<vec3> feet;
        std::vector<vec3> positions;
        if (!resample_track(stroke, height, spacing, feet, positions))
            return stations;

        stations.reserve(feet.size());
        for (std::size_t i = 0; i < feet.size(); ++i)
            stations.push_back(FlightStation{positions[i], feet[i]});
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

    CameraPathBuilder::Source CameraPathBuilder::Source::walk(std::vector<std::vector<vec3>> routes) {
        Source source;
        source.kind = Kind::Walk;
        source.strokes = std::move(routes);
        return source;
    }

    CameraPathBuilder::Source CameraPathBuilder::Source::flight(std::vector<vec3> track) {
        Source source;
        source.kind = Kind::Flight;
        source.strokes.push_back(std::move(track));
        return source;
    }

    CameraPathBuilder::Source CameraPathBuilder::Source::orbit(const vec3 &anchor, const vec3 &center) {
        Source source;
        source.kind = Kind::Orbit;
        source.anchor = anchor;
        source.center = center;
        return source;
    }

    std::vector<Frame> CameraPathBuilder::frames_of(const Source &source) const {
        std::vector<Frame> frames;
        switch (source.kind) {
            case Source::Kind::None:
                return frames;
            case Source::Kind::Walk: {
                // One route per stroke, concatenated in stroke order: exactly the poses of
                // generate_ground_route() called once per stroke. A stroke too short for a
                // route adds nothing, as that function returns 0 for it, and a refusal of the
                // eye height takes the whole walk, since it is the same for every stroke.
                const float eye = eye_height_in_world_units();
                const float spacing = keyframe_spacing_in_world_units();
                if (eye > 1e-4f) {
                    for (const auto &stroke : source.strokes) {
                        std::vector<vec3> feet;
                        std::vector<vec3> positions;
                        if (!resample_track(stroke, eye, spacing, feet, positions))
                            continue;
                        const std::vector<Frame> route =
                                ground_frames(positions, ground_up_direction(), 0.0f);
                        frames.insert(frames.end(), route.begin(), route.end());
                    }
                }
                break;
            }
            case Source::Kind::Flight: {
                // The strokes are one continuous track, joined before the resampling so that
                // the stations keep the equal spacing across a stroke boundary. With no targets
                // each station looks at the ground ahead along the track.
                std::vector<vec3> track;
                for (const auto &stroke : source.strokes)
                    track.insert(track.end(), stroke.begin(), stroke.end());
                frames = flight_frames(flight_stations(track), std::vector<vec3>(), ground_up_direction());
                break;
            }
            case Source::Kind::Orbit: {
                // The axis is re-derived rather than stored: it is the ground up through the
                // picked anchor, so a changed up rule tips a kept orbit the way a fresh pick
                // would. It points the way the camera's up does not, hence the reverse.
                const Line3 axis = Line3::from_point_and_direction(source.anchor, -ground_up_direction());
                frames = orbit_frames(axis, source.center);
                // orbit_frames() closes the loop by repeating its first pose. The
                // interpolator cannot store that duplicate, so remove it before sampling
                // traced aims and keep their path fractions aligned with the stored poses.
                if (frames.size() > 1 &&
                    distance2(frames.front().position(), frames.back().position()) < epsilon_sqr<float>())
                    frames.pop_back();
                break;
            }
        }

        if (frames.empty() || source.aims.empty())
            return frames;

        // Aims the user traced or clicked replace the mode's default, one target per pose
        // sampled along the aim stroke. A flight keeps the path's up as its roll reference,
        // because its poses aim at the ground rather than stand on it; a walk and an orbit
        // keep each pose's own up, which is what makes a walked pose stay upright.
        const std::vector<vec3> targets = aim_targets(source.aims, frames.size());
        if (source.kind == Source::Kind::Flight)
            return aimed_frames(frames, targets, std::vector<vec3>(frames.size(), ground_up_direction()));
        return aimed_frames(frames, targets);
    }

    bool CameraPathBuilder::settings_changed_for(const Source &source, const Settings &built) const {
        // Comparison is exact, and only over the settings the mode reads: a control put back
        // where the path was built from is no change, and a setting outside the mode's set
        // cannot move one of its poses.
        const Settings &now = settings_;
        switch (source.kind) {
            case Source::Kind::Walk:
                // the up rule is the direction the eye height is measured along, the spacing
                // how many feet the stroke is cut into and the slope rule which altitude
                // those feet rise from
                return now.ground_up_axis != built.ground_up_axis ||
                       now.eye_height_in_scene_heights != built.eye_height_in_scene_heights ||
                       now.keyframe_spacing_in_scene_heights != built.keyframe_spacing_in_scene_heights ||
                       now.follow_ground_slopes != built.follow_ground_slopes;
            case Source::Kind::Flight:
                return now.ground_up_axis != built.ground_up_axis ||
                       now.flight_height_in_scene_heights != built.flight_height_in_scene_heights ||
                       now.keyframe_spacing_in_scene_heights != built.keyframe_spacing_in_scene_heights ||
                       now.follow_ground_slopes != built.follow_ground_slopes;
            case Source::Kind::Orbit:
                // the orbit's axis comes from the up rule and its offsets are measured in the
                // object height along it; the counts set how many poses the loops have
                return now.ground_up_axis != built.ground_up_axis ||
                       now.orbit_radius_offset_in_object_heights != built.orbit_radius_offset_in_object_heights ||
                       now.orbit_vertical_offset_in_object_heights !=
                       built.orbit_vertical_offset_in_object_heights ||
                       now.pitch_degrees != built.pitch_degrees ||
                       now.keyframes_per_loop != built.keyframes_per_loop ||
                       now.num_loops != built.num_loops;
            case Source::Kind::None:
                break;
        }
        return false;
    }

    std::vector<Frame> CameraPathBuilder::orbit_frames(const Line3 &axis, const vec3 &center) const {
        std::vector<Frame> frames;
        if (!scene_box_.is_valid() || settings_.keyframes_per_loop <= 0 || settings_.num_loops <= 0)
            return frames;

        const auto up = -axis.direction(); // picking line points inside screen
        // The orbit offsets are relative to the object height: the extent of the
        // scene box along the axis (z range for an upright scene, like the walking mode).
        const auto extent = scene_box_.max_point() - scene_box_.min_point();
        const float object_height =
                std::abs(up.x) * extent.x + std::abs(up.y) * extent.y + std::abs(up.z) * extent.z;

        const auto vertical_offset = settings_.orbit_vertical_offset_in_object_heights * object_height * up;
        const auto at = axis.projection(center + vertical_offset);
        const auto dist_to_axis =
                scene_box_.diagonal_length() + settings_.orbit_radius_offset_in_object_heights * object_height;
        const auto relative_cam0 = geom::orthogonal(up).normalize() * dist_to_axis;

        const auto pitch_angle = geom::to_radians(settings_.pitch_degrees);
        const auto pitch_offset = dist_to_axis * std::tan(pitch_angle);

        const int steps = settings_.keyframes_per_loop * settings_.num_loops;
        const auto angle_step = static_cast<float>(2.0 * M_PI / settings_.keyframes_per_loop);
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
        // Accumulated quaternion rounding can leave the final position a few ulps away
        // from the start. Close it exactly so the interpolator recognizes the duplicate.
        frames.back() = frames.front();
        return frames;
    }

    std::vector<Frame> CameraPathBuilder::ground_frames(const std::vector<vec3> &feet,
                                                        const vec3 &up_direction,
                                                        float eye_height) const {
        std::vector<Frame> frames;
        if (feet.size() < 2 || up_direction.length2() < epsilon_sqr<float>())
            return frames;
        // One up for every pose: the feet keep their own heights (terrain following), the
        // camera never banks.
        vec3 path_up = up_direction;
        path_up.normalize();

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
            vec3 dir = heading(i, path_up);
            dir -= path_up * dot(dir, path_up);
            // A near-vertical step (a cliff) leaves no in-plane heading.
            if (dir.length2() < epsilon_sqr<float>())
                dir = geom::orthogonal(path_up);
            dir.normalize();
            frames.emplace_back(feet[i] + path_up * eye_height, to_orientation(dir, path_up));
        }
        return frames;
    }

    std::vector<Frame> CameraPathBuilder::flight_frames(const std::vector<FlightStation> &stations,
                                                        const std::vector<vec3> &targets, const vec3 &up,
                                                        std::size_t look_ahead) const {
        std::vector<Frame> frames;
        if (stations.size() < 2 || (!targets.empty() && stations.size() != targets.size()))
            return frames;

        // No targets: the default aim is the ground ahead, the foot of a station further
        // along the track lowered by this station's own height above its foot, so a flight
        // over a slope looks along the ground instead of into it. The last stations look as
        // far ahead as the track has, so they never turn back on themselves.
        const std::size_t step = std::max<std::size_t>(1, look_ahead);
        frames.reserve(stations.size());
        for (size_t i = 0; i < stations.size(); ++i) {
            vec3 target = targets.empty() ? vec3(0.0f) : targets[i];
            if (targets.empty()) {
                const FlightStation &ahead = stations[std::min(i + step, stations.size() - 1)];
                const float height = dot(stations[i].position - stations[i].foot, up);
                target = ahead.foot - up * height;
            }

            vec3 roll_up = up;
            const vec3 view_dir = target - stations[i].position;
            // A view along up has no roll reference; carry the previous camera basis forward.
            if (!frames.empty() && up.length2() > epsilon_sqr<float>() &&
                view_dir.length2() > epsilon_sqr<float>() &&
                cross(view_dir, up).length2() < epsilon_sqr<float>() * view_dir.length2()) {
                const quat previous_orientation = frames.back().orientation();
                roll_up = previous_orientation.rotate(vec3(0.0f, 1.0f, 0.0f));
                if (cross(view_dir, roll_up).length2() <
                    epsilon_sqr<float>() * view_dir.length2() * roll_up.length2())
                    roll_up = previous_orientation.rotate(vec3(1.0f, 0.0f, 0.0f));
            }
            frames.emplace_back(stations[i].position,
                                look_at_orientation(stations[i].position, target, roll_up));
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
        settings_.orbit_radius_offset_in_object_heights = offset_in_object_heights;
    }

    void CameraPathBuilder::set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights) {
        settings_.orbit_vertical_offset_in_object_heights = offset_in_object_heights;
    }

    void CameraPathBuilder::set_pitch_degrees(float degrees) {
        settings_.pitch_degrees = degrees;
    }

    void CameraPathBuilder::set_keyframes_per_loop(int count) {
        settings_.keyframes_per_loop = count;
    }

    void CameraPathBuilder::set_num_loops(int count) {
        settings_.num_loops = count;
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
