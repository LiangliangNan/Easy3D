// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/graphics/vulkan/surface.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <exception>
#include <stdexcept>

namespace easy3d {
namespace {
// Toolkit runtime ownership, not a global GPU cache. GLFW requires UI-thread use.
struct GlfwRuntime {
    GlfwRuntime() { if (!glfwInit()) throw std::runtime_error("GLFW initialization failed (a desktop display is required)"); }
    ~GlfwRuntime() { glfwTerminate(); }
    static std::shared_ptr<GlfwRuntime> acquire() {
        static std::weak_ptr<GlfwRuntime> current;
        auto runtime = current.lock();
        if (!runtime) { runtime = std::make_shared<GlfwRuntime>(); current = runtime; }
        return runtime;
    }
};
}
struct VulkanViewer::Window {
    std::shared_ptr<GlfwRuntime> runtime;
    GLFWwindow* handle = nullptr;
    std::string title;
    int width, height;
    int button = -1, last_x = 0, last_y = 0;
    double wheel_x = 0, wheel_y = 0;
    std::exception_ptr error;
    Window(const std::string& text, const VulkanViewerOptions& options)
        : title(text), width(options.width), height(options.height) {
        if (width <= 0 || height <= 0) throw std::invalid_argument("window dimensions must be positive");
        runtime = GlfwRuntime::acquire();
        if (!glfwVulkanSupported()) throw std::runtime_error("GLFW could not find a usable Vulkan loader");
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, options.visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, options.resizable ? GLFW_TRUE : GLFW_FALSE);
        handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!handle) throw std::runtime_error("could not create a GLFW Vulkan window");
    }
    ~Window() { if (handle) glfwDestroyWindow(handle); }
    std::unique_ptr<graphics::RenderDevice> device(const graphics::DeviceOptions& options) {
        uint32_t count = 0;
        const char** required = glfwGetRequiredInstanceExtensions(&count);
        if (!required || !count) throw std::runtime_error("GLFW has no Vulkan surface extensions for this platform");
        graphics::VulkanSurface surface;
        for (uint32_t i = 0; i < count; ++i) surface.instance_extensions.emplace_back(required[i]);
        surface.create = [this](VkInstance instance, VkSurfaceKHR* result) {
            return glfwCreateWindowSurface(instance, handle, nullptr, result);
        };
        return graphics::create_vulkan_window_device(surface, options);
    }
    void position(double px, double py, int& x, int& y) const {
        int ww = 0, wh = 0, fw = 0, fh = 0;
        glfwGetWindowSize(handle, &ww, &wh); glfwGetFramebufferSize(handle, &fw, &fh);
        x = ww > 0 ? int(std::lround(px * fw / ww)) : 0;
        y = wh > 0 ? int(std::lround(py * fh / wh)) : 0;
    }
    void position(int& x, int& y) const {
        double px = 0, py = 0;
        glfwGetCursorPos(handle, &px, &py); position(px, py, x, y);
    }
    int modifiers() const {
        int result = 0;
        if (glfwGetKey(handle, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(handle, GLFW_KEY_RIGHT_SHIFT)) result |= GLFW_MOD_SHIFT;
        if (glfwGetKey(handle, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(handle, GLFW_KEY_RIGHT_CONTROL)) result |= GLFW_MOD_CONTROL;
        if (glfwGetKey(handle, GLFW_KEY_LEFT_ALT) || glfwGetKey(handle, GLFW_KEY_RIGHT_ALT)) result |= GLFW_MOD_ALT;
        if (glfwGetKey(handle, GLFW_KEY_LEFT_SUPER) || glfwGetKey(handle, GLFW_KEY_RIGHT_SUPER)) result |= GLFW_MOD_SUPER;
        return result;
    }
    template<class Function> static void dispatch(GLFWwindow* handle, const Function& call) noexcept {
        auto* viewer = static_cast<VulkanViewer*>(glfwGetWindowUserPointer(handle));
        if (!viewer) return;
        try { call(*viewer); }
        catch (...) { viewer->window_->error = std::current_exception(); glfwSetWindowShouldClose(handle, GLFW_TRUE); }
    }
    void callbacks(VulkanViewer* viewer) {
        glfwSetWindowUserPointer(handle, viewer);
        glfwSetCursorPosCallback(handle, [](GLFWwindow* w, double px, double py) { dispatch(w, [&](VulkanViewer& v) {
            int x, y; auto& host = *v.window_; host.position(px, py, x, y);
            const int dx = x - host.last_x, dy = y - host.last_y;
            host.last_x = x; host.last_y = y;
            if (host.button >= 0) v.mouse_drag_event(x, y, dx, dy, host.button, host.modifiers());
            else v.mouse_free_move_event(x, y, dx, dy, host.modifiers());
        }); });
        glfwSetMouseButtonCallback(handle, [](GLFWwindow* w, int button, int action, int mods) { dispatch(w, [&](VulkanViewer& v) {
            int x, y; auto& host = *v.window_; host.position(x, y); host.last_x = x; host.last_y = y;
            if (action == GLFW_PRESS) { host.button = button; v.mouse_press_event(x, y, button, mods); }
            else { v.mouse_release_event(x, y, button, mods); if (host.button == button) host.button = -1; }
        }); });
        glfwSetScrollCallback(handle, [](GLFWwindow* w, double dx, double dy) { dispatch(w, [&](VulkanViewer& v) {
            auto& host = *v.window_; host.wheel_x += dx; host.wheel_y += dy;
            const int sx = int(host.wheel_x), sy = int(host.wheel_y);
            host.wheel_x -= sx; host.wheel_y -= sy;
            int x, y; host.position(x, y);
            if (sx || sy) v.mouse_scroll_event(x, y, sx, sy);
        }); });
        glfwSetKeyCallback(handle, [](GLFWwindow* w, int key, int, int action, int mods) { dispatch(w, [&](VulkanViewer& v) {
            if (action == GLFW_RELEASE) v.key_release_event(key, mods); else v.key_press_event(key, mods);
        }); });
        glfwSetCharCallback(handle, [](GLFWwindow* w, unsigned int code) { dispatch(w, [&](VulkanViewer& v) { v.char_input_event(code); }); });
        glfwSetDropCallback(handle, [](GLFWwindow* w, int count, const char** files) { dispatch(w, [&](VulkanViewer& v) {
            std::vector<std::string> names;
            for (int i = 0; i < count; ++i) names.emplace_back(files[i]);
            v.drop_event(names);
        }); });
        glfwSetWindowFocusCallback(handle, [](GLFWwindow* w, int focused) { dispatch(w, [&](VulkanViewer& v) {
            if (!focused) { v.window_->button = -1; v.camera()->frame()->action_end(); }
            v.focus_event(focused != 0);
        }); });
        glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* w, int width, int height) { dispatch(w, [&](VulkanViewer& v) {
            if (width > 0 && height > 0) { v.VulkanOffScreen::resize(width, height); v.post_resize(width, height); }
        }); });
    }
};
VulkanViewer::VulkanViewer(const std::string& title, const VulkanViewerOptions& options)
    : VulkanViewer(std::make_shared<Window>(title, options), options.device) {}
VulkanViewer::VulkanViewer(std::shared_ptr<Window> host, const graphics::DeviceOptions& options)
    : VulkanOffScreen(host->width, host->height, host->device(options)), window_(std::move(host)) {
    window_->callbacks(this);
    int w, h; framebuffer_size(w, h);
    if (w > 0 && h > 0) VulkanOffScreen::resize(w, h);
}
VulkanViewer::~VulkanViewer() {
    // Base classes are destroyed after members. Release the surface/device now,
    // while the member-owned GLFW window (and its Metal layer on macOS) still live.
    glfwSetWindowUserPointer(window_->handle, nullptr);
    release_device();
}
void VulkanViewer::poll_events() { glfwPollEvents(); }
void VulkanViewer::request_close() { glfwSetWindowShouldClose(window_->handle, GLFW_TRUE); }
bool VulkanViewer::should_close() const { return glfwWindowShouldClose(window_->handle) != 0; }
void VulkanViewer::show() { glfwShowWindow(window_->handle); }
void VulkanViewer::hide() { glfwHideWindow(window_->handle); }
void VulkanViewer::resize(int w, int h) {
    if (w <= 0 || h <= 0) throw std::invalid_argument("window dimensions must be positive");
    glfwSetWindowSize(window_->handle, w, h);
}
void VulkanViewer::framebuffer_size(int& w, int& h) const { glfwGetFramebufferSize(window_->handle, &w, &h); }
void VulkanViewer::set_title(const std::string& text) { window_->title = text; glfwSetWindowTitle(window_->handle, text.c_str()); }
const std::string& VulkanViewer::title() const { return window_->title; }
void VulkanViewer::update() const { glfwPostEmptyEvent(); }
GLFWwindow* VulkanViewer::window() const { return window_->handle; }
bool VulkanViewer::draw_frame() {
    if (window_->error) std::rethrow_exception(window_->error);
    if (should_close() || glfwGetWindowAttrib(window_->handle, GLFW_ICONIFIED)) return false;
    int w, h; framebuffer_size(w, h);
    if (w <= 0 || h <= 0) return false;
    if (width() != w || height() != h) { VulkanOffScreen::resize(w, h); post_resize(w, h); }
    pre_draw();
    const bool drawn = present();
    if (drawn) post_draw();
    return drawn;
}
int VulkanViewer::run(unsigned max_frames) {
    try {
        if (window_->error) std::rethrow_exception(window_->error);
        unsigned frames = 0;
        while (!should_close()) {
            poll_events();
            if (window_->error) std::rethrow_exception(window_->error);
            if (draw_frame()) { if (max_frames && ++frames >= max_frames) break; }
            else glfwWaitEventsTimeout(0.05);
        }
        return 0;
    } catch (const std::exception& e) { LOG(ERROR) << "Vulkan viewer failed: " << e.what(); return 1; }
}
vec3 VulkanViewer::point_under_pixel(int x, int y, bool& found) const {
    found = false;
    if (x < 0 || y < 0 || x >= width() || y >= height()) return vec3(0);
    const auto frame = capture();
    const auto pixel = size_t(y) * frame.width + size_t(x);
    if (frame.depth[pixel] >= 1 || frame.ids[pixel].object == UINT32_MAX) return vec3(0);
    found = true;
    return camera()->unprojectedCoordinatesOf(vec3(float(x) + 0.5f, float(y) + 0.5f, frame.depth[pixel]));
}
bool VulkanViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && (modifiers & GLFW_MOD_SHIFT)) {
        bool found = false;
        const auto point = point_under_pixel(x, y, found);
        if (found) camera()->setPivotPoint(point);
    }
    camera()->frame()->action_start(); return false;
}
bool VulkanViewer::mouse_release_event(int, int, int, int) { camera()->frame()->action_end(); return false; }
bool VulkanViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) camera()->frame()->action_rotate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
    else if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE)
        camera()->frame()->action_translate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
    return true;
}
bool VulkanViewer::mouse_scroll_event(int, int, int, int dy) { camera()->frame()->action_zoom(dy, camera()); return true; }
bool VulkanViewer::key_press_event(int key, int) {
    if (key == GLFW_KEY_ESCAPE) { request_close(); return true; }
    if (key == GLFW_KEY_F) { fit_screen(); return true; }
    if (key == GLFW_KEY_P) {
        camera()->setType(camera()->type() == Camera::PERSPECTIVE ? Camera::ORTHOGRAPHIC : Camera::PERSPECTIVE);
        return true;
    }
    return false;
}
bool VulkanViewer::drop_event(const std::vector<std::string>& names) {
    for (const auto& file : names) add_model(file);
    return true;
}
}
