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
