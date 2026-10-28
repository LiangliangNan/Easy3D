// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/graphics/vulkan/surface.h>
#include <easy3d/graphics/text_renderer.h>
#include <easy3d/renderer/backends/vulkan/shape.h>
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <exception>
#include <stdexcept>

namespace easy3d {
namespace {
const char* const default_manual = "Left drag: rotate; right drag: pan; middle drag/wheel: zoom.\n"
                         "X/Y/O + drag: constrain axes; Shift-click: pivot; Z-click: focus.\n"
                         "Ctrl/Command-left drag: region zoom; arrows: navigate; Ctrl/Command-arrows: pan.\n"
                         "F/C: fit scene/model; P: projection; Space: align; F2: animation.\n"
                         "M: shading; E/V/B/L: edges/vertices/borders/locks; +/-: point size; []: line width.\n"
                         ",/.: previous/next model; Delete: delete model; D: properties.\n"
                         "Ctrl/Command-C/V: copy/paste view; Ctrl/Command-O/S: open/save; S: snapshot.\n"
                         "Alt-K/D: add/delete camera path; Ctrl/Command-K: play/pause; Ctrl/Command-T: show/hide path.\n";
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
    unsigned samples;
    int button = -1, last_x = 0, last_y = 0;
    double wheel_x = 0, wheel_y = 0;
    std::exception_ptr error;
    Window(const std::string& text, const VulkanViewerOptions& options)
        : title(text), width(options.width), height(options.height), samples(options.samples) {
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
            if (!focused) { v.window_->button = -1; v.cancel_interaction(); }
            v.focus_event(focused != 0);
        }); });
        glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* w, int width, int height) { dispatch(w, [&](VulkanViewer& v) {
            if (width > 0 && height > 0) { v.VulkanOffScreen::resize(width, height); v.cancel_interaction(); v.post_resize(width, height); }
        }); });
    }
};
VulkanViewer::VulkanViewer(const std::string& title, const VulkanViewerOptions& options)
    : VulkanViewer(std::make_shared<Window>(title, options), options.device) {}
VulkanViewer::VulkanViewer(std::shared_ptr<Window> host, const graphics::DeviceOptions& options)
    : VulkanOffScreen(host->width, host->height, host->device(options)), window_(std::move(host)) {
    try {
        manual_ = default_manual;
        kfi_.reset(new KeyFrameInterpolator(camera()->frame()));
        camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
        set_samples(window_->samples);
        window_->callbacks(this);
        int w, h; framebuffer_size(w, h);
        if (w > 0 && h > 0) VulkanOffScreen::resize(w, h);
    } catch (...) {
        // A failed derived constructor destroys members before base classes.
        // Release the Vulkan surface before member-owned native-window cleanup.
        glfwSetWindowUserPointer(window_->handle, nullptr);
        release_device();
        throw;
    }
}
VulkanViewer::~VulkanViewer() {
    if (kfi_) kfi_->stop_interpolation();
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
void VulkanViewer::set_animation(bool enabled) { animation_enabled_ = enabled; update(); }
bool VulkanViewer::is_animating() const { return animation_enabled_ && bool(animation_func_); }
GLFWwindow* VulkanViewer::window() const { return window_->handle; }
void VulkanViewer::set_samples(unsigned samples) {
    if (!samples || samples > 64 || (samples & (samples - 1)) || !(device().supported_sample_counts() & samples))
        throw std::invalid_argument("unsupported Vulkan viewer sample count");
    window_->samples = samples;
}
unsigned VulkanViewer::samples() const { return window_->samples; }
bool VulkanViewer::draw_frame() {
    if (window_->error) std::rethrow_exception(window_->error);
    if (should_close() || glfwGetWindowAttrib(window_->handle, GLFW_ICONIFIED)) return false;
    int w, h; framebuffer_size(w, h);
    if (w <= 0 || h <= 0) return false;
    if (width() != w || height() != h) { VulkanOffScreen::resize(w, h); cancel_interaction(); post_resize(w, h); }
    if (is_animating()) {
        animation_func_(this);
        if (should_close()) return false;
    }
    advance_camera_path();
    print_usage();
    pre_draw();
    update_camera_path_overlays();
    update_interaction_overlays();
    const bool drawn = present(samples());
    if (drawn) post_draw();
    return drawn;
}
void VulkanViewer::advance_camera_path() {
    if (!kfi_->is_interpolation_started()) return;
    const auto now=std::chrono::steady_clock::now();
    if (now<next_camera_path_frame_) return;
    kfi_->advance_interpolation();
    next_camera_path_frame_=now+std::chrono::milliseconds(kfi_->interpolation_period());
}
void VulkanViewer::update_camera_path_overlays() const {
    std::vector<graphics::DrawCommand> commands;
    if (show_camera_path_ && !kfi_->is_interpolation_started()) {
        auto path=camera_path_renderer_->draw_path(*kfi_,*camera());
        auto cameras=camera_path_renderer_->draw_cameras(*kfi_,*camera(),camera()->sceneRadius()*.05f);
        if (path.geometry) commands.push_back(path);
        if (cameras.geometry) commands.push_back(cameras);
    }
    set_viewer_annotations(std::move(commands));
}
void VulkanViewer::restore_scene_bounds() {
    Box3 box;
    for (const auto& model:models()) box.grow(model->bounding_box());
    for (const auto& drawable:drawables()) box.grow(drawable->bounding_box());
    if (box.is_valid()) camera()->setSceneBoundingBox(box.min_point(),box.max_point());
}
void VulkanViewer::update_interaction_overlays(bool force) const {
    const bool pivot = show_pivot_point_ && std::chrono::steady_clock::now()<pivot_expires_;
    int ww=0,wh=0; glfwGetWindowSize(window(),&ww,&wh);
    const float dpi = ww>0 ? float(width())/ww : 1.0f;
    const bool hint_changed = rendered_hint_!=hint_ || hint_dpi_!=dpi || width()!=hint_width_ || height()!=hint_height_;
    if (!force && !hint_changed && pivot==pivot_hint_visible_ && width()==hint_width_ && height()==hint_height_) return;
    if (hint_changed) {
        hint_commands_.clear();
        if (!hint_.empty()) {
            if (!fonts_tried_) {
                fonts_tried_=true;
                try {
                    texter_.reset(new graphics::TextRenderer);
                    texter_->add_font(resource::directory()+"/fonts/en_Roboto-Medium.ttf");
                } catch (const std::exception& e) {
                    texter_.reset();
                    LOG(WARNING) << "Viewer usage font unavailable: " << e.what();
                }
            }
            if (texter_) {
                graphics::TextOptions options;
                options.size=15*dpi; options.x=35*dpi; options.y=22.5f*dpi;
                options.color=vec4(0,0,1,1);
                hint_commands_=texter_->layout(hint_,width(),height(),options).commands;
            }
        }
        rendered_hint_=hint_; hint_dpi_=dpi;
    }
    std::vector<graphics::DrawCommand> commands=hint_commands_;
    if (pivot) {
        const float x=pivot_pixel_.x,y=pivot_pixel_.y,size=10*dpi;
        commands.push_back(vulkan::shape::draw_lines_2d({vec2(x-size,y),vec2(x+size,y),
                         vec2(x,y-size),vec2(x,y+size)},vec4(0,0,0,1),width(),height(),dpi));
    }
    if (zoom_region_ && (mouse_current_x_!=mouse_pressed_x_ || mouse_current_y_!=mouse_pressed_y_)) {
        const Rect rect{float(mouse_pressed_x_),float(mouse_current_x_),float(mouse_pressed_y_),float(mouse_current_y_)};
        commands.push_back(vulkan::shape::draw_quad_filled(rect,vec4(0,0,1,.2f),width(),height()));
        commands.push_back(vulkan::shape::draw_quad_wire(rect,vec4(0,0,1,1),width(),height(),dpi));
    }
    set_viewer_overlays(std::move(commands));
    pivot_hint_visible_=pivot; hint_width_=width(); hint_height_=height();
}
void VulkanViewer::cancel_interaction() {
    pressed_key_ = pressed_button_ = -1;
    show_pivot_point_ = zoom_region_ = false;
    camera()->frame()->action_end();
    update_interaction_overlays(true);
}
void VulkanViewer::clear_scene() {
    VulkanOffScreen::clear_scene();cancel_interaction();update_camera_path_overlays();
}
graphics::FrameImage VulkanViewer::capture() const { return capture(samples()); }
graphics::FrameImage VulkanViewer::capture(unsigned sample_count) const {
    update_camera_path_overlays();
    update_interaction_overlays();
    return VulkanOffScreen::capture(sample_count);
}
bool VulkanViewer::snapshot(const std::string& filename, float scaling, int samples,
                            int back_ground, bool expand) const {
    update_camera_path_overlays();
    update_interaction_overlays();
    return render(filename, scaling, samples, back_ground, expand);
}
int VulkanViewer::run(bool see_all) {
    try {
        if (see_all) fit_screen();
        show();
        return run_frames();
    } catch (const std::exception& e) {
        LOG(ERROR) << "Vulkan viewer failed: " << e.what();
        return 1;
    }
}
int VulkanViewer::run_frames(unsigned max_frames) {
    try {
        if (window_->error) std::rethrow_exception(window_->error);
        print_usage();
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
    return unproject_pixel(x,y,frame.depth[pixel]);
}
bool VulkanViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    mouse_pressed_x_ = mouse_current_x_ = x; mouse_pressed_y_ = mouse_current_y_ = y; pressed_button_ = button;
    zoom_region_ = button == BUTTON_LEFT && modifiers == MODIF_CTRL;
    camera()->frame()->action_start();
    if (pressed_key_ == GLFW_KEY_Z || modifiers == MODIF_SHIFT) {
        if (button == BUTTON_LEFT) {
            bool found = false;
            const auto point = point_under_pixel(x, y, found);
            camera()->setPivotPoint(found ? point : camera()->sceneCenter());
            show_pivot_point_ = found;
            if (found) {
                pivot_pixel_ = vec2(float(x),float(y));
                pivot_expires_ = std::chrono::steady_clock::now()+std::chrono::seconds(10);
            }
            if (found && pressed_key_ == GLFW_KEY_Z && modifiers == 0) {
                // Apply the legacy zoom target directly on the UI thread.
                camera()->setPosition(0.1f*camera()->position()+0.9f*point);
                camera()->lookAt(point);
            }
        } else if (button == BUTTON_RIGHT) {
            camera()->setPivotPoint(camera()->sceneCenter());
            show_pivot_point_ = false;
            if (pressed_key_ == GLFW_KEY_Z && modifiers == 0) camera()->showEntireScene();
        }
    }
    update_interaction_overlays(true);
    return false;
}
bool VulkanViewer::mouse_release_event(int x, int y, int button, int modifiers) {
    if (pressed_button_ == button && button == BUTTON_LEFT && modifiers == MODIF_CTRL &&
        x != mouse_pressed_x_ && y != mouse_pressed_y_)
        camera()->fitScreenRegion(std::min(x,mouse_pressed_x_),std::min(y,mouse_pressed_y_),
                                  std::max(x,mouse_pressed_x_),std::max(y,mouse_pressed_y_));
    camera()->frame()->action_end();
    pressed_button_ = -1;
    zoom_region_ = false;
    update_interaction_overlays(true);
    return false;
}
bool VulkanViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    mouse_current_x_ = x; mouse_current_y_ = y;
    zoom_region_ = pressed_button_ == BUTTON_LEFT && modifiers == MODIF_CTRL;
    update_interaction_overlays(true);
    // Region zoom and clipping/tool manipulation must not move the camera.
    if (modifiers == MODIF_CTRL || modifiers == MODIF_ALT) return false;
    auto axis = ManipulatedFrame::NONE;
    if (pressed_key_ == GLFW_KEY_X) axis = ManipulatedFrame::HORIZONTAL;
    else if (pressed_key_ == GLFW_KEY_Y) axis = ManipulatedFrame::VERTICAL;
    else if (pressed_key_ == GLFW_KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
    if (button == BUTTON_LEFT) camera()->frame()->action_rotate(x,y,dx,dy,camera(),axis);
    else if (button == BUTTON_RIGHT) camera()->frame()->action_translate(x,y,dx,dy,camera(),axis);
    else if (button == BUTTON_MIDDLE && std::abs(dy)>=1) camera()->frame()->action_zoom(dy>0?1:-1,camera());
    return false;
}
bool VulkanViewer::mouse_scroll_event(int, int, int, int dy) { camera()->frame()->action_zoom(dy, camera()); return false; }
bool VulkanViewer::key_press_event(int key, int modifiers) {
    const auto command = commands_.find(Key(key));
    if (command != commands_.end()) {
        const auto binding = command->second.find(Modifier(modifiers));
        if (binding != command->second.end()) return binding->second.first(this,binding->second.second);
    }
    if ((key == GLFW_KEY_ESCAPE && modifiers == 0) || (key == GLFW_KEY_F4 && modifiers == MODIF_ALT)) exit();
    else if (key == GLFW_KEY_F1 && modifiers == 0) {
        if (!manual_.empty()) std::cout << manual_ << '\n';
    } else if (key == GLFW_KEY_F2 && modifiers == 0) set_animation(!animation_enabled_);
    else if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && modifiers == 0)
        camera()->frame()->action_turn((key==GLFW_KEY_LEFT?1.0f:-1.0f)*float(M_PI/180.0),camera());
    else if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && modifiers == 0) {
        const float step=.05f*camera()->sceneRadius();
        camera()->frame()->translate(camera()->frame()->inverseTransformOf(vec3(0,0,key==GLFW_KEY_UP?-step:step)));
    } else if (modifiers == MODIF_CTRL && key >= GLFW_KEY_RIGHT && key <= GLFW_KEY_UP) {
        const float step=.05f*camera()->sceneRadius();
        const vec3 direction=key==GLFW_KEY_LEFT?vec3(-step,0,0):key==GLFW_KEY_RIGHT?vec3(step,0,0):
                             key==GLFW_KEY_UP?vec3(0,step,0):vec3(0,-step,0);
        camera()->frame()->translate(camera()->frame()->inverseTransformOf(direction));
    } else if (modifiers == MODIF_CTRL && (key == GLFW_KEY_MINUS || key == GLFW_KEY_EQUAL))
        camera()->frame()->action_zoom(key==GLFW_KEY_EQUAL?1:-1,camera());
    else if (key == GLFW_KEY_K && modifiers == MODIF_ALT) {
        if (kfi_->add_keyframe(*camera()->frame())) LOG(INFO) << "key frame added to camera path";
    } else if (key == GLFW_KEY_D && modifiers == MODIF_ALT) {
        kfi_->delete_path();
        camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
        set_viewer_annotations({});restore_scene_bounds();update();
    } else if (key == GLFW_KEY_K && modifiers == MODIF_CTRL) {
        if (kfi_->is_interpolation_started()) kfi_->stop_interpolation();
        else {kfi_->start_manual_interpolation();next_camera_path_frame_=std::chrono::steady_clock::now();}
        update();
    } else if (key == GLFW_KEY_T && modifiers == MODIF_CTRL) {
        show_camera_path_=!show_camera_path_;
        if (show_camera_path_) {
            float radius=camera()->sceneRadius();
            for (size_t i=0;i<kfi_->number_of_keyframes();++i)
                radius=std::max(radius,distance(camera()->sceneCenter(),kfi_->keyframe_position(i)));
            camera()->setSceneRadius(radius);
        } else restore_scene_bounds();
        update();
    }
    else if (key == GLFW_KEY_C && modifiers == MODIF_CTRL) copy_view();
    else if (key == GLFW_KEY_V && modifiers == MODIF_CTRL) paste_view();
    else if (key == GLFW_KEY_O && modifiers == MODIF_CTRL) open();
    else if (key == GLFW_KEY_S && modifiers == MODIF_CTRL) save();
    else if (key == GLFW_KEY_S && modifiers == 0) snapshot();
    else if (key == GLFW_KEY_F && modifiers == 0) fit_screen();
    else if (key == GLFW_KEY_C && modifiers == 0) { if (current_model()) fit_screen(current_model()); }
    else if (key == GLFW_KEY_P && modifiers == 0)
        camera()->setType(camera()->type()==Camera::PERSPECTIVE?Camera::ORTHOGRAPHIC:Camera::PERSPECTIVE);
    else if (key == GLFW_KEY_SPACE && modifiers == 0) {
        Frame frame; frame.setTranslation(camera()->pivotPoint()); camera()->frame()->alignWithFrame(&frame,true);
    } else if (key == GLFW_KEY_M && modifiers == 0) {
        if (auto* mesh=dynamic_cast<SurfaceMesh*>(current_model()))
            for (const auto& d:mesh->renderer()->triangles_drawables()) d->set_smooth_shading(!d->smooth_shading());
    } else if (modifiers == 0 && (key==GLFW_KEY_LEFT_BRACKET || key==GLFW_KEY_RIGHT_BRACKET)) {
        for (const auto& m:models()) if (m->renderer()) for (const auto& d:m->renderer()->lines_drawables())
            d->set_line_width(std::max(1.0f,d->line_width()+(key==GLFW_KEY_RIGHT_BRACKET?1:-1)));
    } else if (modifiers == 0 && (key==GLFW_KEY_MINUS || key==GLFW_KEY_EQUAL)) {
        for (const auto& m:models()) if (m->renderer()) for (const auto& d:m->renderer()->points_drawables())
            d->set_point_size(std::max(1.0f,d->point_size()+(key==GLFW_KEY_EQUAL?1:-1)));
    } else if (modifiers == 0 && (key==GLFW_KEY_COMMA || key==GLFW_KEY_PERIOD)) {
        if (!models().empty()) {
            auto i=std::find_if(models().begin(),models().end(),[&](const std::shared_ptr<Model>& m){return m.get()==current_model();});
            const int n=int(models().size()), index=i==models().end()?n-1:int(i-models().begin());
            set_current_model_index((index+n+(key==GLFW_KEY_PERIOD?1:-1))%n); fit_screen(current_model());
        }
    } else if (key==GLFW_KEY_DELETE && modifiers==0) { if (current_model()) delete_model(current_model()); }
    else if (modifiers==0 && current_model() && current_model()->renderer() &&
             (key==GLFW_KEY_E || key==GLFW_KEY_V || key==GLFW_KEY_B || key==GLFW_KEY_L)) {
        auto* renderer=current_model()->renderer();
        Drawable* d=nullptr;
        if (key==GLFW_KEY_E) d=renderer->get_lines_drawable("edges");
        else if (key==GLFW_KEY_V) d=renderer->get_points_drawable("vertices");
        else if (dynamic_cast<SurfaceMesh*>(current_model())) {
            if (key==GLFW_KEY_B) d=renderer->get_lines_drawable("borders");
            else d=renderer->get_points_drawable("locks");
        }
        if (d) d->set_visible(!d->is_visible());
    } else if (key==GLFW_KEY_D && modifiers==0) {
        if (current_model()) current_model()->property_stats(std::cout);
    } else if ((modifiers==MODIF_ALT && (key==GLFW_KEY_C || key==GLFW_KEY_V || key==GLFW_KEY_X)) ||
               (modifiers==MODIF_CTRL && (key==GLFW_KEY_F)) ||
               (modifiers==0 && (key==GLFW_KEY_A || key==GLFW_KEY_R))) {
        LOG(WARNING) << "This viewer shortcut is not yet available in the Vulkan backend";
    } else pressed_key_=key;
    return false;
}
bool VulkanViewer::key_release_event(int key, int) {
    pressed_key_=-1;
    if (key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL ||
        key==GLFW_KEY_LEFT_SUPER || key==GLFW_KEY_RIGHT_SUPER) {
        zoom_region_=false; update_interaction_overlays(true);
    }
    return false;
}
void VulkanViewer::set_usage(const std::string& manual, const std::string& screen_hint) {
    if (manual!="default") manual_=manual;
    hint_=screen_hint;
    update();
}
void VulkanViewer::print_usage() {
    if (usage_printed_) return;
    usage_printed_=true;
    if (!manual_.empty()) std::cout << manual_ << std::endl;
}
void VulkanViewer::copy_view() {
    const auto p=camera()->position(); const auto q=camera()->orientation();
    std::ostringstream out; out << std::setprecision(9) << p.x << ' ' << p.y << ' ' << p.z;
    for (unsigned i=0;i<4;++i) out << ' ' << q[i];
    glfwSetClipboardString(window(),out.str().c_str());
}
void VulkanViewer::paste_view() {
    const char* text=glfwGetClipboardString(window());
    if (!text) return;
    std::istringstream in(text); vec3 p; quat q;
    for (unsigned i=0;i<3;++i) if (!(in>>p[i]) || !std::isfinite(p[i])) return;
    float norm=0;
    for (unsigned i=0;i<4;++i) { if (!(in>>q[i]) || !std::isfinite(q[i])) return; norm+=q[i]*q[i]; }
    std::string extra; if ((in>>extra) || !std::isfinite(norm) || norm<1e-12f) return;
    q.normalize(); camera()->setPosition(p); camera()->setOrientation(q); update();
}
    bool VulkanViewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)", "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)", "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };
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


    bool VulkanViewer::save() const {
        Model* m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm)", "*.obj *.ply *.off *.stl *.sm",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

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


    bool VulkanViewer::snapshot() const {
        const std::string &title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string> &filters = {
                "Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                "All Files (*.*)", "*"
        };

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



bool VulkanViewer::drop_event(const std::vector<std::string>& names) {
    unsigned count=0;
    for (const auto& file : names) if (add_model(file)) ++count;
    if (count) fit_screen();
    return count!=0;
}
}
