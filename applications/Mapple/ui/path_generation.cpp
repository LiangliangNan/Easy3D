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

#include "../mapple_viewer.h"
#include "../keyframe_timeline.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/camera_path_builder.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

const MappleViewer::GeneratedPath *MappleViewer::latest_generated_path() const {
    return generated_paths_.empty() ? nullptr : &generated_paths_.back();
}

MappleViewer::PathTool MappleViewer::path_parameters_tool() const {
    if (path_tool_ == PathTool::GroundWalk || path_tool_ == PathTool::Flight || path_tool_ == PathTool::Orbit)
        return path_tool_;
    return generated_path_tool();
}

MappleViewer::PathTool MappleViewer::generated_path_tool() const {
    const GeneratedPath *path = latest_generated_path();
    if (!path)
        return PathTool::Off;
    switch (path->source.kind) {
        case CameraPathBuilder::Source::Kind::Walk:
            return PathTool::GroundWalk;
        case CameraPathBuilder::Source::Kind::Flight:
            return PathTool::Flight;
        case CameraPathBuilder::Source::Kind::Orbit:
            return PathTool::Orbit;
        case CameraPathBuilder::Source::Kind::None:
            break;
    }
    return PathTool::Off;
}

const char *MappleViewer::generated_path_aims() const {
    const GeneratedPath *path = latest_generated_path();
    if (!path)
        return "";
    if (!path->source.aims.empty())
        return path->source.aims.size() == 1 ? "at the clicked point" : "along the traced aims";
    switch (path->source.kind) {
        case CameraPathBuilder::Source::Kind::Walk:
            return "along the route";
        case CameraPathBuilder::Source::Kind::Flight:
            return "at the ground ahead";
        case CameraPathBuilder::Source::Kind::Orbit:
            return "at the orbit axis";
        case CameraPathBuilder::Source::Kind::None:
            break;
    }
    return "";
}

void MappleViewer::clear_generated_paths() {
    generated_paths_.clear();
}

void MappleViewer::generated_path_keyframe_inserted(int index) {
    for (GeneratedPath &path : generated_paths_)
        path.keyframe_inserted(index);
}

void MappleViewer::generated_path_keyframe_removed(int index) {
    for (GeneratedPath &path : generated_paths_)
        path.keyframe_removed(index);
}

bool MappleViewer::write_camera_path(const std::vector<Frame> &frames,
                                     const std::vector<float> &times,
                                     const std::vector<float> &dwells) {
    if (!kfi_ || frames.size() != times.size() || (!dwells.empty() && dwells.size() != frames.size()))
        return false;

    // add_keyframe() refuses both cases. Check the whole sequence before delete_path(), so
    // a malformed replacement cannot destroy the path that is still on screen.
    for (std::size_t i = 0; i < frames.size(); ++i) {
        if (i > 0 && times[i] <= times[i - 1])
            return false;
        if (i > 0 && distance2(frames[i - 1].position(), frames[i].position()) < epsilon_sqr<float>())
            return false;
    }

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    camera_path_elapsed_ = 0.0f;
    kfi_->delete_path();
    for (std::size_t i = 0; i < frames.size(); ++i) {
        if (!kfi_->add_keyframe(frames[i], times[i]))
            return false;
        if (i < dwells.size())
            kfi_->set_keyframe_dwell(i, dwells[i]);
    }
    return true;
}

void MappleViewer::grow_scene_radius_over_path() {
    if (!kfi_)
        return;
    float radius = camera()->scene_radius();
    for (std::size_t i = 0; i < kfi_->number_of_keyframes(); ++i)
        radius = std::max(radius, distance(camera()->scene_center(), kfi_->keyframe_position(i)));
    camera()->set_scene_radius(radius);
}

bool MappleViewer::rebuild_generated_paths(bool force, bool record_undo) {
    if (!camera_path_builder_ || !kfi_ || generated_paths_.empty())
        return true;

    bool stale = force;
    for (const GeneratedPath &path : generated_paths_)
        stale = stale || path.stale(*camera_path_builder_);
    if (!stale)
        return true;

    const int old_count = static_cast<int>(kfi_->number_of_keyframes());
    int cursor = 0;
    for (const GeneratedPath &path : generated_paths_) {
        if (path.first < cursor || path.first > old_count || path.last >= old_count ||
            path.last < path.first - 1 || path.size() < 0) {
            LOG(ERROR) << "generated path range is no longer valid";
            return false;
        }
        int previous_anchor = -1;
        for (std::size_t i = 0; i < path.manual_before.size(); ++i) {
            const int anchor = path.manual_before[i];
            if (anchor < previous_anchor || anchor > path.size() ||
                path.manual_position(i) < path.first || path.manual_position(i) > path.last) {
                LOG(ERROR) << "generated path's manual keyframe range is no longer valid";
                return false;
            }
            previous_anchor = anchor;
        }
        cursor = path.last + 1;
    }

    // A parameter drag produces one rebuild per changed frame. Keep the first one's undo
    // point until an unrelated edit calls snapshot_camera_path() and clears this flag.
    if (record_undo && (!camera_path_snapshot_.valid || !camera_path_snapshot_.from_rebuild))
        snapshot_camera_path();

    camera_path_builder_->set_scene(models());
    const CameraPathBuilder::Settings settings = camera_path_builder_->settings();

    std::vector<Frame> frames;
    std::vector<float> times;
    std::vector<float> dwells;
    frames.reserve(static_cast<std::size_t>(old_count));
    times.reserve(static_cast<std::size_t>(old_count));
    dwells.reserve(static_cast<std::size_t>(old_count));
    std::vector<int> old_to_new(static_cast<std::size_t>(old_count), -1);
    std::vector<GeneratedPath> rebuilt = generated_paths_;

    auto append = [&](const Frame &frame, float time, float dwell, int old_index) {
        // Preserved manual poses keep their timestamps; only a real collision needs a nudge.
        if (!frames.empty() && time <= times.back())
            time = std::nextafter(times.back(), std::numeric_limits<float>::infinity());
        const int new_index = static_cast<int>(frames.size());
        frames.push_back(frame);
        times.push_back(time);
        dwells.push_back(dwell);
        if (old_index >= 0)
            old_to_new[static_cast<std::size_t>(old_index)] = new_index;
        return new_index;
    };

    // A replaced block shifts the times of all later keyframes by its change in span. Thus
    // an untouched keyframe keeps its original travel interval and dwell, even when an
    // earlier orbit changes density or a traced track gains stations.
    float time_shift = 0.0f;
    cursor = 0;
    for (GeneratedPath &path : rebuilt) {
        const int old_first = path.first;
        const int old_last = path.last;
        for (int i = cursor; i < old_first; ++i) {
            const auto index = static_cast<std::size_t>(i);
            old_to_new[index] = static_cast<int>(frames.size());
            frames.push_back(kfi_->keyframe(index));
            times.push_back(kfi_->keyframe_time(index) + time_shift);
            dwells.push_back(kfi_->keyframe_dwell(index));
        }

        std::vector<int> old_generated;
        std::vector<int> old_manual;
        std::size_t manual_cursor = 0;
        for (int i = old_first; i <= old_last; ++i) {
            if (manual_cursor < path.manual_before.size() && path.manual_position(manual_cursor) == i) {
                old_manual.push_back(i);
                ++manual_cursor;
            } else {
                old_generated.push_back(i);
            }
        }

        const int new_first = static_cast<int>(frames.size());
        const std::vector<Frame> generated = camera_path_builder_->frames_of(path.source);

        // Zero eye/flight height and fewer than two orbit keyframes refuse a rebuild.
        // Keep the last accepted poses, but remember the refused settings so moving an
        // unrelated control does not retry (and warn) on every frame.
        if (generated.empty() && !path.pending) {
            for (int i = old_first; i <= old_last; ++i) {
                const auto index = static_cast<std::size_t>(i);
                old_to_new[index] = static_cast<int>(frames.size());
                frames.push_back(kfi_->keyframe(index));
                times.push_back(kfi_->keyframe_time(index) + time_shift);
                dwells.push_back(kfi_->keyframe_dwell(index));
            }
            path.first = new_first;
            path.last = new_first + path.span() - 1;
            path.built_settings = settings;
            cursor = old_last + 1;
            continue;
        }
        if (generated.empty())
            return false;

        float base_time = old_first < old_count
                          ? kfi_->keyframe_time(static_cast<std::size_t>(old_first)) + time_shift
                          : frames.empty() ? 0.0f : times.back() + 1.0f;
        if (!frames.empty())
            base_time = std::max(base_time, times.back() + 1.0f);

        std::vector<int> accepted_before(generated.size() + 1, 0);
        std::vector<int> source_output(generated.size(), -1);
        int accepted = 0;
        int first_generated_output = -1;
        manual_cursor = 0;
        for (std::size_t i = 0; i < generated.size(); ++i) {
            accepted_before[i] = accepted;
            while (manual_cursor < path.manual_before.size() &&
                   path.manual_before[manual_cursor] <= static_cast<int>(i)) {
                const int old_index = old_manual[manual_cursor];
                append(kfi_->keyframe(static_cast<std::size_t>(old_index)),
                       kfi_->keyframe_time(static_cast<std::size_t>(old_index)) + time_shift,
                       kfi_->keyframe_dwell(static_cast<std::size_t>(old_index)), old_index);
                ++manual_cursor;
            }

            const Frame &frame = generated[i];
            // The interpolator drops a pose coinciding with its predecessor. Do the same
            // before writing, including at a stroke boundary, a manual pose or a block's start.
            if (!frames.empty() &&
                distance2(frames.back().position(), frame.position()) < epsilon_sqr<float>()) {
                source_output[i] = i > 0 ? source_output[i - 1] : -1;
                continue;
            }
            const float dwell = !path.pending && i < old_generated.size()
                                ? kfi_->keyframe_dwell(static_cast<std::size_t>(old_generated[i])) : 0.0f;
            const int output = append(frame, base_time + static_cast<float>(accepted), dwell, -1);
            source_output[i] = output;
            if (first_generated_output < 0)
                first_generated_output = output;
            ++accepted;
        }
        accepted_before.back() = accepted;
        while (manual_cursor < path.manual_before.size()) {
            const int old_index = old_manual[manual_cursor];
            append(kfi_->keyframe(static_cast<std::size_t>(old_index)),
                   kfi_->keyframe_time(static_cast<std::size_t>(old_index)) + time_shift,
                   kfi_->keyframe_dwell(static_cast<std::size_t>(old_index)), old_index);
            ++manual_cursor;
        }

        for (std::size_t i = 0; i < old_generated.size(); ++i) {
            const int source = static_cast<int>(std::min(i, generated.size() - 1));
            int output = source_output[static_cast<std::size_t>(source)];
            if (output < 0 && first_generated_output >= 0)
                output = first_generated_output;
            old_to_new[static_cast<std::size_t>(old_generated[i])] = output;
        }
        for (int &anchor : path.manual_before)
            anchor = accepted_before[static_cast<std::size_t>(anchor)];

        const float old_end = path.span() > 0
                              ? kfi_->keyframe_time(static_cast<std::size_t>(old_last)) : base_time - 1.0f;
        const int block_size = static_cast<int>(frames.size()) - new_first;
        const float new_end = block_size > 0 ? times.back() : base_time - 1.0f;
        time_shift += new_end - old_end;
        path.first = new_first;
        path.last = new_first + block_size - 1;
        path.pending = false;
        path.built_settings = settings;
        cursor = old_last + 1;
    }

    for (int i = cursor; i < old_count; ++i) {
        const auto index = static_cast<std::size_t>(i);
        old_to_new[index] = static_cast<int>(frames.size());
        frames.push_back(kfi_->keyframe(index));
        times.push_back(kfi_->keyframe_time(index) + time_shift);
        dwells.push_back(kfi_->keyframe_dwell(index));
    }

    if (!write_camera_path(frames, times, dwells)) {
        LOG(ERROR) << "the regenerated camera path could not be written";
        return false;
    }
    generated_paths_ = std::move(rebuilt);

    const int old_selected = keyframe_editor_->selected();
    int selected = old_selected >= 0 && old_selected < old_count
                   ? old_to_new[static_cast<std::size_t>(old_selected)] : -1;
    keyframe_editor_->set_selected(selected);

    if (keyframe_timeline_)
        keyframe_timeline_->invalidate();
    if (!frames.empty() && !show_camera_path_)
        set_camera_path_visible(true);
    grow_scene_radius_over_path();
    if (show_camera_path_)
        update_camera_path_guides();
    camera_path_snapshot_.from_rebuild = true;
    update();
    return true;
}

void MappleViewer::commit_ground_route() {
    if (!camera_path_builder_)
        return;
    if (route_segment_.size() >= 2)
        route_segments_.push_back(route_segment_);
    route_segment_.clear();

    CameraPathBuilder::Source source = CameraPathBuilder::Source::walk(std::move(route_segments_));
    route_segments_.clear();
    if (camera_path_builder_->frames_of(source).empty()) {
        LOG(WARNING) << "the stroke is too short to generate walking keyframes";
        return;
    }

    const int before = static_cast<int>(kfi_->number_of_keyframes());
    GeneratedPath path;
    path.source = std::move(source);
    path.built_settings = camera_path_builder_->settings();
    path.first = before;
    path.last = before - 1;
    generated_paths_.push_back(std::move(path));
    if (!rebuild_generated_paths(true, false) || kfi_->number_of_keyframes() <= static_cast<std::size_t>(before)) {
        generated_paths_.pop_back();
        LOG(WARNING) << "the stroke is too short to generate walking keyframes";
        return;
    }

    path_tool_ = PathTool::Off;
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    LOG(INFO) << kfi_->number_of_keyframes() - static_cast<std::size_t>(before)
              << " walking keyframes added to the camera path (aiming along the route)";
    update();
}

void MappleViewer::commit_flight_track() {
    if (!camera_path_builder_)
        return;
    if (route_segment_.size() >= 2)
        route_segments_.push_back(route_segment_);
    route_segment_.clear();

    // A flight bridges the gaps where the drag left the surface, unlike a walk, whose
    // segments stay separate routes.
    std::vector<vec3> points;
    for (const auto &segment : route_segments_)
        points.insert(points.end(), segment.begin(), segment.end());
    route_segments_.clear();

    CameraPathBuilder::Source source = CameraPathBuilder::Source::flight(std::move(points));
    if (camera_path_builder_->frames_of(source).size() < 2) {
        LOG(WARNING) << "the stroke is too short to trace a flight track";
        return;
    }

    const int before = static_cast<int>(kfi_->number_of_keyframes());
    GeneratedPath path;
    path.source = std::move(source);
    path.built_settings = camera_path_builder_->settings();
    path.first = before;
    path.last = before - 1;
    generated_paths_.push_back(std::move(path));
    if (!rebuild_generated_paths(true, false) || kfi_->number_of_keyframes() <= static_cast<std::size_t>(before)) {
        generated_paths_.pop_back();
        LOG(WARNING) << "the flight track could not be turned into keyframes";
        return;
    }

    path_tool_ = PathTool::Off;
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    LOG(INFO) << kfi_->number_of_keyframes() - static_cast<std::size_t>(before)
              << " flight keyframes added to the camera path (aiming at the ground ahead)";
    update();
}

void MappleViewer::apply_orbit_gesture(const vec2 &anchor) {
    if (!camera_path_builder_)
        return;
    camera_path_builder_->set_scene(models());
    if (!camera_path_builder_->scene_box().is_valid()) {
        LOG(WARNING) << "the orbit needs a visible model";
        return;
    }

    const PickResult hit = pick(static_cast<int>(anchor.x), static_cast<int>(anchor.y));
    if (!hit) {
        LOG(WARNING) << "the orbit has to start on a visible model surface";
        return;
    }
    CameraPathBuilder::Source source = CameraPathBuilder::Source::orbit(hit.position, camera()->scene_center());
    if (camera_path_builder_->frames_of(source).empty()) {
        LOG(WARNING) << "the orbit settings could not generate any keyframes";
        return;
    }

    // Register the replacement before writing it. The splice owns the whole old range, so
    // the snapshot taken when the tool was armed still contains the path Ctrl+Z restores.
    std::vector<GeneratedPath> previous = std::move(generated_paths_);
    GeneratedPath path;
    path.source = std::move(source);
    path.built_settings = camera_path_builder_->settings();
    path.first = 0;
    path.last = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    generated_paths_.push_back(std::move(path));
    if (!rebuild_generated_paths(true, false)) {
        generated_paths_ = std::move(previous);
        return;
    }

    path_tool_ = PathTool::Off;
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    LOG(INFO) << "camera path replaced by an orbit (Ctrl+Z restores it)";
    update();
}

void MappleViewer::commit_aim_stroke() {
    if (!camera_path_builder_)
        return;

    // Keep the last segment even when it holds only the click's single sample: one point
    // aims every generated pose there; more points are sampled along the aim stroke.
    std::vector<vec3> points;
    for (const auto &segment : route_segments_)
        points.insert(points.end(), segment.begin(), segment.end());
    points.insert(points.end(), route_segment_.begin(), route_segment_.end());
    route_segment_.clear();
    route_segments_.clear();
    if (points.empty()) {
        LOG(WARNING) << "the aim has to end on a visible model surface";
        return;
    }

    GeneratedPath *path = generated_paths_.empty() ? nullptr : &generated_paths_.back();
    if (!path || path->size() == 0) {
        LOG(WARNING) << "the path to re-aim is gone (it was edited or replaced)";
        path_tool_ = PathTool::Off;
        update();
        return;
    }

    const std::vector<vec3> previous_aims = path->source.aims;
    camera_path_builder_->set_scene(models());
    CameraPathBuilder::Source aimed = path->source;
    aimed.aims = points;
    if (camera_path_builder_->frames_of(aimed).empty()) {
        LOG(WARNING) << "the current path settings cannot generate poses to re-aim";
        return;
    }
    path->source.aims = std::move(points);
    if (!rebuild_generated_paths(true, false)) {
        path->source.aims = std::move(previous_aims);
        return;
    }
    path_tool_ = PathTool::Off;
    LOG(INFO) << latest_generated_path()->size() << " keyframe"
              << (latest_generated_path()->size() == 1 ? "" : "s")
              << " re-aimed (Ctrl+Z restores their poses)";
    update();
}

void MappleViewer::snapshot_camera_path() {
    camera_path_snapshot_.frames.clear();
    camera_path_snapshot_.times.clear();
    camera_path_snapshot_.dwells.clear();
    camera_path_snapshot_.generated_paths.clear();
    for (std::size_t i = 0; kfi_ && i < kfi_->number_of_keyframes(); ++i) {
        camera_path_snapshot_.frames.push_back(kfi_->keyframe(i));
        camera_path_snapshot_.times.push_back(kfi_->keyframe_time(i));
        camera_path_snapshot_.dwells.push_back(kfi_->keyframe_dwell(i));
    }
    camera_path_snapshot_.generated_paths = generated_paths_;
    camera_path_snapshot_.keyframe_index = keyframe_editor_->selected();
    camera_path_snapshot_.valid = true;
    camera_path_snapshot_.from_rebuild = false;
}

bool MappleViewer::undo_camera_path() {
    if (!camera_path_snapshot_.valid || !kfi_)
        return false;
    if (!write_camera_path(camera_path_snapshot_.frames, camera_path_snapshot_.times,
                           camera_path_snapshot_.dwells)) {
        LOG(ERROR) << "the camera path snapshot could not be restored";
        return false;
    }

    if (keyframe_timeline_)
        keyframe_timeline_->invalidate();
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    keyframe_editor_->set_selected(camera_path_snapshot_.keyframe_index);
    camera_path_snapshot_.valid = false;
    camera_path_snapshot_.from_rebuild = false;
    generated_paths_ = camera_path_snapshot_.generated_paths;

    if (count > 0 && !show_camera_path_)
        set_camera_path_visible(true);
    else if (count == 0 && show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    } else
        grow_scene_radius_over_path();
    LOG(INFO) << "camera path restored: " << count << " keyframes";
    update();
    return true;
}
