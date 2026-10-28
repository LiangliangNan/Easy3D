// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VULKAN_VIEWER_H
#define EASY3D_VIEWER_VULKAN_VIEWER_H
#include <easy3d/viewer/vulkan_offscreen.h>
#include <functional>
#include <map>
#include <easy3d/viewer/viewer_input.h>
struct GLFWwindow;

namespace easy3d {
struct VulkanViewerOptions {
    int width = 800, height = 600;
    bool visible = true, resizable = true;
    unsigned samples = 1;
    graphics::DeviceOptions device;
};

// Native Vulkan counterpart of Viewer, using GLFW.
// Construct, manipulate and destroy on the UI/main
// thread. Multiple Vulkan viewers share GLFW lifetime, but own independent GPU
// devices/caches. Do not mix with the legacy GLFW Viewer in the same process.
class VulkanViewer : public VulkanOffScreen, public ViewerInput {
public:
    explicit VulkanViewer(const std::string& title = "Easy3D Vulkan Viewer",
                          const VulkanViewerOptions& options = {});
    ~VulkanViewer() override;
    // Matches Viewer::run(): optionally fit the scene, show the window and run
    // until closed. Passing false preserves an explicitly configured camera.
    int run(bool see_all = true);
    // Native bounded execution, preserving camera/visibility. Zero runs until
    // closed; skipped/minimized frames do not consume a positive frame budget.
    int run_frames(unsigned max_frames = 0);
    bool draw_frame();
    // Called on the UI thread once before pre_draw() for each renderable frame.
    // Like Viewer, the bool result is advisory; use set_animation(false) to pause.
    // Frame-based updates are not a fixed-rate simulation clock. Captures alone
    // do not advance animation. Replace/clear the callback between frames;
    // use set_animation(false) from inside it to stop subsequent updates.
    std::function<bool(VulkanViewer*)> animation_func_;
    void set_animation(bool enabled);
    bool is_animating() const;
    static void poll_events();
    void request_close();
    virtual void exit() { request_close(); }
    bool should_close() const;
    void show();
    void hide();
    void resize(int width, int height); // logical window units; camera uses framebuffer pixels
    void framebuffer_size(int& width, int& height) const;
    void set_title(const std::string& title);
    const std::string& title() const;
    void update() const;
    void set_samples(unsigned samples);
    unsigned samples() const;
    graphics::FrameImage capture() const { return VulkanOffScreen::capture(samples()); }
    graphics::FrameImage capture(unsigned samples) const { return VulkanOffScreen::capture(samples); }
    // Matches Viewer::snapshot() file-output parameters. Snapshot MSAA is
    // independent of the window samples; capture() uses the window samples.
    virtual bool open();
    virtual bool save() const;
    virtual bool snapshot() const;
    using Function = std::function<bool(VulkanViewer*, Model*)>;
    void bind(const Function& func, Model* model, Key key, Modifier modifier = MODIF_NONE) {
        commands_[key][modifier] = std::make_pair(func, model);
    }
    void set_usage(const std::string& manual = "default", const std::string& screen_hint = "");
    bool snapshot(const std::string& filename, float scaling = 1.0f, int samples = 4,
                  int back_ground = 1, bool expand = true) const;
    // Top-left origin, framebuffer-pixel coordinates (also used by callbacks).
    // Synchronous capture is performed only when explicitly requested.
    vec3 point_under_pixel(int x, int y, bool& found) const;
    GLFWwindow* window() const; // borrowed native host; do not destroy; never an OpenGL context
protected:
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
    int pressed_key_ = -1, pressed_button_ = -1;
    int mouse_pressed_x_ = 0, mouse_pressed_y_ = 0;
    std::map<Key, std::map<Modifier, std::pair<Function, Model*>>> commands_;
    std::string manual_, hint_;
    void copy_view();
    void paste_view();
    VulkanViewer(std::shared_ptr<Window> window, const graphics::DeviceOptions& options);
};
}
#endif
