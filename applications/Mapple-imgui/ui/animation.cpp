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

#include <algorithm>
#include <string>

#include <easy3d/core/model.h>
#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

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
        }
        return;
    }

    KeyFrameInterpolator *kfi = kfi_.get();
    const int count = static_cast<int>(kfi->number_of_keyframes());
    camera_keyframe_index_ = std::min(camera_keyframe_index_, count - 1);

    ImGui::Text("%d keyframe%s", count, count == 1 ? "" : "s");
    ImGui::TextDisabled("Move the camera, then add keyframes (Alt+K).");

    if (ImGui::Button("Add Keyframe", ImVec2(120, 0)))
        add_camera_keyframe();
    ImGui::SameLine();
    if (ImGui::Button("Remove Last", ImVec2(110, 0)))
        remove_last_camera_keyframe();
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(70, 0)))
        clear_camera_path();

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

    const char *methods[] = {"Spline interpolation", "Spline fitting"};
    int method = kfi->interpolation_method() == KeyFrameInterpolator::INTERPOLATION ? 0 : 1;
    if (ImGui::Combo("Interpolation", &method, methods, 2))
        kfi->set_interpolation_method(
                method == 0 ? KeyFrameInterpolator::INTERPOLATION : KeyFrameInterpolator::FITTING);

    bool visible = show_camera_path_;
    if (ImGui::Checkbox("Show path and keyframe cameras", &visible))
        set_camera_path_visible(visible);

    ImGui::Separator();
    if (ImGui::Button("Import...", ImVec2(100, 0)))
        import_camera_path();
    ImGui::SameLine();
    if (ImGui::Button("Export...", ImVec2(100, 0)))
        export_camera_path();
    ImGui::SameLine();
    ImGui::TextDisabled(".kf files");

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
    if (show_camera_path_)
        restore_scene_bounds();
    LOG(INFO) << "camera path cleared";
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

void MappleViewer::set_camera_path_visible(bool visible) {
    show_camera_path_ = visible;
    if (visible) {
        float radius = camera()->scene_radius();
        for (std::size_t i = 0; i < kfi_->number_of_keyframes(); ++i)
            radius = std::max(radius, distance(camera()->scene_center(), kfi_->keyframe_position(i)));
        camera()->set_scene_radius(radius);
    } else
        restore_scene_bounds();
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
