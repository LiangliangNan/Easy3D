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
#include <vector>

#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

namespace {

    // Mapple's own list: the aggregate line first, then one entry per model type,
    // matching the Qt app's dialogs. Pairs of description and pattern.
    const std::vector<std::string> &model_open_filters() {
        static const std::vector<std::string> filters = {
                "Supported formats (*.ply *.obj *.off *.stl *.sm *.geojson *.trilist *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx *.plm *.pm *.mesh)",
                "*.ply *.obj *.off *.stl *.sm *.geojson *.trilist *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx *.plm *.pm *.mesh",
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)",
                "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polyhedral Mesh (*.plm *.pm *.mesh)",
                "*.plm *.pm *.mesh",
                "Graph (*.ply)",
                "*.ply",
                "All Files (*.*)",
                "*"};
        return filters;
    }

    const std::vector<std::string> &model_save_filters() {
        static const std::vector<std::string> filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm)",
                "*.obj *.ply *.off *.stl *.sm",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polyhedral Mesh (*.plm *.pm *.mesh)",
                "*.plm *.pm *.mesh",
                "Graph (*.ply)",
                "*.ply",
                "All Files (*.*)",
                "*"};
        return filters;
    }

} // namespace

bool MappleViewer::open() {
    const std::string default_path = settings_.data_directory.empty()
                                             ? resource::directory() + "/data/"
                                             : settings_.data_directory;
    const std::vector<std::string> files =
            dialog::open("Open file(s)", default_path, model_open_filters(), true);

    bool opened = false;
    for (const auto &file : files) {
        if (add_model(file)) {
            settings_.remember_file(file);
            opened = true;
        }
    }
    if (opened)
        fit_screen();
    return opened;
}

bool MappleViewer::save() const {
    Model *model = current_model();
    if (!model) {
        LOG(ERROR) << "no model exists";
        return false;
    }

    std::string default_file_name = model->name();
    if (file_system::extension(default_file_name).empty()) // no extension?
        default_file_name += ".ply";                       // default to ply
    if (!settings_.data_directory.empty())
        default_file_name = settings_.data_directory + "/" + file_system::simple_name(default_file_name);

    const std::string file_name =
            dialog::save("Please choose a file name", default_file_name, model_save_filters(), true);
    if (file_name.empty())
        return false;

    bool saved = false;
    if (const auto *cloud = dynamic_cast<const PointCloud *>(model))
        saved = PointCloudIO::save(file_name, cloud);
    else if (const auto *mesh = dynamic_cast<const SurfaceMesh *>(model))
        saved = SurfaceMeshIO::save(file_name, mesh);
    else if (const auto *graph = dynamic_cast<const Graph *>(model))
        saved = GraphIO::save(file_name, graph);

    if (saved) {
        LOG(INFO) << "file successfully saved to: " << file_name;
        settings_.remember_file(file_name); // settings_ is mutable: save() is const
    }
    return saved;
}

void MappleViewer::open_recent_file(const std::string &file) {
    if (!add_model(file)) {
        // The file was deleted or moved since the settings were loaded.
        settings_.forget_file(file);
        return;
    }
    fit_screen();
    settings_.remember_file(file);
}

// The menu bar only exposes actions that exist. Shortcut strings are display-only;
// the matching keys are handled by Viewer::key_press_event().
void MappleViewer::draw_menu_bar() {
    if (!ImGui::BeginMainMenuBar())
        return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...", "Ctrl+O"))
            open();

        if (ImGui::BeginMenu("Open Recent")) {
            // Copied: opening a file moves it to the front of the list.
            const std::vector<std::string> recent = settings_.recent_files;
            if (recent.empty())
                ImGui::MenuItem("(no recent files)", nullptr, false, false);
            for (std::size_t i = 0; i < recent.size(); ++i) {
                const std::string label =
                        std::to_string(i + 1) + "  " + file_system::simple_name(recent[i]);
                if (ImGui::MenuItem(label.c_str()))
                    open_recent_file(recent[i]);
                ImGui::SetItemTooltip("%s", recent[i].c_str());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Clear Recent Files", nullptr, false, !recent.empty()))
                settings_.clear_recent_files();
            ImGui::EndMenu();
        }

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
            open_snapshot_dialog();

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

        ImGui::Separator();
        const int keyframes = static_cast<int>(kfi_->number_of_keyframes());
        if (ImGui::MenuItem("Camera Path..."))
            open_animation_window();
        if (ImGui::MenuItem("Add Keyframe", "Alt+K"))
            add_camera_keyframe();
        if (ImGui::MenuItem("Remove Last Keyframe", nullptr, false, keyframes > 0))
            remove_last_camera_keyframe();
        if (ImGui::MenuItem("Clear Camera Path", nullptr, false, keyframes > 0))
            clear_camera_path();

        ImGui::Separator();
        if (ImGui::MenuItem(kfi_->is_interpolation_started() ? "Stop Playback" : "Play Camera Path", "Ctrl+K",
                            false, keyframes >= 2))
            toggle_camera_path_playback();
        bool path_visible = show_camera_path_;
        if (ImGui::MenuItem("Show Camera Path", "Ctrl+T", &path_visible))
            set_camera_path_visible(path_visible);

        ImGui::Separator();
        if (ImGui::MenuItem("Import Keyframes..."))
            import_camera_path();
        if (ImGui::MenuItem("Export Keyframes...", nullptr, false, keyframes > 0))
            export_camera_path();
        ImGui::EndMenu();
    }

    draw_menu_property();
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
        if (ImGui::MenuItem("Re-orient Normals..."))
            open_process(ProcessOp::PC_REORIENT_NORMALS);
        if (ImGui::MenuItem("Simplify..."))
            open_process(ProcessOp::PC_SIMPLIFY);
        ImGui::Separator();
        if (ImGui::MenuItem("Extract Primitives (RANSAC)..."))
            open_process(ProcessOp::PC_RANSAC);
        if (ImGui::MenuItem("Poisson Reconstruction..."))
            open_process(ProcessOp::PC_POISSON);
        ImGui::Separator();
        if (ImGui::MenuItem("Delaunay 2D (XY)"))
            run_process(ProcessOp::PC_DELAUNAY_2D);
        if (ImGui::MenuItem("Delaunay 3D"))
            run_process(ProcessOp::PC_DELAUNAY_3D);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Surface Mesh", mesh)) {
        if (ImGui::MenuItem("Smooth..."))
            open_process(ProcessOp::MESH_SMOOTH);
        if (ImGui::MenuItem("Fair..."))
            open_process(ProcessOp::MESH_FAIRING);
        if (ImGui::MenuItem("Simplify..."))
            open_process(ProcessOp::MESH_SIMPLIFY);
        if (ImGui::MenuItem("Remesh..."))
            open_process(ProcessOp::MESH_REMESH);
        ImGui::Separator();
        if (ImGui::BeginMenu("Subdivide")) {
            if (ImGui::MenuItem("Catmull-Clark"))
                run_process(ProcessOp::MESH_SUBDIVIDE_CC);
            if (ImGui::MenuItem("Loop"))
                run_process(ProcessOp::MESH_SUBDIVIDE_LOOP);
            if (ImGui::MenuItem("Sqrt3"))
                run_process(ProcessOp::MESH_SUBDIVIDE_SQRT3);
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Triangulate"))
            run_process(ProcessOp::MESH_TRIANGULATE);
        if (ImGui::MenuItem("Dual Mesh"))
            run_process(ProcessOp::MESH_DUAL);
        ImGui::Separator();
        if (ImGui::MenuItem("Fill Holes..."))
            open_process(ProcessOp::MESH_FILL_HOLES);
        if (ImGui::MenuItem("Polygonize..."))
            open_process(ProcessOp::MESH_POLYGONIZE);
        ImGui::Separator();
        if (ImGui::MenuItem("Parameterize..."))
            open_process(ProcessOp::MESH_PARAMETERIZE);
        if (ImGui::MenuItem("Geodesic Distance"))
            run_process(ProcessOp::MESH_GEODESIC);
        ImGui::Separator();
        if (ImGui::MenuItem("Connected Components"))
            run_process(ProcessOp::MESH_COMPONENTS);
        if (ImGui::MenuItem("Planar Partition"))
            run_process(ProcessOp::MESH_PLANAR_PARTITION);
        if (ImGui::MenuItem("Sample Surface..."))
            open_process(ProcessOp::MESH_SAMPLE);
#ifdef HAS_CGAL
        ImGui::Separator();
        if (ImGui::MenuItem("Clip by Clipping Plane"))
            run_process(ProcessOp::MESH_CLIP);
        if (ImGui::MenuItem("Split by Clipping Plane"))
            run_process(ProcessOp::MESH_SPLIT);
        if (ImGui::MenuItem("Slice by Clipping Plane"))
            run_process(ProcessOp::MESH_SLICE);
#endif
        ImGui::EndMenu();
    }

    ImGui::EndMenu();
}

void MappleViewer::draw_menu_property() {
    if (!ImGui::BeginMenu("Property"))
        return;

    Model *model = current_model();
    const bool mesh = dynamic_cast<SurfaceMesh *>(model) != nullptr;
    const bool graph = dynamic_cast<Graph *>(model) != nullptr;
    const bool indexed_colors_supported = mesh || dynamic_cast<PointCloud *>(model) != nullptr;

    if (ImGui::MenuItem("Compute Height Field", nullptr, false, model != nullptr))
        run_process(ProcessOp::PROP_HEIGHT_FIELD);
    if (ImGui::MenuItem("Compute Curvatures...", nullptr, false, mesh))
        open_process(ProcessOp::PROP_CURVATURES);

    ImGui::Separator();
    if (ImGui::MenuItem("Color from Indexed Colors...", nullptr, false, indexed_colors_supported)) {
        const std::string default_path = settings_.data_directory.empty()
                                                 ? resource::directory() + "/data/"
                                                 : settings_.data_directory;
        const std::string file = dialog::open("Open indexed colors", default_path,
                                              {"Indexed colors (*.ic)", "*.ic", "All Files (*.*)", "*"});
        if (!file.empty()) {
            process_params_.indexed_colors_file = file;
            run_process(ProcessOp::PROP_INDEXED_COLORS);
        }
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Topology Statistics", nullptr, false, mesh || graph))
        run_process(ProcessOp::PROP_TOPOLOGY);
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
