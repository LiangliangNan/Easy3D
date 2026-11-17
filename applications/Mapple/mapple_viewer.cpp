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
#include "camera_path_builder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/gui/picker.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#endif
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/per_pixel_linked_lists_oit.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>

#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>     // glGetString(GL_VERSION)
#endif

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

    const char *transparency_method_name(int method) {
        switch (method) {
            case 0: return "average color blending";
            case 2: return "per-pixel linked lists";
            default: return "dual depth peeling";
        }
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

    // Restore the layout, view toggles and file history of the previous run.
    settings_.load();
    show_rendering_panel_ = settings_.show_rendering_panel;
    show_models_panel_ = settings_.show_models_panel;
    show_log_panel_ = settings_.show_log_panel;
    left_width_ = settings_.left_width;
    right_width_ = settings_.right_width;
    log_height_ = settings_.log_height;
    show_pick_readout_ = settings_.pick_readout;
    log_auto_scroll_ = settings_.log_auto_scroll;
#ifdef HAS_FFMPEG
    video_bitrate_mbps_ = settings_.video_bitrate_mbps;
#endif

    // The camera path belongs to the app: the base Viewer does not own an interpolator.
    kfi_.reset(new KeyFrameInterpolator(camera()->frame()));
}

MappleViewer::~MappleViewer() {
    // Stop the path playback while the camera frame it drives is still alive.
    if (kfi_)
        kfi_->stop_interpolation();
    if (process_job_) {
        // Cancellation is cooperative; commands without progress checks finish first.
        cancel();
        if (process_job_->thread.joinable())
            process_job_->thread.join();
        delete process_job_->result;
        process_job_.reset();
    }
    // Release selection colormaps before the graphics device dies with the viewer.
    selection_styles_.clear();
    // The glyph drawable would otherwise outlive the scene cleanup (this class holds
    // it too) and free its buffer after the window and device are gone.
    release_camera_markers();
    // The effects own framebuffers and shader programs, so they must go while the
    // graphics device is still alive. Destroying the shadow/AO/EDL objects also
    // unregisters them from the per-camera effect registries.
    transparency_.reset();
    shadow_.reset();
    edl_.reset();
    ao_.reset();
    // Destroy the ImGui context and its font texture while the viewer (and its graphics
    // device) is still alive.
    imgui_.reset();

    // Persist the layout, view toggles and file history for the next run.
    settings_.show_rendering_panel = show_rendering_panel_;
    settings_.show_models_panel = show_models_panel_;
    settings_.show_log_panel = show_log_panel_;
    settings_.left_width = left_width_;
    settings_.right_width = right_width_;
    settings_.log_height = log_height_;
    settings_.pick_readout = show_pick_readout_;
    settings_.log_auto_scroll = log_auto_scroll_;
#ifdef HAS_FFMPEG
    settings_.video_bitrate_mbps = video_bitrate_mbps_;
#endif
    settings_.save();
}

void MappleViewer::notify(std::size_t percent, bool update_viewer) {
    // Worker thread: only publish the value; the progress window reads it per frame.
    if (process_job_)
        process_job_->percent.store(static_cast<int>(percent));
}

Model *MappleViewer::add_model(std::shared_ptr<Model> model, bool create_drawables, bool fit_scene) {
    Model *added = Viewer::add_model(std::move(model), create_drawables, fit_scene);
    if (!added)
        return nullptr;

    // The drawables pick up a model's manipulator transform at draw time; attaching it
    // after the renderer was built is safe. Its frame reports moves so the scene redraws.
    if (!added->manipulator()) {
        auto manipulator = std::make_shared<Manipulator>(added);
        manipulator->frame()->modified.connect([this] { update(); });
        added->set_manipulator(manipulator);
    }
    return added;
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

    // The walking and orbit tools create keyframes on the app-owned camera path.
    camera_path_builder_.reset(new CameraPathBuilder(camera(), kfi_.get()));

    // Recreate the effect that was enabled in the previous session. The effect objects
    // are cheap until used, but they must exist before the first frame draws.
    transparency_method_ = setting::effect_transparency_method;
    if (transparency_method_ < 0 || transparency_method_ > 2 || !transparency_method_supported(transparency_method_)) {
        if (transparency_method_ != 1)
            LOG(WARNING) << "transparency method " << transparency_method_
                         << " is not available; using dual depth peeling";
        transparency_method_ = 1;
    }
    setting::effect_transparency_method = transparency_method_;
    if (setting::effect_ao_enabled)
        set_ao_enabled(true);
    else if (setting::effect_edl_enabled)
        set_edl_enabled(true);
    else if (setting::effect_shadow_enabled)
        set_shadow_enabled(true);
    else if (setting::effect_transparency_enabled)
        set_transparency_enabled(true);

#ifdef EASY3D_HAS_VULKAN
    LOG(INFO) << "Mapple UI ready (Vulkan)";
#else
    LOG(INFO) << "Mapple UI ready (OpenGL)";
#endif
}

void MappleViewer::pre_draw() {
    imgui_->new_frame(dpi_scaling(), pixel_ratio());
    Viewer::pre_draw();
    advance_camera_path();
    update_camera_markers();
    draw_gui();

    // A video frame renders the scene on its own inside capture(), so it runs
    // between the UI build and the overlay submission: the freshly built UI
    // commands are installed afterwards and stay out of the captured frame.
    if (recording_video()) {
        // capture() replays the annotation slot, which is otherwise refreshed by draw();
        // match the path to the camera advanced above.
        update_camera_path_annotations();
        advance_video_recording();
    }

    std::vector<graphics::DrawCommand> commands = imgui_->render(framebuffer_width(), framebuffer_height());
    // The rubber band belongs to the scene layer: insert it before the UI geometry
    // so panels and menus always cover it.
    std::vector<graphics::DrawCommand> hints;
    append_selection_overlay(hints);
    append_path_stroke_overlay(hints);
    append_manipulator_frame(hints);
    if (!hints.empty())
        commands.insert(commands.begin(), hints.begin(), hints.end());
    set_overlays(std::move(commands), true);
}

void MappleViewer::post_resize(int w, int h) {
    Viewer::post_resize(w, h);
    if (imgui_)
        imgui_->set_display_size(w, h);
}

void MappleViewer::draw() const {
    // The annotation slot is replayed by Viewer::draw(), so register the path first.
    update_camera_path_annotations();

    // The effects wrap the same scene render the viewer would perform; the clipping
    // plane guide and the model drawables are all part of Viewer::draw(). The Qt app
    // had the same arrangement with the effect classes' own begin()/end().
    if (!ao_ && !edl_ && !shadow_ && !transparency_) {
        Viewer::draw();
        draw_camera_path();
        return;
    }

    std::vector<TrianglesDrawable *> surfaces;
    for (const auto &model : models()) {
        Renderer *renderer = model->renderer();
        if (!renderer || !renderer->is_visible())
            continue;
        for (const auto &drawable : renderer->triangles_drawables())
            if (drawable->is_visible())
                surfaces.push_back(drawable.get());
    }

    if (shadow_) {
        // The virtual ground follows the viewer's background color, as in Qt.
        shadow_->set_virtual_background_color(background_color());
        shadow_->begin(surfaces);
        Viewer::draw();
        shadow_->end();
    } else if (transparency_) {
        // The transparency pass renders the surfaces itself; with no visible
        // triangles there is nothing to composite, so fall back to the scene.
        if (!surfaces.empty())
            transparency_->draw(surfaces);
        else
            Viewer::draw();
    } else if (ao_) {
        ao_->begin(models());
        Viewer::draw();
        ao_->end();
    } else if (edl_) {
        edl_->begin();
        Viewer::draw();
        edl_->end();
    }

    draw_camera_path();
}

void MappleViewer::update_camera_path_annotations() const {
#ifdef EASY3D_HAS_VULKAN
    std::vector<graphics::DrawCommand> commands;
    if (show_camera_path_ && kfi_ && !kfi_->is_interpolation_started()) {
        if (!camera_path_renderer_)
            camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
        auto path = camera_path_renderer_->draw_path(*kfi_, *camera());
        if (path.geometry)
            commands.push_back(path);
    }
    set_viewer_annotations(std::move(commands));
#endif
}

void MappleViewer::draw_camera_path() const {
#ifndef EASY3D_HAS_VULKAN
    // Shown only when it is not animating; on Vulkan this runs through the annotation slot.
    if (show_camera_path_ && kfi_ && !kfi_->is_interpolation_started())
        kfi_->draw_path(camera(), 2.0f);
#endif
}

void MappleViewer::update_camera_markers() {
    // The glyphs are ordinary scene triangles, so they occlude behind the surfaces and
    // the pivot gesture resolves them like any other visible geometry.
    const float width = camera_marker_width();
    const bool show = show_camera_path_ && kfi_ && !kfi_->is_interpolation_started() && width > 0 &&
                      kfi_->number_of_keyframes() > 0;
    if (!show) {
        if (camera_markers_)
            camera_markers_->set_visible(false);
        return;
    }

    const std::size_t count = kfi_->number_of_keyframes();
    bool same = camera_markers_ && camera_markers_width_ == width && camera_markers_frames_.size() == count;
    for (std::size_t i = 0; same && i < count; ++i) {
        const Frame frame = kfi_->keyframe(i);
        same = camera_markers_frames_[i].position() == frame.position();
        for (unsigned j = 0; same && j < 4; ++j)
            same = camera_markers_frames_[i].orientation()[j] == frame.orientation()[j];
    }
    if (same) {
        camera_markers_->set_visible(true);
        return;
    }

    std::vector<vec3> glyph_points;
    std::vector<unsigned int> glyph_indices;
    shape::create_camera(glyph_points, glyph_indices, width, camera()->field_of_view(),
                         float(camera()->screen_height()) / float(camera()->screen_width()));
    camera_markers_faces_per_glyph_ = glyph_indices.size() / 3;
    std::vector<vec3> points;
    std::vector<unsigned int> indices;
    points.reserve(glyph_points.size() * count);
    indices.reserve(glyph_indices.size() * count);
    camera_markers_frames_.clear();
    for (std::size_t i = 0; i < count; ++i) {
        const Frame frame = kfi_->keyframe(i);
        camera_markers_frames_.push_back(frame);
        const mat4 pose = frame.matrix();
        const auto offset = static_cast<unsigned int>(points.size());
        for (const auto &p : glyph_points)
            points.push_back(pose * p);
        for (auto index : glyph_indices)
            indices.push_back(offset + index);
    }

    if (!camera_markers_) {
        camera_markers_ = std::make_shared<TrianglesDrawable>("camera path markers");
        camera_markers_->set_uniform_coloring(vec4(0.2f, 0.7f, 0.3f, 1.0f));
        add_drawable(camera_markers_);
    }
    camera_markers_->update_vertex_buffer(points);
    camera_markers_->update_element_buffer(indices);
    camera_markers_->set_visible(true);
    camera_markers_width_ = width;
}

void MappleViewer::release_camera_markers() {
    if (!camera_markers_)
        return;
    delete_drawable(camera_markers_.get());
    camera_markers_.reset();
    camera_markers_frames_.clear();
    camera_markers_width_ = 0;
    camera_markers_faces_per_glyph_ = 0;
}

bool MappleViewer::begin_keyframe_drag(int x, int y, bool upright_plane) {
    if (!kfi_ || camera_markers_faces_per_glyph_ == 0 || !show_camera_path_ ||
        kfi_->is_interpolation_started())
        return false;
    const PickResult hit = pick(x, y);
    if (!hit || hit.drawable != camera_markers_ || hit.primitive == UINT32_MAX)
        return false;
    const int index = static_cast<int>(hit.primitive / camera_markers_faces_per_glyph_);
    if (index < 0 || index >= static_cast<int>(kfi_->number_of_keyframes()))
        return false;

    dragged_keyframe_ = index;
    camera_keyframe_index_ = index;
    keyframe_drag_start_position_ = kfi_->keyframe_position(static_cast<std::size_t>(index));
    keyframe_drag_plane_point_ = keyframe_drag_start_position_;
    if (upright_plane) {
        // The pose's own up: the terrain plane for a walking pose, the orbit plane
        // for a turntable pose.
        keyframe_drag_plane_normal_ = kfi_->keyframe(index).orientation().rotate(vec3(0.0f, 1.0f, 0.0f));
        if (keyframe_drag_plane_normal_.length2() < epsilon_sqr<float>()) {
            cancel_keyframe_drag();
            return false;
        }
        keyframe_drag_plane_normal_.normalize();
    } else // the screen plane through the pose: the drag follows the cursor
        keyframe_drag_plane_normal_ = camera()->view_direction();
    keyframe_drag_moved_ = false;
    clear_hover();
    update();
    return true;
}

void MappleViewer::drag_keyframe(int x, int y) {
    if (dragged_keyframe_ < 0 || dragged_keyframe_ >= static_cast<int>(kfi_->number_of_keyframes()))
        return;
    vec3 orig, dir;
    camera()->convert_click_to_line(x, y, orig, dir);
    const float denom = dot(dir, keyframe_drag_plane_normal_);
    // The plane is seen edge-on (the pose's up points at the camera): no stable
    // intersection, so keep the last position.
    if (std::abs(denom) < 1e-5f)
        return;
    const float t = dot(keyframe_drag_plane_point_ - orig, keyframe_drag_plane_normal_) / denom;
    if (t <= 0.0f)
        return;
    const vec3 position = orig + dir * t;

    if (!keyframe_drag_moved_) {
        // The gesture becomes an edit - and the undo point - only once it moves.
        snapshot_camera_path();
        keyframe_drag_moved_ = true;
    }
    kfi_->set_keyframe_position(static_cast<std::size_t>(dragged_keyframe_), position);
    // Keep the moved pose inside the view frustum, as adding a keyframe does.
    const float dist = distance(camera()->scene_center(), position);
    if (dist > camera()->scene_radius())
        camera()->set_scene_radius(dist);
}

void MappleViewer::end_keyframe_drag() {
    if (dragged_keyframe_ < 0)
        return;
    if (keyframe_drag_moved_)
        LOG(INFO) << "keyframe " << dragged_keyframe_ + 1 << " moved (Ctrl+Z restores it)";
    dragged_keyframe_ = -1;
    keyframe_drag_moved_ = false;
    update();
}

void MappleViewer::cancel_keyframe_drag() {
    if (dragged_keyframe_ < 0)
        return;
    if (keyframe_drag_moved_ && kfi_ &&
        dragged_keyframe_ < static_cast<int>(kfi_->number_of_keyframes()))
        kfi_->set_keyframe_position(static_cast<std::size_t>(dragged_keyframe_),
                                    keyframe_drag_start_position_);
    dragged_keyframe_ = -1;
    keyframe_drag_moved_ = false;
    update();
}

void MappleViewer::advance_camera_path() {
    if (!kfi_ || !kfi_->is_interpolation_started())
        return;
    const auto now = std::chrono::steady_clock::now();
    if (now < next_camera_path_frame_)
        return;
    kfi_->advance_interpolation();
    next_camera_path_frame_ = now + std::chrono::milliseconds(kfi_->interpolation_period());
}

void MappleViewer::set_ao_enabled(bool enabled) {
    const bool was = ao_ != nullptr;
    if (enabled) {
        set_edl_enabled(false);
        set_shadow_enabled(false);
        set_transparency_enabled(false);
    }
    if (enabled && !ao_) {
        ao_.reset(new AmbientOcclusion(camera()));
        apply_effect_parameters();
    } else if (!enabled)
        ao_.reset();
    setting::effect_ao_enabled = enabled;
    if (was != enabled)
        LOG(INFO) << "ambient occlusion " << (enabled ? "enabled" : "disabled");
    update();
}

void MappleViewer::set_edl_enabled(bool enabled) {
    const bool was = edl_ != nullptr;
    if (enabled) {
        set_ao_enabled(false);
        set_shadow_enabled(false);
        set_transparency_enabled(false);
    }
    if (enabled && !edl_)
        edl_.reset(new EyeDomeLighting(camera()));
    else if (!enabled)
        edl_.reset();
    setting::effect_edl_enabled = enabled;
    if (was != enabled)
        LOG(INFO) << "eye dome lighting " << (enabled ? "enabled" : "disabled");
    update();
}

void MappleViewer::set_shadow_enabled(bool enabled) {
    const bool was = shadow_ != nullptr;
    if (enabled) {
        set_ao_enabled(false);
        set_edl_enabled(false);
        set_transparency_enabled(false);
    }
    if (enabled && !shadow_) {
        shadow_.reset(new SoftShadow(camera()));
        apply_effect_parameters();
    } else if (!enabled)
        shadow_.reset();
    setting::effect_shadow_enabled = enabled;
    if (was != enabled)
        LOG(INFO) << "soft shadow " << (enabled ? "enabled" : "disabled");
    update();
}

void MappleViewer::set_transparency_enabled(bool enabled) {
    const bool was = transparency_ != nullptr;
    if (enabled) {
        set_ao_enabled(false);
        set_edl_enabled(false);
        set_shadow_enabled(false);
    }
    if (enabled && !transparency_)
        create_transparency();
    else if (!enabled)
        transparency_.reset();
    setting::effect_transparency_enabled = enabled;
    if (was != enabled)
        LOG(INFO) << "transparency " << (enabled ? "enabled" : "disabled");
    update();
}

bool MappleViewer::transparency_method_supported(int method) {
    if (method < 0 || method > 2)
        return false;
    if (method != 2)
        return true;
#ifdef EASY3D_HAS_VULKAN
    return true;
#else
    // The linked lists need SSBOs and atomics, i.e. OpenGL 4.3+; macOS caps at 4.1.
    const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    int major = 0, minor = 0;
    return version && std::sscanf(version, "%d.%d", &major, &minor) == 2 &&
           (major > 4 || (major == 4 && minor >= 3));
#endif
}

void MappleViewer::create_transparency() {
    switch (transparency_method_) {
        case 0:
            transparency_.reset(new AverageColorBlending(camera()));
            break;
        case 2: {
            auto *oit = new PerPixelLinkedListsOIT(camera());
            oit->set_max_fragments_per_pixel(setting::effect_transparency_fragments);
            transparency_.reset(oit);
            break;
        }
        default:
            transparency_method_ = 1;
            {
                auto *peeling = new DualDepthPeeling(camera());
                peeling->set_max_peels(setting::effect_transparency_peels);
                transparency_.reset(peeling);
            }
            break;
    }
}

void MappleViewer::set_transparency_method(int method) {
    if (method < 0 || method > 2)
        return;
    if (!transparency_method_supported(method)) {
        LOG(WARNING) << "transparency method \"" << transparency_method_name(method)
                     << "\" is not available on this backend";
        return;
    }
    const bool changed = method != transparency_method_;
    transparency_method_ = method;
    setting::effect_transparency_method = method;
    if (changed && transparency_) {
        transparency_.reset();
        create_transparency();
        LOG(INFO) << "transparency method: " << transparency_method_name(method);
        update();
    }
}

void MappleViewer::apply_effect_parameters() {
    if (ao_) {
        ao_->set_radius_ratio(setting::effect_ao_radius_ratio);
        ao_->set_intensity(setting::effect_ao_intensity);
        ao_->set_bias(setting::effect_ao_bias);
    }
    if (shadow_) {
        shadow_->set_sample_pattern(
                static_cast<SoftShadow::SamplePattern>(setting::effect_shadow_pattern));
        shadow_->set_light_distance(setting::effect_shadow_light_distance);
        shadow_->set_softness(setting::effect_shadow_softness);
        shadow_->set_darkness(setting::effect_shadow_darkness);
    }
    if (auto *peeling = dynamic_cast<DualDepthPeeling *>(transparency_.get())) {
        if (peeling->max_peels() != setting::effect_transparency_peels)
            peeling->set_max_peels(setting::effect_transparency_peels);
    } else if (auto *oit = dynamic_cast<PerPixelLinkedListsOIT *>(transparency_.get()))
        oit->set_max_fragments_per_pixel(setting::effect_transparency_fragments);
    update();
}

bool MappleViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    mouse_captured_ = imgui_ && imgui_->wants_mouse();
    if (mouse_captured_)
        return true;
    // The processing, snapshot and recording windows are modal: clicks outside
    // them must not reach the scene.
    if (process_op_ != ProcessOp::NONE || show_snapshot_dialog_ || recording_video())
        return true;

    // The armed one-shot path tool owns the next plain left press in the scene: a
    // walk drag starts its route stroke there, and an orbit drag starts its axis.
    // Alt still belongs to model manipulation and the clipping plane.
    const CameraPathBuilder::Mode tool =
            camera_path_builder_ ? camera_path_builder_->mode() : CameraPathBuilder::Mode::Off;
    if (tool != CameraPathBuilder::Mode::Off && button == BUTTON_LEFT &&
        !(modifiers & MODIF_ALT) && point_in_scene(x, y)) {
        if (tool == CameraPathBuilder::Mode::GroundWalk) {
            if (camera_path_builder_->scene_box().is_valid()) {
                // The route follows the visible surface under the cursor, so the
                // terrain supplies each stroke point's contact point and the
                // walker's up direction (the surface normal) there; no ground plane
                // is assumed.
                vec3 point, up;
                if (route_point_at(x, y, point, up)) {
                    tracing_route_ = true;
                    route_stroke_screen_.assign(1, vec2(static_cast<float>(x), static_cast<float>(y)));
                    route_segment_.assign(1, RouteSample{point, up});
                    clear_hover();
                    update();
                } else
                    LOG(WARNING) << "the route has to start on a visible model surface";
            } else
                LOG(WARNING) << "the walking route needs a visible model";
        } else {
            orbit_dragging_ = true;
            orbit_drag_from_ = vec2(static_cast<float>(x), static_cast<float>(y));
            orbit_drag_to_ = orbit_drag_from_;
            clear_hover();
            update();
        }
        return true;
    }

    // Keyframe handles own the press when it lands on a visible glyph; a press
    // anywhere else keeps its usual meaning. Shift moves in the pose's own plane
    // (the terrain or orbit plane) instead of the screen plane.
    if (edit_keyframes_ && button == BUTTON_LEFT && !(modifiers & MODIF_ALT) &&
        point_in_scene(x, y) &&
        begin_keyframe_drag(x, y, (modifiers & MODIF_SHIFT) != 0))
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

    // Alt + left-drag rotates the current model, Alt + right-drag moves it. While the
    // clipping plane is enabled Alt belongs to the plane (the Viewer handles it).
    if ((modifiers & MODIF_ALT) && !ClippingPlane::instance()->is_enabled() &&
        (button == BUTTON_LEFT || button == BUTTON_RIGHT) && point_in_scene(x, y)) {
        Model *model = current_model();
        if (model && model->manipulator()) {
            manipulating_model_ = true;
            model->manipulator()->frame()->action_start();
            clear_hover();
            update();
        }
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
    if (process_op_ != ProcessOp::NONE || show_snapshot_dialog_ || recording_video())
        return true;

    if (tracing_route_ && button == BUTTON_LEFT) {
        tracing_route_ = false;
        commit_ground_route();
        route_stroke_screen_.clear();
        update();
        return true;
    }

    if (orbit_dragging_ && button == BUTTON_LEFT) {
        orbit_dragging_ = false;
        apply_orbit_gesture(orbit_drag_from_, vec2(static_cast<float>(x), static_cast<float>(y)));
        update();
        return true;
    }

    if (dragged_keyframe_ >= 0 && button == BUTTON_LEFT) {
        end_keyframe_drag();
        return true;
    }

    if (manipulating_model_) {
        manipulating_model_ = false;
        Model *model = current_model();
        if (model && model->manipulator())
            model->manipulator()->frame()->action_end();
        update();
    }

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
    if (mouse_captured_ || process_op_ != ProcessOp::NONE || show_snapshot_dialog_ || recording_video())
        return true;

    if (tracing_route_) {
        const vec2 sample(static_cast<float>(x), static_cast<float>(y));
        const vec2 delta = sample - route_stroke_screen_.back();
        // One GPU pick per ~4 logical pixels keeps the surface sampling smooth
        // without re-rendering the scene for every drag event.
        if (delta.length2() >= 16.0f) {
            route_stroke_screen_.push_back(sample);
            vec3 point, up;
            if (route_point_at(x, y, point, up)) {
                route_segment_.push_back(RouteSample{point, up});
            } else {
                // Leaving the visible surface ends the current segment; the stroke
                // continues as a new route when the drag comes back on it, instead
                // of cutting straight through the gap.
                if (route_segment_.size() >= 2)
                    route_segments_.push_back(route_segment_);
                route_segment_.clear();
            }
        }
        update();
        return true;
    }

    if (orbit_dragging_) {
        orbit_drag_to_ = vec2(static_cast<float>(x), static_cast<float>(y));
        update();
        return true;
    }

    if (dragged_keyframe_ >= 0) {
        drag_keyframe(x, y);
        update();
        return true;
    }

    if (manipulating_model_) {
        Model *model = current_model();
        if (model && model->manipulator()) {
            auto *frame = model->manipulator()->frame();
            const ivec2 origin = interaction_origin();
            const int cx = x - origin.x, cy = y - origin.y;
            if (button == BUTTON_LEFT)
                frame->action_rotate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
            else if (button == BUTTON_RIGHT)
                frame->action_translate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
        }
        update();
        return true;
    }

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
    if (process_op_ != ProcessOp::NONE || show_snapshot_dialog_ || recording_video()) {
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
           show_snapshot_dialog_ || recording_video() || Viewer::mouse_scroll_event(x, y, dx, dy);
}

bool MappleViewer::key_press_event(int key, int modifiers) {
    if (imgui_ && imgui_->wants_keyboard())
        return true;

    if (recording_video()) {
        // The progress window owns the keyboard; Esc asks the recording to stop.
        if (key == KEY_ESCAPE && modifiers == 0)
            cancel_video_recording();
        return true;
    }
    if (show_snapshot_dialog_) {
        if (key == KEY_ESCAPE && modifiers == 0)
            show_snapshot_dialog_ = false;
        return true;
    }
    if (process_op_ != ProcessOp::NONE) {
        // A background command owns the slot until it finishes; Esc only asks a
        // cancelable command to stop. Otherwise Esc dismisses the parameter window.
        if (key == KEY_ESCAPE && modifiers == 0) {
            if (process_job_)
                request_process_cancel();
            else
                process_op_ = ProcessOp::NONE;
        }
        return true;
    }
    // The menu advertises 'S' for Snapshot..., so route it to the dialog instead of
    // letting Viewer take its own one-shot snapshot.
    if (key == KEY_S && modifiers == 0) {
        open_snapshot_dialog();
        return true;
    }
    // An armed path tool takes the first Esc; selection mode keeps its own.
    if (key == KEY_ESCAPE && modifiers == 0 && camera_path_builder_ &&
        camera_path_builder_->mode() != CameraPathBuilder::Mode::Off) {
        cancel_camera_path_tool();
        return true;
    }
    // A keyframe drag owns Esc while it runs: restore the pose instead of closing.
    if (key == KEY_ESCAPE && modifiers == 0 && dragged_keyframe_ >= 0) {
        cancel_keyframe_drag();
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
    // Camera-path shortcuts; the base Viewer no longer owns an interpolator or these keys.
    if (modifiers == MODIF_ALT && key == KEY_K) {
        add_camera_keyframe();
        return true;
    }
    if (modifiers == MODIF_CTRL && key == KEY_Z) {
        undo_camera_path();
        return true;
    }
    if (modifiers == MODIF_ALT && key == KEY_D) {
        clear_camera_path();
        return true;
    }
    if (modifiers == MODIF_CTRL && key == KEY_K) {
        toggle_camera_path_playback();
        return true;
    }
    if (modifiers == MODIF_CTRL && key == KEY_T) {
        set_camera_path_visible(!show_camera_path_);
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
        cancel_path_gesture();
        if (manipulating_model_) {
            manipulating_model_ = false;
            Model *model = current_model();
            if (model && model->manipulator())
                model->manipulator()->frame()->action_end();
        }
        cancel_selection_gesture();
        pending_pick_ = pending_region_ = pending_hover_ = false;
        clear_hover();
    }
    return Viewer::focus_event(focused);
}

void MappleViewer::draw_gui() {
    if (process_job_)
        poll_process_job();

    // A recording owns the scene for the same reason a background job does: nothing
    // may change the model, the appearance or the camera while frames are written.
    const bool recording = recording_video();
    if (!recording)
        process_pending_work();

    // The UI stays laid out in both cases but inert. Snapshot and video capture drop
    // the UI overlays before rendering their frame, so none of it reaches the output.
    const bool busy = recording || process_job_ != nullptr;
    if (busy)
        ImGui::BeginDisabled();
    draw_menu_bar();
    if (busy)
        ImGui::EndDisabled();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    const float x = viewport->Pos.x;
    const float y = viewport->Pos.y + menu_height_;
    const float w = viewport->Size.x;
    const float h = std::max(0.0f, viewport->Size.y - menu_height_);

    const float toolbar_height = ImGui::GetFrameHeight() + 8.0f;
    const float status_height = ImGui::GetTextLineHeight() + 8.0f;

    if (busy)
        ImGui::BeginDisabled();
    draw_toolbar(x, y, w, toolbar_height);
    if (busy)
        ImGui::EndDisabled();

    if (recording) {
        draw_panels(x, y + toolbar_height, w, std::max(0.0f, h - toolbar_height - status_height));
        draw_record_progress();
        return;
    }

    if (process_job_) {
        // The panels keep their place so the UI does not jump; the model ones are
        // drawn disabled by draw_panels() because the worker owns the model. The
        // status bar stays out: it reads counts the worker is changing.
        draw_panels(x, y + toolbar_height, w, std::max(0.0f, h - toolbar_height - status_height));
        draw_process_progress();
        return;
    }

    draw_status_bar(x, y + h - status_height, w, status_height);
    draw_panels(x, y + toolbar_height, w, std::max(0.0f, h - toolbar_height - status_height));

    draw_about_window();
    draw_process_dialog();
    draw_snapshot_dialog();
    // The camera-path and property windows are non-modal, so they stay out while a
    // modal dialog owns the screen; their state is untouched.
    if (process_op_ == ProcessOp::NONE && !show_snapshot_dialog_) {
        draw_properties_window();
        draw_animation_window();
    }
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
    scalar_styles_.erase(model);
    highlight_ranges_.erase(model);
    texture_styles_.erase(model);
    vector_fields_.erase(model);
    vector_field_scales_.erase(model);
    if (counted_model_ == model) {
        counted_model_ = nullptr;
        selection_count_dirty_ = true;
    }
}

//------------------------------------------------------------------------
// Hover readout
//------------------------------------------------------------------------

bool MappleViewer::has_configured_highlight(Model *model, const Drawable *drawable) const {
    if (!model || !drawable)
        return false;
    auto entry = highlight_ranges_.find(model);
    if (entry == highlight_ranges_.end())
        return false;
    auto range = entry->second.find(drawable->name());
    return range != entry->second.end() && range->second.enabled;
}

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
    // report the primitive in the status bar but must not tint the scene. A range
    // configured in the Rendering panel owns the drawable's highlight.
    const bool highlight = select_mode_ == SelectMode::CLICK &&
                           !has_configured_highlight(result.model.get(), result.drawable.get());
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

void MappleViewer::append_path_stroke_overlay(std::vector<graphics::DrawCommand> &commands) const {
    const bool stroke = tracing_route_ && route_stroke_screen_.size() >= 2;
    const int selected = edit_keyframes_ && kfi_ && show_camera_path_ &&
                                 !kfi_->is_interpolation_started() && camera_keyframe_index_ >= 0 &&
                                 camera_keyframe_index_ < static_cast<int>(kfi_->number_of_keyframes())
                         ? camera_keyframe_index_
                         : -1;
    if (!stroke && !orbit_dragging_ && selected < 0)
        return;
    const unsigned fw = static_cast<unsigned>(framebuffer_width());
    const unsigned fh = static_cast<unsigned>(framebuffer_height());
    if (fw == 0 || fh == 0)
        return;

    // Same coordinate contract as the selection overlay: logical units in, a
    // one-logical-pixel ribbon in framebuffer pixels out.
    const float dpi = dpi_scaling();
    if (stroke) {
        std::vector<vec2> segments;
        segments.reserve(route_stroke_screen_.size() * 2);
        for (std::size_t i = 1; i < route_stroke_screen_.size(); ++i) {
            segments.emplace_back(route_stroke_screen_[i - 1] * dpi);
            segments.emplace_back(route_stroke_screen_[i] * dpi);
        }
        commands.push_back(graphics::shape::draw_lines_2d(segments, setting::highlight_color, fw, fh, dpi));
    }
    if (orbit_dragging_) {
        // The axis projects onto the line through the press point along the dragged
        // segment, so drawing that line across the view previews the orbit axis; a
        // click (no drag) previews the turntable default, the view's up axis.
        vec2 dir = orbit_drag_to_ - orbit_drag_from_;
        if (dir.length2() < 4.0f)
            dir = vec2(0.0f, 1.0f);
        dir.normalize();
        const float reach = std::max(static_cast<float>(fw), static_cast<float>(fh));
        std::vector<vec2> segments(2);
        segments[0] = orbit_drag_from_ * dpi - dir * reach;
        segments[1] = orbit_drag_from_ * dpi + dir * reach;
        commands.push_back(graphics::shape::draw_lines_2d(segments, setting::highlight_color, fw, fh, dpi));
    }
    if (selected >= 0) {
        // The handle Insert/Delete act on: a small square around the pose's projected
        // position (the glyph itself is the scene geometry being edited).
        const vec3 projected =
                camera()->projected_coordinates_of(kfi_->keyframe_position(static_cast<std::size_t>(selected)));
        if (projected.z >= 0.0f && projected.z <= 1.0f) {
            const float r = 7.0f; // logical pixels
            const vec2 corners[4] = {vec2(projected.x - r, projected.y - r),
                                     vec2(projected.x + r, projected.y - r),
                                     vec2(projected.x + r, projected.y + r),
                                     vec2(projected.x - r, projected.y + r)};
            std::vector<vec2> segments;
            segments.reserve(8);
            for (int i = 0; i < 4; ++i) {
                segments.push_back(corners[i] * dpi);
                segments.push_back(corners[(i + 1) % 4] * dpi);
            }
            commands.push_back(graphics::shape::draw_lines_2d(segments, setting::highlight_color, fw, fh, dpi));
        }
    }
}

void MappleViewer::append_manipulator_frame(std::vector<graphics::DrawCommand> &commands) const {
    Model *model = current_model();
    auto *manipulator = model ? model->manipulator() : nullptr;
    if (!manipulator)
        return;
    // Alt announces the manipulation gesture; a transformed model also keeps its box
    // visible so the pending transform (apply or give up) stays apparent.
    if (!ImGui::GetIO().KeyAlt && manipulator->matrix() == mat4::identity())
        return;
    const int fw = framebuffer_width();
    const int fh = framebuffer_height();
    if (fw <= 0 || fh <= 0)
        return;
    graphics::DrawCommand command = manipulator->draw_frame(camera(), fw, fh, dpi_scaling());
    if (command.geometry)
        commands.push_back(command);
}
