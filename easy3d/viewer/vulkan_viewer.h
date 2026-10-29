// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VULKAN_VIEWER_H
#define EASY3D_VIEWER_VULKAN_VIEWER_H
#include <easy3d/viewer/vulkan_offscreen.h>
#include <functional>
#include <chrono>
#include <map>
#include <easy3d/viewer/viewer_base.h>
struct GLFWwindow;

namespace easy3d {
namespace graphics { class TextLayoutEngine; }
namespace vulkan { class KeyFrameInterpolatorRenderer; }
class KeyFrameInterpolator;
struct VulkanViewerOptions {
    int width = 800, height = 600;
    bool visible = true, resizable = true;
    unsigned samples = 1;
    graphics::DeviceOptions device;
};

// Native Vulkan counterpart of Viewer, using GLFW.
// Construct, manipulate and destroy on the UI/main
// thread. Multiple Vulkan viewers share GLFW lifetime, but own independent GPU
// devices/caches. GLFW lifetime is shared with OpenGL Viewer hosts.
class VulkanViewer : public VulkanOffScreen, public ViewerBase {
public:
    explicit VulkanViewer(const std::string& title = "Easy3D Vulkan Viewer",
                          const VulkanViewerOptions& options = {});
    ~VulkanViewer() override;
    Backend backend() const override { return Backend::Vulkan; }
    unsigned validation_errors() const override { return device().validation_errors(); }
    Camera* camera() override { return VulkanOffScreen::camera(); }
    const Camera* camera() const override { return VulkanOffScreen::camera(); }
    // Explicit native hosts keep framebuffer-pixel dimensions for compatibility.
    int width() const { return VulkanOffScreen::width(); }
    int height() const { return VulkanOffScreen::height(); }
    void set_background_color(const vec4& color) override { VulkanOffScreen::set_background_color(color); }
    const vec4& background_color() const override { return VulkanOffScreen::background_color(); }
    using VulkanOffScreen::add_model;
    Model* add_model(const std::string& file, bool create = true) override { return VulkanOffScreen::add_model(file, create); }
    Model* add_model(std::shared_ptr<Model> model, bool create = true) override { return VulkanOffScreen::add_model(std::move(model), create); }
    Model* add_model(Model* model, bool create = true) override { return VulkanOffScreen::add_model(model, create); }
    bool delete_model(Model* model) override { return VulkanOffScreen::delete_model(model); }
    const std::vector<std::shared_ptr<Model>>& models() const override { return VulkanOffScreen::models(); }
    Model* current_model() const override { return VulkanOffScreen::current_model(); }
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override { return VulkanOffScreen::add_drawable(std::move(drawable)); }
    Drawable* add_drawable(Drawable* drawable) override;
    bool delete_drawable(Drawable* drawable) override { return VulkanOffScreen::delete_drawable(drawable); }
    const std::vector<std::shared_ptr<Drawable>>& drawables() const override { return VulkanOffScreen::drawables(); }
    void fit_screen(const Model* model = nullptr) override { VulkanOffScreen::fit_screen(model); }
    void set_animation_callback(const ViewerBase::AnimationFunction& callback) override {
        if (callback) animation_func_ = [callback](VulkanViewer* viewer) { return callback(viewer); };
        else animation_func_ = nullptr;
    }
    // Matches Viewer::run(): optionally fit the scene, show the window and run
    // until closed. Passing false preserves an explicitly configured camera.
    int run(bool see_all = true) override;
    // Native bounded execution, preserving camera/visibility. Zero runs until
    // closed; skipped/minimized frames do not consume a positive frame budget.
    int run_frames(unsigned max_frames = 0) override;
    bool draw_frame() override;
    // Called on the UI thread once before pre_draw() for each renderable frame.
    // Like Viewer, the bool result is advisory; use set_animation(false) to pause.
    // Frame-based updates are not a fixed-rate simulation clock. Captures alone
    // do not advance animation. Replace/clear the callback between frames;
    // use set_animation(false) from inside it to stop subsequent updates.
    std::function<bool(VulkanViewer*)> animation_func_;
    void set_animation(bool enabled) override;
    bool is_animating() const override;
    static void poll_events();
    void request_close() override;
    virtual void exit() override { request_close(); }
    bool should_close() const override;
    void show() override;
    void hide() override;
    void resize(int width, int height) override; // logical window units; camera uses framebuffer pixels
    void framebuffer_size(int& width, int& height) const override;
    int framebuffer_width() const override;
    int framebuffer_height() const override;
    void viewer_size(int& width, int& height) const override; // logical window units
    void set_position(int xpos, int ypos) override;
    float dpi_scaling() const override; // framebuffer pixels per logical window unit
    void set_title(const std::string& title) override;
    const std::string& title() const override;
    void update() const override;
    void set_samples(unsigned samples);
    unsigned samples() const;
    graphics::FrameImage capture() const;
    graphics::FrameImage capture(unsigned samples) const;
    void clear_scene() override;
    // Matches Viewer::snapshot() file-output parameters. Snapshot MSAA is
    // independent of the window samples; capture() uses the window samples.
    virtual bool open() override;
    virtual bool save() const override;
    virtual bool snapshot() const override;
    using Function = std::function<bool(VulkanViewer*, Model*)>;
    void bind(const Function& func, Model* model, Key key, Modifier modifier = MODIF_NONE) {
        commands_[key][modifier] = std::make_pair(func, model);
    }
    void set_usage(const std::string& manual = "default", const std::string& screen_hint = "") override;
    bool snapshot(const std::string& filename, float scaling = 1.0f, int samples = 4,
                  int back_ground = 1, bool expand = true) const;
    // Top-left origin, framebuffer-pixel coordinates (also used by callbacks).
    // Synchronous capture is performed only when explicitly requested.
    vec3 point_under_pixel(int x, int y, bool& found) const;
    GLFWwindow* window() const override; // borrowed native host; do not destroy; never an OpenGL context
protected:
    void bind_command(const ViewerBase::Function& func, Model* model, Key key, Modifier modifier) override {
        bind(Function([func](VulkanViewer* viewer, Model* m) { return func(viewer, m); }), model, key, modifier);
    }
    unsigned window_samples() const override { return samples(); }
    bool snapshot_file(const std::string& f, float scale, int samples, int bg, bool expand) const override {
        return snapshot(f, scale, samples, bg, expand);
    }
    // Same customization points as Viewer: printed at startup/F1 and drawn as a HUD.
    std::string manual_, hint_;
    int pressed_key_ = -1;
    // Camera-path state corresponds to Viewer's kfi_ and show_camera_path_.
    std::unique_ptr<KeyFrameInterpolator> kfi_;
    bool show_camera_path_ = false;
    bool show_frame_rate_ = false;
    bool show_corner_axes_ = true;
    bool show_easy3d_logo_ = true;
    // Corresponds to Viewer::draw_corner_axes(); uses an isolated HUD depth buffer.
    void draw_corner_axes() const;
    std::string framerate_ = "fps: -- (-- ms/frame)";
    virtual void pre_draw() {}
    virtual void post_draw() {}
    virtual void post_resize(int, int) {}
    virtual bool mouse_press_event(int x, int y, int button, int modifiers);
    virtual bool mouse_release_event(int x, int y, int button, int modifiers);
    virtual bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers);
    virtual bool mouse_free_move_event(int, int, int, int, int) { return false; }
    virtual bool mouse_scroll_event(int x, int y, int dx, int dy);
    virtual bool key_press_event(int key, int modifiers);
    virtual bool key_release_event(int key, int modifiers);
    virtual bool char_input_event(unsigned int) { return false; }
    virtual bool drop_event(const std::vector<std::string>& filenames);
    virtual bool focus_event(bool) { return false; }
private:
    struct Window;
    std::shared_ptr<Window> window_;
    bool animation_enabled_ = false;
    std::unique_ptr<vulkan::KeyFrameInterpolatorRenderer> camera_path_renderer_;
    std::chrono::steady_clock::time_point next_camera_path_frame_;
    void advance_camera_path();
    void update_camera_path_overlays() const;
    void restore_scene_bounds();
    int pressed_button_ = -1;
    int mouse_pressed_x_ = 0, mouse_pressed_y_ = 0;
    int mouse_current_x_ = 0, mouse_current_y_ = 0;
    bool show_pivot_point_ = false, zoom_region_ = false;
    vec2 pivot_pixel_ = vec2(0);
    std::chrono::steady_clock::time_point pivot_expires_;
    mutable bool pivot_hint_visible_ = false;
    mutable int hint_width_ = 0, hint_height_ = 0;
    void update_interaction_overlays(bool force = false) const;
    void update_frame_rate();
    void ensure_text_renderer() const;
    std::chrono::steady_clock::time_point frame_rate_start_;
    unsigned frame_rate_frames_ = 0;
    mutable bool rendered_show_frame_rate_ = false, rendered_show_logo_ = false;
    mutable std::vector<graphics::DrawCommand> logo_commands_;
    mutable std::string rendered_frame_rate_;
    mutable std::vector<graphics::DrawCommand> frame_rate_commands_;
    mutable std::shared_ptr<const graphics::Geometry> corner_axes_geometry_;
    void cancel_interaction();
    std::map<Key, std::map<Modifier, std::pair<Function, Model*>>> commands_;
    mutable std::unique_ptr<graphics::TextLayoutEngine> texter_;
    mutable bool fonts_tried_ = false;
    mutable std::string rendered_hint_;
    mutable float hint_dpi_ = 0;
    mutable std::vector<graphics::DrawCommand> hint_commands_;
    bool usage_printed_ = false;
    void print_usage();
    void copy_view();
    void paste_view();
    VulkanViewer(std::shared_ptr<Window> window, const graphics::DeviceOptions& options);
};
}
#endif
