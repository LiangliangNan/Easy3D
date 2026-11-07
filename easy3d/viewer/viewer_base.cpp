// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <cmath>
#include <algorithm>
#include <limits>
namespace easy3d {
ViewerBase::~ViewerBase() = default;

void ViewerBase::poll_events() {
    detail::GlfwRuntime::poll_events();
}

namespace {
void finite_coordinates(const vec3 &point) {
    for (unsigned i = 0; i < 3; ++i)
        if (!std::isfinite(point[i]))
            throw std::invalid_argument("nonfinite viewer coordinates");
}
void finite_matrix(const mat4 &matrix) {
    for (unsigned i = 0; i < 16; ++i)
        if (!std::isfinite(matrix[i]))
            throw std::invalid_argument("nonfinite viewer camera matrix");
}
mat4 checked_inverse(const mat4 &matrix) {
    finite_matrix(matrix);
    const auto det = determinant(matrix);
    if (!std::isfinite(det) || det == 0)
        throw std::invalid_argument("singular viewer camera matrix");
    const auto result = inverse(matrix);
    finite_matrix(result);
    return result;
}
vec3 homogeneous_point(const vec4 &point) {
    if (!std::isfinite(point.w) || point.w == 0)
        throw std::invalid_argument("invalid viewer homogeneous coordinate");
    const vec3 result(point.x / point.w, point.y / point.w, point.z / point.w);
    finite_coordinates(result);
    return result;
}
} // namespace

// Coordinate contract, identical on the OpenGL and Vulkan backends:
//  * World space: the scene's 3D coordinates. The camera's model-view-projection
//    matrix maps world -> clip; these helpers hide that matrix from callers.
//  * Window (viewer) logical coordinates: top-left origin, x right, y down, in
//    logical point units spanning [0,width) x [0,height) from viewer_size().
//    Mouse events and project_point()/unproject_point() use these, so client
//    code is independent of the display's pixel density.
//  * Framebuffer pixels: physical device pixels spanning [0,fw) x [0,fh) from
//    framebuffer_size(). On a HiDPI display fw = width * dpi_scaling(). Only the
//    internal depth read in point_under_pixel() works in these units.
// Depth is always normalized to [0,1] (near=0, far=1) in the public APIs, matching
// Vulkan; OpenGL's native [-1,1] clip depth is remapped here so both agree.
vec3 ViewerBase::project_point(const vec3 &world) const {
    finite_coordinates(world);
    int w, h;
    viewer_size(w, h);
    if (w <= 0 || h <= 0)
        throw std::invalid_argument("viewer projection requires a positive window size");
    const auto matrix = camera()->modelViewProjectionMatrix();
    finite_matrix(matrix);
    const auto ndc = homogeneous_point(matrix * vec4(world, 1));
    const vec3 result((ndc.x * .5f + .5f) * w, (.5f - ndc.y * .5f) * h, ndc.z * .5f + .5f);
    finite_coordinates(result);
    return result;
}
// Inverse of project_point(): window logical coordinates (top-left origin,
// depth in [0,1]) -> world space. See the coordinate contract above.
vec3 ViewerBase::unproject_point(const vec3 &window) const {
    finite_coordinates(window);
    int w, h;
    viewer_size(w, h);
    if (w <= 0 || h <= 0)
        throw std::invalid_argument("viewer unprojection requires a positive window size");
    const auto inv = checked_inverse(camera()->modelViewProjectionMatrix());
    return homogeneous_point(inv * vec4(2 * window.x / w - 1, 1 - 2 * window.y / h, 2 * window.z - 1, 1));
}
// Fresh visible-surface query at a logical window pixel. Maps the logical
// coordinate to its containing framebuffer pixel (DPI-aware), reads the scene
// depth there, and unprojects that pixel's center back to world space. See the
// coordinate contract above; HUD/overlays are excluded by read_scene_depth().
vec3 ViewerBase::point_under_pixel(int x, int y, bool &found) const {
    found = false;
    int w, h, fw, fh;
    viewer_size(w, h);
    framebuffer_size(fw, fh);
    if (w <= 0 || h <= 0 || fw <= 0 || fh <= 0 || x < 0 || y < 0 || x >= w || y >= h)
        return vec3(0);
    const int px = std::min(fw - 1, int(std::floor(double(x) * fw / w)));
    const int py = std::min(fh - 1, int(std::floor(double(y) * fh / h)));
    const mat4 projection = camera()->projectionMatrix(), view = camera()->modelViewMatrix();
    checked_inverse(projection * view);
    // Camera's copy/resize recomputes projection lazily. Prime its caches before
    // installing the exact live matrices, including temporary custom matrices.
    Camera snapshot(*camera());
    snapshot.setScreenWidthAndHeight(fw, fh);
    snapshot.projectionMatrix();
    snapshot.modelViewMatrix();
    snapshot.set_projection_matrix(projection);
    snapshot.set_modelview_matrix(view);
    float depth = 1;
    if (!read_scene_depth(px, py, snapshot, depth) || !std::isfinite(depth) || depth < 0 || depth >= 1)
        return vec3(0);
    const auto world = unproject_point(vec3(float((px+.5)*w/fw),float((py+.5)*h/fh),depth));
    found = true;
    return world;
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
