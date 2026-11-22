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
 * Easy3D is distributed in the hope that it will be useful in your
 * research/work, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */

#include "camera_path_timeline.h"

#include <3rd_party/imgui/imgui.h>

#include <algorithm>
#include <cmath>

CameraPathTimeline::CameraPathTimeline(easy3d::KeyFrameInterpolator* kfi)
        : kfi_(kfi) {
}


void CameraPathTimeline::build_segments() {
    segments_.clear();
    keyframe_times_.clear();
    total_duration_ = 0.0f;
    segments_dirty_ = false;

    if (!kfi_)
        return;

    const std::size_t n = kfi_->number_of_keyframes();
    if (n == 0)
        return;

    float t = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        // A marker belongs to the pose at the start of this keyframe's dwell,
        // or, equivalently, the end of the preceding travel segment.
        keyframe_times_.push_back(t);
        // Dwell at keyframe i.
        const float dwell = kfi_->keyframe_dwell(i);
        if (dwell > 0.0f) {
            Segment seg;
            seg.keyframe_index = static_cast<int>(i);
            seg.is_dwell = true;
            seg.start = t;
            seg.end = t + dwell;
            seg.duration = dwell;
            segments_.push_back(seg);
            t += dwell;
        }

        // Travel from keyframe i to i+1 (not after the last keyframe).
        if (i + 1 < n) {
            const float travel = kfi_->keyframe_time(i + 1) - kfi_->keyframe_time(i);
            if (travel > 0.0f) {
                Segment seg;
                seg.keyframe_index = static_cast<int>(i);
                seg.is_dwell = false;
                seg.start = t;
                seg.end = t + travel;
                seg.duration = travel;
                segments_.push_back(seg);
                t += travel;
            }
        }
    }

    total_duration_ = t;
    segments_dirty_ = false;
}


float CameraPathTimeline::total_duration() const {
    if (!cache_is_current())
        const_cast<CameraPathTimeline*>(this)->build_segments();
    return total_duration_;
}


bool CameraPathTimeline::cache_is_current() const {
    // keyframe_times_ holds exactly one entry per keyframe, so a path that grew or
    // shrank without an invalidate() is caught here: every index the strip hands out
    // then names a keyframe that is still there.
    return kfi_ && !segments_dirty_ &&
           keyframe_times_.size() == kfi_->number_of_keyframes();
}


int CameraPathTimeline::find_keyframe_at(float x, float width) const {
    if (keyframe_times_.empty() || width <= 0.0f)
        return -1;

    if (total_duration_ <= 0.0f)
        return 0;

    const float t = std::max(0.0f, std::min(total_duration_, x / width * total_duration_));
    // A marker always selects its own keyframe. This makes the last keyframe
    // reachable and avoids selecting the preceding travel segment at a shared
    // boundary.
    const float marker_radius = total_duration_ * 7.0f / width;
    for (std::size_t i = 0; i < keyframe_times_.size(); ++i)
        if (std::abs(t - keyframe_times_[i]) <= marker_radius)
            return static_cast<int>(i);
    for (const auto& seg : segments_) {
        if (t >= seg.start && t <= seg.end)
            return seg.keyframe_index;
    }
    return -1;
}


bool CameraPathTimeline::draw(float width, int* selected_keyframe) {
    if (width <= 0.0f || !kfi_)
        return false;

    if (!cache_is_current())
        build_segments();

    const int count = static_cast<int>(kfi_->number_of_keyframes());
    const float height = 24.0f;     // strip height
    const float marker_w = 3.0f;    // keyframe marker width
    const float padding = 4.0f;     // left/right padding inside the strip

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();

    // Background.
    dl->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
                      IM_COL32(40, 40, 40, 255), 3.0f);

    // Timeline origin (after left padding).
    const float origin = p.x + padding;
    const float plot_w = width - 2.0f * padding;

    const float duration = std::max(total_duration_, 1.0f);
    const auto timeline_x = [&](float time) { return origin + time / duration * plot_w; };

    // Draw each segment as a horizontal bar. A path with one keyframe has no
    // segments but still displays its selectable marker below.
    for (const auto& seg : segments_) {
        const float x0 = timeline_x(seg.start);
        const float x1 = timeline_x(seg.end);
        const ImVec2 a(x0, p.y + 4.0f);
        const ImVec2 b(x1, p.y + height - 4.0f);

        if (seg.is_dwell) {
            // Dwell pause: gray with rounded ends.
            dl->AddRectFilled(a, b, IM_COL32(100, 100, 100, 200), 2.0f);
        } else {
            // Travel segment: colored by keyframe index for visual distinction.
            const int idx = seg.keyframe_index >= 0 ? seg.keyframe_index : 0;
            const unsigned hue = static_cast<unsigned>((idx * 137) % 360);
            float red, green, blue;
            ImGui::ColorConvertHSVtoRGB(hue / 360.0f, 0.6f, 0.9f, red, green, blue);
            const ImU32 col = IM_COL32(static_cast<unsigned>(red * 255.0f),
                                       static_cast<unsigned>(green * 255.0f),
                                       static_cast<unsigned>(blue * 255.0f), 255);
            dl->AddRectFilled(a, b, col, 2.0f);
        }
    }

    const int selected = selected_keyframe ? *selected_keyframe : -1;
    for (std::size_t i = 0; i < keyframe_times_.size(); ++i) {
        const float x = timeline_x(keyframe_times_[i]);
        dl->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + height),
                    static_cast<int>(i) == selected ? IM_COL32(255, 210, 70, 255)
                                                     : IM_COL32(200, 200, 200, 180),
                    marker_w);
    }

    // Playhead / current position: red vertical line at the playback's position along
    // the timeline (travel + dwell), drawn while playback runs.
    // if (kfi_->is_interpolation_started()) // Commented out so the redline is always shown.
    {   // Shown unconditionally
        const float elapsed = kfi_->playback_elapsed();
        const float x = origin + elapsed / duration * plot_w;
        dl->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + height), IM_COL32(255, 60, 60, 255), 2.0f);
    }

    ImGui::InvisibleButton("##camera-path-timeline", ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    bool changed = false;
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        const float mx = ImGui::GetMousePos().x;
        const int kf = find_keyframe_at(mx - origin, plot_w);
        if (kf >= 0 && kf < count) {
            if (selected_keyframe && *selected_keyframe != kf) {
                *selected_keyframe = kf;
                changed = true;
            }
        }
    }

    // Tooltip with segment info.
    if (hovered) {
        const float mx = ImGui::GetMousePos().x;
        const int kf = find_keyframe_at(mx - origin, plot_w);
        if (kf >= 0) {
            ImGui::BeginTooltip();
            ImGui::Text("Keyframe %d", kf + 1);
            ImGui::Text("Starts at %.2f s", keyframe_times_[static_cast<std::size_t>(kf)]);
            const float dwell = kfi_->keyframe_dwell(static_cast<std::size_t>(kf));
            if (dwell > 0.0f)
                ImGui::Text("Dwell: %.2f s", dwell);
            ImGui::EndTooltip();
        }
    }

    ImGui::TextDisabled("%.1f s total%s", total_duration_ / kfi_->interpolation_speed(), total_duration_ == 0.0f ? " (single keyframe)" : "");
    return changed;
}
