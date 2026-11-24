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

    // The Camera Path window's fixed width (logical pixels): wide enough for the
    // longest one-line tool hint at the default UI font, so no hint wraps or
    // clips and arming another tool never resizes the panel. The height is
    // auto-fitted instead.
    constexpr float kCameraPathWindowWidth = 430.0f;
}

void MappleViewer::open_animation_window() {
    show_animation_window_ = true;
}

void MappleViewer::draw_animation_window() {
    if (!show_animation_window_)
        return;

    // Fixed width, auto height: the size constraints pin the width and
    // AlwaysAutoResize fits the height to the current mode's widgets.
    ImGui::SetNextWindowSizeConstraints(ImVec2(kCameraPathWindowWidth, 0),
                                        ImVec2(FLT_MAX, FLT_MAX));
    if (!ImGui::Begin("Camera Path", &show_animation_window_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        if (!show_animation_window_) { // closed via the title bar
            // Do not leave the camera moving with no control in sight; Ctrl+K still
            // toggles playback afterwards.
            if (kfi_->is_interpolation_started())
                kfi_->stop_interpolation();
            keyframe_editor_->set_selected(-1);
            // The tool disarms with its window; a gesture in progress is abandoned,
            // and the keyframe handles go away with the editing mode the window opens.
            path_tool_ = PathTool::Off;
            sync_keyframe_editing();   // no handles once the window is gone
            cancel_path_gesture();
        }
        return;
    }

    KeyFrameInterpolator *kfi = kfi_.get();
    const int count = static_cast<int>(kfi->number_of_keyframes());
    const int selected_index = keyframe_editor_->selected();

    if (ImGui::Button("Import...", ImVec2(100, 0)))
        import_camera_path();
    ImGui::SameLine();
    if (ImGui::Button("Export...", ImVec2(100, 0)))
        export_camera_path();
    ImGui::SameLine();
    ImGui::TextDisabled(".kf files");
    // Clear empties the whole path, so it sits apart from the keyframe tools, at the
    // far right of the file row.
    constexpr float kClearWidth = 70.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - kClearWidth);
    if (ImGui::Button("Clear", ImVec2(kClearWidth, 0)))
        clear_camera_path();

    // Mode: how the next path is made. Free keyframes by hand; a path tool owns the
    // scene's left press until its gesture commits, or Esc/Free takes it back.
    ImGui::Separator();
    const auto select_mode = [this](PathTool mode) {
        if (mode == path_tool_)
            return;
        if (mode == PathTool::Off)
            cancel_camera_path_tool();
        else
            arm_camera_path_tool(mode);
    };
    if (ImGui::RadioButton("Free", path_tool_ == PathTool::Off))
        select_mode(PathTool::Off);
    ImGui::SameLine();
    if (ImGui::RadioButton("Orbit...", path_tool_ == PathTool::Orbit))
        select_mode(PathTool::Orbit);
    ImGui::SameLine();
    if (ImGui::RadioButton("Walk...", path_tool_ == PathTool::GroundWalk))
        select_mode(PathTool::GroundWalk);
    ImGui::SameLine();
    if (ImGui::RadioButton("Flight...", path_tool_ == PathTool::Flight))
        select_mode(PathTool::Flight);

    // Read after the radios so a click shows the new mode's hint and parameters at once.
    const PathTool tool = path_tool_;
    if (tool == PathTool::Off) {
        ImGui::TextDisabled("Add keyframes (Alt+K), or choose a mode.");
        // A generated path is playable at once with its mode's default aims; the aim
        // gesture is offered on demand, never forced after the path gesture.
        if (aim_range_.valid && count > 0) {
            ImGui::TextDisabled("Aims of the new path: %s.",
                                aim_range_.mode == PathTool::GroundWalk ? "along the route"
                                : aim_range_.mode == PathTool::Flight ? "at the ground ahead"
                                                                      : "at the orbit axis");
            if (ImGui::Button("Aim...", ImVec2(80, 0)))
                arm_camera_path_tool(PathTool::Aim);
            ImGui::SetItemTooltip("Click a point to aim every keyframe there, or drag to trace the "
                                  "aims along the path. Ctrl+Z restores the previous aims.");
            ImGui::SameLine();
            ImGui::TextDisabled("Not satisfied with the aims?");
        }
    } else if (tool == PathTool::Aim)
        ImGui::TextDisabled("Click to aim every keyframe there, or drag to trace the aims\n"
            "along the path. Esc cancels.");
    else if (tool == PathTool::GroundWalk)
        ImGui::TextDisabled("Drag over the ground surface to trace the route. Poses rise\n"
            "straight up; Esc cancels.");
    else if (tool == PathTool::Flight)
        ImGui::TextDisabled("Drag over the surface to trace the camera track. Poses rise\n"
            "straight up and look at the ground ahead; Esc cancels.");
    else
        ImGui::TextDisabled("Click the surface where the orbit should stand; the axis runs there\n"
            "along the Up rule. Esc cancels.");

    // The mode's own parameters, under its hint.
    if (camera_path_builder_ &&
        (tool == PathTool::GroundWalk || tool == PathTool::Flight || tool == PathTool::Orbit))
        draw_ground_up_rule();
    if (camera_path_builder_ && (tool == PathTool::GroundWalk || tool == PathTool::Flight))
        draw_follow_slopes_rule();

    if (camera_path_builder_ && tool == PathTool::GroundWalk) {
        float height = camera_path_builder_->eye_height_in_scene_heights();
        if (ImGui::DragFloat("Eye height", &height, 0.01f, 0.0f, 100.0f, "%.2f x scene")) {
            camera_path_builder_->set_eye_height_in_scene_heights(height);
            update();
        }
        ImGui::SetItemTooltip("Eye height above the ground, as a fraction of the scene\n"
            "extent along the up direction. With Follow slopes on, poses keep\n"
            "this height above their own contact point.");

        draw_keyframe_spacing();
    } else if (camera_path_builder_ && tool == PathTool::Orbit) {
        ImGui::TextDisabled("Orbit axis has the Up direction through the clicked point");
        ImGui::SetItemTooltip("The camera circles the axis that rises from the surface point clicked,\n"
                              "along the direction the Up rule resolves to. Poses look at the axis;\n"
                              "Aim... re-aims them. The orbit replaces the path; Ctrl+Z restores it.\n");

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
        float height = camera_path_builder_->flight_height_in_scene_heights();
        if (ImGui::DragFloat("Flight height", &height, 0.01f, 0.0f, 100.0f, "%.2f x scene")) {
            camera_path_builder_->set_flight_height_in_scene_heights(height);
            update();
        }
        ImGui::SetItemTooltip("Height above the traced ground, as a fraction of the scene extent\n"
                              "along the up direction. With Follow slopes on, poses keep this\n"
                              "height above their own contact point.");

        draw_keyframe_spacing();
    }

    // Keyframes: made by the gesture in a path mode, by hand in Free (Alt+K or the
    // menu).
    ImGui::Separator();
    ImGui::Text("%d keyframe%s", count, count == 1 ? "" : "s");

    const bool selected = selected_index >= 0;
    ImGui::BeginDisabled(!selected);
    if (ImGui::Button("Insert After", ImVec2(110, 0)))
        insert_camera_keyframe();
    ImGui::SetItemTooltip("Inserts one after the selected, halfway to the next (past the end the last "
                          "segment is extended); drag its glyph to place it.");
    ImGui::SameLine();
    ImGui::EndDisabled();
    // Remove Last needs no selection, so it sits between the two selection-dependent
    // buttons, next to Delete Selected.
    if (ImGui::Button("Remove Last", ImVec2(110, 0)))
        remove_last_camera_keyframe();
    ImGui::SameLine();
    ImGui::BeginDisabled(!selected);
    if (ImGui::Button("Delete Selected", ImVec2(115, 0)))
        delete_camera_keyframe();
    ImGui::SetItemTooltip("Deletes the selected keyframe.");
    ImGui::EndDisabled();

    ImGui::BeginDisabled(count == 0);
    if (ImGui::Button("< Prev", ImVec2(80, 0)))
        goto_camera_keyframe(selected_index <= 0 ? count - 1 : selected_index - 1);
    ImGui::SameLine();
    if (selected_index >= 0)
        ImGui::Text("Keyframe %d / %d", selected_index + 1, count);
    else
        ImGui::TextDisabled("Keyframe - / %d", count);
    ImGui::SameLine();
    if (ImGui::Button("Next >", ImVec2(80, 0)))
        goto_camera_keyframe(selected_index < 0 || selected_index >= count - 1
                                     ? 0 : selected_index + 1);
    ImGui::EndDisabled();

    // The selection note belongs with the keyframe navigation, so it follows Next.
    ImGui::SameLine();
    if (selected)
        ImGui::TextDisabled("no. %d selected", selected_index + 1);
    else
        ImGui::TextDisabled("no keyframe selected");

    // Timeline strip: proportional bars for travel segments, gray for dwell pauses.
    // Clicking selects the corresponding keyframe. It takes a row of its own (the
    // navigation row above is full), and the recording's expected length below reads
    // the same duration.
    if (keyframe_timeline_ && count > 0) {
        int timeline_selected = selected_index;
        if (keyframe_timeline_->draw(ImGui::GetContentRegionAvail().x, &timeline_selected) &&
            timeline_selected >= 0)
            goto_camera_keyframe(timeline_selected);
    }

    const bool playing = kfi->is_interpolation_started();

    // Per-keyframe dwell: how long the camera pauses at the selected keyframe before
    // moving on. It is part of the keyframe, so it sits with the path's rhythm rather
    // than with the recording. A path whose keyframes all dwell 0 is unchanged.
    {
        // The selection is read here rather than reused from the top of the frame: a
        // button above can remove the selected keyframe in this very frame, and the
        // index captured then points past the shortened path.
        const int dwell_index = keyframe_editor_->selected();
        ImGui::BeginDisabled(dwell_index < 0 || playing);
        float dwell = dwell_index >= 0
                      ? kfi->keyframe_dwell(static_cast<std::size_t>(dwell_index)) : 0.0f;
        if (ImGui::DragFloat("Dwell (s)", &dwell, 0.1f, 0.0f, 10.0f, "%.1f s"))
            edit_camera_path_timing(static_cast<std::size_t>(dwell_index), dwell);
        ImGui::EndDisabled();
        if (dwell_index < 0)
            ImGui::SetItemTooltip("Select a keyframe (click its marker on the timeline) to set "
                                  "how long the camera pauses there.");
        else if (playing)
            ImGui::SetItemTooltip("Stop playback to change the timing.");
        else
            ImGui::SetItemTooltip("Seconds the camera pauses at this keyframe. The ease "
                                  "in/out around the pause is added on top. Ctrl+Z restores "
                                  "the path's timing.");
    }

    bool visible = show_camera_path_;
    if (ImGui::Checkbox("Show path and keyframe cameras (Ctrl+T)", &visible))
        set_camera_path_visible(visible);

    // The handles have no switch of their own: in Camera Path editing mode Alt is what
    // brings them out (see MappleViewer::set_alt_held()), so the hint says so.
    ImGui::TextDisabled("Hold Alt to drag the selected keyframe's glyph, ring or arrow\n"
                        "\t(Shift: its own plane), or Alt+click the scene to aim it.");
    if (camera_path_snapshot_.valid)
        ImGui::TextDisabled("Ctrl+Z restores the path from before the last edit; Esc cancels it.");

    // Playback and the interpolation it uses.
    ImGui::Separator();
    if (ImGui::Button(playing ? "Pause" : (camera_path_elapsed_ > 0 ? "Resume" : "Play"), ImVec2(120, 0)))
        toggle_camera_path_playback();
    ImGui::SameLine();
    const float elapsed_time = camera_path_elapsed_ / kfi_->interpolation_speed();
    if (playing)
        ImGui::TextDisabled("playing... %.1f s", elapsed_time);
    else
        ImGui::TextDisabled(camera_path_elapsed_ > 0 ? "Paused at %.1f s" : "Ctrl+K", elapsed_time);

    float speed = kfi->interpolation_speed();
    if (ImGui::SliderFloat("Speed (keyframes/s)", &speed, 0.1f, 10.0f, "%.1f") && speed != kfi->interpolation_speed()) {
        kfi->set_interpolation_speed(speed);
        camera_path_elapsed_ = 0; // this reset the playback
    }
    ImGui::SetItemTooltip("Press 'Play' to check the current speed; slide to adjust.");
    int fps = kfi->frame_rate();
    if (ImGui::SliderInt("Frame rate (frames/s)", &fps, 1, 120, "%d") && fps != kfi->frame_rate())
        kfi->set_frame_rate(fps);

    // Recording and its parameters.
    ImGui::Separator();
#ifdef HAS_FFMPEG
    ImGui::BeginDisabled(count < 2);
    if (ImGui::Button("Record Video...", ImVec2(140, 0)))
        record_camera_path();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("MP4, scene only");
    if (count < 2)
        ImGui::TextDisabled("Recording needs at least two keyframes.");
    else
        // The frames the recording will render: the path's own duration (travel plus
        // dwell, as the timeline strip lays it out) at the interpolation speed.
        ImGui::TextDisabled("%.1f s at speed %.1f; %d frames at %d fps",
                            camera_path_duration() / kfi->interpolation_speed(),
                            kfi->interpolation_speed(),
                            static_cast<int>(camera_path_duration() / kfi->interpolation_speed() *
                                             static_cast<float>(kfi->frame_rate())) + 1,
                            kfi->frame_rate());

    // The recording's own multisample count, on the ladder the Snapshot dialog uses:
    // supported_sample_mask() holds the counts this viewer can render, so an
    // unrenderable one is never offered and the choice needs no step-down warning.
    // Recorded frames go through capture(), so the count is independent of the
    // window's own MSAA.
    static const int ladder[] = {1, 2, 4, 8, 16, 32, 64};
    static const char *const names[] = {"1", "2", "4", "8", "16", "32", "64"};
    const unsigned available = supported_sample_mask();
    const char *shown[IM_ARRAYSIZE(ladder)];
    int values[IM_ARRAYSIZE(ladder)], shown_count = 0, current = 0;
    for (int i = 0; i < IM_ARRAYSIZE(ladder); ++i)
        if (available & unsigned(ladder[i])) {
            shown[shown_count] = names[i];
            values[shown_count] = ladder[i];
            if (video_samples_ >= ladder[i]) current = shown_count;
            ++shown_count;
        }
    // A count the settings file carried from another device lands on the largest
    // offered value at most it, and is saved back as that value; 1 is always offered.
    video_samples_ = values[current];
    if (ImGui::Combo("Samples", &current, shown, shown_count))
        video_samples_ = values[current];
    ImGui::TextDisabled("1 means no multisampling; only the supported counts are listed.");

    ImGui::SliderInt("Bitrate (Mbit/s)", &video_bitrate_mbps_, 1, 32);
    ImGui::TextDisabled("Higher bitrate results in better video quality but a larger video file.");
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
    clear_aim_range(); // the edited path is no longer the one the aim prompt offers
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    if (show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << "keyframe " << kfi_->number_of_keyframes() << " added to the camera path";
    update();
}

void MappleViewer::remove_last_camera_keyframe() {
    if (kfi_->number_of_keyframes() == 0)
        return;
    kfi_->delete_last_keyframe();
    clear_aim_range();
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    LOG(INFO) << "last keyframe removed (" << kfi_->number_of_keyframes() << " remaining)";
    update();
}

void MappleViewer::clear_camera_path() {
    if (kfi_->number_of_keyframes() == 0)
        return;
    kfi_->delete_path();
    clear_aim_range();
    keyframe_editor_->set_selected(-1);
    if (show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    }
    LOG(INFO) << "camera path cleared";
    update();
}

void MappleViewer::insert_camera_keyframe() {
    const int index = keyframe_editor_->insert_after_selected();
    if (index < 0) {
        LOG(ERROR) << "failed to insert a keyframe";
        return;
    }
    clear_aim_range(); // the inserted keyframe shifts the path's indices
    if (keyframe_timeline_)
        keyframe_timeline_->invalidate(); // one more keyframe: one more segment
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << "keyframe no. " << index + 1 << " inserted (" << kfi_->number_of_keyframes()
              << " keyframes)";
    update();
}

void MappleViewer::delete_camera_keyframe() {
    const int index = keyframe_editor_->delete_selected();
    if (index < 0)
        return;
    clear_aim_range(); // the deleted keyframe shifts the path's indices
    if (keyframe_timeline_)
        keyframe_timeline_->invalidate(); // the segment it opened is gone
    if (kfi_->number_of_keyframes() == 0 && show_camera_path_) {
        release_camera_path_guides();
        restore_scene_bounds();
    }
    LOG(INFO) << "keyframe no. " << index + 1 << " deleted (" << kfi_->number_of_keyframes()
              << " remaining)";
    update();
}

void MappleViewer::edit_camera_path_timing(std::size_t index, float dwell_seconds) {
    if (!kfi_ || index >= kfi_->number_of_keyframes())
        return;
    const float dwell = clamp(dwell_seconds, 0.0f, 10.0f);
    if (dwell == kfi_->keyframe_dwell(index))
        return;

    snapshot_camera_path(); // one undo level, as for a pose edit
    kfi_->set_keyframe_dwell(index, dwell);
    // The dwell is part of the path's timing, which the drawn path and the timeline
    // strip are both derived from.
    if (keyframe_timeline_)
        keyframe_timeline_->invalidate();
    if (show_camera_path_)
        update_camera_path_guides();
    LOG(INFO) << "keyframe no. " << index + 1 << " dwells " << dwell << " s";
    update();
}

float MappleViewer::camera_path_duration() const {
    // The library's own timeline (travel + dwell): one definition, so the readout and the
    // recorded frames can never disagree.
    return kfi_ ? kfi_->total_duration() : 0.0f;
}

void MappleViewer::toggle_camera_path_playback() {
    if (kfi_->is_interpolation_started()) {
        kfi_->stop_interpolation();
        LOG(INFO) << "camera path playback stopped";
    } else if (kfi_->number_of_keyframes() < 2) {
        LOG(WARNING) << "camera path playback needs at least two keyframes";
        return;
    } else {
        // A fresh start: the playback clock begins at "now", so pressing Play always
        // walks the path from its first keyframe instead of catching up from a stale
        // deadline (a session-constructed or paused value) and fast-forwarding. The
        // elapsed readout keeps the value left by a paused playback; it is reset when
        // the playback reaches its end (see advance_camera_path()).
        next_camera_path_frame_ = std::chrono::steady_clock::now();
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
    keyframe_editor_->set_selected(index);
    update();
}

bool MappleViewer::aim_selected_keyframe_at(int x, int y) {
    const int index = keyframe_editor_->editable_keyframe();
    if (index < 0 || !point_in_scene(x, y))
        return false;

    // Only scene content is a target: the glyphs, the path and the overlays are
    // annotations, and a click on those keeps its own meaning.
    const PickResult hit = pick(x, y);
    if (!hit || !hit.model)
        return false;

    if (!keyframe_editor_->aim_at(hit.position))
        return false;
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
    sync_keyframe_editing();
    update();
}

void MappleViewer::import_camera_path() {
    const Model *model = current_model();
    const std::string directory = model ? file_system::parent_directory(model->name()) : "./";
    const std::string file_name = dialog::open("Import keyframes from file", directory, keyframe_filters());
    if (file_name.empty())
        return;
    if (kfi_->read_keyframes(file_name)) {
        keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
        clear_aim_range(); // the imported path has no mode default
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

    cancel_path_gesture(); // arming starts from a clean slate
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
    if (tool == PathTool::Aim && !aim_range_.valid) {
        LOG(WARNING) << "there is no generated path to re-aim";
        return;
    }

    path_tool_ = tool;
    snapshot_camera_path(); // Ctrl+Z restores the path the tool started from
    LOG(INFO) << (tool == PathTool::GroundWalk
                  ? "walk route tool armed: drag over the ground surface to trace the route"
                  : tool == PathTool::Orbit
                    ? "orbit tool armed: click the surface to stand the orbit axis on, "
                      "along the Up rule"
                    : tool == PathTool::Flight
                      ? "flight tool armed: drag over the surface to trace the camera track"
                      : "aim tool armed: click a point or drag over the surface to re-aim the last path");
    update();
}

void MappleViewer::cancel_camera_path_tool() {
    cancel_path_gesture();
    path_tool_ = PathTool::Off;
    update();
}

void MappleViewer::cancel_path_gesture() {
    tracing_route_ = false;
    route_stroke_screen_.clear();
    route_segment_.clear();
    route_segments_.clear();
    keyframe_editor_->cancel();
}

bool MappleViewer::route_point_at(int x, int y, vec3 &point) const {
    if (!point_in_scene(x, y))
        return false;
    const PickResult result = pick(x, y);
    if (!result)
        return false;
    // Only where the surface is: the camera's up is the path's one up direction, so no
    // face normal is read and a point cloud works as well as a mesh.
    point = result.position;
    return true;
}

void MappleViewer::draw_keyframe_spacing() {
    // One value for both ground tracks, measured against the scene, so neither the eye
    // height nor the flight height moves the keyframe density.
    float spacing = camera_path_builder_->keyframe_spacing_in_scene_heights();
    // Like the widgets around it, the drag keeps the window's default item width, so the
    // rows of this mode's parameters line up with each other.
    if (ImGui::DragFloat("Keyframe spacing", &spacing, 0.01f, 0.01f, 10.0f, "%.2f x scene")) {
        camera_path_builder_->set_keyframe_spacing_in_scene_heights(spacing);
        update();
    }
    ImGui::SetItemTooltip("Distance between consecutive keyframes along the track, as a fraction\n"
                          "of the scene extent along the up direction. The walk and the flight\n"
                          "share it, so the eye and flight heights do not change it. The stroke\n"
                          "is divided into equal steps of this length, both ends included.");
}

void MappleViewer::draw_ground_up_rule() {
    // One rule for the whole path, in all three generated modes: the camera's up is an
    // anti-gravity direction, not a surface normal. The combo names the rule and the
    // readout beside it shows what the app resolved, so the user never has to infer it
    // from the resulting tilt - or, in Orbit, from the axis it turns around.
    const char *const kRules[] = {"Auto (box shortest)", "+X", "+Y", "+Z", "Up from the view"};
    constexpr int kRuleCount = 5;
    int rule = static_cast<int>(camera_path_builder_->ground_up_axis());
    if (ImGui::Combo("Up", &rule, kRules, kRuleCount)) {
        camera_path_builder_->set_ground_up_axis(static_cast<CameraPathBuilder::GroundUpAxis>(rule));
        update();
    }
    ImGui::SetItemTooltip("The direction the camera's up points, for the whole path. Auto uses the\n"
                          "scene's shortest extent (upright for a building or a terrain); the axes\n"
                          "pin it to the world; Up from the view follows the camera, for a model you\n"
                          "have rotated upright. Walk and flight rise along it from the traced\n"
                          "points, so a slope does not bank the camera; Orbit runs its axis along it.");
    const vec3 up = camera_path_builder_->ground_up_direction();
    ImGui::SameLine();
    ImGui::TextDisabled("( %.2f, %.2f, %.2f )", up.x, up.y, up.z);
    ImGui::SetItemTooltip("The up direction the rule above resolves to right now. The scene's\n"
                          "three axes are drawn through its center while the path is shown.");
}

void MappleViewer::draw_follow_slopes_rule() {
    // Immediately under the up rule: whether the track climbs with the ground. Both ground
    // modes use it, so it sits beside the rule they share rather than in either mode's
    // own parameters. Orbit builds no ground track and does not draw this one.
    bool follow = camera_path_builder_->follow_ground_slopes();
    if (ImGui::Checkbox("Follow slopes", &follow)) {
        camera_path_builder_->set_follow_ground_slopes(follow);
        update();
    }
    ImGui::SetItemTooltip("On, every pose keeps its height above the ground under it, so the\n"
                          "track climbs and falls with a slope. Off, the whole track keeps the\n"
                          "altitude of the stroke's first point: the camera flies level, or\n"
                          "walks level, whatever the ground does beneath it.");
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

    // The stroke carries a contact point per sample: the feet follow the surface while
    // the camera stays upright on the path's one up direction.
    const int before = static_cast<int>(kfi_->number_of_keyframes());
    std::size_t added = 0;
    for (const auto &segment : route_segments_)
        added += camera_path_builder_->generate_ground_route(segment);
    route_segments_.clear();

    if (added == 0) {
        LOG(WARNING) << "the stroke is too short to generate walking keyframes";
        return; // the tool stays armed for another try
    }
    path_tool_ = PathTool::Off; // one-shot
    set_aim_range(PathTool::GroundWalk, before, static_cast<int>(kfi_->number_of_keyframes()) - 1);
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << added << " walking keyframes added to the camera path (aiming along the route)";
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
    std::vector<vec3> points;
    for (const auto &segment : route_segments_)
        points.insert(points.end(), segment.begin(), segment.end());
    route_segments_.clear();

    const std::vector<CameraPathBuilder::FlightStation> stations =
        camera_path_builder_->flight_stations(points);
    if (stations.size() < 2) {
        LOG(WARNING) << "the stroke is too short to trace a flight track";
        return; // the tool stays armed for another try
    }

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    const int before = static_cast<int>(kfi_->number_of_keyframes());
    // No aims: the flight's default, looking at the ground ahead along the track.
    const std::size_t added = camera_path_builder_->generate_flight(stations, {});
    if (added == 0) {
        LOG(WARNING) << "the flight track could not be turned into keyframes";
        return;
    }
    path_tool_ = PathTool::Off; // one-shot
    set_aim_range(PathTool::Flight, before, static_cast<int>(kfi_->number_of_keyframes()) - 1);
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << added << " flight keyframes added to the camera path (aiming at the ground ahead)";
    update();
}

void MappleViewer::set_aim_range(PathTool mode, int first, int last) {
    aim_range_.valid = first >= 0 && last >= first;
    aim_range_.mode = mode;
    aim_range_.first = first;
    aim_range_.last = last;
}

void MappleViewer::clear_aim_range() {
    aim_range_ = AimRange{};
}

void MappleViewer::aim_path_range(const std::vector<vec3> &targets) {
    if (!camera_path_builder_ || !aim_range_.valid)
        return;
    const int count = aim_range_.last - aim_range_.first + 1;
    if (count < 1 || static_cast<int>(targets.size()) != count)
        return;

    std::vector<Frame> frames;
    frames.reserve(static_cast<std::size_t>(count));
    for (int i = aim_range_.first; i <= aim_range_.last; ++i)
        frames.push_back(kfi_->keyframe(static_cast<std::size_t>(i)));

    // A walking pose keeps the up it stands on and an orbit pose keeps the orbit's
    // up, so each pose's own up is the roll reference; a flight keeps the horizon
    // level with the path's up instead.
    std::vector<vec3> roll_ups;
    if (aim_range_.mode == PathTool::Flight)
        roll_ups.assign(static_cast<std::size_t>(count), camera_path_builder_->ground_up_direction());

    const std::vector<Frame> aimed = camera_path_builder_->aimed_frames(frames, targets, roll_ups);
    if (static_cast<int>(aimed.size()) != count) {
        LOG(WARNING) << "the aim could not be applied to the path";
        return;
    }

    snapshot_camera_path(); // Ctrl+Z restores the aims from before the gesture
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    for (int i = 0; i < count; ++i)
        kfi_->set_keyframe_orientation(static_cast<std::size_t>(aim_range_.first + i),
                                       aimed[i].orientation());
    LOG(INFO) << count << " keyframe" << (count == 1 ? "" : "s")
              << " re-aimed (Ctrl+Z restores their poses)";
    update();
}

void MappleViewer::commit_aim_stroke() {
    if (!camera_path_builder_)
        return;

    // The last segment may hold the click's single sample, so all of it is kept: one
    // point aims every keyframe there, more are sampled along the aim stroke.
    std::vector<vec3> points;
    for (const auto &segment : route_segments_)
        points.insert(points.end(), segment.begin(), segment.end());
    points.insert(points.end(), route_segment_.begin(), route_segment_.end());
    route_segment_.clear();
    route_segments_.clear();
    if (points.empty()) {
        LOG(WARNING) << "the aim has to end on a visible model surface";
        return; // the tool stays armed for another try
    }
    if (!aim_range_.valid) {
        LOG(WARNING) << "the path to re-aim is gone (it was edited or replaced)";
        path_tool_ = PathTool::Off;
        update();
        return;
    }

    const int count = aim_range_.last - aim_range_.first + 1;
    std::vector<vec3> targets;
    if (points.size() < 2) // a click: every keyframe aims at the picked point
        targets.assign(static_cast<std::size_t>(count), points.front());
    else
        targets = camera_path_builder_->aim_targets(points, static_cast<std::size_t>(count));
    if (static_cast<int>(targets.size()) != count) {
        LOG(WARNING) << "the aim could not be sampled along the path";
        return;
    }

    aim_path_range(targets);
    path_tool_ = PathTool::Off; // one-shot
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

    // The click says where the axis stands; the up rule says which way it runs.
    const PickResult hit = pick(static_cast<int>(anchor.x), static_cast<int>(anchor.y));
    if (!hit) {
        LOG(WARNING) << "the orbit has to start on a visible model surface";
        return;
    }
    // orbit_frames() takes the axis as the reversed camera up, so the up rule through the
    // anchor is an axis pointing the other way.
    const Line3 axis = Line3::from_point_and_direction(hit.position,
                                                       -camera_path_builder_->ground_up_direction());

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    path_tool_ = PathTool::Off; // one-shot
    // An orbit is one closed path around the axis, so it replaces whatever path
    // was there; Ctrl+Z restores the previous one.
    kfi_->delete_path();
    camera_path_builder_->generate_orbit(axis, camera()->scene_center());
    set_aim_range(PathTool::Orbit, 0, static_cast<int>(kfi_->number_of_keyframes()) - 1);
    keyframe_editor_->set_selected(static_cast<int>(kfi_->number_of_keyframes()) - 1);
    if (!show_camera_path_)
        set_camera_path_visible(true);
    LOG(INFO) << "camera path replaced by an orbit (Ctrl+Z restores it)";
    update();
}

void MappleViewer::snapshot_camera_path() {
    camera_path_snapshot_.frames.clear();
    camera_path_snapshot_.times.clear();
    camera_path_snapshot_.dwells.clear();
    for (std::size_t i = 0; kfi_ && i < kfi_->number_of_keyframes(); ++i) {
        camera_path_snapshot_.frames.push_back(kfi_->keyframe(i));
        camera_path_snapshot_.times.push_back(kfi_->keyframe_time(i));
        camera_path_snapshot_.dwells.push_back(kfi_->keyframe_dwell(i));
    }
    camera_path_snapshot_.keyframe_index = keyframe_editor_->selected();
    camera_path_snapshot_.valid = true;
}

bool MappleViewer::undo_camera_path() {
    if (!camera_path_snapshot_.valid || !kfi_)
        return false;

    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();
    kfi_->delete_path();
    for (std::size_t i = 0; i < camera_path_snapshot_.frames.size(); ++i) {
        kfi_->add_keyframe(camera_path_snapshot_.frames[i], camera_path_snapshot_.times[i]);
        // The dwell belongs to the keyframe: a pose edit restores it, and so does a
        // dwell edit.
        if (i < camera_path_snapshot_.dwells.size())
            kfi_->set_keyframe_dwell(i, camera_path_snapshot_.dwells[i]);
    }
    if (keyframe_timeline_)
        keyframe_timeline_->invalidate();
    const int count = static_cast<int>(kfi_->number_of_keyframes());
    keyframe_editor_->set_selected(camera_path_snapshot_.keyframe_index);
    camera_path_snapshot_.valid = false;
    clear_aim_range(); // the restored path need not be the one the prompt offered

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
    // The count the window shows, resolved once more here: capture() applies the same
    // step-down, and its answer is what the encoder receives.
    recording->samples = static_cast<unsigned>(supported_sample_count(video_samples_));
    recording->file_name = file_name;
    recording->start = std::chrono::steady_clock::now();
    recording->camera = *camera()->frame();
    recording->snapshot_hud = snapshot_hud_;
    // Video frames are scene-only: the capture skips the viewer HUD, and the client
    // overlays are dropped for the captured frame (pre_draw() reinstalls them).
    snapshot_hud_ = false;
    video_recording_.reset(recording);

    LOG(INFO) << "recording " << recording->frames.size() << " frames at " << kfi_->frame_rate()
              << " fps with " << recording->samples << " samples and " << video_bitrate_mbps_
              << " Mbit/s into " << file_name;
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
    // Two different clocks, both worth showing: the video's own length is the frames
    // written at the frame rate, while the wall clock says how long the capture has been
    // running (it renders a frame at a time, so the two rarely match). Without the
    // video's length a pause of a dwell is easy to blame on the capture.
    const double video_seconds = kfi_->frame_rate() > 0
                                 ? double(r.next_frame) / double(kfi_->frame_rate()) : 0.0;
    ImGui::Text("Video so far: %.1f s of %.1f s", video_seconds,
                double(r.frames.size()) / double(std::max(1, kfi_->frame_rate())));
    ImGui::TextDisabled("Capturing for %.1f s", elapsed.count());
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
