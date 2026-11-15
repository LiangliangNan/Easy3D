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
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <easy3d/core/model.h>
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
