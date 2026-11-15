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
#include <sstream>
#include <string>
#include <vector>

#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/version.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

namespace {

constexpr ImGuiWindowFlags kPanelFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

constexpr ImGuiColorEditFlags kSwatch = ImGuiColorEditFlags_NoInputs;

void begin_panel(const char *id, float x, float y, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(1.0f, w), std::max(1.0f, h)), ImGuiCond_Always);
    ImGui::Begin(id, nullptr, kPanelFlags);
}

const char *level_name(el::Level level) {
    switch (level) {
        case el::Level::Trace:
            return "TRACE";
        case el::Level::Debug:
            return "DEBUG";
        case el::Level::Info:
            return "INFO";
        case el::Level::Warning:
            return "WARNING";
        case el::Level::Error:
            return "ERROR";
        case el::Level::Fatal:
            return "FATAL";
        case el::Level::Verbose:
            return "VERBOSE";
        default:
            return "UNKNOWN";
    }
}

ImU32 level_color(el::Level level) {
    switch (level) {
        case el::Level::Error:
        case el::Level::Fatal:
            return IM_COL32(190, 30, 30, 255);
        case el::Level::Warning:
            return IM_COL32(170, 95, 0, 255);
        case el::Level::Debug:
        case el::Level::Trace:
        case el::Level::Verbose:
            return IM_COL32(115, 115, 115, 255);
        default:
            return IM_COL32(25, 25, 25, 255);
    }
}

} // namespace

void MappleViewer::draw_toolbar(float x, float y, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(1.0f, w), std::max(1.0f, h)), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##toolbar", nullptr, kPanelFlags);

    if (ImGui::Button("Open"))
        open();
    ImGui::SameLine();
    ImGui::BeginDisabled(current_model() == nullptr);
    if (ImGui::Button("Save"))
        save();
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Fit"))
        fit_screen();
    ImGui::SameLine();
    if (ImGui::Button("Snapshot"))
        open_snapshot_dialog();
    ImGui::SameLine();
    if (ImGui::Button("Copy View"))
        copy_view();
    ImGui::SameLine();
    if (ImGui::Button("Paste View"))
        paste_view();

    // Selection tools are mode toggles: pressing the active one returns to OFF,
    // where the pointer navigates the scene again.
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    auto mode_button = [this](const char *label, SelectMode mode) {
        const bool active = select_mode_ == mode;
        if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(label))
            set_select_mode(active ? SelectMode::OFF : mode);
        if (active)
            ImGui::PopStyleColor();
        ImGui::SameLine();
    };
    mode_button("Click", SelectMode::CLICK);
    mode_button("Rect", SelectMode::RECT);
    mode_button("Lasso", SelectMode::LASSO);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void MappleViewer::draw_panels(float x, float y, float w, float h) {
    // The scene area, in logical units: whatever the panels do not cover. Input
    // callbacks use it to reject gestures and hover queries over the UI.
    scene_x_ = scene_y_ = scene_w_ = scene_h_ = 0;
    if (w < 1.0f || h < 1.0f)
        return;

    constexpr float split = 4.0f;
    const float max_side = std::max(120.0f, w * 0.4f);
    left_width_ = clamp(left_width_, 160.0f, max_side);
    right_width_ = clamp(right_width_, 160.0f, max_side);
    if (left_width_ + right_width_ + 240.0f > w)
        right_width_ = std::max(0.0f, w - left_width_ - 240.0f);

    float content_h = h;
    float log_max = 80.0f;
    if (show_log_panel_) {
        log_max = std::max(80.0f, h - 160.0f);
        log_height_ = clamp(log_height_, 80.0f, log_max);
        content_h = std::max(1.0f, h - log_height_ - split);
    }

    // A background command or a video recording owns the scene: the model panels stay
    // laid out but disabled, so no control can change the model or its appearance
    // while it is being processed or captured. The log panel is mutex-protected and
    // does not touch the model, so it stays usable.
    const bool scene_busy = process_job_ != nullptr || recording_video();

    if (show_rendering_panel_) {
        ImGui::BeginDisabled(scene_busy);
        draw_rendering_panel(x, y, left_width_, content_h);
        ImGui::EndDisabled();
        draw_splitter("##split-left", x + left_width_, y, split, content_h,
                      true, &left_width_, 1.0f, 160.0f, max_side);
    }
    if (show_models_panel_ && right_width_ >= 160.0f) {
        const float right_x = x + w - right_width_;
        ImGui::BeginDisabled(scene_busy);
        draw_models_panel(right_x, y, right_width_, content_h);
        ImGui::EndDisabled();
        draw_splitter("##split-right", right_x - split, y, split, content_h,
                      true, &right_width_, -1.0f, 160.0f, max_side);
    }
    if (show_log_panel_) {
        const float log_y = y + h - log_height_;
        draw_log_panel(x, log_y, w, log_height_);
        draw_splitter("##split-log", x, log_y - split, w, split,
                      false, &log_height_, -1.0f, 80.0f, log_max);
    }

    const float left = x + (show_rendering_panel_ ? left_width_ + split : 0.0f);
    const float right = x + w - (show_models_panel_ && right_width_ >= 160.0f ? right_width_ + split : 0.0f);
    scene_x_ = static_cast<int>(std::lround(left));
    scene_y_ = static_cast<int>(std::lround(y));
    scene_w_ = std::max(0, static_cast<int>(std::lround(right)) - scene_x_);
    scene_h_ = static_cast<int>(std::lround(content_h));
}

void MappleViewer::draw_rendering_panel(float x, float y, float w, float h) {
    begin_panel("Rendering", x, y, w, h);
    ImGui::TextUnformatted("Rendering");
    ImGui::Separator();

    vec4 background = background_color();
    if (ImGui::ColorEdit3("Background", background.data(), kSwatch)) {
        set_background_color(background);
        update();
    }

    const bool perspective = camera()->type() == Camera::PERSPECTIVE;
    if (ImGui::RadioButton("Perspective", perspective)) {
        camera()->set_type(Camera::PERSPECTIVE);
        update();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Orthographic", !perspective)) {
        camera()->set_type(Camera::ORTHOGRAPHIC);
        update();
    }

    ImGui::Spacing();
    // Clipping is a scene-wide effect, so it sits with the other global rendering
    // controls. The guide and Alt-drag manipulation are handled by Viewer.
    if (ImGui::CollapsingHeader("Clipping Plane")) {
        auto *plane = ClippingPlane::instance();
        bool enabled = plane->is_enabled();
        if (ImGui::Checkbox("Enabled", &enabled)) {
            plane->set_enabled(enabled);
            if (enabled)
                plane->fit_scene(camera()->scene_center(), camera()->scene_radius());
            update();
        }
        ImGui::BeginDisabled(!enabled);
        bool visible = plane->is_visible();
        if (ImGui::Checkbox("Show guide", &visible)) {
            plane->set_visible(visible);
            update();
        }
        bool cross_section = plane->cross_section();
        if (ImGui::Checkbox("Cross section", &cross_section)) {
            plane->set_cross_section(cross_section);
            update();
        }
        float thickness = plane->cross_section_width();
        if (ImGui::SliderFloat("Thickness", &thickness, 0.001f, 0.1f, "%.3f",
                               ImGuiSliderFlags_Logarithmic)) {
            plane->set_cross_section_width(thickness);
            update();
        }
        vec4 guide_color = plane->color();
        if (ImGui::ColorEdit3("Guide color", guide_color.data(), kSwatch)) {
            plane->set_color(guide_color);
            update();
        }
        ImGui::EndDisabled();
        ImGui::TextDisabled("Alt + LMB/RMB drag to move the plane");
    }

    ImGui::Spacing();
    Model *model = current_model();
    if (!model) {
        ImGui::TextDisabled("No active model.");
        ImGui::End();
        return;
    }
    if (!model->renderer()) {
        ImGui::TextDisabled("The active model has no renderer.");
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted(file_system::simple_name(model->name()).c_str());
    ImGui::Separator();
    Renderer *renderer = model->renderer();

    // Fixed order: faces, edges, borders, vertices, locks. Unknown drawables come last.
    // Prefix matches so the polyhedral mesh's "faces:border"/"faces:interior" stay with the faces.
    auto rank = [](const std::string &name) {
        if (name.rfind("faces", 0) == 0) return 0;
        if (name.rfind("edges", 0) == 0) return 1;
        if (name == "borders") return 2;
        if (name.rfind("vertices", 0) == 0) return 3;
        if (name == "locks") return 4;
        return 5;
    };
    std::vector<Drawable *> drawables;
    for (const auto &d : renderer->triangles_drawables())
        drawables.push_back(d.get());
    for (const auto &d : renderer->lines_drawables())
        drawables.push_back(d.get());
    for (const auto &d : renderer->points_drawables())
        drawables.push_back(d.get());
    // Stable, so drawables sharing a name keep the renderer's type order.
    std::stable_sort(drawables.begin(), drawables.end(), [&rank](const Drawable *a, const Drawable *b) {
        return rank(a->name()) < rank(b->name());
    });

    // Distinct back color is a State property, so faces and points share these controls.
    auto draw_back_color = [this](Drawable *d) {
        bool back_color = d->distinct_back_color();
        if (ImGui::Checkbox("Back color", &back_color)) {
            d->set_distinct_back_color(back_color);
            update();
        }
        if (back_color) {
            ImGui::SameLine();
            vec4 color_back = d->back_color();
            if (ImGui::ColorEdit3("##back", color_back.data(), kSwatch)) {
                d->set_back_color(color_back);
                update();
            }
        }
    };

    for (Drawable *drawable : drawables) {
        auto *triangles = dynamic_cast<TrianglesDrawable *>(drawable);
        auto *lines = dynamic_cast<LinesDrawable *>(drawable);
        auto *points = dynamic_cast<PointsDrawable *>(drawable);

        ImGui::PushID(drawable);
        ImGui::TextUnformatted(drawable->name().c_str());
        ImGui::Indent();

        bool visible = drawable->is_visible();
        if (ImGui::Checkbox("Visible", &visible)) {
            drawable->set_visible(visible);
            update();
        }

        ImGui::SameLine();
        vec4 color = drawable->color();
        if (ImGui::ColorEdit3("Color", color.data(), kSwatch)) {
            drawable->set_uniform_coloring(color);
            update();
        }

        if (triangles) {
            bool smooth = triangles->smooth_shading();
            if (ImGui::Checkbox("Smooth", &smooth)) {
                triangles->set_smooth_shading(smooth);
                update();
            }

            float opacity = triangles->opacity();
            if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
                triangles->set_opacity(opacity);
                update();
            }

            draw_back_color(triangles);
        } else if (lines) {
            float width = lines->line_width();
            if (ImGui::SliderFloat("Width", &width, 1.0f, 10.0f, "%.1f px")) {
                lines->set_line_width(width);
                update();
            }
        } else if (points) {
            float size = points->point_size();
            if (ImGui::SliderFloat("Size", &size, 1.0f, 20.0f, "%.1f px")) {
                points->set_point_size(size);
                update();
            }

            draw_back_color(points);
        }

        ImGui::Unindent();
        ImGui::PopID();
        ImGui::Spacing();
    }

    ImGui::End();
}

void MappleViewer::draw_model_info(Model *model) {
    if (!model)
        return;
    if (!ImGui::CollapsingHeader("Model Info"))
        return;

    if (process_job_ && model == process_job_->target) {
        // The worker owns this model; its counts, bounds and properties are in flux.
        ImGui::TextDisabled("%s is being processed in the background.", process_title(process_op_));
        ImGui::Spacing();
        return;
    }

    ImGui::TextWrapped("Path: %s", model->name().c_str());
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model))
        ImGui::Text("%u vertices, %u edges, %u faces", mesh->n_vertices(), mesh->n_edges(), mesh->n_faces());
    else if (auto *cloud = dynamic_cast<PointCloud *>(model))
        ImGui::Text("%u points", cloud->n_vertices());

    const Box3 &box = model->bounding_box();
    if (box.is_valid()) {
        const vec3 &min_corner = box.min_point();
        const vec3 &max_corner = box.max_point();
        ImGui::Text("Bounds min: (%.3f, %.3f, %.3f)", min_corner.x, min_corner.y, min_corner.z);
        ImGui::Text("Bounds max: (%.3f, %.3f, %.3f)", max_corner.x, max_corner.y, max_corner.z);
    }

    std::ostringstream stats;
    model->property_stats(stats);
    const std::string properties = stats.str();
    if (!properties.empty() && ImGui::TreeNode("Properties")) {
        ImGui::BeginChild("##model-properties", ImVec2(0, 110), false);
        ImGui::TextUnformatted(properties.c_str());
        ImGui::EndChild();
        ImGui::TreePop();
    }
    ImGui::Spacing();
}

void MappleViewer::draw_models_panel(float x, float y, float w, float h) {
    begin_panel("Models", x, y, w, h);
    ImGui::TextUnformatted("Models");
    ImGui::SameLine();
    ImGui::TextDisabled("(%d)", static_cast<int>(models().size()));
    ImGui::Separator();

    if (models().empty())
        ImGui::TextDisabled("No models. Drop a file here or use File > Open.");

    // The info belongs to the selected model, so it sits directly above the list that
    // selects it, not in the rendering panel (which restyles the active model).
    if (Model *current = current_model()) {
        draw_model_info(current);
        ImGui::Separator();
    }

    const float buttons_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##models-list", ImVec2(0, -buttons_height));

    for (std::size_t i = 0; i < models().size(); ++i) {
        Model *model = models()[i].get();
        ImGui::PushID(static_cast<int>(i));

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        if (model == current_model())
            flags |= ImGuiTreeNodeFlags_Selected;

        const std::string label = file_system::simple_name(model->name());
        const bool open = ImGui::TreeNodeEx(label.c_str(), flags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            set_current_model(model);
            update();
            // Selection view follows the active model.
            if (select_mode_ != SelectMode::OFF)
                refresh_selection_display(model);
        }
        if (ImGui::BeginPopupContextItem("##model-menu")) {
            if (ImGui::MenuItem("Fit to this model")) {
                fit_screen(model);
                update();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                delete_model_confirm(model);
            ImGui::EndPopup();
        }

        if (open) {
            if (Renderer *renderer = model->renderer()) {
                auto drawable_row = [this](Drawable *d) {
                    ImGui::PushID(d);
                    bool visible = d->is_visible();
                    if (ImGui::Checkbox(d->name().c_str(), &visible)) {
                        d->set_visible(visible);
                        update();
                    }
                    ImGui::PopID();
                };
                for (const auto &drawable : renderer->triangles_drawables())
                    drawable_row(drawable.get());
                for (const auto &drawable : renderer->lines_drawables())
                    drawable_row(drawable.get());
                for (const auto &drawable : renderer->points_drawables())
                    drawable_row(drawable.get());
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();

    Model *model = current_model();
    ImGui::BeginDisabled(model == nullptr);
    if (ImGui::Button("Fit")) {
        fit_screen(model);
        update();
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
        delete_model_confirm(model);
    ImGui::EndDisabled();

    ImGui::End();

    // Delete after the list has been drawn: removing a model inside the loop would
    // invalidate the iteration state.
    if (pending_delete_) {
        Model *doomed = pending_delete_;
        pending_delete_ = nullptr;
        forget_model(doomed);
        if (delete_model(doomed))
            update();
    }
}

void MappleViewer::draw_log_panel(float x, float y, float w, float h) {
    begin_panel("Log", x, y, w, h);
    ImGui::TextUnformatted("Log");
    ImGui::Separator();

    auto compose = [this]() {
        std::lock_guard<std::mutex> lock(log_mutex_);
        std::string text;
        for (const LogLine &line : log_) {
            text += '[';
            text += level_name(line.level);
            text += "] ";
            text += line.text;
            text += '\n';
        }
        return text;
    };

    if (ImGui::Button("Clear")) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        log_.clear();
        ++log_generation_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy"))
        ImGui::SetClipboardText(compose().c_str());
    ImGui::SameLine();
    if (ImGui::Button("Export...")) {
        const std::string file_name = dialog::save(
                "Export log", "mapple-log.txt",
                {"Text files (*.txt)", "*.txt", "All files (*.*)", "*"});
        if (!file_name.empty()) {
            std::ofstream output(file_name);
            if (output)
                output << compose();
            else
                LOG(ERROR) << "failed to write the log file: " << file_name;
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &log_auto_scroll_);

    ImGui::Separator();
    ImGui::BeginChild("##log-lines", ImVec2(0, 0));

    bool scroll_to_bottom = false;
    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (log_generation_ != last_seen_generation_) {
            scroll_to_bottom = log_auto_scroll_;
            last_seen_generation_ = log_generation_;
        }
        ImGui::PushTextWrapPos(0.0f);
        for (const LogLine &line : log_) {
            ImGui::PushStyleColor(ImGuiCol_Text, level_color(line.level));
            ImGui::TextUnformatted(line.text.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopTextWrapPos();
    }
    if (scroll_to_bottom)
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void MappleViewer::draw_status_bar(float x, float y, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(1.0f, w), std::max(1.0f, h)), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##status", nullptr, kPanelFlags);

    ImGui::Text("Models: %d", static_cast<int>(models().size()));
    Model *model = current_model();
    if (model) {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        const std::string name = file_system::simple_name(model->name());
        if (auto *mesh = dynamic_cast<SurfaceMesh *>(model))
            ImGui::Text("%s  %u vertices, %u faces", name.c_str(), mesh->n_vertices(), mesh->n_faces());
        else if (auto *cloud = dynamic_cast<PointCloud *>(model))
            ImGui::Text("%s  %u points", name.c_str(), cloud->n_vertices());
        else
            ImGui::TextUnformatted(name.c_str());
    }

    if (select_mode_ != SelectMode::OFF) {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        const char *mode = select_mode_ == SelectMode::CLICK ? "Click"
                         : select_mode_ == SelectMode::RECT  ? "Rectangle"
                                                             : "Lasso";
        ImGui::Text("Select: %s", mode);
        const std::size_t count = selection_count();
        if (count > 0) {
            ImGui::SameLine();
            ImGui::Text("(%d selected)", static_cast<int>(count));
        }
    }

    if (!hover_text_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextDisabled("%s", hover_text_.c_str());
    }

    const char *hint = select_mode_ == SelectMode::OFF
                       ? "LMB rotate   RMB pan   MMB zoom   F fit"
                       : "LMB select   Shift add   Ctrl remove   Esc exit";
    const float hint_width = ImGui::CalcTextSize(hint).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - hint_width - ImGui::GetStyle().WindowPadding.x);
    ImGui::TextDisabled("%s", hint);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void MappleViewer::draw_about_window() {
    if (show_about_) {
        ImGui::OpenPopup("About Mapple");
        show_about_ = false;
    }
    if (!ImGui::BeginPopupModal("About Mapple", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    ImGui::TextUnformatted("Mapple");
    ImGui::TextDisabled("Dear ImGui shell on Easy3D");
    ImGui::Separator();
    ImGui::Text("Easy3D %s", version_string().c_str());
#ifdef EASY3D_HAS_VULKAN
    ImGui::TextUnformatted("Backend: Vulkan");
#else
    ImGui::TextUnformatted("Backend: OpenGL");
#endif
    ImGui::Separator();
    ImGui::TextUnformatted("Liangliang Nan");
    ImGui::TextUnformatted("liangliang.nan@gmail.com");
    ImGui::TextUnformatted("https://3d.bk.tudelft.nl/liangliang/");
    ImGui::Separator();
    if (ImGui::Button("Close"))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void MappleViewer::draw_splitter(const char *id, float x, float y, float w, float h,
                                 bool resize_width, float *value, float sign,
                                 float min_value, float max_value) {
    if (!value || w <= 0.0f || h <= 0.0f)
        return;

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(id, nullptr, flags);
    ImGui::InvisibleButton("##grip", ImVec2(w, h));
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
        ImGui::SetMouseCursor(resize_width ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
    if (ImGui::IsItemActive()) {
        const ImVec2 delta = ImGui::GetIO().MouseDelta;
        const float amount = (resize_width ? delta.x : delta.y) * sign;
        *value = clamp(*value + amount, min_value, max_value);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

void MappleViewer::delete_model_confirm(Model *model) {
    if (!model)
        return;
    const std::string name = file_system::simple_name(model->name());
    const dialog::Response response = dialog::message(
            "Delete model", "Delete \"" + name + "\"?", dialog::Choice::yes_no, dialog::Type::question);
    if (response == dialog::Response::yes)
        pending_delete_ = model;
}
