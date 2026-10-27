// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VULKAN_VIEWER_H
#define EASY3D_VIEWER_VULKAN_VIEWER_H
#include <easy3d/viewer/vulkan_offscreen.h>
struct GLFWwindow;

namespace easy3d {
struct VulkanViewerOptions {
    int width = 800, height = 600;
    bool visible = true, resizable = true;
    unsigned samples = 1;
    graphics::DeviceOptions device;
};

// Native Vulkan/GLFW host. Construct, manipulate and destroy on the UI/main
// thread. Multiple Vulkan viewers share GLFW lifetime, but own independent GPU
// devices/caches. Do not mix with the legacy GLFW Viewer in the same process.
class VulkanViewer : public VulkanOffScreen {
public:
    explicit VulkanViewer(const std::string& title = "Easy3D Vulkan Viewer",
                          const VulkanViewerOptions& options = {});
    ~VulkanViewer() override;
    // Zero runs until the window closes. Positive values are a finite frame
    // budget for automated demos. Skipped/minimized frames do not use the budget.
    int run(unsigned max_frames = 0);
    bool draw_frame();
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
    bool snapshot(const std::string& filename) const { return render(filename, 1.0f, int(samples())); }
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
    virtual bool key_release_event(int, int) { return false; }
    virtual bool char_input_event(unsigned int) { return false; }
    virtual bool drop_event(const std::vector<std::string>& filenames);
    virtual bool focus_event(bool) { return false; }
private:
    struct Window;
    std::shared_ptr<Window> window_;
    VulkanViewer(std::shared_ptr<Window> window, const graphics::DeviceOptions& options);
};
}
#endif
