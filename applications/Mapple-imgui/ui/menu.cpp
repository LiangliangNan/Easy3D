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

#include <string>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

// The menu bar only exposes actions that exist. Shortcut strings are display-only;
// the matching keys are handled by Viewer::key_press_event().
void MappleViewer::draw_menu_bar() {
    if (!ImGui::BeginMainMenuBar())
        return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...", "Ctrl+O"))
            open();
        if (ImGui::MenuItem("Save As...", "Ctrl+S", false, current_model() != nullptr))
            save();

        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4"))
            request_close();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Fit Scene", "F"))
            fit_screen();
        if (ImGui::MenuItem("Fit Active Model", "C", false, current_model() != nullptr))
            fit_screen(current_model());

        ImGui::Separator();
        if (ImGui::MenuItem("Snapshot...", "S"))
            snapshot();

        ImGui::Separator();
        ImGui::MenuItem("Rendering Panel", nullptr, &show_rendering_panel_);
        ImGui::MenuItem("Models Panel", nullptr, &show_models_panel_);
        ImGui::MenuItem("Log Panel", nullptr, &show_log_panel_);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera")) {
        if (ImGui::MenuItem("Copy View", "Ctrl+C"))
            copy_view();
        if (ImGui::MenuItem("Paste View", "Ctrl+V"))
            paste_view();

        ImGui::Separator();
        const bool perspective = camera()->type() == Camera::PERSPECTIVE;
        if (ImGui::MenuItem("Perspective", nullptr, perspective)) {
            camera()->set_type(Camera::PERSPECTIVE);
            update();
        }
        if (ImGui::MenuItem("Orthographic", nullptr, !perspective)) {
            camera()->set_type(Camera::ORTHOGRAPHIC);
            update();
        }
        ImGui::EndMenu();
    }

    draw_menu_process();
    draw_menu_select();

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About Mapple"))
            show_about_ = true;
        ImGui::EndMenu();
    }

    // The main viewport's work area reflects the menu bar only from the next frame;
    // recording the height locally keeps the layout correct on the first frame too.
    menu_height_ = ImGui::GetWindowHeight();
    ImGui::EndMainMenuBar();
}

void MappleViewer::draw_menu_process() {
    if (!ImGui::BeginMenu("Process"))
        return;

    Model *model = current_model();
    const bool cloud = dynamic_cast<PointCloud *>(model) != nullptr;
    const bool mesh = dynamic_cast<SurfaceMesh *>(model) != nullptr;

    if (ImGui::BeginMenu("Point Cloud", cloud)) {
        if (ImGui::MenuItem("Estimate Normals..."))
            open_process(ProcessOp::PC_ESTIMATE_NORMALS);
        if (ImGui::MenuItem("Simplify..."))
            open_process(ProcessOp::PC_SIMPLIFY);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Surface Mesh", mesh)) {
        if (ImGui::MenuItem("Smooth..."))
            open_process(ProcessOp::MESH_SMOOTH);
        if (ImGui::MenuItem("Simplify..."))
            open_process(ProcessOp::MESH_SIMPLIFY);
        if (ImGui::MenuItem("Remesh..."))
            open_process(ProcessOp::MESH_REMESH);
        ImGui::Separator();
        if (ImGui::MenuItem("Fill Holes..."))
            open_process(ProcessOp::MESH_FILL_HOLES);
        if (ImGui::MenuItem("Polygonize..."))
            open_process(ProcessOp::MESH_POLYGONIZE);
        ImGui::EndMenu();
    }

    ImGui::EndMenu();
}

void MappleViewer::draw_menu_select() {
    if (!ImGui::BeginMenu("Select"))
        return;

    auto mode_item = [this](const char *label, SelectMode mode, const char *shortcut) {
        if (ImGui::MenuItem(label, shortcut, select_mode_ == mode))
            set_select_mode(mode);
    };
    mode_item("Off", SelectMode::OFF, "Esc");
    mode_item("Click", SelectMode::CLICK, nullptr);
    mode_item("Rectangle", SelectMode::RECT, nullptr);
    mode_item("Lasso", SelectMode::LASSO, nullptr);

    ImGui::Separator();
    const bool has_model = current_model() != nullptr;
    if (ImGui::MenuItem("Select All", "Ctrl+A", false, has_model))
        select_all_primitives();
    if (ImGui::MenuItem("Invert Selection", "Ctrl+I", false, has_model))
        invert_selection();
    if (ImGui::MenuItem("Clear Selection", nullptr, false, has_model && selection_count() > 0))
        clear_selection();

    ImGui::Separator();
    const std::size_t count = selection_count();
    const std::string label = "Delete Selected (" + std::to_string(count) + ")";
    if (ImGui::MenuItem(label.c_str(), "Del", false, count > 0))
        confirm_delete_selected();

    ImGui::Separator();
    ImGui::MenuItem("Pick Readout", nullptr, &show_pick_readout_);
    ImGui::EndMenu();
}
