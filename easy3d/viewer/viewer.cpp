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

#include <easy3d/viewer/viewer.h>

// Standard headers used by the backend-independent section below.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>


// Headers shared by both backends.
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/resource.h>
#include <easy3d/renderer/text_layout.h>
#ifdef EASY3D_HAS_VULKAN
// vulkan.h must precede glfw3.h (first pulled in by viewer_loop.h): GLFW declares
// glfwCreateWindowSurface only when VK_VERSION_1_0 is defined.
#include <vulkan/vulkan.h>
#endif
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <easy3d/viewer/backends/viewer_loop.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h> // for glfw functions

#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/transparency.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/backends/vulkan/drawable.h>
#include <easy3d/renderer/backends/vulkan/clipping_plane.h>
#include <easy3d/renderer/backends/vulkan/surface.h>
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#else
// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/opengl.h> // for gl functions
#include <easy3d/renderer/backends/opengl/overlay_renderer.h>
#include <easy3d/renderer/backends/opengl/point_selection.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/util/timer.h>
#include <easy3d/util/string.h>
#include <easy3d/core/matrix.h>
#endif


namespace easy3d {
    void Viewer::ensure_text_layout_engine() const {
        if (fonts_tried_)
            return;
        fonts_tried_ = true;
        try {
            text_layout_.reset(new graphics::TextLayoutEngine);
            text_layout_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
            text_layout_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");
        } catch (const std::exception &e) {
            text_layout_.reset();
            LOG(WARNING) << "Viewer HUD font unavailable: " << e.what();
        }
    }

    bool Viewer::update_hud_text(std::vector<graphics::DrawCommand> &commands) const {
        const float dpi = dpi_scaling();
        const bool viewport_changed = hint_dpi_ != dpi || framebuffer_width() != hint_width_ ||
                                      framebuffer_height() != hint_height_ || rendered_show_logo_ != show_easy3d_logo_;
        const bool hint_changed =
            viewport_changed || rendered_hint_ != hint_ || rendered_show_frame_rate_ != show_frame_rate_;
        const std::string frame_rate_text = show_frame_rate_ ? framerate_ : std::string();
        const bool frame_rate_changed = viewport_changed || rendered_frame_rate_ != frame_rate_text;
        const bool changed = hint_changed || frame_rate_changed;

        if (changed) {
            if (show_easy3d_logo_ || !hint_.empty() || !frame_rate_text.empty())
                ensure_text_layout_engine();
            if (viewport_changed) {
                logo_commands_.clear();
                if (text_layout_ && show_easy3d_logo_) {
                    graphics::TextOptions options;
                    options.size = 15 * dpi;
                    options.x = 20 * dpi;
                    options.y = 22.5f * dpi;
                    logo_commands_ =
                        text_layout_->layout("Easy3D", framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_show_logo_ = show_easy3d_logo_;
            }
            if (hint_changed) {
                hint_commands_.clear();
                if (text_layout_ && !hint_.empty()) {
                    graphics::TextOptions options;
                    options.font = 1;
                    options.size = 15 * dpi;
                    options.x = 35 * dpi;
                    options.y = 22.5f * (1 + int(show_frame_rate_) + int(show_easy3d_logo_)) * dpi;
                    options.color = vec4(0, 0, 1, 1);
                    hint_commands_ =
                        text_layout_->layout(hint_, framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_hint_ = hint_;
                rendered_show_frame_rate_ = show_frame_rate_;
            }
            if (frame_rate_changed) {
                frame_rate_commands_.clear();
                if (text_layout_ && !frame_rate_text.empty()) {
                    graphics::TextOptions options;
                    options.font = 1;
                    options.size = 15 * dpi;
                    options.x = 20 * dpi;
                    options.y = 22.5f * (1 + int(show_easy3d_logo_)) * dpi;
                    frame_rate_commands_ =
                        text_layout_->layout(frame_rate_text, framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_frame_rate_ = frame_rate_text;
            }
            hint_width_ = framebuffer_width();
            hint_height_ = framebuffer_height();
            hint_dpi_ = dpi;
        }

        commands = logo_commands_;
        commands.insert(commands.end(), frame_rate_commands_.begin(), frame_rate_commands_.end());
        commands.insert(commands.end(), hint_commands_.begin(), hint_commands_.end());
        return changed;
    }
} // namespace easy3d

#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
    struct Viewer::Impl {
        std::unique_ptr<graphics::RenderDevice> device;
        Viewer &owner;
        explicit Impl(Viewer &viewer) : owner(viewer) {}
        std::vector<graphics::DrawCommand> overlays, viewer_overlays, viewer_3d_overlays, viewer_annotations;
        bool overlays_foreground = false;
        std::vector<vec4> clipping_planes;
        mutable std::vector<vec4> shared_clipping_planes;
        mutable vulkan::ClippingPlaneRenderer clipping_plane_renderer;
        vulkan::DrawableRenderer drawable_renderer;

        // While active, draw() renders the file-snapshot frame into an offscreen
        // image instead of presenting it; snapshot_frame() owns the state.
        struct SnapshotFrame {
            bool active = false;
            int width = 0, height = 0;
            unsigned samples = 1;
            vec4 background{};
            graphics::FrameImage image;
        } snapshot;

        void frame_size(int &width, int &height) const {
            if (snapshot.active) {
                width = snapshot.width;
                height = snapshot.height;
            } else
                owner.framebuffer_size(width, height);
        }

        bool submit(std::vector<graphics::DrawCommand> commands,
                    graphics::TransparencyMethod method = graphics::TransparencyMethod::sorted,
                    unsigned peel_layers = 0) {
            int width = 0, height = 0;
            frame_size(width, height);
            if (width <= 0 || height <= 0)
                return false;
            if (!snapshot.active)
                return device->present(unsigned(width), unsigned(height), commands, owner.background_color(),
                                       unsigned(owner.samples()), method, peel_layers);
            snapshot.image = device->render(unsigned(width), unsigned(height), commands, snapshot.background,
                                            snapshot.samples, method, peel_layers);
            return true;
        }

        const std::vector<vec4> &active_clipping_planes() const {
            const auto &plane = *ClippingPlane::instance();
            if (!plane.is_enabled())
                return clipping_planes;
            shared_clipping_planes = vulkan::ClippingPlaneRenderer::planes(plane);
            return shared_clipping_planes;
        }

        graphics::FrameImage capture(const Camera &cam,
                                     const vec4 &bg,
                                     unsigned samples = 1,
                                     int width = 0,
                                     int height = 0,
                                     bool include_hud = true) const {
            if (!width)
                width = cam.screen_width();
            if (!height)
                height = cam.screen_height();
            return device->render(uint32_t(width), uint32_t(height),
                                  owner.frame_commands(cam, width, height, nullptr, include_hud), bg, samples);
        }
    };
    namespace {
        const char *const default_manual =
            "--------------------------------------------------------------\n"
            " Easy3D Viewer Usage (Shortcuts might be overridden):         \n"
            "--------------------------------------------------------------\n"
            "  F1:                      Show help                                          \n"
            "  F2:                      Toggle animation (if available)                    \n"
            "  Ctrl + 'o':              Open file                                          \n"
            "  Ctrl + 's':              Save file                                          \n"
            "  Fn + Delete:             Delete current model                               \n"
            "  '<' or '>':              Switch between models                              \n"
            "  's':                     Take a snapshot                                    \n"
            "  'p':                     Switch between perspective and orthographic projections\n"
            "  Ctrl + 'f':              Toggle frame rate display                          \n"
            "  'a':                     Toggle axes visibility                             \n"
            "  Alt + 'c':               Toggle clipping plane                              \n"
            "  Alt + 'v':               Toggle clipping plane visibility                   \n"
            "  Alt + 'x':               Toggle cross-section                               \n"
            "  Alt + Left drag:         Rotate the clipping plane                          \n"
            "  Alt + Right drag:        Move the clipping plane along its normal           \n"
            "  Left drag:               Rotate the camera                                  \n"
            "  Right drag:              Move the camera                                    \n"
            "  'x' + Left drag:         Rotate the camera around the horizontal axis       \n"
            "  'x' + Right drag:        Move the camera along the horizontal axis          \n"
            "  'y' + Left drag:         Rotate the camera around the vertical axis         \n"
            "  'y' + Right drag:        Move the camera along the vertical axis            \n"
            "  'o' + Left drag:         Rotate the camera around the ortho-screen axis     \n"
            "  Middle button or Wheel:  Zoom in/out                                        \n"
            "  Ctrl + '+'/'-':          Zoom in/out                                        \n"
            "  Left/Right Arrow:        Turn camera left/right                             \n"
            "  Ctrl + Left/Right Arrow: Move camera left/right                             \n"
            "  Up/Down Arrow:           Move camera forward/backward                       \n"
            "  Ctrl + Up/Down Arrow:    Move camera up/down                                \n"
            "  Ctrl + 'c':              Copy current view status to clipboard              \n"
            "  Ctrl + 'v':              Restore view status from clipboard                 \n"
            "  Alt + 'k':               Add key frame to the camera path                   \n"
            "  Alt + 'd':               Delete the camera path                             \n"
            "  Ctrl + 'k':              Play/pause the camera path                         \n"
            "  Ctrl + 't':              Toggle camera path visibility                      \n"
            "  'f':                     Fit screen (all models)                            \n"
            "  'c':                     Fit screen (current model only)                    \n"
            "  'z' + Left click:        Zoom to target point on model                      \n"
            "  'z' + Right click:       Zoom to fit screen                                 \n"
            "  Shift + Left click:      Define a target point on model                     \n"
            "  Shift + Right click:     Undefine the target point (if any) on model        \n"
            "  Ctrl + Left drag:        Zoom to fit region                                 \n"
            "  '-'/'=':                 Decrease/Increase point size                       \n"
            "  '['/']':                 Decrease/Increase line width                       \n"
            "  'b':                     Toggle borders visibility                          \n"
            "  'e':                     Toggle edges visibility                            \n"
            "  'v':                     Toggle vertices visibility                         \n"
            "  'l':                     Toggle locked vertices visibility                  \n"
            "  'm':                     Toggle smooth shading (for SurfaceMesh)            \n"
            "  'd':                     Print model properties                             \n"
            "  Space:                   Align camera with world axes                       \n"
            "  Escape / Alt + F4:       Close viewer                                       \n"
            "-------------------------------------------------------------------";

    }
    struct Viewer::Window {
        std::shared_ptr<detail::GlfwRuntime> runtime;
        GLFWwindow *handle = nullptr;
        std::string title;
        int width, height;
        unsigned samples;
        Window(const std::string &text, int w, int h, int sample_count, bool full_screen, bool visible, bool resizable)
            : title(text), width(w), height(h), samples(sample_count ? sample_count : 1) { // 0: no MSAA, as in OpenGL
            if (w <= 0 || h <= 0)
                throw std::invalid_argument("window dimensions must be positive");
            if (sample_count < 0 || sample_count > 64 || (sample_count && (sample_count & (sample_count - 1))))
                throw std::invalid_argument("invalid viewer sample count");
            runtime = detail::GlfwRuntime::acquire(true);
            if (!glfwVulkanSupported())
                throw std::runtime_error("GLFW could not find Vulkan loader/driver support");
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
            GLFWmonitor *monitor = nullptr;
            if (full_screen) {
                monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
                if (!mode)
                    throw std::runtime_error("no monitor available for a full-screen window");
                width = mode->width;
                height = mode->height;
            }
#ifndef NDEBUG
            const std::string title_str = title + " - Debug Version";
#else
            const std::string title_str = title;
#endif
            handle = glfwCreateWindow(width, height, title_str.c_str(), monitor, nullptr);
            if (!handle)
                throw std::runtime_error("could not create a GLFW Vulkan window");
        }
        ~Window() {
            if (handle)
                glfwDestroyWindow(handle);
        }
        std::unique_ptr<graphics::RenderDevice> device(const VulkanOptions &options) {
            uint32_t count = 0;
            const char **required = glfwGetRequiredInstanceExtensions(&count);
            if (!required || !count)
                throw std::runtime_error("GLFW has no Vulkan surface extensions for this platform");
            graphics::VulkanSurface surface;
            surface.loader = runtime->vulkan_loader();
            for (uint32_t i = 0; i < count; ++i)
                surface.instance_extensions.emplace_back(required[i]);
            surface.create = [this](VkInstance instance, VkSurfaceKHR *result) {
                return glfwCreateWindowSurface(instance, handle, nullptr, result);
            };
            return graphics::create_vulkan_window_device(surface, options.validation, options.sync_validation,
                                                         options.device_name, options.use_native_large_points,
                                                         options.profiling);
        }
        int modifiers() const {
            return detail::GlfwRuntime::modifiers(handle);
        }
        template <class Function> static void dispatch(GLFWwindow *handle, const Function &call) noexcept {
            auto *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(handle));
            if (!viewer)
                return;
            try {
                call(*viewer);
            } catch (...) {
                viewer->callback_error_ = std::current_exception();
                glfwSetWindowShouldClose(handle, GLFW_TRUE);
            }
        }
        void callbacks(Viewer *viewer) {
            glfwSetWindowUserPointer(handle, viewer);
            glfwSetWindowRefreshCallback(handle, [](GLFWwindow *w) { dispatch(w, [](Viewer &v) { v.update(); }); });
            // Native events use logical window coordinates, identical to OpenGL; the
            // shared ViewerBase routers own all input state and event ordering.
            glfwSetCursorPosCallback(handle, [](GLFWwindow *w, double px, double py) {
                dispatch(w, [&](Viewer &v) {
                    v.handle_cursor_pos(int(px), int(py), detail::GlfwRuntime::modifiers(w));
                });
            });
            glfwSetMouseButtonCallback(handle, [](GLFWwindow *w, int button, int action, int mods) {
                dispatch(w, [&](Viewer &v) {
                    double px, py;
                    glfwGetCursorPos(w, &px, &py);
                    v.handle_mouse_button(int(px), int(py), button, action, mods);
                });
            });
            glfwSetScrollCallback(handle, [](GLFWwindow *w, double dx, double dy) {
                dispatch(w, [&](Viewer &v) {
                    double px, py;
                    glfwGetCursorPos(w, &px, &py);
                    v.handle_scroll(int(px), int(py), dx, dy);
                });
            });
            glfwSetKeyCallback(handle, [](GLFWwindow *w, int key, int, int action, int mods) {
                dispatch(w, [&](Viewer &v) { v.handle_key(key, action, mods); });
            });
            glfwSetCharCallback(handle, [](GLFWwindow *w, unsigned int code) {
                dispatch(w, [&](Viewer &v) { v.handle_character(code); });
            });
            glfwSetDropCallback(handle, [](GLFWwindow *w, int count, const char **files) {
                dispatch(w, [&](Viewer &v) {
                    std::vector<std::string> names;
                    for (int i = 0; i < count; ++i)
                        names.emplace_back(files[i]);
                    v.handle_drop(names);
                });
            });
            glfwSetWindowFocusCallback(handle, [](GLFWwindow *w, int focused) {
                dispatch(w, [&](Viewer &v) { v.handle_focus(focused != 0); });
            });
            glfwSetFramebufferSizeCallback(handle, [](GLFWwindow *w, int width, int height) {
                dispatch(w, [&](Viewer &v) {
                    glfwGetWindowSize(w, &v.width_, &v.height_);
                    if (width > 0 && height > 0) {
                        v.resize_render_target(width, height);
                        v.cancel_interaction();
                    }
                    v.notify_resize();
                });
            });
        }
    };
} // namespace easy3d
#endif


//============================================================================
// Backend-independent implementation.
//
// The frame loop, window/event plumbing, HUD bookkeeping, model/drawable
// management and file IO behave identically on OpenGL and Vulkan. Only the
// few genuinely backend-specific steps (context/device binding, presentation,
// overlay encoding and the pivot mark) are guarded inline below. Rendering,
// picking, capture and window creation live in the backend sections that follow.
//============================================================================
namespace easy3d {
    unsigned Viewer::validation_errors() const {
#ifdef EASY3D_HAS_VULKAN
        return device().validation_errors();
#else
        return 0;
#endif
    }

    void Viewer::poll_events() {
        ViewerBase::poll_events();
    }

    void Viewer::request_close() {
        if (window_)
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        update();
    }

    bool Viewer::should_close() const {
        return window_ && glfwWindowShouldClose(window_) != 0;
    }

    void Viewer::show() {
        if (window_)
            glfwShowWindow(window_);
    }

    void Viewer::hide() {
        if (window_)
            glfwHideWindow(window_);
    }

    void Viewer::exit() {
        request_close();
    }

    void Viewer::update() const {
        glfwPostEmptyEvent();
    }

    void Viewer::resize(int w, int h) {
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        if (window_)
            glfwSetWindowSize(window_, w, h);
    }

    void Viewer::framebuffer_size(int &w, int &h) const {
        if (window_)
            glfwGetFramebufferSize(window_, &w, &h);
        else {
            w = camera()->screen_width();
            h = camera()->screen_height();
        }
    }

    void Viewer::viewer_size(int &w, int &h) const {
        if (window_)
            glfwGetWindowSize(window_, &w, &h);
        else {
            w = width_;
            h = height_;
        }
    }

    void Viewer::notify_resize() {
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        if (w == resize_width_ && h == resize_height_ && fw == resize_fb_width_ && fh == resize_fb_height_)
            return;
        resize_width_ = w;
        resize_height_ = h;
        resize_fb_width_ = fw;
        resize_fb_height_ = fh;
        post_resize(w, h);
    }

    void Viewer::set_position(int x, int y) {
        if (window_)
            glfwSetWindowPos(window_, x, y);
    }

    void Viewer::set_title(const std::string &text) {
        if (text == title_)
            return;
        title_ = text;
        if (window_)
            glfwSetWindowTitle(window_, text.c_str());
#ifdef EASY3D_HAS_VULKAN
        if (window_host_)
            window_host_->title = text;
#endif
    }

    void Viewer::set_animation(bool enabled) {
        animation_enabled_ = enabled;
        update();
    }

    bool Viewer::is_animating() const {
        return animation_enabled_ && bool(animation_func_);
    }

    void Viewer::set_usage(const std::string &manual, const std::string &screen_hint) {
        if (manual != "default") {
            manual_ = manual;
            usage_printed_ = false;
        }
        hint_ = screen_hint;
        update();
    }

    void Viewer::print_usage() {
        if (usage_printed_)
            return;
        if (!manual_.empty()) {
            std::cout << manual_ << std::endl;
            usage_printed_ = true;
        }
    }

    void Viewer::update_frame_rate() {
        if (!show_frame_rate_)
            return;
        ++frame_rate_frames_;
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - frame_rate_start_).count();
        if (elapsed < 1.0)
            return;
        std::ostringstream out;
        out << "fps: " << std::fixed << std::setprecision(0) << frame_rate_frames_ / elapsed << " ("
            << std::setprecision(1) << 1000.0 * elapsed / frame_rate_frames_ << " ms/frame)";
        framerate_ = out.str();
        frame_rate_frames_ = 0;
        frame_rate_start_ = now;
    }

    void Viewer::restore_scene_bounds() {
        Box3 box;
        for (const auto &model : models())
            box.grow(model->bounding_box());
        for (const auto &drawable : drawables())
            if (!dynamic_cast<LinesDrawable2D *>(drawable.get()))
                box.grow(drawable->bounding_box());
        if (box.is_valid())
            camera()->set_scene_bounding_box(box.min_point(), box.max_point());
    }

    bool Viewer::draw_frame() {
#ifdef EASY3D_HAS_VULKAN
        if (!window_)
            return false;
#endif
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        if (should_close() || glfwGetWindowAttrib(window_, GLFW_ICONIFIED))
            return false;
        int w, h;
        framebuffer_size(w, h);
        if (w <= 0 || h <= 0)
            return false;
#ifdef EASY3D_HAS_VULKAN
        if (framebuffer_width_ != w || framebuffer_height_ != h) {
            resize_render_target(w, h);
            cancel_interaction();
            viewer_size(width_, height_);
            notify_resize();
        }
#else
        glfwMakeContextCurrent(window_);
        // Hidden windows may not receive size callbacks (e.g., X11 never reports
        // ConfigureNotify for unmapped windows), so poll like the Vulkan path.
        int lw = 0, lh = 0;
        viewer_size(lw, lh);
        if (lw != width_ || lh != height_ || w != framebuffer_width_ || h != framebuffer_height_)
            callback_event_resize(lw, lh);
#endif
        if (!initialized_) {
            init();
            initialized_ = true;
        }
        if (is_animating()) {
            animation_func_(this);
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            if (should_close())
                return false;
        }
        advance_camera_path();
        print_usage();
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        if (should_close())
            return false;
        pre_draw();
#ifdef EASY3D_HAS_VULKAN
        update_camera_path_overlays();
        update_interaction_overlays();
#endif
        draw();
        update_frame_rate();
        post_draw();
#ifndef EASY3D_HAS_VULKAN
        glfwSwapBuffers(window_);
#endif
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        return true;
    }

    int Viewer::run(bool see_all) {
        try {
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            if (!initialized_) {
                init();
                initialized_ = true;
            }
            if (see_all)
                fit_screen();
            if (visible_)
                show();
            return run_frames();
        } catch (const std::exception &e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    int Viewer::run_frames(unsigned max_frames) {
        try {
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            return detail::run_viewer_frames(*this, max_frames, [this] {
                return is_animating() || show_frame_rate_ || kfi_->is_interpolation_started();
            });
        } catch (const std::exception &e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    void Viewer::advance_camera_path() {
        if (!kfi_->is_interpolation_started())
            return;
        const auto now = std::chrono::steady_clock::now();
        if (now < next_camera_path_frame_)
            return;
        kfi_->advance_interpolation();
        next_camera_path_frame_ = now + std::chrono::milliseconds(kfi_->interpolation_period());
    }

    //------------------------------------------------------------------------
    // Model and drawable management
    //------------------------------------------------------------------------
    Model *Viewer::current_model() const {
        return model_idx_ < 0 || size_t(model_idx_) >= models_.size() ? nullptr : models_[model_idx_].get();
    }

    Model *Viewer::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        if (!model || model->empty())
            return nullptr;
        for (const auto &current : models_)
            if (model == current)
                return current.get();
        // Construct first: unsupported default coloring fails without replacing the model's existing renderer.
        auto renderer = std::make_shared<Renderer>(model.get(), create);
        model->set_renderer(renderer);
        models_.push_back(model);
        model_idx_ = int(models_.size()) - 1;
        if (fit_scene)
            fit_screen();
        return model.get();
    }

    Model *Viewer::add_model(Model *model, bool create, bool fit_scene) {
        if (!model)
            return nullptr;
        // Do not manufacture a second shared_ptr control block for an owned model.
        for (const auto &current : models_)
            if (current.get() == model)
                return model;
        return add_model(std::shared_ptr<Model>(model), create, fit_scene);
    }

    Model *Viewer::add_model(const std::string &file_path, bool create_drawables, bool fit_scene) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        const std::string file_name = file_system::convert_to_native_style(file_path);
        for (const auto &m : models_) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model has already been added to the viewer: " << file_name;
                return m.get();
            }
        }
        const auto ext = file_system::extension(file_name, true);
        std::shared_ptr<Model> model;
        if (ext == "ply" && io::PlyReader::num_instances(file_name, "face") > 0)
            model.reset(SurfaceMeshIO::load(file_name));
        else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0)
            model.reset(GraphIO::load(file_name));
        else if (ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist")
            model.reset(SurfaceMeshIO::load(file_name));
        else if (ext == "plm" || ext == "pm" || ext == "mesh")
            model.reset(PolyMeshIO::load(file_name));
        else if (ext == "ptx") {
            // A PTX file may store several scans; each becomes its own model.
            io::PointCloudIO_ptx serializer(file_name);
            PointCloud *cloud = nullptr;
            Model *last = nullptr;
            while ((cloud = serializer.load_next())) {
                last = add_model(std::shared_ptr<PointCloud>(cloud), create_drawables, fit_scene);
                update();
            }
            return last; // the last cloud in the file
        } else
            model.reset(PointCloudIO::load(file_name));
        if (model) {
            model->set_name(file_name);
            return add_model(model, create_drawables, fit_scene);
        }
        return nullptr;
    }

    bool Viewer::delete_model(Model *model) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        auto &values = models_;
        auto found = std::find_if(values.begin(), values.end(),
                                  [model](const std::shared_ptr<Model> &p) { return p.get() == model; });
        if (found == values.end())
            return false;
        // Shortcuts carry a borrowed model pointer: drop them before the model can be destroyed.
        unbind_shortcut(model);
        values.erase(found);
        model_idx_ = int(models_.size()) - 1;
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
#endif
        return true;
    }

    Drawable *Viewer::add_drawable(std::shared_ptr<Drawable> drawable) {
        if (!drawable)
            return nullptr;
        for (const auto &d : drawables_)
            if (d == drawable)
                return d.get();
        drawables_.push_back(drawable);
        return drawable.get();
    }

    Drawable *Viewer::add_drawable(Drawable *drawable) {
        return add_drawable(std::shared_ptr<Drawable>(drawable));
    }

    bool Viewer::delete_drawable(Drawable *drawable) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        auto &values = drawables_;
        auto found = std::find_if(values.begin(), values.end(),
                                  [drawable](const std::shared_ptr<Drawable> &p) { return p.get() == drawable; });
        if (found == values.end())
            return false;
        values.erase(found);
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
#endif
        return true;
    }

    void Viewer::clear_scene() {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        // Model-bound shortcuts would dangle once the models are gone; model-free actions survive.
        for (const auto& model : models_)
            unbind_shortcut(model.get());
        models_.clear();
        model_idx_ = -1;
        drawables_.clear();
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
        impl_->overlays.clear();
        impl_->viewer_overlays.clear();
        impl_->viewer_3d_overlays.clear();
        impl_->viewer_annotations.clear();
        if (window_) {
            cancel_interaction();
            update_camera_path_overlays();
        }
#else
        if (overlay_renderer_)
            overlay_renderer_->set({}, false);
#endif
    }

    void Viewer::fit_screen(const Model *selected) {
        Box3 box;
        if (selected)
            box.grow(selected->bounding_box());
        else {
            for (const auto &model : models_)
                box.grow(model->bounding_box());
            for (const auto &drawable : drawables_)
                if (!dynamic_cast<LinesDrawable2D *>(drawable.get()))
                    box.grow(drawable->bounding_box());
        }
        if (box.is_valid()) {
            camera()->set_scene_bounding_box(box.min_point(), box.max_point());
            camera()->show_entire_scene();
            ClippingPlane::instance()->fit_scene(camera()->scene_center(), camera()->scene_radius());
        }
    }

    //------------------------------------------------------------------------
    // File IO
    //------------------------------------------------------------------------
    bool Viewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
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
        const std::vector<std::string> &file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto &file_name : file_names) {
            if (add_model(file_name))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }

    bool Viewer::save() const {
        Model *m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {"Surface Mesh (*.obj *.ply *.off *.stl *.sm)",
                                                   "*.obj *.ply *.off *.stl *.sm",
                                                   "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                                                   "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                                                   "Polyhedral Mesh (*.plm *.pm *.mesh)",
                                                   "*.plm *.pm *.mesh",
                                                   "Graph (*.ply)",
                                                   "*.ply",
                                                   "All Files (*.*)",
                                                   "*"};

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply";                       // default to ply

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud *>(m))
            saved = PointCloudIO::save(file_name, dynamic_cast<const PointCloud *>(m));
        else if (dynamic_cast<const SurfaceMesh *>(m))
            saved = SurfaceMeshIO::save(file_name, dynamic_cast<const SurfaceMesh *>(m));
        else if (dynamic_cast<const Graph *>(m))
            saved = GraphIO::save(file_name, dynamic_cast<const Graph *>(m));

        if (saved)
            LOG(INFO) << "file successfully saved";

        return saved;
    }

    bool Viewer::snapshot() const {
        const std::string &title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string> &filters = {"Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                                                   "All Files (*.*)", "*"};

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
            LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            return false;
        }

        return snapshot(file_name, 1.0f, int(samples()));
    }

    void Viewer::copy_view() {
        const auto p = camera()->position();
        const auto q = camera()->orientation();
        std::ostringstream out;
        out << std::setprecision(9) << p.x << ' ' << p.y << ' ' << p.z;
        for (unsigned i = 0; i < 4; ++i)
            out << ' ' << q[i];
        glfwSetClipboardString(window(), out.str().c_str());
    }

    void Viewer::paste_view() {
        const char *text = glfwGetClipboardString(window());
        if (!text)
            return;
        std::istringstream in(text);
        vec3 p;
        quat q;
        for (unsigned i = 0; i < 3; ++i)
            if (!(in >> p[i]) || !std::isfinite(p[i]))
                return;
        float norm = 0;
        for (unsigned i = 0; i < 4; ++i) {
            if (!(in >> q[i]) || !std::isfinite(q[i]))
                return;
            norm += q[i] * q[i];
        }
        std::string extra;
        if ((in >> extra) || !std::isfinite(norm) || norm < 1e-12f)
            return;
        q.normalize();
        // I prefer to animate the view change rather than snap to it.
        Frame new_frame;
        new_frame.set_position(p);
        new_frame.set_orientation(q);
        const float duration = 0.5f;
        camera()->interpolate_to(&new_frame, duration);
        update();
    }

    //------------------------------------------------------------------------
    // Event handlers (logical window coordinates on both backends)
    //------------------------------------------------------------------------
    bool Viewer::mouse_free_move_event(int, int, int, int, int) {
        // highlight geometry primitives here
        return false;
    }

    bool Viewer::char_input_event(unsigned int) {
        //switch (codepoint) {
        //case '-':	break;
        //case 'c':	break;
        //case 'C':	break;
        //case '0':	break;
        //default:
        //	return false;
        //}
        return false;
    }

    bool Viewer::focus_event(bool) {
        return false;
    }

    void Viewer::end_gesture() {
        zoom_region_ = false;
        camera()->frame()->action_end();
        ClippingPlane::instance()->manipulator()->frame()->action_end();
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
    }

    void Viewer::cancel_interaction() {
        input_.reset();
        show_pivot_point_ = false;
        end_gesture();
    }

    bool Viewer::mouse_scroll_event(int, int, int, int dy) {
        camera()->frame()->action_zoom(dy, camera());
        return false;
    }

    bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
        // The router already recorded these; assigning again keeps direct calls (for
        // example from tests or subclasses) consistent.
        input_.press_x = input_.x = x;
        input_.press_y = input_.y = y;
        input_.button = button;
        input_.press_modifiers = modifiers;
        zoom_region_ = button == BUTTON_LEFT && modifiers == MODIF_CTRL;
        camera()->frame()->action_start();
        if (modifiers == MODIF_ALT && ClippingPlane::instance()->is_enabled())
            ClippingPlane::instance()->manipulator()->frame()->action_start();
        if (input_.key == KEY_Z || modifiers == MODIF_SHIFT) {
            if (button == BUTTON_LEFT) {
                bool found = false;
                const vec3 point = point_under_pixel(x, y, found);
                camera()->set_pivot_point(found ? point : camera()->scene_center());
                show_pivot_point_ = found;
                if (found) {
#ifdef EASY3D_HAS_VULKAN
                    // pivot_pixel_ is logical; the overlay expires after ten seconds.
                    pivot_pixel_ = vec2(float(x), float(y));
                    pivot_expires_ = std::chrono::steady_clock::now() + std::chrono::seconds(10);
#else
                    // We draw the pivot mark in screen space (a 3D version is in
                    // Tutorial_204_Viewer_Qt) and hide the hint after a delay.
                    if (!drawable_pivot_)
                        drawable_pivot_ = std::unique_ptr<LinesDrawable2D>(new LinesDrawable2D("pivot"));
                    // The pivot cross is ten logical pixels long and one logical pixel wide;
                    // both backends draw it in framebuffer pixels, so the arm is 10 * dpi and
                    // the ribbon width is dpi (one logical pixel; a fixed width of one would be
                    // a thinner device-pixel hairline on HiDPI). Screen-space lines are
                    // corner-based (texel (i, j) spans [i, i+1) x [j, j+1)), so center the
                    // ribbon on the marked pixel's center instead of its corner.
                    const float dpi = dpi_scaling();
                    const float px = (x + 0.5f) * dpi, py = (y + 0.5f) * dpi, size = 10.0f * dpi;
                    const std::vector<vec2> points = {vec2(px - size, py), vec2(px + size, py),
                                                      vec2(px, py - size), vec2(px, py + size)};
                    drawable_pivot_->update_vertex_buffer(points, framebuffer_width(), framebuffer_height(), true);
                    drawable_pivot_->set_line_width(dpi);
                    Timer<>::single_shot(10000, [&]() {
                        show_pivot_point_ = false;
                        update();
                    });
#endif
                    if (input_.key == KEY_Z && modifiers == 0) {
#if 1 // with animation  Do NOT delete!!!
                        camera()->interpolate_to_look_at(point);
#else // without animation  Do NOT delete!!!
                        // Apply the legacy zoom target directly on the UI thread.
                        camera()->set_position(0.1f * camera()->position() + 0.9f * point);
                        camera()->look_at(point);
#endif
                    }
                }
            } else if (button == BUTTON_RIGHT) {
                camera()->set_pivot_point(camera()->scene_center());
                show_pivot_point_ = false;
                if (input_.key == KEY_Z && modifiers == 0) {
#if 1 // with animation  Do NOT delete!!!
                    camera()->interpolate_to_fit_scene();
#else // without animation  Do NOT delete!!!
                    camera()->show_entire_scene();
#endif
                }
            }
        }
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
        return false;
    }

    bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
        if (input_.button == button && button == BUTTON_LEFT && modifiers == MODIF_CTRL && x != input_.press_x &&
            y != input_.press_y) {
            const ivec2 origin = interaction_origin();
            camera()->fit_screen_region(std::min(x, input_.press_x) - origin.x,
                                      std::min(y, input_.press_y) - origin.y,
                                      std::max(x, input_.press_x) - origin.x,
                                      std::max(y, input_.press_y) - origin.y);
        }
        input_.button = -1;
        input_.press_modifiers = 0;
        end_gesture();
        return false;
    }

    bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        input_.x = x;
        input_.y = y;
        zoom_region_ = input_.button == BUTTON_LEFT && modifiers == MODIF_CTRL;
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
        // Region zoom and clipping/tool manipulation must not move the camera.
        if (modifiers == MODIF_CTRL)
            return false;
        // Camera actions are cell-local in multi-view layouts; overlays stay absolute.
        const ivec2 origin = interaction_origin();
        const int cx = x - origin.x, cy = y - origin.y;
        if (modifiers == MODIF_ALT) {
            // the clipping plane can be manipulated only when it is enabled
            if (ClippingPlane::instance()->is_enabled()) {
                auto *frame = ClippingPlane::instance()->manipulator()->frame();
                if (button == BUTTON_LEFT)
                    frame->action_rotate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
                else if (button == BUTTON_RIGHT)
                    frame->action_translate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
            }
            return false;
        }
        // axis-constrained rotation/translation of the camera
        auto axis = ManipulatedFrame::NONE;
        if (input_.key == KEY_X)
            axis = ManipulatedFrame::HORIZONTAL;
        else if (input_.key == KEY_Y)
            axis = ManipulatedFrame::VERTICAL;
        else if (input_.key == KEY_O)
            axis = ManipulatedFrame::ORTHOGONAL;
        if (button == BUTTON_LEFT)
            camera()->frame()->action_rotate(cx, cy, dx, dy, camera(), axis);
        else if (button == BUTTON_RIGHT)
            camera()->frame()->action_translate(cx, cy, dx, dy, camera(), axis);
        else if (button == BUTTON_MIDDLE && std::abs(dy) >= 1)
            camera()->frame()->action_zoom(dy > 0 ? 1 : -1, camera());
        return false;
    }

    bool Viewer::key_release_event(int key, int) {
        input_.key = -1;
        if (key == KEY_LEFT_CONTROL || key == KEY_RIGHT_CONTROL || key == KEY_LEFT_SUPER ||
            key == KEY_RIGHT_SUPER) {
            zoom_region_ = false;
#ifdef EASY3D_HAS_VULKAN
            update_interaction_overlays(true);
#endif
        }
        return false;
    }

    bool Viewer::drop_event(const std::vector<std::string> &names) {
        unsigned count = 0;
        for (const auto &file : names)
            if (add_model(file))
                ++count;
        if (count)
            fit_screen();
        return count != 0;
    }

    bool Viewer::key_press_event(int key, int modifiers) {
        const auto command = commands_.find(Key(key));
        if (command != commands_.end()) {
            const auto binding = command->second.find(Modifier(modifiers));
            if (binding != command->second.end()) {
                // The callable may unbind itself or delete its model (which prunes the binding),
                // so copies the bound callable to keep it alive for the duration of the call.
                const Function func = binding->second.first; // copies the bound callable
                return func(this, binding->second.second);
            }
        }
        if ((key == KEY_ESCAPE && modifiers == 0) || (key == KEY_F4 && modifiers == MODIF_ALT))
            exit();
        else if (key == KEY_F1 && modifiers == 0) {
            if (!manual_.empty())
                std::cout << manual_ << '\n';
        } else if (key == KEY_F2 && modifiers == 0)
            set_animation(!animation_enabled_);
        else if ((key == KEY_LEFT || key == KEY_RIGHT) && modifiers == 0)
            // turn left/right, 1 degree each step
            camera()->frame()->action_turn((key == KEY_LEFT ? 1.0f : -1.0f) * float(M_PI / 180.0), camera());
        else if ((key == KEY_UP || key == KEY_DOWN) && modifiers == 0) {
            // move camera forward/backward
            const float step = .05f * camera()->scene_radius();
            camera()->frame()->translate(
                camera()->frame()->inverse_transform_of(vec3(0, 0, key == KEY_UP ? -step : step)));
        } else if (modifiers == MODIF_CTRL && key >= KEY_RIGHT && key <= KEY_UP) {
            // move camera left/right/up/down
            const float step = .05f * camera()->scene_radius();
            const vec3 direction = key == KEY_LEFT    ? vec3(-step, 0, 0)
                                   : key == KEY_RIGHT ? vec3(step, 0, 0)
                                   : key == KEY_UP    ? vec3(0, step, 0)
                                                           : vec3(0, -step, 0);
            camera()->frame()->translate(camera()->frame()->inverse_transform_of(direction));
        } else if (modifiers == MODIF_CTRL && (key == KEY_MINUS || key == KEY_EQUAL))
            camera()->frame()->action_zoom(key == KEY_EQUAL ? 1 : -1, camera());
        else if (key == KEY_K && modifiers == MODIF_ALT) {
            // add key frame
            if (kfi_->add_keyframe(*camera()->frame()))
                LOG(INFO) << "key frame added to camera path";
        } else if (key == KEY_D && modifiers == MODIF_ALT) {
            // delete path
            kfi_->delete_path();
#ifdef EASY3D_HAS_VULKAN
            camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
            set_viewer_annotations({});
#endif
            restore_scene_bounds();
            update();
        } else if (key == KEY_K && modifiers == MODIF_CTRL) {
            // play the path
            if (kfi_->is_interpolation_started())
                kfi_->stop_interpolation();
            else {
                kfi_->start_manual_interpolation();
                next_camera_path_frame_ = std::chrono::steady_clock::now();
            }
            update();
        } else if (key == KEY_T && modifiers == MODIF_CTRL) {
            show_camera_path_ = !show_camera_path_;
            if (show_camera_path_) {
                float radius = camera()->scene_radius();
                for (size_t i = 0; i < kfi_->number_of_keyframes(); ++i)
                    radius = std::max(radius, distance(camera()->scene_center(), kfi_->keyframe_position(i)));
                camera()->set_scene_radius(radius);
            } else
                restore_scene_bounds();
            update();
        } else if (key == KEY_C && modifiers == MODIF_CTRL)
            copy_view(); // copy camera
        else if (key == KEY_V && modifiers == MODIF_CTRL)
            paste_view(); // paste camera
        else if (key == KEY_O && modifiers == MODIF_CTRL)
            open();
        else if (key == KEY_S && modifiers == MODIF_CTRL)
            save();
        else if (key == KEY_S && modifiers == 0)
            snapshot();
        else if (key == KEY_F && modifiers == MODIF_CTRL) {
            show_frame_rate_ = !show_frame_rate_;
            if (show_frame_rate_) {
                frame_rate_start_ = std::chrono::steady_clock::now();
                frame_rate_frames_ = 0;
                framerate_ = "fps: -- (-- ms/frame)";
            }
            update();
        } else if (key == KEY_F && modifiers == 0)
            fit_screen();
        else if (key == KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
        } else if (key == KEY_P && modifiers == 0)
            camera()->set_type(camera()->type() == Camera::PERSPECTIVE ? Camera::ORTHOGRAPHIC : Camera::PERSPECTIVE);
        else if (key == KEY_SPACE && modifiers == 0) {
            // Aligns camera
            Frame frame;
            frame.set_translation(camera()->pivot_point());
            camera()->frame()->align_with_frame(&frame, true);
        } else if (key == KEY_M && modifiers == 0) {
            if (auto *mesh = dynamic_cast<SurfaceMesh *>(current_model()))
                for (const auto &d : mesh->renderer()->triangles_drawables())
                    d->set_smooth_shading(!d->smooth_shading());
        } else if (modifiers == 0 && (key == KEY_LEFT_BRACKET || key == KEY_RIGHT_BRACKET)) {
            for (const auto &m : models())
                if (m->renderer())
                    for (const auto &d : m->renderer()->lines_drawables())
                        d->set_line_width(std::max(1.0f, d->line_width() + (key == KEY_RIGHT_BRACKET ? 1 : -1)));
        } else if (modifiers == 0 && (key == KEY_MINUS || key == KEY_EQUAL)) {
            for (const auto &m : models())
                if (m->renderer())
                    for (const auto &d : m->renderer()->points_drawables())
                        d->set_point_size(std::max(1.0f, d->point_size() + (key == KEY_EQUAL ? 1 : -1)));
        } else if (modifiers == 0 && (key == KEY_COMMA || key == KEY_PERIOD)) {
            if (!models().empty()) {
                auto i = std::find_if(models().begin(), models().end(),
                                      [&](const std::shared_ptr<Model> &m) { return m.get() == current_model(); });
                const int n = int(models().size()), index = i == models().end() ? n - 1 : int(i - models().begin());
                model_idx_ = (index + n + (key == KEY_PERIOD ? 1 : -1)) % n;
                fit_screen(current_model());
            }
        } else if (key == KEY_DELETE && modifiers == 0) {
            if (current_model())
                delete_model(current_model());
        } else if (modifiers == 0 && current_model() && current_model()->renderer() &&
                   (key == KEY_E || key == KEY_V || key == KEY_B || key == KEY_L)) {
            auto *renderer = current_model()->renderer();
            Drawable *d = nullptr;
            if (key == KEY_E)
                d = renderer->get_lines_drawable("edges");
            else if (key == KEY_V)
                d = renderer->get_points_drawable("vertices");
            else if (dynamic_cast<SurfaceMesh *>(current_model())) {
                if (key == KEY_B)
                    d = renderer->get_lines_drawable("borders");
                else
                    d = renderer->get_points_drawable("locks"); // locked vertices
            }
            if (d)
                d->set_visible(!d->is_visible());
        } else if (key == KEY_D && modifiers == 0) {
            if (current_model())
                current_model()->property_stats(std::cout);
        } else if (modifiers == MODIF_ALT && key == KEY_C) {
            // toggle clipping plane
            auto *plane = ClippingPlane::instance();
            plane->set_enabled(!plane->is_enabled());
            plane->fit_scene(camera()->scene_center(), camera()->scene_radius());
        } else if (modifiers == MODIF_ALT && key == KEY_V) {
            // toggle clipping plane visibility
            auto *plane = ClippingPlane::instance();
            if (plane->is_enabled())
                plane->set_visible(!plane->is_visible());
        } else if (modifiers == MODIF_ALT && key == KEY_X) {
            auto *plane = ClippingPlane::instance();
            if (plane->is_enabled())
                plane->set_cross_section(!plane->cross_section());
        } else if (modifiers == 0 && key == KEY_A) {
            show_corner_axes_ = !show_corner_axes_;
        } else if (modifiers == 0 && key == KEY_R) {
#ifdef EASY3D_HAS_VULKAN
            LOG(WARNING) << "This viewer shortcut is not yet available in the Vulkan backend";
#else
            // Reload the shader(s) - useful for writing/debugging shader code.
            ShaderManager::reload();
#endif
        } else
            input_.key = key;
        return false;
    }
} // namespace easy3dd


#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
    std::vector<graphics::DrawCommand> Viewer::draw_commands(
        const Camera &cam, int width, int, std::vector<DrawSource> *sources, bool include_hidden_models) const {
        impl_->drawable_renderer.prune();
        std::vector<graphics::DrawCommand> commands;
        const auto &planes = impl_->active_clipping_planes();
        // Drawable point/line sizes are logical pixels; the device rasterizes plain
        // points/lines in framebuffer pixels. Scale by the target-to-camera ratio,
        // not dpi_scaling(): it is the DPI scale only when the target is the window
        // framebuffer (live frames, picking, depth reads). A file snapshot captures
        // at output size while the camera keeps its logical size, so the ratio is
        // output/logical and keeps plain sizes in proportion; a MultiViewer cell's
        // integer cell_bounds make it target-cell/logical-cell, approximately the
        // DPI scale. Impostor radii are world-space and already carry the logical
        // sizing through world_pixel_ratio().
        const float pixel_scale = cam.screen_width() > 0 ? float(width) / float(cam.screen_width()) : 1.0f;
        auto append = [&](const std::shared_ptr<Drawable> &d, uint32_t id, const std::shared_ptr<Model> &model,
                          bool selected, bool depth_bias = false) {
            if (!d->is_visible())
                return;
            auto command = impl_->drawable_renderer.command(d, id, cam, planes, selected);
            command.point_size *= pixel_scale;
            command.line_width *= pixel_scale;
            command.depth_bias = depth_bias;
            if (sources) {
                if (sources->size() >= UINT32_MAX)
                    throw std::length_error("too many picking sources");
                id = uint32_t(sources->size());
                sources->push_back({model, d});
                command.object_id = id;
            }
            commands.push_back(std::move(command));
        };
        for (size_t i = 0; i < models_.size(); ++i) {
            const auto *renderer = models_[i]->renderer();
            if (!renderer || (!include_hidden_models && !renderer->is_visible()))
                continue;
            // Match the OpenGL viewer: surfaces are offset only while this model
            // also draws visible lines, so coplanar edges do not z-fight.
            bool has_visible_lines = false;
            for (const auto &d : renderer->lines_drawables())
                if (d->is_visible())
                    has_visible_lines = true;
            for (const auto &d : renderer->triangles_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected(), has_visible_lines);
            for (const auto &d : renderer->lines_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
            for (const auto &d : renderer->points_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
        }
        for (size_t i = 0; i < drawables_.size(); ++i)
            append(drawables_[i], uint32_t(models_.size() + i), nullptr, false);
        const auto guide = impl_->clipping_plane_renderer.draw(*ClippingPlane::instance(), cam);
        for (auto command : guide) {
            command.object_id = UINT32_MAX;
            commands.push_back(std::move(command));
        }
        return commands;
    }
    std::vector<graphics::DrawCommand> Viewer::frame_commands(
        const Camera &cam, int width, int height, std::vector<DrawSource> *sources, bool include_hud) const {
        auto commands = draw_commands(cam, width, height, sources);
        // Recorded overlay viewports use the host framebuffer's pixels. Keep
        // their NDC transforms and scale the clip rectangles for file output.
        const int fw = framebuffer_width(), fh = framebuffer_height();
        const auto append_overlays = [&](const std::vector<graphics::DrawCommand> &source) {
            for (auto command : source) {
                auto &v = command.viewport;
                if (v.z > 0 && v.w > 0 && fw > 0 && fh > 0 && (width != fw || height != fh)) {
                    const int x = int(std::lround(double(v.x) * width / fw));
                    const int y = int(std::lround(double(v.y) * height / fh));
                    const int right = int(std::lround(double(v.x + v.z) * width / fw));
                    const int bottom = int(std::lround(double(v.y + v.w) * height / fh));
                    if (right <= x || bottom <= y)
                        continue;
                    v = ivec4(x, y, right - x, bottom - y);
                }
                commands.push_back(std::move(command));
            }
        };
        append_overlays(impl_->viewer_annotations);
        if (!impl_->overlays_foreground)
            append_overlays(impl_->overlays);
        if (include_hud) {
            append_overlays(impl_->viewer_3d_overlays);
            append_overlays(impl_->viewer_overlays);
        }
        if (impl_->overlays_foreground)
            append_overlays(impl_->overlays);
        return commands;
    }
    Viewer::Viewer(int width, int height, const VulkanOptions &options)
        : window_(nullptr), width_(width), height_(height), camera_(new Camera), samples_(1),
          background_color_(setting::background_color), show_frame_rate_(false),
          show_camera_path_(false), show_easy3d_logo_(false), model_idx_(-1), impl_(new Impl(*this)) {
        resize_render_target(width, height);
        impl_->device =
            graphics::create_vulkan_device(options.validation, options.sync_validation, options.device_name,
                                           options.use_native_large_points, options.profiling);
        camera()->set_type(Camera::PERSPECTIVE);
        camera()->set_up_vector(vec3(0, 0, 1));
        camera()->set_view_direction(vec3(-1, 0, 0));
        camera()->show_entire_scene();
        // Transparency effects only know the camera. Rendering and presentation stay
        // in the viewer, as in Viewer::draw().
        Transparency::set_frame_renderer(camera(), [this](const std::vector<TrianglesDrawable *> &surfaces, int method,
                                                          int peels) { draw_transparency(surfaces, method, peels); });
    }
    void Viewer::release_device() {
        impl_->device.reset();
    }
    void Viewer::resize_render_target(int w, int h) {
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("offscreen dimensions must be positive");
        framebuffer_width_ = w;
        framebuffer_height_ = h;
        if (!window_) {
            width_ = w;
            height_ = h;
        }
        // The camera always carries logical window dimensions (framebuffer pixels
        // are GPU-only). For an offscreen canvas logical units equal pixels.
        int cw, ch;
        viewer_size(cw, ch);
        camera()->set_screen_width_and_height(cw, ch);
    }
    void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground) {
        for (auto &command : commands)
            command.overlay = true;
        impl_->overlays = std::move(commands);
        impl_->overlays_foreground = foreground;
    }
    void Viewer::set_viewer_overlays(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands) {
            command.overlay = true;
            command.overlay_depth_test = command.overlay_independent_depth = false;
        }
        impl_->viewer_overlays = std::move(commands);
    }
    void Viewer::set_viewer_3d_overlays(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands) {
            command.overlay = command.overlay_independent_depth = true;
            command.overlay_depth_test = false;
        }
        impl_->viewer_3d_overlays = std::move(commands);
    }
    void Viewer::set_viewer_annotations(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands) {
            command.overlay = command.overlay_depth_test = true;
            command.overlay_independent_depth = false;
        }
        impl_->viewer_annotations = std::move(commands);
    }
    void Viewer::set_clipping_planes(const std::vector<vec4> &planes) {
        impl_->clipping_planes = graphics::normalize_clipping_planes(planes);
    }
    const std::vector<vec4> &Viewer::clipping_planes() const {
        return impl_->active_clipping_planes();
    }
    std::vector<vec3> Viewer::scan_visible_points(unsigned samples) const {
        Camera snapshot(*camera());
        // Camera's copy constructor regenerates matrices from its parameters. Keep
        // effective matrices too, including externally calibrated/custom cameras.
        snapshot.frame()->set_pivot_point(camera()->pivot_point()); // do not re-adjust the copied ortho coefficient
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        const auto inv = transform::checked_inverse(snapshot.model_view_projection_matrix());
        const auto frame = impl_->capture(snapshot, background_color(), samples, framebuffer_width(), framebuffer_height());
        std::vector<vec3> points;
        points.reserve(std::count_if(frame.depth.begin(), frame.depth.end(), [](float d) { return d >= 0 && d < 1; }));
        for (unsigned y = 0; y < frame.height; ++y)
            for (unsigned x = 0; x < frame.width; ++x) {
                const size_t pixel = size_t(y) * frame.width + x;
                const float d = frame.depth[pixel];
                if (frame.ids[pixel].object == UINT32_MAX || !std::isfinite(d) || d < 0 || d >= 1)
                    continue;
                const int viewport[] = {0, 0, int(frame.width), int(frame.height)};
                points.push_back(transform::unproject_inverse(vec3(x + .5f, y + .5f, d), inv, viewport, false));
            }
        return points;
    }
    Viewer::PickResult Viewer::pick(int x, int y) const {
        PickResult result;
        int px, py;
        if (!window_pixel(x, y, px, py))
            return result;
        int fw, fh;
        framebuffer_size(fw, fh);
        // One backend draw re-renders the visible scene with object/primitive IDs and
        // depth. Sampling the cursor pixel therefore names the nearest drawable (and
        // its triangle for meshes) without any CPU ray casting.
        // Keep commands and source mapping together for this exact synchronous draw.
        // This whole-scene query is intentionally not delegated to the Picker classes:
        // they operate on one caller-supplied model with a borrowed camera and own
        // their offscreen resources, while the viewer already owns every model,
        // drawable and the device, so it can answer in a single submission.
        std::vector<DrawSource> sources;
        const auto commands = draw_commands(*camera(), fw, fh, &sources);
        const auto sample = impl_->device->sample_pixel(fw, fh, commands, px, py);
        const auto id = sample.id;
        if (id.object >= sources.size() || sample.depth >= 1)
            return result;
        result.model = sources[id.object].model;
        result.drawable = sources[id.object].drawable;
        result.primitive = id.primitive;
        result.depth = sample.depth;
        result.position = pixel_to_world(px, py, result.depth);
        const auto &faces = result.drawable->cpu_buffers().source_faces;
        auto *mesh = dynamic_cast<SurfaceMesh *>(result.model.get());
        if (mesh && id.primitive < faces.size()) {
            const SurfaceMesh::Face f(faces[id.primitive]);
            if (mesh->is_valid(f) && !mesh->is_deleted(f))
                result.face_index = f.idx();
        }
        return result;
    }
    bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
        const int width = framebuffer_width(), height = framebuffer_height();
        const auto commands = draw_commands(snapshot, width, height);
        const auto sample = impl_->device->sample_pixel(width, height, commands, x, y);
        depth = sample.depth;
        return sample.id.object != UINT32_MAX;
    }
    const graphics::RenderDevice &Viewer::device() const {
        return *impl_->device;
    }
    std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest &request) const {
        return impl_->device->select_points(request);
    }
    std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable *drawable) const {
        if (!drawable || drawable->type() != Drawable::DT_POINTS)
            throw std::invalid_argument("selection requires a point drawable");
        for (const auto &model : models_)
            if (model->renderer())
                for (const auto &d : model->renderer()->points_drawables())
                    if (d.get() == drawable)
                        return impl_->drawable_renderer
                            .command(d, 0, *camera(), impl_->clipping_planes, model->renderer()->is_selected())
                            .geometry->point_data();
        for (const auto &d : drawables_)
            if (d.get() == drawable)
                return impl_->drawable_renderer.command(d, 0, *camera(), impl_->clipping_planes).geometry->point_data();
        throw std::invalid_argument("point drawable does not belong to this viewer");
    }
    void Viewer::init() {}
    void Viewer::pre_draw() {}
    void Viewer::post_draw() {}
    void Viewer::draw() const {
        int width = 0, height = 0;
        impl_->frame_size(width, height);
        Shadow::FrameData shadow;
        Shadow *active_shadow = Shadow::active(camera());
        const bool has_shadow = active_shadow && Shadow::frame_data(camera(), shadow);
        std::vector<DrawSource> sources;
        auto commands =
            frame_commands(*camera(), width, height, has_shadow ? &sources : nullptr, !impl_->snapshot.active);
        if (has_shadow) {
            const mat4 clip_to_shadow = shadow.shadow_matrix * inverse(camera()->model_view_projection_matrix());
            const auto &surfaces = active_shadow->surfaces();
            for (size_t i = 0; i < sources.size() && i < commands.size(); ++i) {
                auto *triangles = dynamic_cast<TrianglesDrawable *>(sources[i].drawable.get());
                if (!triangles || std::find(surfaces.begin(), surfaces.end(), triangles) == surfaces.end())
                    continue;
                if (!commands[i].geometry || commands[i].geometry->topology() != graphics::Topology::triangles)
                    continue;
                commands[i].shadow_caster = true;
                commands[i].shadow_matrix = clip_to_shadow;
                commands[i].shadow_map_size = shadow.shadow_map_size;
            }
            if (!shadow.background_corners.empty()) {
                std::vector<graphics::Vertex> vertices(4);
                for (unsigned i = 0; i < 4; ++i)
                    vertices[i].position = shadow.background_corners[i];
                graphics::DrawCommand background;
                background.geometry =
                    std::make_shared<graphics::Geometry>(std::move(vertices), std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
                background.color = shadow.background_color;
                background.object_id = UINT32_MAX;
                background.model_view_projection = camera()->model_view_projection_matrix();
                background.shadow_background = true;
                background.shadow_matrix = clip_to_shadow;
                background.shadow_map_size = shadow.shadow_map_size;
                commands.push_back(std::move(background));
            }
            commands.insert(commands.begin(),
                            graphics::shape::draw_shadow(unsigned(width), unsigned(height), clip_to_shadow,
                                                         shadow.darkness, shadow.softness, shadow.light_near,
                                                         shadow.light_far, shadow.shadow_map_size));
        }
        float radius, bias;
        mat4 projection;
        if (AmbientOcclusion::frame_params(camera(), radius, bias, projection))
            commands.insert(commands.begin(), graphics::shape::draw_ambient_occlusion(unsigned(width), unsigned(height),
                                                                                      radius, bias, projection));
        float z_near, z_far, scene_size;
        if (EyeDomeLighting::frame_params(camera(), z_near, z_far, scene_size))
            commands.insert(commands.begin(), graphics::shape::draw_eye_dome_lighting(unsigned(width), unsigned(height),
                                                                                      z_near, z_far, scene_size));
        impl_->submit(std::move(commands));
    }
    void Viewer::draw_transparency(const std::vector<TrianglesDrawable *> &surfaces, int method, int peels) {
        int width = 0, height = 0;
        impl_->frame_size(width, height);
        std::vector<DrawSource> sources;
        auto commands = frame_commands(*camera(), width, height, &sources, !impl_->snapshot.active);
        for (size_t i = 0; i < sources.size() && i < commands.size(); ++i) {
            auto *triangles = dynamic_cast<TrianglesDrawable *>(sources[i].drawable.get());
            if (!triangles || std::find(surfaces.begin(), surfaces.end(), triangles) == surfaces.end())
                continue;
            if (commands[i].geometry && commands[i].geometry->topology() == graphics::Topology::triangles) {
                commands[i].transparent = true;
                commands[i].color.w = triangles->opacity();
            }
        }
        const auto transparency = method == 1   ? graphics::TransparencyMethod::average
                                  : method == 2 ? graphics::TransparencyMethod::peeling
                                  : method == 3 ? graphics::TransparencyMethod::linked_lists
                                                : graphics::TransparencyMethod::sorted;
        impl_->submit(std::move(commands), transparency, peels > 0 ? unsigned(peels) : 0u);
    }
    bool Viewer::snapshot_frame(int width,
                                int height,
                                const mat4 &projection,
                                const vec4 &background,
                                unsigned samples,
                                graphics::FrameImage &image) {
        auto &frame = impl_->snapshot;
        // Keep the camera's logical screen dimensions: only the projection changes
        // for the output frustum. draw_commands() then scales plain point/line sizes
        // by output/logical and impostors keep their relative size, as on OpenGL.
        const mat4 source_projection = camera()->projection_matrix();
        struct Restore {
            Viewer &self;
            mat4 projection;
            ~Restore() {
                self.impl_->snapshot.active = false;
                self.camera()->set_projection_matrix(projection);
            }
        } restore{*this, source_projection};
        frame.width = width;
        frame.height = height;
        frame.samples = samples;
        frame.background = background;
        frame.image = graphics::FrameImage();
        frame.active = true;
        camera()->set_projection_matrix(projection);
        draw();
        if (frame.image.rgba.size() != size_t(width) * size_t(height) * 4)
            throw std::runtime_error("the captured frame does not match the requested size");
        image = std::move(frame.image);
        return true;
    }
    void Viewer::cleanup() {
        release_device();
    }
    Viewer::Viewer(const std::string &title,
                   int width,
                   int height,
                   int samples,
                   bool full_screen,
                   bool visible,
                   bool resizable,
                   const VulkanOptions &vk_options)
        : Viewer(std::make_shared<Window>(title, width, height, samples, full_screen, visible, resizable), vk_options) {
        if (width <= 16 && height <= 16)
            LOG(WARNING) << "Viewer dimensions " << width << "x" << height
                         << " are unusually small and may be legacy OpenGL context-version arguments; "
                            "use Viewer(title, width, height, samples, full_screen, visible, resizable, options)";
    }
    Viewer::Viewer(std::shared_ptr<Window> host, const VulkanOptions &options)
        : window_(host->handle), width_(host->width), height_(host->height), title_(host->title), camera_(new Camera),
          samples_(1), background_color_(setting::background_color), show_frame_rate_(false),
          show_camera_path_(false), show_easy3d_logo_(true), model_idx_(-1), window_host_(std::move(host)),
          impl_(new Impl(*this)) {
        try {
            // Initialize logging (if it has not been initialized yet)
            if (!logging::is_initialized())
                logging::initialize(false, true, true, false, "default", 9);

            impl_->device = window_host_->device(options);
            manual_ = default_manual;
            kfi_.reset(new KeyFrameInterpolator(camera()->frame()));
            camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
            { // The default of 4 samples must not fail on a device without 4x MSAA: step a valid
                // power of two down to a supported count with a warning (as OpenGL does); invalid
                // counts are still rejected by set_samples().
                unsigned s = window_host_->samples;
                const unsigned supported = device().supported_sample_counts();
                if (s > 1 && s <= 64 && !(s & (s - 1)))
                    while (s > 1 && !(supported & s))
                        s >>= 1;
                if (s != window_host_->samples) {
                    LOG(WARNING) << "MSAA with " << window_host_->samples << " samples is not supported, using " << s;
                    window_host_->samples = s;
                }
            }
            set_samples(window_host_->samples);
            window_host_->callbacks(this);
            int w, h;
            framebuffer_size(w, h);
            if (w > 0 && h > 0)
                resize_render_target(w, h);
            camera()->set_up_vector(vec3(0, 0, 1));
            camera()->set_view_direction(vec3(-1, 0, 0));
            camera()->show_entire_scene();
            // Transparency effects only know the camera, so route their frames back to
            // this viewer for rendering and presentation.
            Transparency::set_frame_renderer(camera(),
                                             [this](const std::vector<TrianglesDrawable *> &surfaces, int method,
                                                    int peels) { draw_transparency(surfaces, method, peels); });
            VLOG(1) << "Vulkan device: " << impl_->device->name() << ", Samples: " << window_host_->samples;
        } catch (...) {
            // A failed derived constructor destroys members before base classes.
            // Release the Vulkan surface before member-owned native-window cleanup.
            glfwSetWindowUserPointer(window_host_->handle, nullptr);
            Transparency::set_frame_renderer(camera(), nullptr);
            release_device();
            throw;
        }
    }
    Viewer::~Viewer() {
        if (kfi_)
            kfi_->stop_interpolation();
        // Base classes are destroyed after members. Release the surface/device now,
        // while the member-owned GLFW window (and its Metal layer on macOS) still live.
        if (window_)
            glfwSetWindowUserPointer(window_, nullptr);
        Transparency::set_frame_renderer(camera(), nullptr);
        release_device();
    }
    float Viewer::dpi_scaling() const {
        if (!window_)
            return 1.0f;
        int w = 0, h = 0;
        glfwGetWindowSize(window(), &w, &h);
        return w > 0 ? float(framebuffer_width()) / w : 1.0f;
    }
    void Viewer::set_samples(unsigned samples) {
        if (!samples || samples > 64 || (samples & (samples - 1)) || !(device().supported_sample_counts() & samples))
            throw std::invalid_argument("unsupported Vulkan viewer sample count");
        window_host_->samples = samples;
        samples_ = int(samples);
    }
    void Viewer::update_camera_path_overlays() const {
        std::vector<graphics::DrawCommand> commands;
        if (show_camera_path_ && !kfi_->is_interpolation_started()) {
            auto path = camera_path_renderer_->draw_path(*kfi_, *camera());
            auto cameras = camera_path_renderer_->draw_cameras(*kfi_, *camera(), camera()->scene_radius() * .05f);
            if (path.geometry)
                commands.push_back(path);
            if (cameras.geometry)
                commands.push_back(cameras);
        }
        set_viewer_annotations(std::move(commands));
    }
    void Viewer::draw_corner_axes() const {
        if (!show_corner_axes_) {
            set_viewer_3d_overlays({});
            return;
        }
        if (!corner_axes_geometry_) {
            // Same cylinders, cones, sphere and RGB colors as Viewer.
            std::vector<vec3> points, normals, colors;
            for (const auto &axis : {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)}) {
                shape::create_cylinder(.03f, 10, vec3(0), .5f * axis, axis, points, normals, colors);
                shape::create_cone(.06f, 20, .5f * axis, .7f * axis, axis, points, normals, colors);
            }
            shape::create_sphere(vec3(0), .06f, 20, 20, vec3(0, 1, 1), points, normals, colors);
            std::vector<graphics::Vertex> vertices(points.size());
            for (size_t i = 0; i < points.size(); ++i) {
                vertices[i].position = points[i];
                vertices[i].normal = normals[i];
                vertices[i].color = vec4(colors[i], 1);
            }
            corner_axes_geometry_ = std::make_shared<graphics::Geometry>(std::move(vertices));
        }
        const float dpi = dpi_scaling();
        const float sx = 100 * dpi / framebuffer_width(), sy = 100 * dpi / framebuffer_height();
        const mat4 view = camera()->orientation().inverse().matrix();
        const mat4 viewport = mat4::translation(vec3(-1 + sx, -1 + sy, 0)) * mat4::scale(sx, sy, 1, 1);
        graphics::DrawCommand axes;
        axes.geometry = corner_axes_geometry_;
        axes.model_view_projection = viewport * transform::ortho(-1, 1, -1, 1, -1, 1) * view;
        axes.normal_matrix = mat3(view);
        axes.lighting = true;
        set_viewer_3d_overlays({axes});
    }
    void Viewer::update_interaction_overlays(bool force) const {
        draw_corner_axes();
        const bool pivot = show_pivot_point_ && std::chrono::steady_clock::now() < pivot_expires_;
        const float dpi = dpi_scaling();
        std::vector<graphics::DrawCommand> commands;
        const bool text_changed = update_hud_text(commands);
        if (!force && !text_changed && pivot == pivot_hint_visible_)
            return;
        if (pivot) {
            // The pivot cross is ten logical pixels long and one logical pixel wide;
            // both backends draw it in framebuffer pixels, so the arm is 10 * dpi and
            // the ribbon width is dpi (one logical pixel; a fixed width of one would be
            // a thinner device-pixel hairline on HiDPI). Screen-space lines are
            // corner-based (texel (i, j) spans [i, i+1) x [j, j+1)), so center the
            // ribbon on the marked pixel's center instead of its corner.
            const float x = (pivot_pixel_.x + 0.5f) * dpi, y = (pivot_pixel_.y + 0.5f) * dpi, size = 10.0f * dpi;
            commands.push_back(graphics::shape::draw_lines_2d(
                {vec2(x - size, y), vec2(x + size, y), vec2(x, y - size), vec2(x, y + size)}, vec4(0, 0, 0, 1),
                framebuffer_width(), framebuffer_height(), dpi));
        }
        if (zoom_region_ && (input_.x != input_.press_x || input_.y != input_.press_y)) {
            // Mouse coordinates are logical; scale into the framebuffer overlay space.
            // The one-logical-pixel wire is centered on its endpoints, so place them on
            // the boundary texels' centers: a wire centered on a texel boundary covers
            // only half of each neighbour and never resolves to a solid color under
            // MSAA, and lavapipe's fill rule can drop it entirely. Same convention as
            // the pivot cross above.
            const Rect rect{(input_.press_x + 0.5f) * dpi, (input_.x + 0.5f) * dpi,
                            (input_.press_y + 0.5f) * dpi, (input_.y + 0.5f) * dpi};
            commands.push_back(
                graphics::shape::draw_quad_filled(rect, vec4(0, 0, 1, .2f), framebuffer_width(), framebuffer_height()));
            commands.push_back(graphics::shape::draw_quad_wire(rect, vec4(0, 0, 1, 1), framebuffer_width(),
                                                               framebuffer_height(), dpi));
        }
        set_viewer_overlays(std::move(commands));
        pivot_hint_visible_ = pivot;
    }
    graphics::FrameImage Viewer::capture() const {
        return capture(samples());
    }
    graphics::FrameImage Viewer::capture(unsigned sample_count) const {
        if (!initialized_) {
            auto *host = const_cast<Viewer *>(this);
            host->init();
            host->initialized_ = true;
        }
        if (window_) {
            update_camera_path_overlays();
            update_interaction_overlays();
        }
        return impl_->capture(*camera(), background_color(), sample_count, framebuffer_width(), framebuffer_height(),
                              snapshot_hud_);
    }
} // namespace easy3d
#else // OpenGL
namespace easy3d {
    float Viewer::dpi_scaling() const {
        return dpi_scaling_;
    }
    Viewer::Viewer(const std::string &title /* = "Easy3D Viewer" */,
                   int width /* = 800 */,
                   int height /* = 600 */,
                   int samples /* = 1 */,
                   bool full_screen /* = false */,
                   bool visible /* = true */,
                   bool resizable /* = true */,
                   const OpenGLOptions &gl_options /* = {} */
                   )
        : window_(nullptr), dpi_scaling_(1.0), title_(title), samples_(0),
          visible_(visible), background_color_(setting::background_color),
          show_easy3d_logo_(true), show_pivot_point_(false), show_frame_rate_(false),
          show_camera_path_(false), model_idx_(-1) {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        if (samples < 0 || samples > 64 || (samples && (samples & (samples - 1))))
            throw std::invalid_argument("invalid viewer sample count");
        // Avoid locale-related number parsing issues.
        setlocale(LC_NUMERIC, "C");

        animation_func_ = nullptr;

        // Initialize logging (if it has not been initialized yet)
        if (!logging::is_initialized())
            logging::initialize(false, true, true, false, "default", 9);

        if (width <= 16 && height <= 16)
            LOG(WARNING) << "Viewer dimensions " << width << "x" << height
                         << " are unusually small and may be legacy OpenGL context-version arguments; "
                            "use Viewer(title, width, height, samples, full_screen, visible, resizable, options)";

        // create and setup window
        window_ = create_window(title, int(samples), gl_options.major, gl_options.minor, full_screen, resizable,
                                gl_options.depth_bits, gl_options.stencil_bits, int(width), int(height));
        try {
            setup_callbacks(window_);

            // create and set up the camera
            camera_ = std::unique_ptr<Camera>(new Camera);
            camera_->set_screen_width_and_height(width_, height_);
            camera_->set_type(Camera::PERSPECTIVE);
            camera_->set_up_vector(vec3(0, 0, 1));       // Z pointing up
            camera_->set_view_direction(vec3(-1, 0, 0)); // X pointing out
            camera_->show_entire_scene();

            easy3d::connect(&camera_->frame_modified, this, &Viewer::update);

            kfi_ = std::unique_ptr<KeyFrameInterpolator>(new KeyFrameInterpolator(camera_->frame()));
            easy3d::connect(&kfi_->interpolation_stopped, this, &Viewer::update);


            /* Poll for events once before starting a potentially lengthy loading process.*/
            glfwPollEvents();

            manual_ = "--------------------------------------------------------------\n"
                      " Easy3D Viewer Usage (Shortcuts might be overridden):         \n"
                      "--------------------------------------------------------------\n"
                      "  F1:                      Show help                          \n"
                      "  F2:                      Toggle animation (if available)    \n"
                      "  Ctrl + 'o':              Open file                          \n"
                      "  Ctrl + 's':              Save file                          \n"
                      "  Fn + Delete:             Delete current model               \n"
                      "  '<' or '>':              Switch between models              \n"
                      "  's':                     Take a snapshot                    \n"
                      "  'p':                     Switch between perspective and orthographic projections \n"
                      "  'a':                     Toggle axes visibility                           \n"
                      "  Ctrl + 'f':              Toggle frame rate display                        \n"
                      "  Left drag:               Rotate the camera                                \n"
                      "  Right drag:              Move the camera                                  \n"
                      "  'x' + Left drag:         Rotate the camera around the horizontal axis     \n"
                      "  'x' + Right drag:        Move the camera along the horizontal axis        \n"
                      "  'y' + Left drag:         Rotate the camera around the vertical axis       \n"
                      "  'y' + Right drag:        Move the camera along the vertical axis          \n"
                      "  'o' + Left drag:         Rotate the camera around the ortho-screen axis   \n"
                      "  Middle button or Wheel:  Zoom in/out                                      \n"
                      "  Ctrl + '+'/'-':          Zoom in/out                                      \n"
                      "  Left/Right Arrow:        Turn camera left/right                  \n"
                      "  Ctrl + Left/Right Arrow: Move camera left/right                  \n"
                      "  Up/Down Arrow:           Move camera forward/backward            \n"
                      "  Ctrl + Up/Down Arrow:    Move camera up/down                     \n"
                      "  Ctrl + 'c':              Copy current view status to clipboard   \n"
                      "  Ctrl + 'v':              Restore view status from clipboard      \n"
                      "  Alt + 'c':               Toggle clipping plane                   \n"
                      "  Alt + 'v':               Toggle clipping plane visibility        \n"
                      "  Alt + 'x':               Toggle cross-section                    \n"
                      "  Alt + 'k':               Add key frame to the camera path        \n"
                      "  Alt + 'd':               Delete the camera path                  \n"
                      "  Ctrl + 'k':              Play the camera path                    \n"
                      "  Ctrl + 't':              Toggle camera path visibility           \n"
                      "  'f':                     Fit screen (all models)                 \n"
                      "  'c':                     Fit screen (current model only)         \n"
                      "  'z' + Left click:        Zoom to target point on model           \n"
                      "  'z' + Right click:       Zoom to fit screen                      \n"
                      "  Shift + Left click:      Define a target point on model          \n"
                      "  Shift + Right click:     Undefine the target point (if any) on model\n"
                      "  Ctrl + Left drag:        Zoom to fit region                         \n"
                      "  '-'/'=':                 Decrease/Increase point size               \n"
                      "  '{'/'}':                 Decrease/Increase line width               \n"
                      "  'b':                     Toggle borders visibility                  \n"
                      "  'e':                     Toggle edges visibility                    \n"
                      "  'v':                     Toggle vertices visibility                 \n"
                      "  'm':                     Toggle smooth shading (for SurfaceMesh)    \n"
                      "  'd':                     Print model info (drawables, properties)   \n"
                      "-------------------------------------------------------------------";
        } catch (...) {
            cleanup();
            throw;
        }
    }
    GLFWwindow *Viewer::create_window(const std::string &title,
                                      int samples,
                                      int gl_major,
                                      int gl_minor,
                                      bool full_screen,
                                      bool resizable,
                                      int depth_bits,
                                      int stencil_bits,
                                      int width,
                                      int height) {
        if (samples == 1)
            samples = 0; // One sample means no MSAA in the unified API.
        runtime_ = detail::GlfwRuntime::acquire();

        // Reset the hints, allowing viewers to have different hints.
        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_SAMPLES, samples);

        glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);

        /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
           Default value is an OpenGL 3.2 core profile context. */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);

#ifdef __APPLE__
        // The only OpenGL 3.x and 4.x contexts currently supported by macOS are
        // forward-compatible, core profile contexts.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
        if (gl_major >= 3) {
            if (gl_minor >= 2)
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
            if (gl_minor >= 0)
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
        }
#endif

        // make the whole window transparent
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

#ifndef NDEBUG
        const std::string title_str = title + " - Debug Version";
#else
        const std::string title_str = title;
#endif

        GLFWwindow *window = nullptr;
        if (full_screen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
            window = glfwCreateWindow(width, height, title_str.c_str(), monitor, runtime_->opengl_share_window());
        } else {
            window = glfwCreateWindow(width, height, title_str.c_str(), nullptr, runtime_->opengl_share_window());
        }

        if (!window) {
            std::string error_message =
                "could not create an OpenGL " + std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!";
            LOG(ERROR) << error_message;
            throw std::runtime_error(error_message);
        }
        const auto previous_context = glfwGetCurrentContext();
        auto close_failed_window = [previous_context](GLFWwindow *failed) {
            glfwDestroyWindow(failed);
            glfwMakeContextCurrent(previous_context);
        };
        std::unique_ptr<GLFWwindow, decltype(close_failed_window)> guard(window, close_failed_window);
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Enable vsync
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Load OpenGL and its extensions
        if (!OpenglUtil::init()) {
            LOG(ERROR) << "failed to load OpenGL and its extensions!";
            throw std::runtime_error("failed to load OpenGL and its extensions!");
        }

#ifndef NDEBUG
        opengl::setup_gl_debug_callback();
#endif

        VLOG(1) << "OpenGL vendor: " << glGetString(GL_VENDOR);
        VLOG(1) << "OpenGL renderer: " << glGetString(GL_RENDERER);
        VLOG(1) << "OpenGL version requested: " << gl_major << "." << gl_minor;
        VLOG(1) << "OpenGL version received: " << glGetString(GL_VERSION);
        VLOG(1) << "GLSL version received: " << glGetString(GL_SHADING_LANGUAGE_VERSION);

        glGetIntegerv(GL_SAMPLES, &samples_);
        int max_num = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &max_num);

        // warn the user if the requests were not satisfied
        if (samples > 0 && samples_ != samples) {
            if (samples_ == 0)
                LOG(WARNING) << "MSAA is not available (" << samples << " samples requested)";
            else
                LOG(WARNING) << "MSAA is available with " << samples_ << " samples (" << samples
                             << " requested but max support is " << max_num << ")";
        } else
            VLOG(1) << "samples received: " << samples_ << " (" << samples << " requested, max support is " << max_num
                    << ")";

        // the request viewer size might not be satisfied, thus we query the actual size
        glfwGetWindowSize(window, &width_, &height_);
        VLOG(1) << "viewer size (width/height): " << width_ << "/" << height_;

        float xscale(0), yscale(0);
        glfwGetWindowContentScale(window, &xscale, &yscale);
        LOG_IF(xscale != yscale, ERROR) << "window content scale in X and Y directions not identical (X: " << xscale
                                        << ", Y: " << yscale << ")";
        dpi_scaling_ = static_cast<float>(xscale + yscale) * 0.5f;
        VLOG(1) << "DPI scaling: " << dpi_scaling();

        glfwGetFramebufferSize(window, &framebuffer_width_, &framebuffer_height_);
        runtime_->attach_opengl_window(window);
        return guard.release();
    }
    template <class Callback> void Viewer::dispatch_callback(GLFWwindow *window, const Callback &callback) noexcept {
        auto *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
        if (!viewer)
            return;
        try {
            glfwMakeContextCurrent(window);
            callback(*viewer);
            if (viewer->callback_error_)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        } catch (...) {
            if (!viewer->callback_error_)
                viewer->callback_error_ = std::current_exception();
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
    void Viewer::setup_callbacks(GLFWwindow *window) {
        glfwSetWindowRefreshCallback(window,
                                     [](GLFWwindow *win) { dispatch_callback(win, [](Viewer &v) { v.update(); }); });
        // Camera/input coordinates are logical window units; the shared ViewerBase
        // routers own all input state and event ordering on both backends.
        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
            dispatch_callback(win, [&](Viewer &v) {
                v.handle_cursor_pos(int(x), int(y), detail::GlfwRuntime::modifiers(win));
            });
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int modifiers) {
            dispatch_callback(win, [&](Viewer &v) {
                double x, y;
                glfwGetCursorPos(win, &x, &y);
                v.handle_mouse_button(int(x), int(y), button, action, modifiers);
            });
        });
        glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int, int action, int mods) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_key(key, action, mods); });
        });
        glfwSetCharCallback(window, [](GLFWwindow *win, unsigned int codepoint) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_character(codepoint); });
        });
        glfwSetDropCallback(window, [](GLFWwindow *win, int count, const char **filenames) {
            dispatch_callback(win, [&](Viewer &v) {
                std::vector<std::string> names(count);
                for (int i = 0; i < count; ++i)
                    names[i] = filenames[i];
                v.handle_drop(names);
            });
        });
        glfwSetScrollCallback(window, [](GLFWwindow *win, double dx, double dy) {
            dispatch_callback(win, [&](Viewer &v) {
                double x, y;
                glfwGetCursorPos(win, &x, &y);
                v.handle_scroll(int(x), int(y), dx, dy);
            });
        });
        glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h) {
            dispatch_callback(win, [&](Viewer &v) { v.callback_event_resize(w, h); });
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int, int) {
            dispatch_callback(win, [&](Viewer &v) {
                int w, h;
                glfwGetWindowSize(win, &w, &h);
                v.callback_event_resize(w, h);
            });
        });
        glfwSetWindowFocusCallback(window, [](GLFWwindow *win, int focused) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_focus(focused != 0); });
        });
        glfwSetWindowCloseCallback(
            window, [](GLFWwindow *win) { dispatch_callback(win, [](Viewer &v) { v.request_close(); }); });
    }
    void Viewer::callback_event_resize(int w, int h) {
        try {
            int fw, fh;
            glfwGetFramebufferSize(window_, &fw, &fh);
            width_ = w;
            height_ = h;
            framebuffer_width_ = fw;
            framebuffer_height_ = fh;
            if (w > 0 && h > 0 && (camera_->screen_width() != w || camera_->screen_height() != h))
                camera_->set_screen_width_and_height(w, h);
            glViewport(0, 0, fw, fh);
            notify_resize();
        } catch (const std::exception &e) {
            if (!callback_error_)
                callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
        }
    }
    Viewer::~Viewer() {
        cleanup();

        LOG(INFO) << "viewer terminated. Bye!";
    }
    void Viewer::cleanup() {
        // viewer may have already been destroyed by the user
        if (!window_)
            return;

        const auto previous_context = glfwGetCurrentContext();
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, nullptr);
        if (kfi_)
            kfi_->stop_interpolation();
        kfi_.reset();
        camera_.reset();
        drawable_axes_.reset();
        drawable_pivot_.reset();
        text_layout_.reset();
        hud_overlay_renderer_.reset();
        overlay_renderer_.reset();
        point_selector_.reset();

        clear_scene();

        if (runtime_->last_opengl_window()) {
            ShaderManager::terminate();
            TextureManager::terminate();
        }

        const auto closing = window_;
        runtime_->detach_opengl_window(window_);
        glfwDestroyWindow(window_);
        window_ = nullptr;
        if (previous_context && previous_context != closing)
            glfwMakeContextCurrent(previous_context);
        runtime_.reset();
    }
    void Viewer::init() {
        glfwMakeContextCurrent(window_);
        //// back face culling?
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK); // culls only the back faces

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearDepth(1.0f);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

        // camera is manipulated by the mouse, working in the screen coordinate system
        // (different from the viewport or framebuffer size, which are in pixel coordinates)
        if (camera_->screen_width() != width_ || camera_->screen_height() != height_)
            camera_->set_screen_width_and_height(width_, height_);
        int fw, fh;
        glfwGetFramebufferSize(window_, &fw, &fh);
        glViewport(0, 0, fw, fh);

        ensure_text_layout_engine();

        initialized_ = true;
        print_usage();
    }
    void Viewer::draw_corner_axes() const {
        if (!show_corner_axes_)
            return;
        ShaderProgram *program = ShaderManager::get_program("surface/surface");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")};
            program = ShaderManager::create_program_from_files("surface/surface", attributes);
        }
        if (!program)
            return;

        if (!drawable_axes_) {
            float base = 0.5f; // the cylinder length, relative to the allowed region
            float head = 0.2f; // the cone length, relative to the allowed region
            std::vector<vec3> points, normals, colors;
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
            shape::create_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals,
                               colors);
            shape::create_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals,
                               colors);
            shape::create_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals,
                               colors);
            shape::create_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
            const_cast<Viewer *>(this)->drawable_axes_ =
                std::unique_ptr<TrianglesDrawable>(new TrianglesDrawable("corner_axes"));
            drawable_axes_->update_vertex_buffer(points);
            drawable_axes_->update_normal_buffer(normals);
            drawable_axes_->update_color_buffer(colors);
            drawable_axes_->set_property_coloring(State::VERTEX);
        }
        // The viewport is changed to fit the lower left corner.
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        static int corner_frame_size = static_cast<int>(100 * dpi_scaling());
        glViewport(0, 0, corner_frame_size, corner_frame_size);

        // To make the axis appear over other objects: reserve a tiny bit of the
        // front depth range. NOTE: do remember to restore it later.
        glDepthRange(0, 0.01f);

        const mat4 &proj = transform::ortho(-1, 1, -1, 1, -1, 1);
        const mat4 &view = camera_->orientation().inverse().matrix();
        const mat4 &MVP = proj * view;

        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera_->model_view_matrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program
            ->set_uniform("MVP_MANIP", MVP) // MANIP is identity for the corner axes
            ->set_uniform("MANIP", mat4::identity())
            ->set_uniform("NORMAL", mat3::identity()) // needs be padded when using uniform blocks
            ->set_uniform("lighting", true)
            ->set_uniform("two_sides_lighting", false)
            ->set_uniform("smooth_shading", true)
            ->set_uniform("wLightPos", wLightPos)
            ->set_uniform("wCamPos", wCamPos)
            ->set_uniform("ssaoEnabled", false)
            ->set_uniform("per_vertex_color", true)
            ->set_uniform("distinct_back_color", false)
            ->set_block_uniform("Material", "ambient", setting::material_ambient)
            ->set_block_uniform("Material", "specular", setting::material_specular)
            ->set_block_uniform("Material", "shininess", &setting::material_shininess)
            ->set_uniform("highlight", false)
            ->set_uniform("clippingPlaneEnabled", false)
            ->set_uniform("selected", false)
            ->set_uniform("highlight_color", setting::highlight_color)
            ->set_uniform("use_texture", false);
        drawable_axes_->gl_draw();
        program->release();

        // restore the viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glDepthRange(0.0f, 1.0f);
    }
    void Viewer::pre_draw() {
        glfwMakeContextCurrent(window_);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    void Viewer::post_draw() {
        std::vector<graphics::DrawCommand> text_commands;
        update_hud_text(text_commands);
        const bool has_text = !text_commands.empty();
        if (!hud_overlay_renderer_ && has_text)
            hud_overlay_renderer_.reset(new opengl::OverlayRenderer);
        if (hud_overlay_renderer_) {
            for (auto &command : text_commands)
                command.overlay = true;
            hud_overlay_renderer_->set(std::move(text_commands), true);
            if (has_text) {
                opengl::QueryState state;
                state.prepare(framebuffer_width(), framebuffer_height());
                hud_overlay_renderer_->draw(true, framebuffer_width(), framebuffer_height(), overlay_tile_, {});
            }
        }

        // shown only when it is not animating
        if (show_camera_path_ && !kfi_->is_interpolation_started()) {
            kfi_->draw_path(camera(), 2.0f);
            kfi_->draw_cameras(camera(), camera()->scene_radius() * 0.05f);
        }

        if (show_pivot_point_)
            drawable_pivot_->draw(camera());

        // ------------- draw the picking region with transparency  ---------------

        if (zoom_region_ && (input_.x != input_.press_x || input_.y != input_.press_y)) {
            const Rect rect(static_cast<float>(input_.press_x), static_cast<float>(input_.x),
                            static_cast<float>(input_.press_y), static_cast<float>(input_.y));
            if (rect.width() > 0 || rect.height() > 0) {
                // draw the boundary of the rect
                shape::draw_quad_wire(rect, vec4(0.0f, 0.0f, 1.0f, 1.0f), width(), height(), -1.0f);
                // draw the transparent face
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                shape::draw_quad_filled(rect, vec4(0.0f, 0.0f, 1.0f, 0.2f), width(), height(), -0.9f);
                glDisable(GL_BLEND);
            }
        }

        // ------- draw the axes indicating the orientation of the model  ----------

        draw_corner_axes();
        draw_overlays(true);
    }
    void Viewer::draw() const {
#if 0
        // Example code: split the view to have different rendering styles for the same object/scene.
        glEnable(GL_SCISSOR_TEST);
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glScissor(viewport[0], viewport[1], viewport[2] * 0.5f, viewport[3]);
        for (const auto m : models_) {
            for (auto d : m->renderer()->triangles_drawables())
                d->draw(camera());
        }
        glScissor(viewport[2] * 0.5f, viewport[1], viewport[2] * 0.5f, viewport[3]);
        for (const auto m : models_) {
            for (auto d : m->renderer()->lines_drawables())
                d->draw(camera());
        }
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);

#else

        Shadow *active_shadow = Shadow::active(camera());
        for (const auto m : models_) {
            if (!m->renderer()->is_visible())
                continue;

            // Let's check if edges and surfaces are both shown. If true, we
            // make the depth coordinates of the surface smaller, so that displaying
            // the mesh and the surface together does not cause Z-fighting.
            std::size_t count = 0;
            for (auto d : m->renderer()->lines_drawables()) {
                if (d->is_visible()) {
                    d->draw(camera());
                    easy3d_debug_log_gl_error++ count;
                }
            }

            for (auto d : m->renderer()->points_drawables()) {
                if (d->is_visible())
                    d->draw(camera());
                easy3d_debug_log_gl_error
            }

            if (count > 0) {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(0.5f, -0.0001f);
            }
            for (auto d : m->renderer()->triangles_drawables()) {
                if (!d->is_visible())
                    continue;
                if (active_shadow && std::find(active_shadow->surfaces().begin(), active_shadow->surfaces().end(),
                                               d.get()) != active_shadow->surfaces().end())
                    continue;
                d->draw(camera());
                easy3d_debug_log_gl_error
            }
            if (count > 0)
                glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (active_shadow)
            active_shadow->draw(active_shadow->surfaces());

        for (auto d : drawables_) {
            if (d->is_visible())
                d->draw(camera());
        }

#endif

        // draw the clipping plane (not in postDraw so it can be effective when taking snapshots)
        ClippingPlane::instance()->draw(camera());
        easy3d_debug_log_gl_error draw_overlays(false);
    }
    namespace {
        struct QueryContext {
            GLFWwindow *previous;
            explicit QueryContext(GLFWwindow *window) : previous(glfwGetCurrentContext()) {
                glfwMakeContextCurrent(window);
            }
            ~QueryContext() {
                glfwMakeContextCurrent(previous);
            }
        };
        using opengl::QueryState;
        void query_draw(const Drawable *drawable, const Camera &camera) {
            if (!drawable || !drawable->is_visible() || dynamic_cast<const LinesDrawable2D *>(drawable))
                return;
            // Standard drawable rendering only, matching the native scene encoder.
            // Custom Viewer::draw() and application frame hooks do not run here.
            if (auto *d = dynamic_cast<const TrianglesDrawable *>(drawable))
                d->TrianglesDrawable::draw(&camera);
            else if (auto *d = dynamic_cast<const LinesDrawable *>(drawable))
                d->LinesDrawable::draw(&camera);
            else if (auto *d = dynamic_cast<const PointsDrawable *>(drawable))
                d->PointsDrawable::draw(&camera);
            else
                throw std::invalid_argument("unsupported drawable in scene depth query");
        }
    } // namespace
    const std::vector<vec4> &Viewer::clipping_planes() const {
        selection_planes_.clear();
        const auto *clipping = ClippingPlane::instance();
        if (clipping->is_enabled()) {
            const auto n = clipping->normal(), c = clipping->center();
            selection_planes_.push_back(vec4(-n, dot(n, c)));
            if (clipping->cross_section())
                selection_planes_.push_back(
                    vec4(n, -dot(n, c) + clipping->cross_section_width() * clipping->scene_radius()));
        }
        selection_planes_ = graphics::normalize_clipping_planes(selection_planes_);
        return selection_planes_;
    }
    std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable *drawable) const {
        if (!drawable || drawable->type() != Drawable::DT_POINTS)
            throw std::invalid_argument("selection requires a point drawable");
        std::shared_ptr<Drawable> owner;
        for (const auto &model : models_)
            if (model->renderer())
                for (const auto &d : model->renderer()->points_drawables())
                    if (d.get() == drawable)
                        owner = d;
        for (const auto &d : drawables_)
            if (d.get() == drawable)
                owner = d;
        if (!owner)
            throw std::invalid_argument("point drawable does not belong to this viewer");
        QueryContext context(window_);
        if (GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2) {
            GLint running = 0;
            glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE, &running);
            if (running)
                throw std::logic_error("selection cannot interrupt active transform feedback");
        }
        QueryState state;
        state.prepare(framebuffer_width(), framebuffer_height());
        if (!point_selector_)
            point_selector_.reset(new opengl::PointSelector);
        return point_selector_->points(owner);
    }
    std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest &request) const {
        QueryContext context(window_);
        if (GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2) {
            GLint running = 0;
            glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE, &running);
            if (running)
                throw std::logic_error("selection cannot interrupt active transform feedback");
        }
        QueryState state;
        state.prepare(framebuffer_width(), framebuffer_height());
        if (!point_selector_)
            point_selector_.reset(new opengl::PointSelector);
        return point_selector_->select(request);
    }
    void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground) {
        QueryContext context(window_);
        QueryState state;
        state.prepare(framebuffer_width(), framebuffer_height());
        if (!overlay_renderer_)
            overlay_renderer_.reset(new opengl::OverlayRenderer);
        overlay_renderer_->set(std::move(commands), foreground);
        update();
    }
    void Viewer::draw_overlays(bool foreground) const {
        if (!overlay_renderer_)
            return;
        QueryState state;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        state.prepare(viewport[2], viewport[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        overlay_renderer_->draw(foreground, framebuffer_width(), framebuffer_height(), overlay_tile_,
                                [&](int width, int height) {
                                    QueryState depth_state;
                                    depth_state.prepare(width, height);
                                    Camera snapshot(*camera());
                                    snapshot.frame()->set_pivot_point(camera()->pivot_point());
                                    snapshot.projection_matrix();
                                    snapshot.model_view_matrix();
                                    mat4 projection = camera()->projection_matrix();
                                    if (overlay_tile_.z > 0) {
                                        mat4 full = mat4::identity();
                                        full(0, 0) = float(viewport[2]) / width;
                                        full(1, 1) = float(viewport[3]) / height;
                                        full(0, 3) = float(2 * overlay_tile_.x + viewport[2]) / width - 1;
                                        full(1, 3) = 1 - float(2 * overlay_tile_.y + viewport[3]) / height;
                                        projection = full * projection;
                                    }
                                    snapshot.set_projection_matrix(projection);
                                    snapshot.set_model_view_matrix(camera()->model_view_matrix());
                                    draw_scene(snapshot);
                                });
    }
    graphics::FrameImage Viewer::capture() const {
        return capture(samples());
    }
    graphics::FrameImage Viewer::capture(unsigned count) const {
        if (!count || count > 64 || (count & (count - 1)))
            throw std::invalid_argument("invalid capture sample count");
        QueryContext context(window_);
        QueryState state;
        GLint maximum = 0, extent = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maximum);
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &extent);
        const int w = framebuffer_width(), h = framebuffer_height();
        if (w <= 0 || h <= 0 || w > extent || h > extent || (count > 1 && count > unsigned(maximum)))
            throw std::invalid_argument("unsupported capture dimensions or samples");
        state.prepare(w, h);
        auto *self = const_cast<Viewer *>(this);
        if (!initialized_) {
            self->init();
            self->initialized_ = true;
        }
        Camera snapshot(*camera());
        snapshot.frame()->set_pivot_point(camera()->pivot_point());
        snapshot.projection_matrix();
        snapshot.model_view_matrix();
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        for (unsigned i = 0; i < 16; ++i)
            if (!std::isfinite(snapshot.model_view_projection_matrix()[i]))
                throw std::invalid_argument("nonfinite capture camera");
        graphics::FrameImage frame;
        frame.width = w;
        frame.height = h;
        {
            FramebufferObject color(w, h, count <= 1 ? 0 : int(count));
            if (!color.add_color_buffer() || !color.add_depth_buffer() || !color.bind())
                throw std::runtime_error("cannot create capture color target");
            state.prepare(w, h);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            const auto bg = background_color();
            glClearColor(bg.x, bg.y, bg.z, bg.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            draw_scene(snapshot, nullptr, true);
            glViewport(0, 0, w, h);
            draw_overlays(false);
            if (snapshot_hud_)
                self->Viewer::post_draw();
            else
                draw_overlays(true);
            glDisable(GL_SCISSOR_TEST);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            if (!color.read_color(0, frame.rgba, GL_RGBA, true))
                throw std::runtime_error("cannot read capture color");
        }
        FramebufferObject depth(w, h, 0);
        if (!depth.add_color_buffer() || !depth.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !depth.bind())
            throw std::runtime_error("cannot create capture depth target");
        frame.depth.assign(size_t(w) * h, 1);
        frame.ids.assign(size_t(w) * h, {UINT32_MAX, UINT32_MAX});
        std::vector<float> candidate(frame.depth.size());
        const auto append = [&](const std::shared_ptr<Model> &model, const std::shared_ptr<Drawable> &drawable,
                                uint32_t id) {
            if (!drawable->is_visible() || dynamic_cast<LinesDrawable2D *>(drawable.get()))
                return;
            state.prepare(w, h);
            glClear(GL_DEPTH_BUFFER_BIT);
            DrawSource source{model, drawable};
            draw_scene(snapshot, &source);
            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, candidate.data());
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    const auto p = size_t(y) * w + x;
                    const auto d = candidate[size_t(h - 1 - y) * w + x];
                    if (d >= 0 && d < frame.depth[p]) {
                        frame.depth[p] = d;
                        frame.ids[p] = {id, UINT32_MAX};
                    }
                }
        };
        if (models_.size() + drawables_.size() >= UINT32_MAX)
            throw std::length_error("too many capture objects");
        for (size_t i = 0; i < models_.size(); ++i) {
            const auto &model = models_[i];
            if (!model->renderer())
                continue;
            for (const auto &d : model->renderer()->triangles_drawables())
                append(model, d, uint32_t(i));
            for (const auto &d : model->renderer()->lines_drawables())
                append(model, d, uint32_t(i));
            for (const auto &d : model->renderer()->points_drawables())
                append(model, d, uint32_t(i));
        }
        for (size_t i = 0; i < drawables_.size(); ++i)
            append(nullptr, drawables_[i], uint32_t(models_.size() + i));
        return frame;
    }
    void Viewer::prepare_pick_view(int, int, int width, int height) const {
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, width, height);
    }
    Viewer::PickResult Viewer::pick(int x, int y) const {
        PickResult result;
        int px, py;
        if (!window_pixel(x, y, px, py))
            return result;
        int fw, fh;
        framebuffer_size(fw, fh);
        // Two offscreen passes share one framebuffer:
        //  1. Draw each visible drawable with its normal shaders and keep the nearest
        //     depth at the cursor. This identifies the model/drawable actually seen.
        //  2. For a triangle drawable, redraw it with primitive IDs as colors and read
        //     the same pixel to recover the triangle. Both passes use the same
        //     transform and clipping, so the depth winner and the ID agree.
        // This whole-scene query is intentionally not delegated to the Picker classes:
        // they operate on one caller-supplied model with a borrowed camera and own
        // their offscreen resources, while the viewer already owns every model,
        // drawable and the context, so it can resolve the hit here directly.
        QueryContext context(window_);
        QueryState state;
        // Keep the camera logical (as in the on-screen draw); the framebuffer target is
        // physical, so world-space impostor radii project to the same size as the scene.
        Camera snapshot(*camera());
        snapshot.projection_matrix();
        snapshot.model_view_matrix();
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        state.prepare(fw, fh);
        FramebufferObject target(fw, fh, 0);
        if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid() ||
            !target.bind())
            throw std::runtime_error("cannot create picking framebuffer");
        // Reuse the actual drawable shaders: point impostors, clipping and transforms
        // must agree with scene depth. Keep the renderer's shared ownership intact.
        const auto sample = [&](const std::shared_ptr<Drawable> &drawable, const std::shared_ptr<Model> &model) {
            if (!drawable->is_visible() || dynamic_cast<LinesDrawable2D *>(drawable.get()))
                return;
            state.prepare(fw, fh);
            glClear(GL_DEPTH_BUFFER_BIT);
            DrawSource source{model, drawable};
            draw_scene(snapshot, &source);
            float depth = 1;
            glReadPixels(px, fh - 1 - py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            if (depth >= 0 && depth < result.depth) {
                result.model = model;
                result.drawable = drawable;
                result.depth = depth;
            }
        };
        for (const auto &model : models_) {
            const auto *renderer = model->renderer();
            if (!renderer)
                continue;
            for (const auto &d : renderer->triangles_drawables())
                sample(d, model);
            for (const auto &d : renderer->lines_drawables())
                sample(d, model);
            for (const auto &d : renderer->points_drawables())
                sample(d, model);
        }
        for (const auto &d : drawables_)
            sample(d, nullptr);
        if (!result)
            return result;
        const auto world = [&](float depth) { return pixel_to_world(px, py, depth); };
        result.position = world(result.depth);
        if (result.drawable->type() == Drawable::DT_TRIANGLES) {
            // Recover the primitive from the same rasterized surface: the selection
            // shader applies the drawable transform and clipping, so the depth winner
            // and the reported primitive agree.
            const std::string name = "selection/selection_single_primitive";
            auto *program = ShaderManager::get_program(name);
            if (!program)
                program = ShaderManager::create_program_from_files(name, {{ShaderProgram::POSITION, "vtx_position"}});
            if (!program)
                throw std::runtime_error("picking requires the selection shader '" + name + "'");
            state.prepare(fw, fh);
            prepare_pick_view(px, py, fw, fh);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            program->bind();
            const mat4 MANIP = result.drawable->manipulated_matrix();
            program->set_uniform("MANIP", MANIP)
                ->set_uniform("MVP_MANIP", snapshot.model_view_projection_matrix() * MANIP);
            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, result.drawable->plane_clip_discard_primitive());
            result.drawable->gl_draw();
            program->release();
            unsigned char rgba[4] = {255, 255, 255, 255};
            glReadPixels(px, fh - 1 - py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
            const int id = color::encode(rgba[0], rgba[1], rgba[2], rgba[3]);
            if (id >= 0) {
                result.primitive = uint32_t(id);
                auto *mesh = dynamic_cast<SurfaceMesh *>(result.model.get());
                if (mesh && result.model->renderer()->get_triangles_drawable("faces", false) == result.drawable.get()) {
                    const auto ranges = mesh->get_face_property<std::pair<int, int>>("f:triangle_range");
                    if (ranges)
                        for (auto face : mesh->faces()) {
                            const auto range = ranges[face];
                            if (range.first >= 0 && result.primitive >= uint32_t(range.first) &&
                                result.primitive <= uint32_t(range.second)) {
                                result.face_index = face.idx();
                                break;
                            }
                        }
                }
            }
        }
        return result;
    }
    bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
        // x/y are framebuffer pixels; the camera carries logical screen dimensions.
        // Render into a framebuffer-sized target so impostor radii (world-space, from
        // the logical camera) project to the same physical size as the on-screen scene.
        const int fw = framebuffer_width(), fh = framebuffer_height();
        QueryContext context(window_);
        QueryState state;
        state.prepare(fw, fh);
        FramebufferObject target(fw, fh, 0);
        if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid())
            throw std::runtime_error("cannot create single-sample scene depth framebuffer");
        if (!target.bind())
            throw std::runtime_error("cannot bind scene depth framebuffer");
        glClear(GL_DEPTH_BUFFER_BIT);
        draw_scene(snapshot);
        glReadPixels(x, fh - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        return true;
    }
    void Viewer::draw_scene(const Camera &snapshot, const DrawSource *source, bool guides) const {
        if (source) {
            if (source->model && (!source->model->renderer() || !source->model->renderer()->is_visible()))
                return;
            query_draw(source->drawable.get(), snapshot);
            return;
        }
        for (const auto &model : models_) {
            const auto *renderer = model->renderer();
            if (!renderer || !renderer->is_visible())
                continue;
            for (const auto &d : renderer->triangles_drawables())
                query_draw(d.get(), snapshot);
            for (const auto &d : renderer->lines_drawables())
                query_draw(d.get(), snapshot);
            for (const auto &d : renderer->points_drawables())
                query_draw(d.get(), snapshot);
        }
        for (const auto &drawable : drawables_)
            query_draw(drawable.get(), snapshot);
        if (guides) {
            ClippingPlane::instance()->draw(&snapshot);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            for (const auto &d : drawables_)
                if (d->is_visible())
                    if (auto *flat = dynamic_cast<LinesDrawable2D *>(d.get()))
                        flat->draw(&snapshot);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }
    }
} // namespace easy3d
#endif // !EASY3D_HAS_VULKAN
