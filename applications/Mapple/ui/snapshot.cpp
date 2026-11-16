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
#include <cmath>
#include <string>

#include <easy3d/core/model.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

void MappleViewer::open_snapshot_dialog() {
    if (process_op_ != ProcessOp::NONE)
        return;
    // Start from the window's own sample count; the dialog can raise it.
    snapshot_params_.samples = std::max(1, samples());
    show_snapshot_dialog_ = true;
}

void MappleViewer::draw_snapshot_dialog() {
    if (!show_snapshot_dialog_)
        return;

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::GetBackgroundDrawList()->AddRectFilled(
            viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
            IM_COL32(0, 0, 0, 96));

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin("Snapshot", nullptr, flags)) {
        ImGui::End();
        return;
    }

    SnapshotParams &p = snapshot_params_;
    ImGui::TextDisabled("Scene only: panels, menus, the HUD and this dialog are not captured.");
    ImGui::Spacing();

    ImGui::InputFloat("Scale", &p.scale, 0.25f, 1.0f, "%.2f");
    p.scale = clamp(p.scale, 0.1f, 8.0f);
    const int width = static_cast<int>(std::ceil(framebuffer_width() * p.scale));
    const int height = static_cast<int>(std::ceil(framebuffer_height() * p.scale));
    ImGui::TextDisabled("Output: %d x %d pixels", width, height);

    ImGui::InputInt("Samples", &p.samples);
    p.samples = std::max(1, p.samples);

    const char *backgrounds[] = {"Current color", "White", "Transparent"};
    ImGui::Combo("Background", &p.background, backgrounds, 3);
    ImGui::Checkbox("Expand frustum", &p.expand);
    ImGui::TextDisabled("Matches the visible scene when the output aspect differs.");

    ImGui::Spacing();
    if (ImGui::Button("Save...", ImVec2(100, 0))) {
        const Model *model = current_model();
        std::string default_name("untitled.png");
        if (model)
            default_name = file_system::replace_extension(model->name(), "png");
        const std::string file_name = dialog::save(
                "Save snapshot", default_name,
                {"Image files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                 "All files (*.*)", "*"});
        if (!file_name.empty()) {
            const std::string ext = file_system::extension(file_name, true);
            if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
                LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            } else {
                // The panels, menus and this dialog are client overlays, and the
                // viewer's snapshot renders client overlays on purpose (annotations
                // belong in captures). Drop them for this frame; pre_draw() rebuilds
                // them from the next frame on, so the window itself is unaffected.
                set_overlays({});
                if (snapshot(file_name, p.scale, p.samples, p.background, p.expand)) {
                    LOG(INFO) << "snapshot saved to " << file_name;

                    const auto state_file = file_system::replace_extension(file_name, "view");
                    std::ofstream output(state_file);
                    if (output) {
                        save_view_state(state_file);
                        LOG(INFO) << "view state saved to " << state_file;
                    }
                }
                else
                    LOG(ERROR) << "failed to save the snapshot: " << file_name;
                show_snapshot_dialog_ = false;
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(100, 0)))
        show_snapshot_dialog_ = false;
    ImGui::SameLine();
    ImGui::TextDisabled("Esc also closes.");
    ImGui::End();
}
