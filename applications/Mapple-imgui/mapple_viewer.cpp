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

#include "mapple_viewer.h"

#include "imgui_layer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

namespace {

    // The drawable that shows v:select / f:select for a model.
    struct SelectionTarget {
        Drawable *drawable;
        State::Location location;
        const char *property;
    };

    bool selection_target(Model *model, SelectionTarget &target) {
        if (!model || !model->renderer())
            return false;
        Renderer *renderer = model->renderer();
        if (dynamic_cast<PointCloud *>(model)) {
            PointsDrawable *drawable = renderer->get_points_drawable("vertices", false);
            if (!drawable && !renderer->points_drawables().empty())
                drawable = renderer->points_drawables().front().get();
            if (!drawable)
                return false;
            target.drawable = drawable;
            target.location = State::VERTEX;
            target.property = "v:select";
            return true;
        }
        if (dynamic_cast<SurfaceMesh *>(model)) {
            TrianglesDrawable *drawable = renderer->get_triangles_drawable("faces", false);
            if (!drawable && !renderer->triangles_drawables().empty())
                drawable = renderer->triangles_drawables().front().get();
            if (!drawable)
                return false;
            target.drawable = drawable;
            target.location = State::FACE;
            target.property = "f:select";
            return true;
        }
        return false;
    }

    // texel 0 reproduces the drawable's own color, texel 1 is the highlight color.
    // Scalar coloring samples this as a 1D colormap at texture coordinate x.
    Texture *create_selection_colormap(const vec4 &unselected) {
        std::vector<unsigned char> rgba(8, 255);
        const vec4 colors[2] = {unselected, setting::highlight_color};
        for (int i = 0; i < 2; ++i) {
            for (int c = 0; c < 3; ++c)
                rgba[4 * i + c] = static_cast<unsigned char>(clamp(colors[i][c], 0.0f, 1.0f) * 255.0f + 0.5f);
            rgba[4 * i + 3] = 255;
        }
        return Texture::create(rgba, 2, 1, 4, Texture::CLAMP_TO_EDGE, Texture::NEAREST);
    }

    // A drag may leave the scene over a panel; keep the region inside the scene rect.
    float clamp_scene_coord(int value, int origin, int extent) {
        const int last = origin + std::max(0, extent - 1);
        return static_cast<float>(std::min(std::max(value, origin), last));
    }

    bool has_any_selection(Model *model) {        if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
            auto select = cloud->get_vertex_property<bool>("v:select");
            if (!select)
                return false;
            for (auto v : cloud->vertices())
                if (select[v])
                    return true;
        } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
            auto select = mesh->get_face_property<bool>("f:select");
            if (!select)
                return false;
            for (auto f : mesh->faces())
                if (select[f])
                    return true;
        }
        return false;
    }

} // namespace

MappleViewer::MappleViewer(const std::string &title,
                           int width,
                           int height,
                           int samples,
                           bool full_screen,
                           bool visible,
                           bool resizable,
#ifdef EASY3D_HAS_VULKAN
                           const VulkanOptions &options
#else
                           const OpenGLOptions &options
#endif
        )
        : Viewer(title, width, height, samples, full_screen, visible, resizable, options) {
    // The vendored GLFW is a private dependency of easy3d_viewer, so an app that also calls
    // GLFW (here through the ImGui backend) links its own static copy. On Windows that copy
    // must be initialized before it can operate on the viewer's window. See issue #178.
#if defined(_WIN32) && defined(_MSC_VER)
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed for Mapple's GLFW copy");
#endif
}

MappleViewer::~MappleViewer() {
    // Release selection colormaps before the graphics device dies with the viewer.
    selection_styles_.clear();
    // Destroy the ImGui context and its font texture while the viewer (and its graphics
    // device) is still alive.
    imgui_.reset();
}

void MappleViewer::send(el::Level level, const std::string &msg) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    log_.push_back({level, msg});
    while (log_.size() > max_log_lines_)
        log_.pop_front();
    ++log_generation_;
}

float MappleViewer::pixel_ratio() const {
    const int w = width();
    return w > 0 ? static_cast<float>(framebuffer_width()) / static_cast<float>(w) : 1.0f;
}

void MappleViewer::init() {
    Viewer::init();

    // The UI replaces the viewer's screen-space HUD: panels cover the framebuffer's
    // corners, and the information is shown in the panels and the status bar.
    show_easy3d_logo_ = false;
    show_frame_rate_ = false;
    show_corner_axes_ = false;

    imgui_.reset(new ImGuiLayer);
    if (!imgui_->init(window(), dpi_scaling(), pixel_ratio()))
        throw std::runtime_error("failed to initialize Dear ImGui");
    imgui_->set_display_size(width(), height());

#ifdef EASY3D_HAS_VULKAN
    LOG(INFO) << "Mapple UI ready (Vulkan)";
#else
    LOG(INFO) << "Mapple UI ready (OpenGL)";
#endif
}

void MappleViewer::pre_draw() {
    imgui_->new_frame(dpi_scaling(), pixel_ratio());
    Viewer::pre_draw();
    draw_gui();

    std::vector<graphics::DrawCommand> commands = imgui_->render(framebuffer_width(), framebuffer_height());
    // The rubber band belongs to the scene layer: insert it before the UI geometry
    // so panels and menus always cover it.
    std::vector<graphics::DrawCommand> hints;
    append_selection_overlay(hints);
    if (!hints.empty())
        commands.insert(commands.begin(), hints.begin(), hints.end());
    set_overlays(std::move(commands), true);
}

void MappleViewer::post_resize(int w, int h) {
    Viewer::post_resize(w, h);
    if (imgui_)
        imgui_->set_display_size(w, h);
}

bool MappleViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    mouse_captured_ = imgui_ && imgui_->wants_mouse();
    if (mouse_captured_)
        return true;
    // The processing window is modal: clicks outside it must not reach the scene.
    if (process_op_ != ProcessOp::NONE)
        return true;

    // Alt is the clipping-plane drag modifier; it takes precedence over a selection
    // gesture so the plane stays movable while a selection tool is active.
    if (select_mode_ != SelectMode::OFF && button == BUTTON_LEFT && !(modifiers & MODIF_ALT) &&
        point_in_scene(x, y) && current_model()) {
        selecting_ = true;
        select_start_ = select_end_ = vec2(static_cast<float>(x), static_cast<float>(y));
        lasso_.clear();
        lasso_.push_back(select_start_);
        select_action_ = (modifiers & MODIF_CTRL) ? SelectAction::REMOVE
                       : (modifiers & MODIF_SHIFT) ? SelectAction::ADD
                                                   : SelectAction::REPLACE;
        clear_hover();
        update();
        return true;
    }

    return Viewer::mouse_press_event(x, y, button, modifiers);
}

bool MappleViewer::mouse_release_event(int x, int y, int button, int modifiers) {
    // The release belongs to whichever side received the press, even if the pointer
    // has moved over a panel since; otherwise the scene would miss its release.
    const bool captured = mouse_captured_;
    mouse_captured_ = false;
    if (captured)
        return true;
    if (process_op_ != ProcessOp::NONE)
        return true;

    if (selecting_ && button == BUTTON_LEFT) {
        const float dx = static_cast<float>(x) - select_start_.x;
        const float dy = static_cast<float>(y) - select_start_.y;
        const bool moved = std::fabs(dx) > 2.0f || std::fabs(dy) > 2.0f;
        const bool region = select_mode_ == SelectMode::RECT ? moved
                          : select_mode_ == SelectMode::LASSO ? moved && lasso_.size() > 2
                                                              : false;
        if (region) {
            select_end_ = vec2(clamp_scene_coord(x, scene_x_, scene_w_),
                               clamp_scene_coord(y, scene_y_, scene_h_));
            if (select_mode_ == SelectMode::LASSO)
                lasso_.push_back(select_end_);
            pending_region_ = true;
        } else {
            pending_pick_ = true;
            pending_x_ = x;
            pending_y_ = y;
        }
        pending_action_ = select_action_;
        selecting_ = false;
        update();
        return true;
    }

    return Viewer::mouse_release_event(x, y, button, modifiers);
}

bool MappleViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (mouse_captured_ || process_op_ != ProcessOp::NONE)
        return true;

    if (selecting_) {
        select_end_ = vec2(clamp_scene_coord(x, scene_x_, scene_w_),
                           clamp_scene_coord(y, scene_y_, scene_h_));
        if (select_mode_ == SelectMode::LASSO) {
            const float lx = select_end_.x - lasso_.back().x;
            const float ly = select_end_.y - lasso_.back().y;
            if (lx * lx + ly * ly >= 4.0f) // sample at least ~2 pixels apart
                lasso_.push_back(select_end_);
        }
        update();
        return true;
    }

    return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}

bool MappleViewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    if (imgui_ && imgui_->wants_mouse()) {
        clear_hover();
        return true;
    }
    if (process_op_ != ProcessOp::NONE) {
        clear_hover();
        return true;
    }

    if (show_pick_readout_ && point_in_scene(x, y)) {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - hover_stamp_);
        if (elapsed.count() >= 50) {
            pending_hover_ = true;
            hover_x_ = x;
            hover_y_ = y;
        }
    } else
        clear_hover();

    return Viewer::mouse_free_move_event(x, y, dx, dy, modifiers);
}

bool MappleViewer::mouse_scroll_event(int x, int y, int dx, int dy) {
    return (imgui_ && imgui_->wants_mouse()) || process_op_ != ProcessOp::NONE ||
           Viewer::mouse_scroll_event(x, y, dx, dy);
}

bool MappleViewer::key_press_event(int key, int modifiers) {
    if (imgui_ && imgui_->wants_keyboard())
        return true;

    if (process_op_ != ProcessOp::NONE) {
        // Esc dismisses the processing window; everything else stays with it.
        if (key == KEY_ESCAPE && modifiers == 0)
            process_op_ = ProcessOp::NONE;
        return true;
    }
    // Escape leaves selection mode. With no selection tool active it would close the
    // window, so ask first: the two meanings are easy to confuse.
    if (key == KEY_ESCAPE && modifiers == 0) {
        if (select_mode_ != SelectMode::OFF) {
            cancel_selection_gesture();
            pending_mode_ = true;
            pending_select_mode_ = SelectMode::OFF;
        } else {
            const dialog::Response response = dialog::message(
                    "Quit Mapple", "Close Mapple?", dialog::Choice::yes_no, dialog::Type::question);
            if (response == dialog::Response::yes)
                request_close();
        }
        return true;
    }
    if (key == KEY_DELETE && modifiers == 0) {
        if (selection_count() > 0)
            confirm_delete_selected();
        return true;
    }
    if (key == KEY_A && modifiers == MODIF_CTRL) {
        pending_command_ = SelectCommand::SELECT_ALL;
        return true;
    }
    if (key == KEY_I && modifiers == MODIF_CTRL) {
        pending_command_ = SelectCommand::INVERT_SELECTION;
        return true;
    }

    return Viewer::key_press_event(key, modifiers);
}

bool MappleViewer::key_release_event(int key, int modifiers) {
    return (imgui_ && imgui_->wants_keyboard()) || Viewer::key_release_event(key, modifiers);
}

bool MappleViewer::char_input_event(unsigned int codepoint) {
    return (imgui_ && imgui_->wants_keyboard()) || Viewer::char_input_event(codepoint);
}

bool MappleViewer::focus_event(bool focused) {
    if (!focused) {
        cancel_selection_gesture();
        pending_pick_ = pending_region_ = pending_hover_ = false;
        clear_hover();
    }
    return Viewer::focus_event(focused);
}

void MappleViewer::draw_gui() {
    process_pending_work();

    draw_menu_bar();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    const float x = viewport->Pos.x;
    const float y = viewport->Pos.y + menu_height_;
    const float w = viewport->Size.x;
    const float h = std::max(0.0f, viewport->Size.y - menu_height_);

    const float toolbar_height = ImGui::GetFrameHeight() + 8.0f;
    const float status_height = ImGui::GetTextLineHeight() + 8.0f;

    draw_toolbar(x, y, w, toolbar_height);
    draw_status_bar(x, y + h - status_height, w, status_height);
    draw_panels(x, y + toolbar_height, w, std::max(0.0f, h - toolbar_height - status_height));

    draw_about_window();
    draw_process_dialog();
}

//------------------------------------------------------------------------
// Selection
//------------------------------------------------------------------------

bool MappleViewer::point_in_scene(int x, int y) const {
    return scene_w_ > 0 && scene_h_ > 0 &&
           x >= scene_x_ && x < scene_x_ + scene_w_ &&
           y >= scene_y_ && y < scene_y_ + scene_h_;
}

void MappleViewer::set_select_mode(SelectMode mode) {
    if (mode == select_mode_)
        return;
    cancel_selection_gesture();
    // The selection display outlives the tool: switching modes (including Off) only
    // changes how the pointer interacts with the scene. Only an empty selection or
    // Clear Selection restores the drawable's own appearance.
    clear_hover();
    if (mode != SelectMode::OFF) {
        Model *model = current_model();
        if (model && has_any_selection(model))
            refresh_selection_display(model);
    }
    select_mode_ = mode;
    update();
}

void MappleViewer::process_pending_work() {
    if (pending_mode_) {
        pending_mode_ = false;
        set_select_mode(pending_select_mode_);
    }

    if (pending_command_ != SelectCommand::NONE) {
        const SelectCommand command = pending_command_;
        pending_command_ = SelectCommand::NONE;
        switch (command) {
            case SelectCommand::SELECT_ALL:
                select_all_primitives();
                break;
            case SelectCommand::CLEAR_SELECTION:
                clear_selection();
                break;
            case SelectCommand::INVERT_SELECTION:
                invert_selection();
                break;
            case SelectCommand::DELETE_SELECTION:
                delete_selected_primitives();
                break;
            case SelectCommand::NONE:
                break;
        }
    }

    if (pending_pick_) {
        pending_pick_ = false;
        const int x = pending_x_, y = pending_y_;
        perform_selection(x, y, pending_action_);
    } else if (pending_region_) {
        pending_region_ = false;
        select_region(pending_action_);
        lasso_.clear();
    }

    if (pending_hover_) {
        pending_hover_ = false;
        update_hover(hover_x_, hover_y_);
    }
}

void MappleViewer::cancel_selection_gesture() {
    selecting_ = false;
    select_start_ = select_end_ = vec2(0);
    lasso_.clear();
}

void MappleViewer::perform_selection(int x, int y, SelectAction action) {
    Model *model = current_model();
    if (!model || !point_in_scene(x, y))
        return;

    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (!select)
            select = cloud->add_vertex_property<bool>("v:select", false);
        if (!point_picker_)
            point_picker_.reset(new PointCloudPicker(camera()));
        const PointCloud::Vertex vertex = point_picker_->pick_vertex(cloud, x, y);
        if (action == SelectAction::REPLACE)
            for (auto v : cloud->vertices())
                select[v] = false;
        if (vertex.is_valid() && !cloud->is_deleted(vertex))
            select[vertex] = action != SelectAction::REMOVE;
        finish_selection_change(model);
        update();
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (!select)
            select = mesh->add_face_property<bool>("f:select", false);
        if (!mesh_picker_)
            mesh_picker_.reset(new SurfaceMeshPicker(camera()));
        const SurfaceMesh::Face face = mesh_picker_->pick_face(mesh, x, y);
        if (action == SelectAction::REPLACE)
            for (auto f : mesh->faces())
                select[f] = false;
        if (face.is_valid() && !mesh->is_deleted(face))
            select[face] = action != SelectAction::REMOVE;
        finish_selection_change(model);
        update();
    }
}

void MappleViewer::select_region(SelectAction action) {
    Model *model = current_model();
    if (!model)
        return;

    const bool deselect = action == SelectAction::REMOVE;
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (!select)
            select = cloud->add_vertex_property<bool>("v:select", false);
        if (!point_picker_)
            point_picker_.reset(new PointCloudPicker(camera()));
        if (action == SelectAction::REPLACE)
            for (auto v : cloud->vertices())
                select[v] = false;
        if (select_mode_ == SelectMode::LASSO && lasso_.size() >= 3)
            point_picker_->pick_vertices(cloud, Polygon2(lasso_.begin(), lasso_.end()), deselect);
        else {
            const Rect rect(select_start_.x, select_end_.x, select_start_.y, select_end_.y);
            point_picker_->pick_vertices(cloud, rect, deselect);
        }
        finish_selection_change(model);
        update();
        LOG(INFO) << selection_count() << " of " << cloud->n_vertices() << " points selected";
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (!select)
            select = mesh->add_face_property<bool>("f:select", false);
        if (!mesh_picker_)
            mesh_picker_.reset(new SurfaceMeshPicker(camera()));
        std::vector<SurfaceMesh::Face> faces;
        if (select_mode_ == SelectMode::LASSO && lasso_.size() >= 3)
            faces = mesh_picker_->pick_faces(mesh, Polygon2(lasso_.begin(), lasso_.end()));
        else {
            const Rect rect(select_start_.x, select_end_.x, select_start_.y, select_end_.y);
            faces = mesh_picker_->pick_faces(mesh, rect);
        }
        if (action == SelectAction::REPLACE)
            for (auto f : mesh->faces())
                select[f] = false;
        for (auto f : faces)
            select[f] = !deselect;
        finish_selection_change(model);
        update();
        LOG(INFO) << selection_count() << " of " << mesh->n_faces() << " faces selected";
    }
}

void MappleViewer::select_all_primitives() {
    Model *model = current_model();
    if (!model)
        return;
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (!select)
            select = cloud->add_vertex_property<bool>("v:select", false);
        for (auto v : cloud->vertices())
            select[v] = true;
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (!select)
            select = mesh->add_face_property<bool>("f:select", false);
        for (auto f : mesh->faces())
            select[f] = true;
    } else
        return;
    if (select_mode_ == SelectMode::OFF)
        set_select_mode(SelectMode::CLICK);
    finish_selection_change(model);
    update();
}

void MappleViewer::clear_selection() {
    Model *model = current_model();
    if (!model)
        return;
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (select)
            for (auto v : cloud->vertices())
                select[v] = false;
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (select)
            for (auto f : mesh->faces())
                select[f] = false;
    }
    if (selection_styles_.find(model) != selection_styles_.end())
        restore_selection_style(model);
    invalidate_selection_count();
    update();
}

void MappleViewer::invert_selection() {
    Model *model = current_model();
    if (!model)
        return;
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (!select)
            select = cloud->add_vertex_property<bool>("v:select", false);
        for (auto v : cloud->vertices())
            select[v] = !select[v];
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (!select)
            select = mesh->add_face_property<bool>("f:select", false);
        for (auto f : mesh->faces())
            select[f] = !select[f];
    } else
        return;
    if (select_mode_ == SelectMode::OFF)
        set_select_mode(SelectMode::CLICK);
    finish_selection_change(model);
    update();
}

void MappleViewer::confirm_delete_selected() {
    const std::size_t count = selection_count();
    if (count == 0)
        return;
    Model *model = current_model();
    const bool mesh = dynamic_cast<SurfaceMesh *>(model) != nullptr;
    const std::string message = "Delete " + std::to_string(count) +
                                (mesh ? " selected faces?" : " selected points?");
    const dialog::Response response =
            dialog::message("Delete selection", message, dialog::Choice::yes_no, dialog::Type::question);
    if (response == dialog::Response::yes)
        pending_command_ = SelectCommand::DELETE_SELECTION;
}

void MappleViewer::delete_selected_primitives() {
    Model *model = current_model();
    if (!model)
        return;

    std::size_t count = 0;
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (select) {
            std::vector<SurfaceMesh::Face> doomed;
            for (auto f : mesh->faces())
                if (select[f])
                    doomed.push_back(f);
            for (auto f : doomed)
                mesh->delete_face(f);
            count = doomed.size();
            if (count) {
                mesh->collect_garbage();
                if (mesh->manipulator())
                    mesh->manipulator()->reset();
                // Topology changed: every drawable (faces, edges, borders, vertices)
                // still holds the old buffers until the renderer is invalidated.
                if (mesh->renderer())
                    mesh->renderer()->update();
                LOG(INFO) << count << " faces deleted";
            }
        }
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (select) {
            std::vector<PointCloud::Vertex> doomed;
            for (auto v : cloud->vertices())
                if (select[v])
                    doomed.push_back(v);
            for (auto v : doomed)
                cloud->delete_vertex(v);
            count = doomed.size();
            if (count) {
                cloud->collect_garbage();
                if (cloud->manipulator())
                    cloud->manipulator()->reset();
                if (cloud->renderer())
                    cloud->renderer()->update();
                LOG(INFO) << count << " points deleted";
            }
        }
    }

    if (count) {
        finish_selection_change(model);
        clear_hover();
        update();
    }
}

std::size_t MappleViewer::selection_count() {
    Model *model = current_model();
    if (!model)
        return 0;
    if (!selection_count_dirty_ && counted_model_ == model)
        return selected_count_;

    counted_model_ = model;
    selection_count_dirty_ = false;
    std::size_t count = 0;
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>("v:select");
        if (select)
            for (auto v : cloud->vertices())
                if (select[v])
                    ++count;
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>("f:select");
        if (select)
            for (auto f : mesh->faces())
                if (select[f])
                    ++count;
    }
    selected_count_ = count;
    return count;
}

void MappleViewer::refresh_selection_display(Model *model) {
    SelectionTarget target;
    if (!selection_target(model, target))
        return;

    auto entry = selection_styles_.find(model);
    if (entry == selection_styles_.end()) {
        SelectionStyle style;
        style.state = static_cast<const State &>(*target.drawable);
        style.colormap.reset(create_selection_colormap(target.drawable->color()));
        if (!style.colormap) {
            LOG(WARNING) << "failed to create the selection colormap";
            return;
        }
        entry = selection_styles_.emplace(model, std::move(style)).first;
        target.drawable->set_distinct_back_color(false);
        target.drawable->set_scalar_coloring(target.location, target.property,
                                             entry->second.colormap.get(), 0.0f, 0.0f);
        buffer::update(model, target.drawable);
    } else if (target.drawable->coloring_method() != State::SCALAR_FIELD ||
               target.drawable->property_location() != target.location ||
               target.drawable->property_name() != target.property) {
        // The selection display survives mode changes and may be overridden by a
        // restyle in the Rendering panel; re-apply it before refreshing texcoords.
        target.drawable->set_distinct_back_color(false);
        target.drawable->set_scalar_coloring(target.location, target.property,
                                             entry->second.colormap.get(), 0.0f, 0.0f);
        buffer::update(model, target.drawable);
    }

    // Fast path: selection changes only move the scalar coordinate of each element.
    if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        auto select = cloud->get_vertex_property<bool>(target.property);
        if (!select)
            return;
        std::vector<vec2> texcoords(target.drawable->num_vertices());
        for (auto v : cloud->vertices())
            texcoords[v.idx()] = vec2(select[v] ? 1.0f : 0.0f, 0.5f);
        target.drawable->update_texcoord_buffer(texcoords);
    } else if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        auto select = mesh->get_face_property<bool>(target.property);
        auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");
        if (!select || !triangle_range)
            return;
        std::vector<vec2> texcoords(target.drawable->num_vertices());
        for (auto f : mesh->faces()) {
            const vec2 coord(select[f] ? 1.0f : 0.0f, 0.5f);
            for (int i = triangle_range[f].first; i <= triangle_range[f].second; ++i)
                texcoords[i * 3] = texcoords[i * 3 + 1] = texcoords[i * 3 + 2] = coord;
        }
        target.drawable->update_texcoord_buffer(texcoords);
    }

    update();
}

void MappleViewer::finish_selection_change(Model *model) {
    if (!model)
        return;
    if (has_any_selection(model))
        refresh_selection_display(model);
    else
        restore_selection_style(model);
    invalidate_selection_count();
}

void MappleViewer::restore_selection_style(Model *model) {
    auto entry = selection_styles_.find(model);
    if (entry == selection_styles_.end())
        return;
    SelectionTarget target;
    if (selection_target(model, target)) {
        static_cast<State &>(*target.drawable) = entry->second.state;
        buffer::update(model, target.drawable);
    }
    selection_styles_.erase(entry);
    invalidate_selection_count();
    update();
}

void MappleViewer::forget_model(Model *model) {
    if (!model)
        return;
    if (hover_model_ == model)
        clear_hover();
    selection_styles_.erase(model);
    if (counted_model_ == model) {
        counted_model_ = nullptr;
        selection_count_dirty_ = true;
    }
}

//------------------------------------------------------------------------
// Hover readout
//------------------------------------------------------------------------

void MappleViewer::update_hover(int x, int y) {
    hover_stamp_ = std::chrono::steady_clock::now();
    if (!show_pick_readout_ || !point_in_scene(x, y) || selecting_) {
        clear_hover();
        return;
    }

    const PickResult result = pick(x, y);
    if (!result || !result.drawable) {
        clear_hover();
        return;
    }

    bool changed = false;
    const std::shared_ptr<Drawable> previous = hover_drawable_.lock();
    // The highlight is a picking aid of the Click (pick) tool. Other modes still
    // report the primitive in the status bar but must not tint the scene.
    const bool highlight = select_mode_ == SelectMode::CLICK;
    if (!highlight) {
        if (previous) {
            previous->set_highlight(false);
            previous->set_highlight_range({-1, -1});
            changed = true;
        }
        hover_drawable_.reset();
        hover_range_ = {-1, -1};
    } else {
        if (previous != result.drawable) {
            if (previous) {
                previous->set_highlight(false);
                previous->set_highlight_range({-1, -1});
            }
            hover_drawable_ = result.drawable;
            result.drawable->set_highlight(true);
            hover_range_ = {-1, -1}; // the range below belongs to the previous drawable
            changed = true;
        }
        if (result.primitive != UINT32_MAX) {
            const std::pair<int, int> range(static_cast<int>(result.primitive),
                                            static_cast<int>(result.primitive));
            if (hover_range_ != range) {
                result.drawable->set_highlight_range(range);
                hover_range_ = range;
                changed = true;
            }
        } else if (hover_range_ != std::make_pair(-1, -1)) {
            // Same drawable, but this hit has no primitive index to highlight.
            result.drawable->set_highlight_range({-1, -1});
            hover_range_ = {-1, -1};
            changed = true;
        }
    }

    hover_model_ = result.model.get();
    const std::string name = result.model ? file_system::simple_name(result.model->name()) : "drawable";
    char buffer[256];
    if (result.drawable->type() == Drawable::DT_TRIANGLES && result.face_index >= 0)
        std::snprintf(buffer, sizeof(buffer), "%s | face %d | (%.4f, %.4f, %.4f)", name.c_str(),
                      result.face_index, result.position.x, result.position.y, result.position.z);
    else if (result.drawable->type() == Drawable::DT_POINTS)
        std::snprintf(buffer, sizeof(buffer), "%s | point %u | (%.4f, %.4f, %.4f)", name.c_str(),
                      result.primitive, result.position.x, result.position.y, result.position.z);
    else
        std::snprintf(buffer, sizeof(buffer), "%s | (%.4f, %.4f, %.4f)", name.c_str(),
                      result.position.x, result.position.y, result.position.z);
    if (hover_text_ != buffer) {
        hover_text_ = buffer;
        changed = true;
    }

    if (changed)
        update();
}

void MappleViewer::clear_hover() {
    bool changed = false;
    if (const std::shared_ptr<Drawable> drawable = hover_drawable_.lock()) {
        drawable->set_highlight(false);
        drawable->set_highlight_range({-1, -1});
        changed = true;
    }
    hover_drawable_.reset();
    hover_model_ = nullptr;
    hover_range_ = {-1, -1};
    if (!hover_text_.empty()) {
        hover_text_.clear();
        changed = true;
    }
    if (changed)
        update();
}

void MappleViewer::append_selection_overlay(std::vector<graphics::DrawCommand> &commands) const {
    if (!selecting_)
        return;
    const unsigned fw = static_cast<unsigned>(framebuffer_width());
    const unsigned fh = static_cast<unsigned>(framebuffer_height());
    if (fw == 0 || fh == 0)
        return;

    // Mouse coordinates are logical window units; the overlay draws in framebuffer
    // pixels, and a one-logical-pixel ribbon is dpi device pixels wide.
    const float dpi = dpi_scaling();
    const vec4 color = setting::highlight_color;

    if (select_mode_ == SelectMode::RECT) {
        // Place the wire on texel centers (see Viewer's zoom-region hint) so the
        // thin outline resolves to a solid color along both shared boundaries.
        const Rect rect((std::min(select_start_.x, select_end_.x) + 0.5f) * dpi,
                        (std::max(select_start_.x, select_end_.x) + 0.5f) * dpi,
                        (std::min(select_start_.y, select_end_.y) + 0.5f) * dpi,
                        (std::max(select_start_.y, select_end_.y) + 0.5f) * dpi);
        commands.push_back(graphics::shape::draw_quad_filled(
                rect, vec4(color.x, color.y, color.z, 0.15f), fw, fh));
        commands.push_back(graphics::shape::draw_quad_wire(rect, color, fw, fh, dpi));
    } else if (select_mode_ == SelectMode::LASSO && lasso_.size() >= 2) {
        std::vector<vec2> segments;
        segments.reserve(lasso_.size() * 2 + 2);
        for (std::size_t i = 1; i < lasso_.size(); ++i) {
            segments.emplace_back(lasso_[i - 1] * dpi);
            segments.emplace_back(lasso_[i] * dpi);
        }
        // Close the loop: the picker treats the points as a polygon, so show the
        // segment that joins the last point back to the first.
        segments.emplace_back(lasso_.back() * dpi);
        segments.emplace_back(lasso_.front() * dpi);
        commands.push_back(graphics::shape::draw_lines_2d(segments, color, fw, fh, dpi));
    }
}
