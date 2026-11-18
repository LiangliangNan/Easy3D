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
#include <easy3d/renderer/camera_path_builder.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <easy3d/core/model.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#ifdef HAS_FFMPEG
#include <easy3d/video/video_encoder.h>
#endif

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

namespace {
    std::vector<std::string> keyframe_filters() {
        return {"Keyframe files (*.kf)", "*.kf", "All files (*.*)", "*"};
    }
}

void MappleViewer::open_animation_window() {
    show_animation_window_ = true;
}

void MappleViewer::draw_animation_window() {
    if (!show_animation_window_)
        return;

    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Camera Path", &show_animation_window_)) {
        ImGui::End();
        if (!show_animation_window_) { // closed via the title bar
            // Do not leave the camera moving with no control in sight; Ctrl+K still
            // toggles playback afterwards.
            if (kfi_->is_interpolation_started())
                kfi_->stop_interpolation();
            camera_keyframe_index_ = -1;
            // The tool disarms with its window; a gesture in progress is abandoned,
            // and the keyframe handles go away with the window that turns them on.
            path_tool_ = PathTool::Off;
            edit_keyframes_ = false;
            cancel_path_gesture();
            clear_flight_gesture();
        }
        return;
    }

    KeyFrameInterpolator *kfi = kfi_.get();
    const int count = static_cast<int>(kfi->number_of_keyframes());
    camera_keyframe_index_ = std::min(camera_keyframe_index_, count - 1);

    if (ImGui::Button("Import...", ImVec2(100, 0)))
        import_camera_path();
    ImGui::SameLine();
    if (ImGui::Button("Export...", ImVec2(100, 0)))
        export_camera_path();
    ImGui::SameLine();
    ImGui::TextDisabled(".kf files");

    ImGui::Separator();
    ImGui::Text("%d keyframe%s", count, count == 1 ? "" : "s");

    if (ImGui::Button("Walk a route...", ImVec2(120, 0)))
        arm_camera_path_tool(PathTool::GroundWalk);
    ImGui::SameLine();
    if (ImGui::Button("Orbit...", ImVec2(80, 0)))
        arm_camera_path_tool(PathTool::Orbit);
    ImGui::SameLine();
    if (ImGui::Button("Flight...", ImVec2(80, 0)))
        arm_camera_path_tool(PathTool::Flight);

    // Read after the buttons so arming shows its hint and its parameters at once.
    const PathTool tool = path_tool_;
    if (tool != PathTool::Off) {
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(70, 0)))
            cancel_camera_path_tool();
        if (tool == PathTool::Flight)
            ImGui::TextDisabled(flight_aim_ready_
                                ? "Click to aim at one target, or drag to trace the aim along the track. "
                                  "Esc cancels."
                                : "Drag over the surface to trace the camera track. Esc cancels.");
        else
            ImGui::TextDisabled(tool == PathTool::GroundWalk
                                    ? "Drag over the ground surface to trace the route. Esc cancels."
                                    : "Drag across the scene to set the orbit axis; a click uses the "
                                      "vertical axis. Esc cancels.");
    } else
        ImGui::TextDisabled("Move the camera, then add keyframes (Alt+K).");
    if (camera_path_snapshot_.valid)
        ImGui::TextDisabled("Ctrl+Z restores the path from before the last tool or edit.");

    if (ImGui::Button("Add Keyframe", ImVec2(120, 0)))
        add_camera_keyframe();
    ImGui::SameLine();
    if (ImGui::Button("Remove Last", ImVec2(110, 0)))
        remove_last_camera_keyframe();
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(70, 0)))
        clear_camera_path();

    const bool selected = camera_keyframe_index_ >= 0 && camera_keyframe_index_ < count;
    ImGui::BeginDisabled(!selected);
    if (ImGui::Button("Insert After", ImVec2(110, 0)))
        insert_camera_keyframe();
    ImGui::SetItemTooltip("Inserts a keyframe after the selected one, halfway to the next (the last "
                          "segment is extended past the end); drag its glyph to place it.");
    ImGui::SameLine();
    if (ImGui::Button("Delete Selected", ImVec2(115, 0)))
        delete_camera_keyframe();
    ImGui::SetItemTooltip("Deletes the selected keyframe.");
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (selected)
        ImGui::TextDisabled("no. %d selected", camera_keyframe_index_ + 1);
    else
        ImGui::TextDisabled("no keyframe selected");

    ImGui::BeginDisabled(count == 0);
    if (ImGui::Button("< Prev", ImVec2(80, 0)))
        goto_camera_keyframe(camera_keyframe_index_ <= 0 ? count - 1 : camera_keyframe_index_ - 1);
    ImGui::SameLine();
    if (camera_keyframe_index_ >= 0)
        ImGui::Text("Keyframe %d / %d", camera_keyframe_index_ + 1, count);
    else
        ImGui::TextDisabled("Keyframe - / %d", count);
    ImGui::SameLine();
    if (ImGui::Button("Next >", ImVec2(80, 0)))
        goto_camera_keyframe(camera_keyframe_index_ < 0 || camera_keyframe_index_ >= count - 1
                                     ? 0 : camera_keyframe_index_ + 1);
    ImGui::EndDisabled();

    ImGui::Separator();
    const bool playing = kfi->is_interpolation_started();
    if (ImGui::Button(playing ? "Stop" : "Play", ImVec2(120, 0)))
        toggle_camera_path_playback();
    ImGui::SameLine();
    ImGui::TextDisabled(playing ? "playing..." : "Ctrl+K");

    float speed = kfi->interpolation_speed();
    if (ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f, "%.2f") && speed != kfi->interpolation_speed())
        kfi->set_interpolation_speed(speed);
    int fps = kfi->frame_rate();
    if (ImGui::DragInt("Frame rate", &fps, 1.0f, 1, 120) && fps != kfi->frame_rate())
        kfi->set_frame_rate(fps);

    const char *methods[] = {"interpolation", "fitting"};
    int method = kfi->interpolation_method() == KeyFrameInterpolator::INTERPOLATION ? 0 : 1;
    if (ImGui::Combo("Spline strategy", &method, methods, 2))
        kfi->set_interpolation_method(
                method == 0 ? KeyFrameInterpolator::INTERPOLATION : KeyFrameInterpolator::FITTING);

    bool visible = show_camera_path_;
    if (ImGui::Checkbox("Show path and keyframe cameras", &visible))
        set_camera_path_visible(visible);

    bool edit = edit_keyframes_;
    if (ImGui::Checkbox("Edit keyframe handles", &edit)) {
        edit_keyframes_ = edit;
        if (edit) {
            if (!show_camera_path_)
                set_camera_path_visible(true);
            if (camera_keyframe_index_ < 0 && count > 0)
                camera_keyframe_index_ = count - 1;
        } else
            cancel_keyframe_drag();
        update();
    }
    ImGui::SetItemTooltip("The selected keyframe shows a gizmo sized to its camera glyph. Drag the "
                          "glyph itself to move it in the screen plane through its position (the "
                          "plane of its up - the terrain or orbit plane - with Shift), an axis arrow "
                          "to move it along that axis, or a ring to turn its pose around that axis. "
                          "Clicking the scene aims the selected keyframe at the clicked point. "
                          "Ctrl+Z restores the path from before the drag; Esc cancels it.");
    if (edit_keyframes_ && selected_keyframe() >= 0 && !playing)
        ImGui::TextDisabled("Click the scene to aim the selected keyframe at the clicked point.");

    if (camera_path_builder_ && tool == PathTool::GroundWalk) {
        ImGui::Separator();
        ImGui::TextDisabled("Up  surface normal under each foot");
        ImGui::SetItemTooltip("The walker stands upright on the traced surface: each pose rises along "
                              "the surface normal under its foot, so the route tilts with the terrain. "
                              "Where the surface has no normal (e.g. a point cloud), the view's up "
                              "direction is used.");

        float height = camera_path_builder_->eye_height_in_scene_heights();
        if (ImGui::DragFloat("Eye height", &height, 0.01f, 0.0f, 100.0f, "%.2f x scene")) {
            camera_path_builder_->set_eye_height_in_scene_heights(height);
            update();
        }
        ImGui::SetItemTooltip("The eye height above the ground, relative to the scene extent "
                              "along the view's up direction; each pose rises along its foot's "
                              "surface normal.");

        float spacing = camera_path_builder_->keyframe_spacing_in_character_heights();
        if (ImGui::DragFloat("Keyframe spacing", &spacing, 0.02f, 0.1f, 10.0f, "%.2f x height")) {
            camera_path_builder_->set_keyframe_spacing_in_character_heights(spacing);
            update();
        }
        ImGui::SetItemTooltip("Distance between keyframes along the route, relative to the character height.");
    } else if (camera_path_builder_ && tool == PathTool::Orbit) {
        ImGui::Separator();
        ImGui::TextDisabled("Axis  drag across the scene; a click uses the vertical axis");
        ImGui::SetItemTooltip("The axis runs along the dragged screen segment through the surface under "
                              "the press, and the camera circles around it. A click (no drag) uses the "
                              "view's up axis through that point. Ctrl+Z restores the path from before "
                              "the tool was armed.");
        int apply = orbit_replaces_path_ ? 0 : 1;
        if (ImGui::RadioButton("Replace path", &apply, 0))
            orbit_replaces_path_ = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Add to the path", &apply, 1))
            orbit_replaces_path_ = false;

        float zoom = camera_path_builder_->orbit_radius_offset_in_object_heights();
        if (ImGui::DragFloat("Zoom out", &zoom, 0.01f, -100.0f, 100.0f, "%.2f x height"))
            camera_path_builder_->set_orbit_radius_offset_in_object_heights(zoom);
        float offset = camera_path_builder_->orbit_vertical_offset_in_object_heights();
        if (ImGui::DragFloat("Vertical offset", &offset, 0.01f, -100.0f, 100.0f, "%.2f x height"))
            camera_path_builder_->set_orbit_vertical_offset_in_object_heights(offset);
        float pitch = camera_path_builder_->pitch_degrees();
        if (ImGui::DragFloat("Pitch angle", &pitch, 0.5f, -89.0f, 89.0f, "%.1f deg"))
            camera_path_builder_->set_pitch_degrees(pitch);
        int keyframes_per_loop = camera_path_builder_->keyframes_per_loop();
        if (ImGui::DragInt("Keyframes per loop", &keyframes_per_loop, 0.5f, 1, 99))
            camera_path_builder_->set_keyframes_per_loop(keyframes_per_loop);
        int loops = camera_path_builder_->num_loops();
        if (ImGui::DragInt("Loops", &loops, 0.2f, 1, 99))
            camera_path_builder_->set_num_loops(loops);
    } else if (camera_path_builder_ && tool == PathTool::Flight) {
        ImGui::Separator();
        ImGui::TextDisabled("Track  drag over the surface; poses rise along its normal");
        ImGui::SetItemTooltip("The camera track follows the traced surface points, each lifted along "
                              "the surface normal under it. Where the surface has no normal (e.g. a "
                              "point cloud), the view's up direction is used. After the track, click "
                              "one point to aim at it, or drag to trace the aim along the track.");

        float height = camera_path_builder_->flight_height_in_scene_heights();
        if (ImGui::DragFloat("Flight height", &height, 0.01f, 0.0f, 100.0f, "%.2f x scene")) {
            camera_path_builder_->set_flight_height_in_scene_heights(height);
            update();
        }
        ImGui::SetItemTooltip("The camera height above the traced surface, relative to the scene extent "
                              "along the view's up direction; each pose rises along its foot's "
                              "surface normal.");

        float spacing = camera_path_builder_->keyframe_spacing_in_character_heights();
        if (ImGui::DragFloat("Keyframe spacing", &spacing, 0.02f, 0.1f, 10.0f, "%.2f x height")) {
            camera_path_builder_->set_keyframe_spacing_in_character_heights(spacing);
            update();
        }
        ImGui::SetItemTooltip("Distance between keyframes along the track, relative to the flight height.");
    }

    // The orbit gesture applies Replace/Append directly; Ctrl+Z restores the path
    // from before the tool was armed.

    ImGui::Separator();
#ifdef HAS_FFMPEG
    ImGui::SliderInt("Bitrate (Mbit/s)", &video_bitrate_mbps_, 1, 50);
    if (count < 2)
        ImGui::TextDisabled("Recording needs at least two keyframes.");
    ImGui::BeginDisabled(count < 2);
    if (ImGui::Button("Record Video...", ImVec2(140, 0)))
        record_camera_path();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("MP4, scene only");
#else
    ImGui::BeginDisabled();
    ImGui::Button("Record Video...", ImVec2(140, 0));
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("requires FFmpeg");
#endif

    ImGui::End();
}

void MappleViewer::add_camera_keyframe() {
    if (!kfi_->add_keyframe(*camera()->frame()))
        return;
    camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    if (show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << "keyframe " << kfi_->number_of_keyframes() << " added to the camera path";
    update();
}

void MappleViewer::remove_last_camera_keyframe() {
    if (kfi_->number_of_keyframes() == 0)
        return;
    kfi_->delete_last_keyframe();
    camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    LOG(INFO) << "last keyframe removed (" << kfi_->number_of_keyframes() << " remaining)";
    update();
}

void MappleViewer::clear_camera_path() {
    if (kfi_->number_of_keyframes() == 0)
        return;
    kfi_->delete_path();
    camera_keyframe_index_ = -1;
    if (show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    }
    LOG(INFO) << "camera path cleared";
    update();
}

bool MappleViewer::rebuild_camera_path(const std::vector<Frame> &frames, const std::vector<float> &times) {
    if (!kfi_ || frames.size() != times.size())
        return false;
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    kfi_->delete_path();
    for (std::size_t i = 0; i < frames.size(); ++i) {
        if (!kfi_->add_keyframe(frames[i], times[i]))
            return false;
    }
    return true;
}

void MappleViewer::insert_camera_keyframe() {
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    if (count == 0)
        return;
    const int after = camera_keyframe_index_ >= 0 ? std::min(camera_keyframe_index_, count - 1) : count - 1;

    // Halfway to the next keyframe, so the section keeps its timing and shape
    // between the two neighbours; past the last keyframe the last segment is
    // extended, so the new glyph does not land on top of an existing one.
    Frame frame;
    float time = 0.0f;
    if (after + 1 < count) {
        const Frame a = kfi_->keyframe(after);
        const Frame b = kfi_->keyframe(after + 1);
        frame.set_position_and_orientation(0.5f * (a.position() + b.position()),
                                           quat::slerp(a.orientation(), b.orientation(), 0.5f));
        time = 0.5f * (kfi_->keyframe_time(after) + kfi_->keyframe_time(after + 1));
    } else {
        const Frame last = kfi_->keyframe(after);
        vec3 position = last.position();
        float gap = 1.0f;
        if (after > 0) {
            position += last.position() - kfi_->keyframe_position(after - 1);
            gap = kfi_->keyframe_time(after) - kfi_->keyframe_time(after - 1);
        }
        frame.set_position_and_orientation(position, last.orientation());
        time = kfi_->keyframe_time(after) + std::max(gap, 1e-3f);
    }

    std::vector<Frame> frames;
    std::vector<float> times;
    frames.reserve(count + 1);
    times.reserve(count + 1);
    for (int i = 0; i < after + 1; ++i) {
        frames.push_back(kfi_->keyframe(i));
        times.push_back(kfi_->keyframe_time(i));
    }
    frames.push_back(frame);
    times.push_back(time);
    for (int i = after + 1; i < count; ++i) {
        frames.push_back(kfi_->keyframe(i));
        times.push_back(kfi_->keyframe_time(i));
    }

    snapshot_camera_path(); // Ctrl+Z restores the path from before the insert
    if (!rebuild_camera_path(frames, times)) {
        LOG(ERROR) << "failed to insert a keyframe after no. " << after + 1;
        return;
    }
    camera_keyframe_index_ = after + 1;
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << "keyframe inserted after no. " << after + 1 << " (" << frames.size() << " keyframes)";
    update();
}

void MappleViewer::delete_camera_keyframe() {
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    if (count == 0 || camera_keyframe_index_ < 0 || camera_keyframe_index_ >= count)
        return;
    const int index = camera_keyframe_index_;

    std::vector<Frame> frames;
    std::vector<float> times;
    frames.reserve(count - 1);
    times.reserve(count - 1);
    for (int i = 0; i < count; ++i) {
        if (i == index)
            continue;
        frames.push_back(kfi_->keyframe(i));
        times.push_back(kfi_->keyframe_time(i));
    }

    snapshot_camera_path(); // Ctrl+Z restores the deleted keyframe
    if (!rebuild_camera_path(frames, times)) {
        LOG(ERROR) << "failed to delete keyframe no. " << index + 1;
        return;
    }
    camera_keyframe_index_ = frames.empty() ? -1 : std::min(index, static_cast<int>(frames.size()) - 1);
    if (frames.empty() && show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    }
    LOG(INFO) << "keyframe no. " << index + 1 << " deleted (" << frames.size() << " remaining)";
    update();
}

void MappleViewer::toggle_camera_path_playback() {
    if (kfi_->is_interpolation_started()) {
        kfi_->stop_interpolation();
        LOG(INFO) << "camera path playback stopped";
    } else if (kfi_->number_of_keyframes() < 2) {
        LOG(WARNING) << "camera path playback needs at least two keyframes";
        return;
    } else {
        kfi_->start_manual_interpolation();
        LOG(INFO) << "camera path playback started (" << kfi_->number_of_keyframes() << " keyframes)";
    }
    update();
}

void MappleViewer::goto_camera_keyframe(int index) {
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    if (count == 0)
        return;
    index = clamp(index, 0, count - 1);
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(index));
    camera()->frame()->set_position_and_orientation(frame.position(), frame.orientation());
    camera_keyframe_index_ = index;
    update();
}

bool MappleViewer::aim_selected_keyframe_at(int x, int y) {
    const int index = selected_keyframe();
    if (index < 0 || !kfi_ || !point_in_scene(x, y))
        return false;

    // Only scene content is a target: the glyphs, the path and the overlays are
    // annotations, and a click on those keeps its own meaning.
    const PickResult hit = pick(x, y);
    if (!hit || !hit.model)
        return false;

    const Frame frame = kfi_->keyframe(static_cast<std::size_t>(index));
    vec3 direction = hit.position - frame.position();
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

    snapshot_camera_path(); // Ctrl+Z restores the previous pose
    kfi_->set_keyframe_orientation(static_cast<std::size_t>(index), orientation);
    LOG(INFO) << "keyframe " << index + 1 << " aimed at the clicked point (Ctrl+Z restores it)";
    update();
    return true;
}

void MappleViewer::set_camera_path_visible(bool visible) {
    show_camera_path_ = visible;
    if (visible) {
        float radius = camera()->scene_radius();
        for (std::size_t i = 0; i < kfi_->number_of_keyframes(); ++i)
            radius = std::max(radius, distance(camera()->scene_center(), kfi_->keyframe_position(i)));
        camera()->set_scene_radius(radius);
    } else {
        // The glyphs are scene geometry; they must be gone before the bounds are
        // rebuilt from the scene, or they would keep the keyframes in the bounds.
        release_camera_path_guides();
        restore_scene_bounds();
    }
    update();
}

void MappleViewer::import_camera_path() {
    const Model *model = current_model();
    const std::string directory = model ? file_system::parent_directory(model->name()) : "./";
    const std::string file_name = dialog::open("Import keyframes from file", directory, keyframe_filters());
    if (file_name.empty())
        return;
    if (kfi_->read_keyframes(file_name)) {
        camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
        if (show_camera_path_)
            set_camera_path_visible(true);
        LOG(INFO) << kfi_->number_of_keyframes() << " keyframes loaded from " << file_name;
    } else
        LOG(ERROR) << "failed to read keyframes from " << file_name;
    update();
}

void MappleViewer::export_camera_path() {
    if (kfi_->number_of_keyframes() == 0) {
        LOG(WARNING) << "nothing to export (the camera path is empty)";
        return;
    }
    const Model *model = current_model();
    std::string default_name("./keyframes.kf");
    if (model)
        default_name = file_system::replace_extension(model->name(), "kf");
    const std::string file_name = dialog::save("Export keyframes to file", default_name, keyframe_filters());
    if (file_name.empty())
        return;
    if (kfi_->save_keyframes(file_name))
        LOG(INFO) << kfi_->number_of_keyframes() << " keyframes saved to " << file_name;
    else
        LOG(ERROR) << "failed to save keyframes to " << file_name;
}

void MappleViewer::arm_camera_path_tool(PathTool tool) {
    if (!camera_path_builder_)
        return;

    cancel_path_gesture();
    clear_flight_gesture(); // arming starts from a clean slate
    if (path_tool_ == tool) { // clicking the armed tool disarms it
        path_tool_ = PathTool::Off;
        update();
        return;
    }
    if (measure_armed_)
        clear_measurement(); // the measure tool would take this tool's presses

    if (tool == PathTool::GroundWalk || tool == PathTool::Orbit || tool == PathTool::Flight) {
        camera_path_builder_->set_scene(models());
        if (!camera_path_builder_->scene_box().is_valid()) {
            LOG(WARNING) << "the camera path tools need a visible model";
            path_tool_ = PathTool::Off;
            update();
            return;
        }
    }

    path_tool_ = tool;
    snapshot_camera_path(); // Ctrl+Z restores the path the tool started from
    LOG(INFO) << (tool == PathTool::GroundWalk
                  ? "walk route tool armed: drag over the ground surface to trace the route"
                  : tool == PathTool::Orbit
                    ? "orbit tool armed: drag across the scene to set the orbit axis "
                      "(a click uses the vertical axis)"
                    : "flight tool armed: drag over the surface to trace the camera track");
    update();
}

void MappleViewer::cancel_camera_path_tool() {
    cancel_path_gesture();
    clear_flight_gesture();
    path_tool_ = PathTool::Off;
    update();
}

void MappleViewer::cancel_path_gesture() {
    tracing_route_ = false;
    orbit_dragging_ = false;
    flight_tracking_ = false;
    flight_aiming_ = false;
    route_stroke_screen_.clear();
    route_segment_.clear();
    route_segments_.clear();
    cancel_keyframe_drag();
}

bool MappleViewer::route_point_at(int x, int y, vec3 &point, vec3 &up) const {
    if (!point_in_scene(x, y))
        return false;
    const PickResult result = pick(x, y);
    if (!result)
        return false;
    point = result.position;

    // The walker stands on the visible side of the surface, so its up is the picked
    // face's normal pointing back at the camera; a point cloud or a standalone
    // drawable has no faces and falls back to the view's up. The face winding is
    // arbitrary and the view's up does not resolve it (a slope can face any way
    // relative to it), so the camera's view direction decides the sign.
    up = camera_path_builder_->ground_up_direction();
    const auto mesh = std::dynamic_pointer_cast<SurfaceMesh>(result.model);
    if (mesh && result.face_index >= 0 && result.face_index < static_cast<int>(mesh->n_faces())) {
        vec3 normal = mesh->compute_face_normal(SurfaceMesh::Face(result.face_index));
        if (normal.length2() > epsilon_sqr<float>()) {
            normal.normalize();
            up = normal;
        }
    }
    // The view direction points from the camera into the scene, so the walker's up
    // must oppose it; otherwise the eye centers would land behind the surface.
    if (dot(up, camera()->view_direction()) > 0.0f)
        up = -up;
    return true;
}

float MappleViewer::camera_marker_width() const {
    // The glyphs are scene geometry, so size them from the models: the scene radius
    // would include the glyphs' own bounds and grow with each rebuild. Without a model
    // the keyframe spread is used, which does not depend on the glyph size.
    Box3 box;
    for (const auto &model : models())
        box.grow(model->bounding_box());
    if (!box.is_valid() && kfi_)
        for (std::size_t i = 0; i < kfi_->number_of_keyframes(); ++i)
            box.grow(kfi_->keyframe_position(i));
    return box.is_valid() ? box.diagonal_length() * 0.03f : 0.0f;
}

void MappleViewer::commit_ground_route() {
    if (!camera_path_builder_)
        return;
    if (route_segment_.size() >= 2)
        route_segments_.push_back(route_segment_);
    route_segment_.clear();

    // The stroke carries a surface sample per point: the contact point and the
    // walker's up there, so the poses tilt with the terrain.
    std::size_t added = 0;
    for (const auto &segment : route_segments_) {
        std::vector<vec3> points, ups;
        points.reserve(segment.size());
        ups.reserve(segment.size());
        for (const auto &sample : segment) {
            points.push_back(sample.point);
            ups.push_back(sample.up);
        }
        added += camera_path_builder_->generate_ground_route(points, ups);
    }
    route_segments_.clear();

    if (added == 0) {
        LOG(WARNING) << "the stroke is too short to generate walking keyframes";
        return; // the tool stays armed for another try
    }
    path_tool_ = PathTool::Off; // one-shot
    camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << added << " walking keyframes added to the camera path";
    update();
}

void MappleViewer::commit_flight_track() {
    if (!camera_path_builder_)
        return;
    if (route_segment_.size() >= 2)
        route_segments_.push_back(route_segment_);
    route_segment_.clear();

    // A flight bridges the gaps where the drag left the surface, so all segments are
    // concatenated into one camera track (unlike the walking route, whose segments
    // become separate walks).
    std::vector<vec3> points, ups;
    for (const auto &segment : route_segments_) {
        for (const auto &sample : segment) {
            points.push_back(sample.point);
            ups.push_back(sample.up);
        }
    }
    route_segments_.clear();

    flight_positions_ = camera_path_builder_->flight_stations(points, ups);
    if (flight_positions_.size() < 2) {
        flight_positions_.clear();
        LOG(WARNING) << "the stroke is too short to trace a flight track";
        return; // the tool stays armed for another try
    }
    flight_aim_ready_ = true;
    LOG(INFO) << flight_positions_.size()
              << " flight stations traced: click to aim at one target, or drag to trace the aim";
    update();
}

void MappleViewer::commit_flight_aim() {
    if (!camera_path_builder_ || !flight_aim_ready_ || flight_positions_.size() < 2) {
        clear_flight_gesture();
        return;
    }

    // The last segment may hold the click's single sample, so all of it is kept: one
    // point aims every station, more are sampled along the aim stroke.
    std::vector<vec3> points;
    for (const auto &segment : route_segments_)
        for (const auto &sample : segment)
            points.push_back(sample.point);
    for (const auto &sample : route_segment_)
        points.push_back(sample.point);
    route_segment_.clear();
    route_segments_.clear();
    if (points.empty()) {
        LOG(WARNING) << "the aim has to end on a visible model surface";
        return; // the track keeps its stations for another aim
    }

    std::vector<vec3> targets;
    if (points.size() < 2) // a click: every station aims at the picked point
        targets.assign(flight_positions_.size(), points.front());
    else
        targets = camera_path_builder_->aim_targets(points, flight_positions_.size());
    if (targets.size() != flight_positions_.size()) {
        LOG(WARNING) << "the aim could not be sampled along the flight track";
        return;
    }

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    const std::size_t added = camera_path_builder_->generate_flight(flight_positions_, targets);
    if (added == 0) {
        LOG(WARNING) << "the flight track could not be turned into keyframes";
        return;
    }

    path_tool_ = PathTool::Off; // one-shot
    flight_aim_ready_ = false;
    flight_positions_.clear();
    camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << added << " flight keyframes added to the camera path";
    update();
}

void MappleViewer::clear_flight_gesture() {
    flight_tracking_ = false;
    flight_aiming_ = false;
    flight_aim_ready_ = false;
    flight_positions_.clear();
}

void MappleViewer::apply_orbit_gesture(const vec2 &from, const vec2 &to) {
    if (!camera_path_builder_)
        return;
    camera_path_builder_->set_scene(models());
    if (!camera_path_builder_->scene_box().is_valid()) {
        LOG(WARNING) << "the orbit needs a visible model";
        return;
    }

    // The axis passes through the surface under the press; a click (no drag) keeps
    // the turntable default, the view's up axis through that point.
    const PickResult hit = pick(static_cast<int>(from.x), static_cast<int>(from.y));
    if (!hit) {
        LOG(WARNING) << "the orbit has to start on a visible model surface";
        return;
    }
    const vec3 up = camera_path_builder_->ground_up_direction();
    vec3 direction = -up;
    const vec2 drag = to - from;
    if (drag.length2() > 4.0f) { // 2 logical pixels: below that the gesture is a click
        // The image-plane direction that projects onto the dragged segment.
        direction = camera()->right_vector() * drag.x - camera()->up_vector() * drag.y;
        direction.normalize();
        // orbit_frames() keeps the axis as the camera's up; point a dragged axis so
        // that up stays the view's up, and an upward drag does not orbit upside down.
        if (dot(-direction, up) < 0.0f)
            direction = -direction;
    }
    const Line3 axis = Line3::from_point_and_direction(hit.position, direction);

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    path_tool_ = PathTool::Off; // one-shot
    if (orbit_replaces_path_)
        kfi_->delete_path();
    camera_path_builder_->generate_orbit(axis, camera()->scene_center());
    camera_keyframe_index_ = static_cast<int>(kfi_->number_of_keyframes()) - 1;
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << (orbit_replaces_path_ ? "camera path replaced by an orbit (Ctrl+Z restores it)"
                                       : "orbit added to the camera path (Ctrl+Z removes it)");
    update();
}

void MappleViewer::snapshot_camera_path() {
    camera_path_snapshot_.frames.clear();
    camera_path_snapshot_.times.clear();
    for (std::size_t i = 0; kfi_ && i < kfi_->number_of_keyframes(); ++i) {
        camera_path_snapshot_.frames.push_back(kfi_->keyframe(i));
        camera_path_snapshot_.times.push_back(kfi_->keyframe_time(i));
    }
    camera_path_snapshot_.keyframe_index = camera_keyframe_index_;
    camera_path_snapshot_.valid = true;
}

bool MappleViewer::undo_camera_path() {
    if (!camera_path_snapshot_.valid || !kfi_)
        return false;

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    kfi_->delete_path();
    for (std::size_t i = 0; i < camera_path_snapshot_.frames.size(); ++i)
        kfi_->add_keyframe(camera_path_snapshot_.frames[i], camera_path_snapshot_.times[i]);
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    camera_keyframe_index_ = std::min(camera_path_snapshot_.keyframe_index, count - 1);
    camera_path_snapshot_.valid = false;

    if (count > 0 && !show_camera_path_)
        set_camera_path_visible(true);
    else if (count == 0 && show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    }
    LOG(INFO) << "camera path restored: " << count << " keyframes";
    update();
    return true;
}

//------------------------------------------------------------------------
// Video recording
//------------------------------------------------------------------------

#ifdef HAS_FFMPEG

MappleViewer::VideoRecording::~VideoRecording() = default;

bool MappleViewer::recording_video() const {
    return video_recording_ != nullptr;
}

void MappleViewer::cancel_video_recording() {
    if (video_recording_)
        video_recording_->cancel_requested = true;
}

void MappleViewer::record_camera_path() {
    if (video_recording_)
        return;
    if (kfi_->number_of_keyframes() < 2) {
        LOG(WARNING) << "recording needs at least two keyframes";
        return;
    }

    std::vector<Frame> frames;
    try {
        frames = kfi_->interpolate(); // the whole path, one sample per video frame
    } catch (const std::exception &e) {
        LOG(ERROR) << "cannot record the camera path: " << e.what();
        return;
    }
    if (frames.empty()) {
        LOG(WARNING) << "nothing to record (the camera path is empty)";
        return;
    }

    const Model *model = current_model();
    std::string default_name("camera_path.mp4");
    if (model)
        default_name = file_system::replace_extension(model->name(), "mp4");
    const std::string file_name = dialog::save(
            "Record the camera path into a video", default_name,
            {"Video files (*.mp4 *.mov *.avi)", "*.mp4 *.mov *.avi", "All files (*.*)", "*"});
    if (file_name.empty())
        return;

    // The encoder needs multiples of 8; cropping at most 7 rows/columns keeps the
    // video identical to the view (its top-left region).
    const int width = framebuffer_width() / 8 * 8;
    const int height = framebuffer_height() / 8 * 8;
    if (width < 8 || height < 8) {
        LOG(ERROR) << "the view is too small to record (" << framebuffer_width() << " x "
                   << framebuffer_height() << " pixels)";
        return;
    }

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();

    auto *recording = new VideoRecording;
    recording->encoder.reset(new VideoEncoder(file_name, kfi_->frame_rate(),
                                              video_bitrate_mbps_ * 1024 * 1024));
    recording->frames = std::move(frames);
    recording->width = width;
    recording->height = height;
    recording->capture_width = framebuffer_width();
    recording->capture_height = framebuffer_height();
    recording->samples = static_cast<unsigned>(std::max(1, samples()));
    recording->file_name = file_name;
    recording->start = std::chrono::steady_clock::now();
    recording->camera = *camera()->frame();
    recording->snapshot_hud = snapshot_hud_;
    // Video frames are scene-only: the capture skips the viewer HUD, and the client
    // overlays are dropped for the captured frame (pre_draw() reinstalls them).
    snapshot_hud_ = false;
    video_recording_.reset(recording);

    LOG(INFO) << "recording " << recording->frames.size() << " frames at " << kfi_->frame_rate()
              << " fps and " << video_bitrate_mbps_ << " Mbit/s into " << file_name;
    update();
}

void MappleViewer::advance_video_recording() {
    if (!video_recording_)
        return;
    VideoRecording &r = *video_recording_;

    // The client overlays (draw_gui() already rebuilt the UI this frame) must not
    // enter the captured frame.
    set_overlays({});

    std::string error;
    if (!r.cancel_requested && r.next_frame < r.frames.size()) {
        if (framebuffer_width() != r.capture_width || framebuffer_height() != r.capture_height)
            error = "the view was resized during the recording";
        else {
            const Frame &f = r.frames[r.next_frame];
            camera()->frame()->set_position_and_orientation(f.position(), f.orientation());
            try {
                // Video frames never inspect depth or picking IDs, so skip those
                // full-resolution passes and readbacks.
                const graphics::FrameImage image = capture(r.samples, graphics::FrameReadback::color);
                const unsigned char *data = image.rgba.data();
                if (image.width != unsigned(r.width) || image.height != unsigned(r.height)) {
                    r.pixels.resize(std::size_t(r.width) * r.height * 4);
                    for (int y = 0; y < r.height; ++y)
                        std::memcpy(r.pixels.data() + std::size_t(y) * r.width * 4,
                                    image.rgba.data() + std::size_t(y) * image.width * 4,
                                    std::size_t(r.width) * 4);
                    data = r.pixels.data();
                }
                if (!r.encoder->encode(data, r.width, r.height, VideoEncoder::PIX_FMT_RGBA_8888))
                    error = "the video encoder rejected a frame";
            } catch (const std::exception &e) {
                error = e.what();
            }
            if (error.empty())
                ++r.next_frame;
        }
    }

    const bool cancelled = r.cancel_requested;
    if (error.empty() && !cancelled && r.next_frame < r.frames.size()) {
        update();
        return;
    }

    // Done, failed or cancelled: finalize the file and restore the user's view.
    const std::size_t rendered = r.next_frame;
    const std::size_t total = r.frames.size();
    const std::string file_name = r.file_name;
    const Frame camera_state = r.camera;
    const bool snapshot_hud = r.snapshot_hud;
    const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - r.start).count();

    video_recording_.reset(); // destroys the encoder, which finalizes the stream
    snapshot_hud_ = snapshot_hud;
    camera()->frame()->set_position_and_orientation(camera_state.position(), camera_state.orientation());
    update();

    if (!error.empty())
        LOG(ERROR) << "video recording failed after " << rendered << " of " << total
                   << " frames: " << error;
    else if (cancelled)
        LOG(WARNING) << "video recording cancelled; " << rendered << " frames written to " << file_name;
    else
        LOG(INFO) << "video saved to " << file_name << " (" << total << " frames, "
                  << static_cast<int>(seconds + 0.5) << " s)";
}

void MappleViewer::draw_record_progress() {
    if (!video_recording_)
        return;
    const VideoRecording &r = *video_recording_;

    // Dim the scene; the window itself stays fully lit.
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::GetBackgroundDrawList()->AddRectFilled(
            viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
            IM_COL32(0, 0, 0, 96));

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin("Record Camera Path", nullptr, flags)) {
        ImGui::End();
        return;
    }

    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - r.start;
    ImGui::TextDisabled("%s", r.file_name.c_str());
    ImGui::TextDisabled("%d x %d pixels, %d fps", r.width, r.height, kfi_->frame_rate());
    ImGui::Spacing();

    char label[64];
    std::snprintf(label, sizeof(label), "%d / %d frames", static_cast<int>(r.next_frame),
                  static_cast<int>(r.frames.size()));
    const float progress = r.frames.empty() ? 0.0f
                                            : static_cast<float>(r.next_frame) / static_cast<float>(r.frames.size());
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), label);
    ImGui::Text("Running for %.1f s", elapsed.count());
    ImGui::Spacing();
    if (ImGui::Button("Cancel", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        cancel_video_recording();
    ImGui::SameLine();
    ImGui::TextDisabled("Esc cancels; frames written so far are kept.");
    ImGui::End();
}

#else // !HAS_FFMPEG

bool MappleViewer::recording_video() const { return false; }

void MappleViewer::record_camera_path() {
    LOG(WARNING) << "video recording requires FFmpeg (configure Easy3D with -DEasy3D_ENABLE_FFMPEG=ON)";
}

void MappleViewer::advance_video_recording() {}

void MappleViewer::draw_record_progress() {}

void MappleViewer::cancel_video_recording() {}

#endif // HAS_FFMPEG
