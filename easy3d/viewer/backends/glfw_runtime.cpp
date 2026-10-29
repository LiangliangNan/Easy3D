// SPDX-License-Identifier: GPL-3.0-or-later
#include "glfw_runtime.h"
#include <easy3d/renderer/backend.h>
#include <easy3d/util/logging.h>
#ifdef EASY3D_HAS_VULKAN_VIEWER
#include <easy3d/renderer/backends/vulkan/loader.h>
#endif
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
namespace easy3d { namespace detail {
GlfwRuntime::GlfwRuntime(bool require_vulkan) {
#ifdef EASY3D_HAS_VULKAN_VIEWER
    // GLFW snapshots this loader hint at initialization. Prepare it even when
    // an OpenGL host opens first; missing Vulkan must still allow OpenGL use.
    try { loader_ = graphics::internal::VulkanLoader::open(); }
    catch (const BackendUnavailable& e) { vulkan_error_ = e.what(); }
    if (require_vulkan && !loader_) throw BackendUnavailable(vulkan_error_);
    glfwInitVulkanLoader(loader_ ? loader_->get_instance_proc_addr() : nullptr);
#else
    vulkan_error_ = "Vulkan window support was not built";
    if (require_vulkan) throw BackendUnavailable(vulkan_error_);
#endif
    glfwSetErrorCallback([](int error, const char* description) {
        LOG(ERROR) << "GLFW error " << error << ": " << description;
    });
    if (!glfwInit()) {
        glfwTerminate();
#ifdef EASY3D_HAS_VULKAN_VIEWER
        glfwInitVulkanLoader(nullptr);
#endif
        throw BackendUnavailable("GLFW initialization failed (a desktop display is required)");
    }
}
void GlfwRuntime::poll_events() { glfwPollEvents(); }
GlfwRuntime::~GlfwRuntime() {
    glfwTerminate();
#ifdef EASY3D_HAS_VULKAN_VIEWER
    glfwInitVulkanLoader(nullptr);
#endif
}
std::shared_ptr<GlfwRuntime> GlfwRuntime::acquire(bool require_vulkan) {
    static std::weak_ptr<GlfwRuntime> current;
    auto runtime = current.lock();
    if (!runtime) {
        runtime = std::shared_ptr<GlfwRuntime>(new GlfwRuntime(require_vulkan));
        current = runtime;
    }
    if (require_vulkan && !runtime->loader_) throw BackendUnavailable(runtime->vulkan_error_);
    return runtime;
}
GLFWwindow* GlfwRuntime::opengl_share_window() const {
    return opengl_windows_.empty() ? nullptr : opengl_windows_.front();
}
void GlfwRuntime::attach_opengl_window(GLFWwindow* window) { opengl_windows_.push_back(window); }
void GlfwRuntime::detach_opengl_window(GLFWwindow* window) {
    opengl_windows_.erase(std::remove(opengl_windows_.begin(), opengl_windows_.end(), window), opengl_windows_.end());
}
} }
