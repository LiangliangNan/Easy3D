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
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <easy3d/core/graph.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/setting.h>
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

// One entry of the per-drawable color scheme combo. A scheme names a property and
// how the renderer consumes it.
struct ColorScheme {
    std::string label;      // shown in the combo
    easy3d::State::Method method;
    easy3d::State::Location location;
    std::string property;
};

// The panel draws field arrows into lines drawables it creates as "vector - <field>".
// They are rendering aids of the field selection, not drawables of the model, so the
// drawable lists leave them out.
bool is_vector_field_drawable(const std::string &name) {
    return name.rfind("vector - ", 0) == 0;
}

// Scalar fields supported by the buffer update: the numeric property types plus
// what the renderer can turn into texture coordinates. Bool selection properties
// are excluded; the selection has its own display.
template <class MODEL>
bool vertex_scalar(const MODEL *model, const std::string &name) {
    return model->template get_vertex_property<float>(name) ||
           model->template get_vertex_property<double>(name) ||
           model->template get_vertex_property<int>(name) ||
           model->template get_vertex_property<unsigned int>(name) ||
           model->template get_vertex_property<char>(name) ||
           model->template get_vertex_property<unsigned char>(name);
}

template <class MODEL>
bool edge_scalar(const MODEL *model, const std::string &name) {
    return model->template get_edge_property<float>(name) ||
           model->template get_edge_property<double>(name) ||
           model->template get_edge_property<int>(name) ||
           model->template get_edge_property<unsigned int>(name) ||
           model->template get_edge_property<char>(name) ||
           model->template get_edge_property<unsigned char>(name);
}

template <class MODEL>
bool face_scalar(const MODEL *model, const std::string &name) {
    return model->template get_face_property<float>(name) ||
           model->template get_face_property<double>(name) ||
           model->template get_face_property<int>(name) ||
           model->template get_face_property<unsigned int>(name) ||
           model->template get_face_property<char>(name) ||
           model->template get_face_property<unsigned char>(name);
}

template <class IS_SCALAR>
void append_schemes(const std::vector<std::string> &names, State::Location location,
                    const IS_SCALAR &is_scalar, std::vector<ColorScheme> &schemes) {
    for (std::size_t i = 0; i < names.size(); ++i) {
        const std::string &name = names[i];
        ColorScheme scheme;
        scheme.location = location;
        scheme.property = name;
        // "_color" covers the baked AO colors ("v:ao_color").
        if (name.find(":color") != std::string::npos || name.find("_color") != std::string::npos) {
            scheme.label = name;
            scheme.method = State::COLOR_PROPERTY;
        } else if (name.find(":texcoord") != std::string::npos) {
            scheme.label = name;
            scheme.method = State::TEXTURED;
        } else if (is_scalar(name)) {
            scheme.label = "scalar - " + name;
            scheme.method = State::SCALAR_FIELD;
        } else
            continue;
        schemes.push_back(scheme);
    }
}

// The schemes a drawable can be colored by. Faces belong to triangles, edges to
// lines, vertices to all three; everything comes from the drawable's own model,
// so a drawable never offers a property its shader cannot read.
void collect_color_schemes(Model *model, Drawable *drawable, std::vector<ColorScheme> &schemes) {
    ColorScheme uniform;
    uniform.label = "uniform color";
    uniform.method = State::UNIFORM_COLOR;
    uniform.location = State::VERTEX;
    uniform.property = "uniform color";
    schemes.push_back(uniform);

    const bool triangles = dynamic_cast<TrianglesDrawable *>(drawable) != nullptr;
    const bool lines = dynamic_cast<LinesDrawable *>(drawable) != nullptr;

    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        append_schemes(mesh->vertex_properties(), State::VERTEX,
                       [mesh](const std::string &name) { return vertex_scalar(mesh, name); }, schemes);
        if (lines)
            append_schemes(mesh->edge_properties(), State::EDGE,
                           [mesh](const std::string &name) { return edge_scalar(mesh, name); }, schemes);
        if (triangles) {
            append_schemes(mesh->face_properties(), State::FACE,
                           [mesh](const std::string &name) { return face_scalar(mesh, name); }, schemes);
            append_schemes(mesh->halfedge_properties(), State::HALFEDGE,
                           [](const std::string &) { return false; }, schemes);
        }
    } else if (auto *poly = dynamic_cast<PolyMesh *>(model)) {
        append_schemes(poly->vertex_properties(), State::VERTEX,
                       [poly](const std::string &name) { return vertex_scalar(poly, name); }, schemes);
        if (lines)
            append_schemes(poly->edge_properties(), State::EDGE,
                           [poly](const std::string &name) { return edge_scalar(poly, name); }, schemes);
        if (triangles)
            append_schemes(poly->face_properties(), State::FACE,
                           [poly](const std::string &name) { return face_scalar(poly, name); }, schemes);
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        append_schemes(graph->vertex_properties(), State::VERTEX,
                       [graph](const std::string &name) { return vertex_scalar(graph, name); }, schemes);
        if (lines)
            append_schemes(graph->edge_properties(), State::EDGE,
                           [graph](const std::string &name) { return edge_scalar(graph, name); }, schemes);
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        append_schemes(cloud->vertex_properties(), State::VERTEX,
                       [cloud](const std::string &name) { return vertex_scalar(cloud, name); }, schemes);
    }
}

// The combo caption of the drawable's current scheme, matching the labels above.
std::string scheme_label(const Drawable *drawable) {
    switch (drawable->coloring_method()) {
        case State::UNIFORM_COLOR:
            return "uniform color";
        case State::SCALAR_FIELD:
            return "scalar - " + drawable->property_name();
        default:
            return drawable->property_name();
    }
}

bool is_current_scheme(const Drawable *drawable, const ColorScheme &scheme) {
    if (drawable->coloring_method() != scheme.method)
        return false;
    if (scheme.method == State::UNIFORM_COLOR)
        return true;
    return drawable->property_location() == scheme.location &&
           drawable->property_name() == scheme.property;
}

// The colormaps shipped in resources/colormaps; "random" is generated.
struct ColormapEntry {
    std::string name;
    std::string file; // empty for the generated random colormap
};

const std::vector<ColormapEntry> &colormaps() {
    static std::vector<ColormapEntry> entries;
    if (entries.empty()) {
        const std::string dir = resource::directory() + "/colormaps/";
        const char *names[] = {"default", "french", "rainbow", "blue_red",
                               "blue_white", "blue_yellow", "black_white", "random"};
        for (const char *name : names) {
            ColormapEntry entry;
            entry.name = name;
            if (entry.name != "random")
                entry.file = dir + entry.name + ".png";
            if (entry.file.empty() || file_system::is_file(entry.file))
                entries.push_back(entry);
        }
    }
    return entries;
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

    // Scene-wide controls, above the per-model section: the background, the
    // projection, and the two full-frame passes, each collapsed to one header.
    vec4 background = background_color();
    if (ImGui::ColorEdit3("Background", background.data(), kSwatch)) {
        set_background_color(background);
        update();
    }

    const bool perspective = camera()->type() == Camera::PERSPECTIVE;
    if (ImGui::RadioButton("Persp", perspective)) {
        camera()->set_type(Camera::PERSPECTIVE);
        update();
    }
    ImGui::SetItemTooltip("Perspective projection");
    ImGui::SameLine();
    if (ImGui::RadioButton("Ortho", !perspective)) {
        camera()->set_type(Camera::ORTHOGRAPHIC);
        update();
    }
    ImGui::SetItemTooltip("Orthographic projection");

    ImGui::Spacing();
    // Qt's Global Setting panel exposed these four effects; each is a full-frame
    // pass that owns the scene render, so at most one runs at a time. The header
    // names the running one, and the trailing id keeps its open state stable.
    std::string effects_state;
    if (ao_enabled())
        effects_state = "ambient occlusion";
    else if (edl_enabled())
        effects_state = "eye dome lighting";
    else if (shadow_enabled())
        effects_state = "shadow";
    else if (transparency_enabled())
        effects_state = "transparency";
    std::string effects_label = "Effects";
    if (!effects_state.empty())
        effects_label += " (" + effects_state + ")";
    const bool effects_open = ImGui::CollapsingHeader((effects_label + "###effects").c_str());
    ImGui::SetItemTooltip("Only one effect runs at a time; enabling one turns the others off.");
    if (effects_open) {
        bool ao = ao_enabled();
        if (ImGui::Checkbox("Ambient Occlusion", &ao))
            set_ao_enabled(ao);
        if (ao) {
            float ao_radius = setting::effect_ao_radius_ratio;
            if (ImGui::SliderFloat("Radius ratio", &ao_radius, 0.01f, 0.1f, "%.3f")) {
                setting::effect_ao_radius_ratio = ao_radius;
                apply_effect_parameters();
            }
            float ao_intensity = setting::effect_ao_intensity;
            if (ImGui::SliderFloat("Intensity##ao", &ao_intensity, 0.0f, 1.0f, "%.2f")) {
                setting::effect_ao_intensity = ao_intensity;
                apply_effect_parameters();
            }
            float ao_bias = setting::effect_ao_bias;
            if (ImGui::SliderFloat("Bias##ao", &ao_bias, 0.001f, 0.05f, "%.4f")) {
                setting::effect_ao_bias = ao_bias;
                apply_effect_parameters();
            }
        }

        bool edl = edl_enabled();
        if (ImGui::Checkbox("Eye Dome Lighting", &edl))
            set_edl_enabled(edl);

        bool shadow = shadow_enabled();
        if (ImGui::Checkbox("Shadow", &shadow))
            set_shadow_enabled(shadow);
        if (shadow) {
            static const char *patterns[] = {"Poisson 25 x 25", "Poisson 32 x 64", "Poisson 64 x 128",
                                             "Poisson 100 x 100", "regular 49 x 225"};
            int pattern = setting::effect_shadow_pattern;
            if (ImGui::Combo("Light pattern", &pattern, patterns, 5)) {
                setting::effect_shadow_pattern = pattern;
                apply_effect_parameters();
            }
            float light_distance = setting::effect_shadow_light_distance;
            if (ImGui::SliderFloat("Light distance", &light_distance, 3.0f, 50.0f, "%.0f")) {
                setting::effect_shadow_light_distance = light_distance;
                apply_effect_parameters();
            }
            float softness = setting::effect_shadow_softness;
            if (ImGui::SliderFloat("Softness", &softness, 0.0f, 1.0f, "%.2f")) {
                setting::effect_shadow_softness = softness;
                apply_effect_parameters();
            }
            float darkness = setting::effect_shadow_darkness;
            if (ImGui::SliderFloat("Darkness", &darkness, 0.0f, 1.0f, "%.2f")) {
                setting::effect_shadow_darkness = darkness;
                apply_effect_parameters();
            }
        }

        bool transparency = transparency_enabled();
        if (ImGui::Checkbox("Transparency", &transparency))
            set_transparency_enabled(transparency);
        ImGui::SetItemTooltip("Each face drawable sets its own opacity.");
        if (transparency) {
            static const char *methods[] = {"Average color blending", "Dual depth peeling",
                                            "Per-pixel linked lists"};
            const int method = transparency_method();
            const bool combo_open = ImGui::BeginCombo("Method", methods[method]);
            ImGui::SetItemTooltip("How the transparent surfaces are composited.");
            if (combo_open) {
                for (int i = 0; i < 3; ++i) {
                    const bool supported = transparency_method_supported(i);
                    if (!supported)
                        ImGui::BeginDisabled();
                    if (ImGui::Selectable(methods[i], i == method))
                        set_transparency_method(i);
                    if (i == method)
                        ImGui::SetItemDefaultFocus();
                    if (!supported) {
                        ImGui::EndDisabled();
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                            ImGui::SetTooltip("Requires OpenGL 4.3+ (macOS OpenGL is limited to "
                                              "4.1); use the Vulkan backend.");
                    }
                }
                ImGui::EndCombo();
            }
            if (method == 0)
                ImGui::SetItemTooltip("Faster approximation; ignores the order of fragments.");
            else if (method == 1) {
                ImGui::SetItemTooltip("Order-independent approximation; layers bound the "
                                      "peeling passes.");
                int layers = setting::effect_transparency_peels;
                if (ImGui::SliderInt("Layers", &layers, 0, 16, layers == 0 ? "unbounded" : "%d"))
                    setting::effect_transparency_peels = layers;
                // The slider reports no value change on the frame the mouse is released,
                // so the effect only ever saw edits checked outside the change branch.
                if (ImGui::IsItemDeactivatedAfterEdit())
                    apply_effect_parameters();
            } else {
                ImGui::SetItemTooltip("Exact order-independent result; needs more GPU memory.");
                int fragments = setting::effect_transparency_fragments;
                if (ImGui::SliderInt("Fragments/pixel", &fragments, 1, 32))
                    setting::effect_transparency_fragments = fragments;
                if (ImGui::IsItemDeactivatedAfterEdit())
                    apply_effect_parameters();
            }
        }
    }

    ImGui::Spacing();
    // Clipping is a scene-wide effect, so it sits with the other global rendering
    // controls. The guide and Alt-drag manipulation are handled by Viewer.
    auto *plane = ClippingPlane::instance();
    const bool clipping_enabled = plane->is_enabled();
    const std::string clipping_label = std::string("Clipping Plane") +
                                       (clipping_enabled ? " (on)" : "") + "###clipping";
    const bool clipping_open = ImGui::CollapsingHeader(clipping_label.c_str());
    ImGui::SetItemTooltip("Alt + LMB/RMB drag moves the plane");
    if (clipping_open) {
        bool enabled = clipping_enabled;
        if (ImGui::Checkbox("Enabled", &enabled)) {
            plane->set_enabled(enabled);
            if (enabled)
                plane->fit_scene(camera()->scene_center(), camera()->scene_radius());
            update();
        }
        if (enabled) {
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
            if (cross_section) {
                float thickness = plane->cross_section_width();
                if (ImGui::SliderFloat("Thickness", &thickness, 0.001f, 0.1f, "%.3f",
                                       ImGuiSliderFlags_Logarithmic)) {
                    plane->set_cross_section_width(thickness);
                    update();
                }
            }
            if (visible) {
                vec4 guide_color = plane->color();
                if (ImGui::ColorEdit3("Guide color", guide_color.data(), kSwatch)) {
                    plane->set_color(guide_color);
                    update();
                }
            }
        }
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

    ImGui::SeparatorText(file_system::simple_name(model->name()).c_str());
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
    // The panel's "vector - <field>" lines drawables draw property arrows; they are
    // rendering aids of the field selection, not drawables of the model.
    drawables.erase(std::remove_if(drawables.begin(), drawables.end(),
                                   [](const Drawable *d) {
                                       return is_vector_field_drawable(d->name());
                                   }),
                    drawables.end());
    // Stable, so drawables sharing a name keep the renderer's type order.
    std::stable_sort(drawables.begin(), drawables.end(), [&rank](const Drawable *a, const Drawable *b) {
        return rank(a->name()) < rank(b->name());
    });

    // Distinct back color is a State property, so faces and points share these controls.
    // It only takes effect when both sides are lit, so it is shown only then.
    auto draw_back_color = [this](Drawable *d) {
        bool back_color = d->distinct_back_color();
        if (ImGui::Checkbox("Back color", &back_color)) {
            d->set_distinct_back_color(back_color);
            update();
        }
        ImGui::SetItemTooltip("Use a different color for the back faces.");
        if (back_color) {
            ImGui::SameLine();
            vec4 color_back = d->back_color();
            if (ImGui::ColorEdit3("##back", color_back.data(), kSwatch)) {
                d->set_back_color(color_back);
                update();
            }
        }
    };

    // The editor below belongs to this drawable. It is looked up by name, so the
    // selection survives the renderer updates that recreate the drawables.
    Drawable *active = nullptr;
    for (Drawable *drawable : drawables) {
        if (drawable->name() == active_drawable_name_) {
            active = drawable;
            break;
        }
    }
    if (!active && !drawables.empty()) {
        active = drawables.front();
        active_drawable_name_ = active->name();
    }

    // One line per drawable: visibility, uniform color, name, and current scheme.
    // The row selectable runs underneath the checkbox and the swatch, so clicking
    // anywhere else on the line selects the drawable for the editor below.
    for (Drawable *drawable : drawables) {
        ImGui::PushID(drawable);
        const bool selected = (drawable == active);
        const ImVec2 row_min = ImGui::GetCursorScreenPos();
        const float row_w = ImGui::GetContentRegionAvail().x;
        const float row_h = ImGui::GetFrameHeight();
        if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_AllowOverlap, ImVec2(0.0f, row_h)))
            active_drawable_name_ = drawable->name();

        ImGui::SameLine(0.0f, 0.0f);
        bool visible = drawable->is_visible();
        if (ImGui::Checkbox("##visible", &visible)) {
            drawable->set_visible(visible);
            update();
        }
        ImGui::SetItemTooltip(visible ? "Hide" : "Show");

        ImGui::SameLine();
        vec4 color = drawable->color();
        if (ImGui::ColorEdit3("##color", color.data(), kSwatch)) {
            // Picking a color is an explicit request for plain coloring, even when
            // the drawable is currently colored by a property or a scalar field.
            drawable->set_uniform_coloring(color);
            active_drawable_name_ = drawable->name();
            update();
        }
        ImGui::SetItemTooltip("Uniform color");
        const float swatch_right = ImGui::GetItemRectMax().x;

        // Name on the left, scheme on the right, clipped where they meet.
        const std::string scheme = scheme_label(drawable);
        const float text_y = row_min.y + std::floor((row_h - ImGui::GetTextLineHeight()) * 0.5f);
        const float name_x = swatch_right + ImGui::GetStyle().ItemSpacing.x;
        const float row_right = row_min.x + row_w;
        const float scheme_x = std::max(name_x + 30.0f,
                                        row_right - ImGui::CalcTextSize(scheme.c_str()).x);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const ImU32 name_color = ImGui::GetColorU32(visible ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        draw_list->PushClipRect(ImVec2(name_x, row_min.y), ImVec2(scheme_x - 6.0f, row_min.y + row_h), true);
        draw_list->AddText(ImVec2(name_x, text_y), name_color, drawable->name().c_str());
        draw_list->PopClipRect();
        draw_list->PushClipRect(ImVec2(scheme_x, row_min.y), ImVec2(row_right, row_min.y + row_h), true);
        draw_list->AddText(ImVec2(scheme_x, text_y), ImGui::GetColorU32(ImGuiCol_TextDisabled), scheme.c_str());
        draw_list->PopClipRect();

        ImGui::PopID();
    }

    if (!active) {
        ImGui::TextDisabled("No drawables.");
        ImGui::End();
        return;
    }

    ImGui::Spacing();
    ImGui::SeparatorText(active->name().c_str());
    draw_color_scheme(model, active);

    // State has a lit flag and a two-sided flag; together they make three modes.
    const bool lit = active->lighting();
    int lighting_mode = !lit ? 0 : (active->lighting_two_sides() ? 2 : 1);
    static const char *lighting_modes[] = {"unlit", "front", "front and back"};
    if (ImGui::Combo("Lighting", &lighting_mode, lighting_modes, 3)) {
        active->set_lighting(lighting_mode != 0);
        active->set_lighting_two_sides(lighting_mode == 2);
        update();
    }
    ImGui::SetItemTooltip("Unlit ignores the normals; front and back also shades the back faces.");
    // The two-sided flag survives "Unlit", but without lighting no side is shaded.
    const bool two_sides = lit && active->lighting_two_sides();

    if (auto *triangles = dynamic_cast<TrianglesDrawable *>(active)) {
        // Smooth shading interpolates the normals, which only an unlit pass ignores.
        if (lit) {
            bool smooth = triangles->smooth_shading();
            if (ImGui::Checkbox("Smooth shading", &smooth)) {
                triangles->set_smooth_shading(smooth);
                update();
            }
        }

        // Opacity only reaches the screen through the transparency pass.
        if (transparency_enabled()) {
            float opacity = triangles->opacity();
            if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
                triangles->set_opacity(opacity);
                update();
            }
            ImGui::SetItemTooltip("Only takes effect while the Transparency effect is on.");
        }

        if (two_sides)
            draw_back_color(triangles);
    } else if (auto *lines = dynamic_cast<LinesDrawable *>(active)) {
        static const char *imposters[] = {"plain", "cylinder", "cone"};
        int imposter = static_cast<int>(lines->impostor_type());
        if (ImGui::Combo("Imposter", &imposter, imposters, 3)) {
            lines->set_impostor_type(static_cast<LinesDrawable::ImposterType>(imposter));
            update();
        }

        float width = lines->line_width();
        if (ImGui::SliderFloat("Width", &width, 1.0f, 10.0f, "%.1f px")) {
            lines->set_line_width(width);
            update();
        }
        ImGui::SetItemTooltip("Line width (the diameter of an imposter) in logical pixels.");
    } else if (auto *points = dynamic_cast<PointsDrawable *>(active)) {
        static const char *imposters[] = {"plain", "sphere", "surfel"};
        int imposter = static_cast<int>(points->impostor_type());
        if (ImGui::Combo("Imposter", &imposter, imposters, 3)) {
            points->set_impostor_type(static_cast<PointsDrawable::ImposterType>(imposter));
            update();
        }

        float size = points->point_size();
        if (ImGui::SliderFloat("Size", &size, 1.0f, 20.0f, "%.1f px")) {
            points->set_point_size(size);
            update();
        }
        ImGui::SetItemTooltip("Point size (the diameter of an imposter) in logical pixels.");

        if (two_sides)
            draw_back_color(points);
    }

    draw_vector_field_controls(model, active);

    // A range of rendered primitives tinted with the highlight color. The range
    // lives here rather than in the drawable so the hover aid can yield to it.
    ImGui::Spacing();
    HighlightRange &highlight = highlight_ranges_[model][active->name()];
    bool highlight_changed = ImGui::Checkbox("Highlight range", &highlight.enabled);
    ImGui::SetItemTooltip("Tints a range of rendered primitives (points, lines, or "
                          "triangles; polygons are triangulated).");
    if (highlight.enabled) {
        int first = highlight.first;
        int last = highlight.last;
        bool edited = false;
        ImGui::SetNextItemWidth(70.0f);
        if (ImGui::DragInt("From", &first, 1.0f, 0, 100000000))
            edited = true;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70.0f);
        if (ImGui::DragInt("To", &last, 1.0f, 0, 100000000))
            edited = true;
        if (edited) {
            highlight.first = std::max(0, std::min(first, last));
            highlight.last = std::max(first, last);
            highlight_changed = true;
        }
    }

    if (highlight_changed) {
        if (highlight.enabled) {
            active->set_highlight_range({highlight.first, highlight.last});
            active->set_highlight(true);
            // The hover aid must not replace this range; it re-applies on the next hover.
            if (hover_drawable_.lock().get() == active) {
                hover_drawable_.reset();
                hover_range_ = {-1, -1};
            }
        } else {
            active->set_highlight(false);
            active->set_highlight_range({-1, -1});
        }
        update();
    }

    ImGui::End();
}

// A texture for a textured drawable from the panel's texture style: the chosen
// image, optionally quantized into stripes. nullptr lets the renderer install its
// default checkerboard.
Texture *texture_from_style(const std::string &file, bool discrete, int stripes) {
    if (file.empty())
        return nullptr;
    if (discrete)
        return TextureManager::request(file, std::max(2, std::min(stripes, 64)),
                                       Texture::REPEAT, Texture::LINEAR);
    return TextureManager::request(file, Texture::REPEAT, Texture::LINEAR);
}

// The location at which a drawable samples a vector field: points on vertices,
// lines on edges, triangles on faces.
State::Location vector_field_location(const Drawable *drawable) {
    if (dynamic_cast<const TrianglesDrawable *>(drawable))
        return State::FACE;
    if (dynamic_cast<const LinesDrawable *>(drawable))
        return State::EDGE;
    return State::VERTEX;
}

// vec3 properties that make sense as vectors. Positions and colors are data, not
// directions, so they are skipped. Mirrors the vector fields the buffer update
// supports for each model type.
template <class MODEL>
void append_vector_fields_on_vertices(const MODEL *model, std::vector<std::string> &fields) {
    for (const std::string &name : model->vertex_properties()) {
        if (name == "v:point" || name == "v:color")
            continue;
        if (model->template get_vertex_property<vec3>(name))
            fields.push_back(name);
    }
}

template <class MODEL>
void append_vector_fields_on_edges(const MODEL *model, std::vector<std::string> &fields) {
    for (const std::string &name : model->edge_properties()) {
        if (name == "e:color")
            continue;
        if (model->template get_edge_property<vec3>(name))
            fields.push_back(name);
    }
}

template <class MODEL>
void append_vector_fields_on_faces(const MODEL *model, std::vector<std::string> &fields) {
    for (const std::string &name : model->face_properties()) {
        if (name == "f:color")
            continue;
        if (model->template get_face_property<vec3>(name))
            fields.push_back(name);
    }
}

// The vector fields a drawable can show. Graph models are excluded: the buffer
// update has no vector-field variant for them.
void collect_vector_fields(const Model *model, const Drawable *drawable,
                           std::vector<std::string> &fields) {
    const State::Location location = vector_field_location(drawable);
    if (location == State::FACE) {
        if (auto *mesh = dynamic_cast<const SurfaceMesh *>(model))
            append_vector_fields_on_faces(mesh, fields);
        else if (auto *poly = dynamic_cast<const PolyMesh *>(model))
            append_vector_fields_on_faces(poly, fields);
    } else if (location == State::EDGE) {
        if (auto *mesh = dynamic_cast<const SurfaceMesh *>(model))
            append_vector_fields_on_edges(mesh, fields);
        else if (auto *poly = dynamic_cast<const PolyMesh *>(model))
            append_vector_fields_on_edges(poly, fields);
    } else {
        if (auto *mesh = dynamic_cast<const SurfaceMesh *>(model))
            append_vector_fields_on_vertices(mesh, fields);
        else if (auto *poly = dynamic_cast<const PolyMesh *>(model))
            append_vector_fields_on_vertices(poly, fields);
        else if (auto *cloud = dynamic_cast<const PointCloud *>(model))
            append_vector_fields_on_vertices(cloud, fields);
    }
}

Texture *MappleViewer::scalar_texture(const ScalarFieldStyle &style) const {
    const std::vector<ColormapEntry> &entries = colormaps();
    if (entries.empty())
        return nullptr;
    const int index = std::max(0, std::min(style.colormap, static_cast<int>(entries.size()) - 1));
    const ColormapEntry &entry = entries[index];
    const int stripes = std::max(1, std::min(style.stripes, 256));
    if (entry.name == "random")
        return TextureManager::request(stripes, 256 / stripes, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    if (style.discrete)
        return TextureManager::request(entry.file, stripes, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    return TextureManager::request(entry.file, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
}

void MappleViewer::apply_scalar_style(Drawable *drawable, State::Location location,
                                      const std::string &property, const ScalarFieldStyle &style) {
    drawable->set_scalar_coloring(location, property, scalar_texture(style),
                                  style.clamp_lower, style.clamp_upper);
    drawable->set_clamp_range(style.clamp);
    drawable->update();
}

void MappleViewer::draw_color_scheme(Model *model, Drawable *drawable) {
    // A background command edits the model's properties on a worker thread, and the
    // scheme list reads them, so it waits until the job has finished.
    if (process_job_ && process_job_->target == model) {
        ImGui::TextDisabled("Color scheme unavailable while processing.");
        return;
    }

    std::vector<ColorScheme> schemes;
    collect_color_schemes(model, drawable, schemes);

    ImGui::TextUnformatted("Color scheme");
    ImGui::SetNextItemWidth(-1.0f);
    const std::string preview = scheme_label(drawable);
    if (ImGui::BeginCombo("##scheme", preview.c_str())) {
        for (std::size_t i = 0; i < schemes.size(); ++i) {
            const ColorScheme &scheme = schemes[i];
            if (!ImGui::Selectable(scheme.label.c_str(), is_current_scheme(drawable, scheme)))
                continue;
            if (scheme.method == State::SCALAR_FIELD) {
                ScalarFieldStyle &style = scalar_styles_[model][drawable->name()];
                apply_scalar_style(drawable, scheme.location, scheme.property, style);
            } else {
                switch (scheme.method) {
                    case State::UNIFORM_COLOR:
                        drawable->set_uniform_coloring(drawable->color());
                        break;
                    case State::COLOR_PROPERTY:
                        drawable->set_property_coloring(scheme.location, scheme.property);
                        break;
                    case State::TEXTURED: {
                        TextureStyle &style = texture_styles_[model][drawable->name()];
                        drawable->set_texture_coloring(scheme.location, scheme.property,
                                                       texture_from_style(style.file, style.discrete,
                                                                          style.stripes));
                        break;
                    }
                    default:
                        break;
                }
                drawable->update();
            }
            update();
        }
        ImGui::EndCombo();
    }

    if (drawable->coloring_method() == State::TEXTURED) {
        draw_texture_controls(model, drawable);
        return;
    }

    if (drawable->coloring_method() != State::SCALAR_FIELD)
        return;

    ScalarFieldStyle &style = scalar_styles_[model][drawable->name()];
    bool changed = false;

    const std::vector<ColormapEntry> &entries = colormaps();
    if (!entries.empty()) {
        const int index = std::max(0, std::min(style.colormap, static_cast<int>(entries.size()) - 1));
        if (ImGui::BeginCombo("Colormap", entries[index].name.c_str())) {
            for (std::size_t i = 0; i < entries.size(); ++i) {
                if (ImGui::Selectable(entries[i].name.c_str(), static_cast<int>(i) == index)) {
                    style.colormap = static_cast<int>(i);
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
    }

    if (ImGui::Checkbox("Discrete", &style.discrete))
        changed = true;
    if (style.discrete && ImGui::SliderInt("Stripes", &style.stripes, 2, 64))
        changed = true;

    if (ImGui::Checkbox("Clamp range", &style.clamp))
        changed = true;
    if (style.clamp) {
        int lower = static_cast<int>(std::lround(style.clamp_lower * 100.0f));
        int upper = static_cast<int>(std::lround(style.clamp_upper * 100.0f));
        if (ImGui::SliderInt("Lower %", &lower, 0, 99)) {
            style.clamp_lower = std::min(lower, 99 - upper) / 100.0f;
            changed = true;
        }
        if (ImGui::SliderInt("Upper %", &upper, 0, 99)) {
            style.clamp_upper = std::min(upper, 99 - lower) / 100.0f;
            changed = true;
        }
    }

    if (changed) {
        apply_scalar_style(drawable, drawable->property_location(), drawable->property_name(), style);
        update();
    }
}

void MappleViewer::draw_texture_controls(Model *model, Drawable *drawable) {
    TextureStyle &style = texture_styles_[model][drawable->name()];
    // A drawable that was already textured (e.g., set up when the model loaded)
    // seeds the file field from the texture actually in use.
    const Texture *texture = drawable->texture();
    if (style.file.empty() && texture && file_system::is_file(texture->name()))
        style.file = texture->name();

    bool changed = false;

    const float button_width = ImGui::CalcTextSize("Choose...").x + 2.0f * ImGui::GetStyle().FramePadding.x;
    const float row_right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

    ImGui::TextUnformatted("Texture");
    ImGui::SameLine();
    // The name is shortened rather than drawn under the button for long paths.
    std::string file_label = style.file.empty() ? std::string("(default)")
                                                : file_system::simple_name(style.file);
    const float name_x = ImGui::GetCursorScreenPos().x;
    const float name_limit = row_right - button_width - 8.0f - name_x;
    if (ImGui::CalcTextSize(file_label.c_str()).x > name_limit) {
        while (!file_label.empty() && ImGui::CalcTextSize((file_label + "...").c_str()).x > name_limit)
            file_label.pop_back();
        file_label += "...";
    }
    ImGui::TextDisabled("%s", file_label.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - button_width);
    if (ImGui::Button("Choose...")) {
        const std::string file = dialog::open(
                "Choose a texture image", resource::directory() + "/textures/",
                {"Image Files (*.png *.jpg *.jpeg *.bmp *.tga)", "*.png *.jpg *.jpeg *.bmp *.tga",
                 "All Files (*.*)", "*"});
        if (!file.empty()) {
            style.file = file;
            changed = true;
        }
    }
    ImGui::SetItemTooltip("The image the texture coordinates sample.");

    if (ImGui::Checkbox("Discrete", &style.discrete))
        changed = true;
    ImGui::SetItemTooltip("Quantize the image into a few uniformly colored stripes.");
    if (style.discrete && ImGui::SliderInt("Stripes", &style.stripes, 2, 64))
        changed = true;

    int repeat = static_cast<int>(std::lround(drawable->texture_repeat()));
    if (ImGui::DragInt("Repeat", &repeat, 0.05f, 0, 100)) {
        drawable->set_texture_repeat(static_cast<float>(std::max(0, repeat)));
        drawable->update();
        update();
    }
    int fraction = static_cast<int>(std::lround(drawable->texture_fractional_repeat()));
    if (ImGui::DragInt("Fraction", &fraction, 0.5f, 0, 99)) {
        drawable->set_texture_fractional_repeat(static_cast<float>(std::max(0, std::min(fraction, 99))));
        drawable->update();
        update();
    }
    ImGui::SetItemTooltip("Texture repeats; 100 fractional repeats make one whole repeat.");

    if (changed) {
        if (Texture *tex = texture_from_style(style.file, style.discrete, style.stripes))
            drawable->set_texture_coloring(drawable->property_location(), drawable->property_name(),
                                           tex, drawable->texture_repeat(),
                                           drawable->texture_fractional_repeat());
        drawable->update();
        update();
    }
}

void MappleViewer::show_vector_field(Model *model, const std::string &field,
                                     State::Location location) {
    if (!model || field.empty())
        return;
    Renderer *renderer = model->renderer();
    const std::string name = "vector - " + field;
    LinesDrawable *drawable = renderer->get_lines_drawable(name, false);
    if (!drawable) {
        drawable = renderer->add_lines_drawable(name);
        if (!drawable)
            return;
        drawable->set_update_func([this, field, location](Model *m, Drawable *d) {
            // Read the scale at update time: it changes without rebuilding the
            // drawable, and the map outlives every buffer update.
            float scale = 1.0f;
            auto model_entry = vector_field_scales_.find(m);
            if (model_entry != vector_field_scales_.end()) {
                auto scale_entry = model_entry->second.find(field);
                if (scale_entry != model_entry->second.end())
                    scale = scale_entry->second;
            }
            auto *lines = static_cast<LinesDrawable *>(d);
            if (auto *mesh = dynamic_cast<SurfaceMesh *>(m))
                buffer::update(mesh, lines, field, location, scale);
            else if (auto *poly = dynamic_cast<PolyMesh *>(m))
                buffer::update(poly, lines, field, location, scale);
            else if (auto *cloud = dynamic_cast<PointCloud *>(m))
                buffer::update(cloud, lines, field, scale);
        });
    }
    drawable->set_visible(true);
    drawable->update();
    update();
}

void MappleViewer::draw_vector_field_controls(Model *model, Drawable *drawable) {
    std::vector<std::string> fields;
    collect_vector_fields(model, drawable, fields);
    if (fields.empty())
        return;

    std::string &current = vector_fields_[model][drawable->name()];
    // The field is gone after the model was processed: drop it and its arrows.
    if (!current.empty() && std::find(fields.begin(), fields.end(), current) == fields.end()) {
        if (auto *lines = model->renderer()->get_lines_drawable("vector - " + current, false))
            lines->set_visible(false);
        current.clear();
    }

    const std::string preview = current.empty() ? "Disabled" : current;
    if (ImGui::BeginCombo("Vector field", preview.c_str())) {
        if (ImGui::Selectable("Disabled", current.empty()) && !current.empty()) {
            if (auto *lines = model->renderer()->get_lines_drawable("vector - " + current, false)) {
                lines->set_visible(false);
                update();
            }
            current.clear();
        }
        for (const std::string &field : fields) {
            if (ImGui::Selectable(field.c_str(), field == current)) {
                current = field;
                show_vector_field(model, field, vector_field_location(drawable));
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SetItemTooltip("Draws the vector property as arrows in a \"vector - <field>\" "
                          "lines drawable that every drawable showing this field shares.");

    if (current.empty())
        return;

    auto &scales = vector_field_scales_[model];
    auto entry = scales.find(current);
    if (entry == scales.end())
        entry = scales.emplace(current, 1.0f).first;
    float &scale = entry->second;
    if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.01f, 100.0f, "%.2f")) {
        if (auto *lines = model->renderer()->get_lines_drawable("vector - " + current, false)) {
            lines->update();
            update();
        }
    }
    ImGui::SetItemTooltip("Vector length relative to the model scale (see easy3d::buffer).");
}

void MappleViewer::draw_model_info(Model *model) {
    if (!model)
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
        ImGui::BeginChild("##model-properties", ImVec2(0, 110), 0);
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

    const float buttons_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##models-list", ImVec2(0, -buttons_height));

    for (std::size_t i = 0; i < models().size(); ++i) {
        Model *model = models()[i].get();
        ImGui::PushID(static_cast<int>(i));

        const std::string label = file_system::simple_name(model->name());
        const bool selected = model == current_model();
        const bool clicked = ImGui::Selectable(label.c_str(), selected);
        if (clicked) {
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

        // The info describes the model whose name was clicked, so it sits directly
        // below that name; clicking another name moves it there.
        if (selected || clicked) {
            ImGui::Indent();
            draw_model_info(model);
            ImGui::Unindent();
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
    ImGui::TextDisabled("I'm good software, though I have defects.");
    ImGui::Separator();
    ImGui::Text("Easy3D %s - %d", version_string().c_str(), EASY3D_RELEASE_DATE);
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
