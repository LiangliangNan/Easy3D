// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/backends/viewer.h>
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
#include <easy3d/viewer/backends/glfw_runtime.h>
#endif
#include <cmath>
#include <limits>
namespace easy3d {
ViewerBase::~ViewerBase() = default;
namespace {
std::unique_ptr<ViewerBase> create_backend(Backend backend, const std::string& title, const ViewerOptions& o) {
    switch (backend) {
    case Backend::Vulkan:
#ifdef EASY3D_HAS_VULKAN_VIEWER
        return detail::create_vulkan_viewer(title, o);
#else
        throw BackendUnavailable("Vulkan window support was not built");
#endif
    case Backend::OpenGL:
#ifndef EASY3D_NO_OPENGL
        return detail::create_opengl_viewer(title, o);
#else
        throw BackendUnavailable("OpenGL window support was not built");
#endif
    default: throw std::invalid_argument("invalid viewer backend");
    }
}
}
std::unique_ptr<ViewerBase> ViewerBase::create(const std::string& title, const ViewerOptions& options) {
    if (options.width <= 0 || options.height <= 0)
        throw std::invalid_argument("viewer dimensions must be positive");
    if (!options.samples || options.samples > 64 || (options.samples & (options.samples - 1)))
        throw std::invalid_argument("viewer samples must be a power of two between 1 and 64");
    std::unique_ptr<ViewerBase> host;
    std::string fallback;
    if (options.backend == Backend::Automatic) {
        try { host = create_backend(Backend::Vulkan, title, options); }
        catch (const BackendUnavailable& e) {
            fallback = e.what();
            try { host = create_backend(Backend::OpenGL, title, options); }
            catch (const BackendUnavailable& gl) {
                throw BackendUnavailable("No interactive backend available. Vulkan: " + fallback + "; OpenGL: " + gl.what());
            }
        }
    } else host = create_backend(options.backend, title, options);
    host->fallback_reason_ = std::move(fallback);
    // Match the original Viewer defaults before applications configure a scene.
    auto* camera = host->camera();
    camera->setType(Camera::PERSPECTIVE);
    camera->setUpVector(vec3(0, 0, 1));
    camera->setViewDirection(vec3(-1, 0, 0));
    camera->showEntireScene();
    if (options.visible) host->show();
    LOG(INFO) << "Viewer backend: " << (host->backend() == Backend::Vulkan ? "Vulkan" : "OpenGL")
              << (host->fallback_reason_.empty() ? "" : "; Vulkan unavailable: " + host->fallback_reason_);
    return host;
}
void ViewerBase::poll_events() {
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
    detail::GlfwRuntime::poll_events();
#else
    throw BackendUnavailable("Interactive window support was not built");
#endif
}
bool ViewerBase::snapshot(const std::string& filename, float scaling, int sample_count, int background, bool expand) const {
    if (!std::isfinite(scaling) || scaling <= 0 || sample_count <= 0 || sample_count > 64 ||
        (sample_count & (sample_count - 1)) || background < 0 || background > 2 ||
        double(framebuffer_width()) * scaling > std::numeric_limits<int>::max() ||
        double(framebuffer_height()) * scaling > std::numeric_limits<int>::max()) return false;
    return snapshot_file(filename, scaling, sample_count, background, expand);
}
void ViewerBase::set_callbacks(ViewerCallbacks callbacks) {
    auto replacement = std::make_shared<ViewerCallbacks>(std::move(callbacks));
    viewer_size(resize_width_, resize_height_);
    framebuffer_size(resize_fb_width_, resize_fb_height_);
    callbacks_ = std::move(replacement);
}
void ViewerBase::notify_pre_draw() {
    const auto callbacks = callbacks_;
    if (callbacks && callbacks->pre_draw) callbacks->pre_draw(this);
}
void ViewerBase::notify_post_draw() {
    const auto callbacks = callbacks_;
    if (callbacks && callbacks->post_draw) callbacks->post_draw(this);
}
void ViewerBase::notify_resize() {
    int w, h, fw, fh; viewer_size(w, h); framebuffer_size(fw, fh);
    if (w == resize_width_ && h == resize_height_ && fw == resize_fb_width_ && fh == resize_fb_height_) return;
    resize_width_ = w; resize_height_ = h; resize_fb_width_ = fw; resize_fb_height_ = fh;
    const auto callbacks = callbacks_;
    if (callbacks && callbacks->post_resize) callbacks->post_resize(this, w, h);
}
bool ViewerBase::dispatch_mouse_press(int x, int y, int button, int modifiers) {
    const auto callbacks = callbacks_;
    const bool handled = callbacks && callbacks->mouse_press_event && callbacks->mouse_press_event(this, x, y, button, modifiers);
    callback_button_ = button; callback_capture_ = handled;
    return handled;
}
bool ViewerBase::dispatch_mouse_release(int x, int y, int button, int modifiers) {
    const bool captured = callback_capture_ && callback_button_ == button;
    if (callback_button_ == button) { callback_button_ = -1; callback_capture_ = false; }
    const auto callbacks = callbacks_;
    const bool handled = callbacks && callbacks->mouse_release_event && callbacks->mouse_release_event(this, x, y, button, modifiers);
    return handled || captured;
}
bool ViewerBase::dispatch_mouse_drag(int x, int y, int dx, int dy, int button, int modifiers) {
    const auto callbacks = callbacks_;
    const bool handled = callbacks && callbacks->mouse_drag_event && callbacks->mouse_drag_event(this, x, y, dx, dy, button, modifiers);
    return handled || (callback_capture_ && callback_button_ == button);
}
bool ViewerBase::dispatch_mouse_move(int x, int y, int dx, int dy, int modifiers) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->mouse_free_move_event && callbacks->mouse_free_move_event(this, x, y, dx, dy, modifiers);
}
bool ViewerBase::dispatch_mouse_scroll(int x, int y, int dx, int dy) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->mouse_scroll_event && callbacks->mouse_scroll_event(this, x, y, dx, dy);
}
bool ViewerBase::dispatch_key_press(int key, int modifiers) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->key_press_event && callbacks->key_press_event(this, key, modifiers);
}
bool ViewerBase::dispatch_key_release(int key, int modifiers) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->key_release_event && callbacks->key_release_event(this, key, modifiers);
}
bool ViewerBase::dispatch_character(unsigned int code) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->char_input_event && callbacks->char_input_event(this, code);
}
bool ViewerBase::dispatch_drop(const std::vector<std::string>& files) {
    const auto callbacks = callbacks_;
    return callbacks && callbacks->drop_event && callbacks->drop_event(this, files);
}
bool ViewerBase::dispatch_focus(bool focused) {
    if (!focused) { callback_button_ = -1; callback_capture_ = false; }
    const auto callbacks = callbacks_;
    return callbacks && callbacks->focus_event && callbacks->focus_event(this, focused);
}

}
