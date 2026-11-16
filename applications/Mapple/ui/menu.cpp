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
#include <fstream>
#include <string>
#include <vector>

#include <easy3d/core/graph.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/string.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <3rd_party/nlohmann/json.hpp>

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

namespace {

    const std::vector<std::string> &view_state_filters() {
        static const std::vector<std::string> filters = {
                "Viewer state (*.view)", "*.view",
                "All Files (*.*)", "*"};
        return filters;
    }

    nlohmann::json as_json(const vec3 &v) { return {v.x, v.y, v.z}; }
    nlohmann::json as_json(const vec4 &v) { return {v.x, v.y, v.z, v.w}; }
    nlohmann::json as_json(const quat &q) { return {q[0], q[1], q[2], q[3]}; }

    // A missing or mistyped key leaves the value untouched and returns false, so a
    // file written by a different Mapple version restores the parameters it has.
    bool read(const nlohmann::json &object, const char *key, int &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_number_integer())
            return false;
        value = it->get<int>();
        return true;
    }

    bool read(const nlohmann::json &object, const char *key, float &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_number())
            return false;
        value = it->get<float>();
        return true;
    }

    bool read(const nlohmann::json &object, const char *key, bool &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_boolean())
            return false;
        value = it->get<bool>();
        return true;
    }

    bool read(const nlohmann::json &object, const char *key, std::string &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_string())
            return false;
        value = it->get<std::string>();
        return true;
    }

    template <size_t N>
    bool read(const nlohmann::json &object, const char *key, Vec<N, float> &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_array() || it->size() != N)
            return false;
        Vec<N, float> parsed;
        for (size_t i = 0; i < N; ++i) {
            if (!(*it)[i].is_number())
                return false;
            parsed[i] = (*it)[i].get<float>();
        }
        value = parsed;
        return true;
    }

    bool read(const nlohmann::json &object, const char *key, quat &value) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_array() || it->size() != 4)
            return false;
        quat parsed;
        for (unsigned i = 0; i < 4; ++i) {
            if (!(*it)[i].is_number())
                return false;
            parsed[i] = (*it)[i].get<float>();
        }
        float norm = 0.0f;
        for (unsigned i = 0; i < 4; ++i)
            norm += parsed[i] * parsed[i];
        if (!std::isfinite(norm) || norm < 1e-12f)
            return false;
        parsed.normalize();
        value = parsed;
        return true;
    }

    // A state saved on a monitor that is no longer connected must not move the
    // window out of reach, so its position is restored only when visible.
    bool on_a_monitor(int x, int y) {
        int count = 0;
        GLFWmonitor **monitors = glfwGetMonitors(&count);
        for (int i = 0; i < count; ++i) {
            int mx, my, mw, mh;
            glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
            if (x >= mx && x < mx + mw && y >= my && y < my + mh)
                return true;
        }
        return false;
    }

} // namespace


void MappleViewer::save_view_state() {
    std::string default_name = "view.view";
    if (const Model *model = current_model())
        default_name = file_system::replace_extension(model->name(), "view");
    else if (!settings_.data_directory.empty())
        default_name = settings_.data_directory + "/view.view";

    const std::string file_name =
            dialog::save("Save the view state to a file", default_name, view_state_filters());
    if (file_name.empty())
        return;

    save_view_state(file_name);
}


void MappleViewer::save_view_state(const std::string& file_path) {
    if (file_path.empty())
        return;

    int x = 0, y = 0;
    if (window())
        glfwGetWindowPos(window(), &x, &y);

    const Camera *cam = camera();
    const ManipulatedCameraFrame *frame = cam->frame();

    nlohmann::json state;
    state["saved"] = string::current_time();
    state["window"] = {{"width", width()}, {"height", height()}, {"x", x}, {"y", y}};
    state["background"] = as_json(background_color());
    state["camera"] = {
            {"type", cam->type() == Camera::PERSPECTIVE ? "perspective" : "orthographic"},
            {"z_clipping_coefficient", cam->z_clipping_coefficient()},
            {"z_near_coefficient", cam->z_near_coefficient()},
            {"scene_radius", cam->scene_radius()},
            {"scene_center", as_json(cam->scene_center())},
            {"field_of_view", cam->field_of_view()},
            {"position", as_json(frame->position())},
            {"orientation", as_json(frame->orientation())},
            {"pivot_point", as_json(cam->pivot_point())},
            {"rotation_sensitivity", frame->rotation_sensitivity()},
            {"translation_sensitivity", frame->translation_sensitivity()},
            {"wheel_sensitivity", frame->wheel_sensitivity()},
            {"zoom_sensitivity", frame->zoom_sensitivity()},
            {"zooms_on_pivot_point", frame->zooms_on_pivot_point()},
    };

    // The dialog's settings belong to the figure this state was saved for, so a
    // restored file reopens the snapshot dialog exactly as the figure was made.
    state["snapshot"] = {
            {"scale", snapshot_params_.scale},
            {"samples", snapshot_params_.samples},
            {"background", snapshot_params_.background},
            {"expand", snapshot_params_.expand},
    };

    std::ofstream output(file_path);
    if (!output.is_open()) {
        LOG(ERROR) << "unable to create the view state file " << file_path;
        return;
    }

    // "saved" identifies the file at a glance, so it goes on top; the other fields
    // keep the alphabetical order json gives them.
    nlohmann::ordered_json ordered;
    ordered["saved"] = state["saved"];
    for (auto it = state.begin(); it != state.end(); ++it) {
        if (it.key() != "saved")
            ordered[it.key()] = it.value();
    }
    output << ordered.dump(4) << std::endl;
    if (!output.good()) {
        LOG(ERROR) << "failed to write the view state to " << file_path;
        return;
    }
    LOG(INFO) << "view state saved to " << file_path;
    // The user will likely restore this state soon; keep the path here.
    settings_.data_directory = file_system::parent_directory(file_path);
}

void MappleViewer::restore_view_state() {
    const std::string default_path = settings_.data_directory.empty()
                                             ? resource::directory() + "/data/"
                                             : settings_.data_directory;
    const std::string file_name =
            dialog::open("Restore the view state from a file", default_path, view_state_filters());
    if (file_name.empty())
        return;

    nlohmann::json state;
    try {
        std::ifstream input(file_name);
        if (!input.is_open()) {
            LOG(ERROR) << "unable to read the view state from " << file_name;
            return;
        }
        input >> state;
    } catch (const std::exception &e) {
        LOG(ERROR) << "unable to read the view state from " << file_name << ": " << e.what();
        return;
    }
    if (!state.is_object()) {
        LOG(ERROR) << file_name << " does not contain a view state";
        return;
    }

    // Playback would overwrite the restored camera on the next frame.
    if (kfi_->is_interpolation_started())
        kfi_->stop_interpolation();

    // The window first: resizing updates the camera's aspect ratio.
    const auto window_state = state.find("window");
    if (window_state != state.end() && window_state->is_object()) {
        int w = width(), h = height();
        read(*window_state, "width", w);
        read(*window_state, "height", h);
        if (w > 0 && h > 0)
            resize(w, h);
        int x = 0, y = 0;
        if (read(*window_state, "x", x) && read(*window_state, "y", y) && on_a_monitor(x, y))
            set_position(x, y);
    }

    vec4 background = background_color();
    if (read(state, "background", background))
        set_background_color(background);

    const auto camera_state = state.find("camera");
    if (camera_state != state.end() && camera_state->is_object()) {
        const nlohmann::json &c = *camera_state;
        std::string type;
        if (read(c, "type", type)) {
            if (type == "perspective")
                camera()->set_type(Camera::PERSPECTIVE);
            else if (type == "orthographic")
                camera()->set_type(Camera::ORTHOGRAPHIC);
        }

        float value = 0.0f;
        if (read(c, "z_clipping_coefficient", value))
            camera()->set_z_clipping_coefficient(value);
        if (read(c, "z_near_coefficient", value))
            camera()->set_z_near_coefficient(value);
        if (read(c, "scene_radius", value))
            camera()->set_scene_radius(value);
        vec3 point;
        if (read(c, "scene_center", point))
            camera()->set_scene_center(point);
        if (read(c, "field_of_view", value))
            camera()->set_field_of_view(value);

        ManipulatedCameraFrame *frame = camera()->frame();
        if (read(c, "rotation_sensitivity", value))
            frame->set_rotation_sensitivity(value);
        if (read(c, "translation_sensitivity", value))
            frame->set_translation_sensitivity(value);
        if (read(c, "wheel_sensitivity", value))
            frame->set_wheel_sensitivity(value);
        if (read(c, "zoom_sensitivity", value))
            frame->set_zoom_sensitivity(value);
        bool zooms_on_pivot_point = frame->zooms_on_pivot_point();
        if (read(c, "zooms_on_pivot_point", zooms_on_pivot_point))
            frame->set_zooms_on_pivot_point(zooms_on_pivot_point);

        if (read(c, "pivot_point", point))
            camera()->set_pivot_point(point);
        if (read(c, "position", point))
            camera()->set_position(point);
        quat orientation;
        if (read(c, "orientation", orientation))
            camera()->set_orientation(orientation);
    }

    const auto snapshot_state = state.find("snapshot");
    if (snapshot_state != state.end() && snapshot_state->is_object()) {
        const nlohmann::json &s = *snapshot_state;
        read(s, "scale", snapshot_params_.scale);
        read(s, "samples", snapshot_params_.samples);
        read(s, "background", snapshot_params_.background);
        read(s, "expand", snapshot_params_.expand);
        // The same limits the dialog enforces.
        snapshot_params_.scale = std::max(0.1f, std::min(snapshot_params_.scale, 8.0f));
        snapshot_params_.samples = std::max(1, snapshot_params_.samples);
        snapshot_params_.background = std::max(0, std::min(snapshot_params_.background, 2));
        snapshot_samples_restored_ = true;
    }

    update();
    LOG(INFO) << "view state restored from " << file_name;
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
        if (ImGui::MenuItem("Save View State..."))
            save_view_state();
        if (ImGui::MenuItem("Restore View State..."))
            restore_view_state();

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
    draw_menu_edit();
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

namespace {

    bool is_transformed(const easy3d::Model *model) {
        const easy3d::Manipulator *manipulator = model ? model->manipulator() : nullptr;
        return manipulator && manipulator->matrix() != easy3d::mat4::identity();
    }

} // namespace

void MappleViewer::draw_menu_edit() {
    if (!ImGui::BeginMenu("Edit"))
        return;

    Model *model = current_model();
    const bool cloud = dynamic_cast<PointCloud *>(model) != nullptr;
    const bool mesh = dynamic_cast<SurfaceMesh *>(model) != nullptr;

    if (ImGui::MenuItem("Add Gaussian Noise...", nullptr, false, cloud || mesh))
        open_process(ProcessOp::EDIT_GAUSSIAN_NOISE);

    ImGui::Separator();
    if (ImGui::MenuItem("Translational Recenter", nullptr, false, !models().empty()))
        translational_recenter();

    ImGui::Separator();
    bool any_transformed = false;
    for (const auto &candidate : models()) {
        if (is_transformed(candidate.get())) {
            any_transformed = true;
            break;
        }
    }
    if (ImGui::MenuItem("Apply Manipulated Transformation", nullptr, false, any_transformed))
        apply_manipulated_transformation();
    if (ImGui::MenuItem("Give Up Manipulated Transformation", nullptr, false, is_transformed(model)))
        discard_manipulated_transformation();
    ImGui::EndMenu();
}

void MappleViewer::translational_recenter() {
    if (models().empty())
        return;

    const vec3 origin = models().front()->bounding_box().center();
    for (const auto &entry : models()) {
        Model *model = entry.get();
        for (auto &point : model->points())
            point -= origin;
        model->invalidate_bounding_box();
        if (model->manipulator())
            model->manipulator()->reset();
        if (model->renderer())
            model->renderer()->update();
    }
    LOG(INFO) << "scene moved so that the first model's bounding-box center is at the origin";
    fit_screen();
}

void MappleViewer::apply_manipulated_transformation() {
    std::size_t count = 0;
    for (const auto &entry : models()) {
        Model *model = entry.get();
        auto *manipulator = model->manipulator();
        if (!manipulator)
            continue;
        const mat4 manipulation = manipulator->matrix();
        // Matrices that are a no-op need no baking and no redraw.
        if (manipulation == mat4::identity())
            continue;

        for (auto &point : model->points())
            point = manipulation * point;

        if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
            model->invalidate_bounding_box();
            mesh->update_face_normals();
            mesh->update_vertex_normals();
        } else {
            model->invalidate_bounding_box();
            if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
                // Graph is a point cloud too; both keep their normals in v:normal.
                auto normals = cloud->get_vertex_property<vec3>("v:normal");
                if (normals) {
                    const mat3 normal_matrix = transform::normal_matrix(manipulation);
                    for (auto v : cloud->vertices())
                        normals[v] = normal_matrix * normals[v];
                }
            }
        }

        manipulator->reset();
        if (model->renderer())
            model->renderer()->update();
        ++count;
    }

    if (count > 0)
        LOG(INFO) << "manipulated transformation applied to " << count << " model(s)";
    update();
}

void MappleViewer::discard_manipulated_transformation() {
    Model *model = current_model();
    if (!model || !model->manipulator())
        return;
    model->manipulator()->reset();
    LOG(INFO) << "manipulated transformation discarded";
    update();
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
        if (ImGui::MenuItem("Normalize Normals"))
            run_process(ProcessOp::PC_NORMALIZE_NORMALS);
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

    // Always open: "Create from Text..." generates a new mesh and needs no current
    // model, while the commands that edit a mesh are disabled without one.
    if (ImGui::BeginMenu("Surface Mesh")) {
        ImGui::BeginDisabled(!mesh);
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
        if (ImGui::BeginMenu("Repair")) {
            if (ImGui::MenuItem("Repair Polygon Soup"))
                run_process(ProcessOp::MESH_REPAIR_SOUP);
            if (ImGui::MenuItem("Orient and Stitch Polygon Soup"))
                run_process(ProcessOp::MESH_ORIENT_STITCH_SOUP);
            if (ImGui::MenuItem("Stitch with Reorientation"))
                run_process(ProcessOp::MESH_STITCH_REORIENT);
            if (ImGui::MenuItem("Stitch without Reorientation"))
                run_process(ProcessOp::MESH_STITCH);
            ImGui::Separator();
            if (ImGui::MenuItem("Orient Outward"))
                run_process(ProcessOp::MESH_ORIENT_OUTWARD);
            if (ImGui::MenuItem("Reverse Orientation"))
                run_process(ProcessOp::MESH_REVERSE_ORIENTATION);
            if (ImGui::MenuItem("Remove Isolated Vertices"))
                run_process(ProcessOp::MESH_REMOVE_ISOLATED);
            if (ImGui::MenuItem("Remove Duplicate and Folding Faces"))
                run_process(ProcessOp::MESH_REMOVE_DUPLICATE_FACES);
            ImGui::Separator();
            if (ImGui::MenuItem("Detect Self-Intersections"))
                run_process(ProcessOp::MESH_DETECT_SELF_INTERSECTIONS);
            if (ImGui::MenuItem("Remesh Self-Intersections"))
                run_process(ProcessOp::MESH_REMESH_SELF_INTERSECTIONS);
            ImGui::EndMenu();
        }
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
        if (ImGui::MenuItem("Tetrahedralization"))
            run_process(ProcessOp::MESH_TETRAHEDRALIZE);
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
        ImGui::EndDisabled();
        ImGui::Separator();
        if (ImGui::MenuItem("Create from Text..."))
            open_process(ProcessOp::MESH_CREATE_FROM_TEXT);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Polyhedral Mesh", dynamic_cast<PolyMesh *>(model) != nullptr)) {
        if (ImGui::MenuItem("Extract Boundary"))
            run_process(ProcessOp::POLY_EXTRACT_BOUNDARY);
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

    if (ImGui::MenuItem("Compute Height Field", nullptr, false, model != nullptr))
        run_process(ProcessOp::PROP_HEIGHT_FIELD);
    if (ImGui::MenuItem("Compute Curvatures...", nullptr, false, mesh))
        open_process(ProcessOp::PROP_CURVATURES);
    if (ImGui::MenuItem("Compute Ambient Occlusion...", nullptr, false, mesh))
        open_process(ProcessOp::PROP_AMBIENT_OCCLUSION);

    ImGui::Separator();
    if (ImGui::MenuItem("Manipulate Properties...", nullptr, false, model != nullptr))
        open_properties_window();

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
