// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_BACKENDS_GLFW_RUNTIME_H
#define EASY3D_VIEWER_BACKENDS_GLFW_RUNTIME_H
#include <memory>
#include <string>
#include <vector>
struct GLFWwindow;
namespace easy3d {
namespace graphics { namespace internal { class VulkanLoader; } }
namespace detail {
// UI-thread ownership shared by all Easy3D GLFW hosts, across both backends.
// Hosts destroy their GPU resources and windows before releasing this lease.
class GlfwRuntime {
public:
    static std::shared_ptr<GlfwRuntime> acquire(bool require_vulkan = false);
    ~GlfwRuntime();
    static void poll_events();
    std::shared_ptr<graphics::internal::VulkanLoader> vulkan_loader() const { return loader_; }
    GLFWwindow* opengl_share_window() const;
    void attach_opengl_window(GLFWwindow* window);
    void detach_opengl_window(GLFWwindow* window);
    bool last_opengl_window() const { return opengl_windows_.size() == 1; }
private:
    explicit GlfwRuntime(bool require_vulkan);
    GlfwRuntime(const GlfwRuntime&) = delete;
    GlfwRuntime& operator=(const GlfwRuntime&) = delete;
    std::shared_ptr<graphics::internal::VulkanLoader> loader_;
    std::string vulkan_error_;
    std::vector<GLFWwindow*> opengl_windows_;
};
} }
#endif
