// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_SURFACE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_SURFACE_H
#include <easy3d/renderer/render_device.h>
#include <vulkan/vulkan.h>
#include <functional>

namespace easy3d { namespace graphics {
// Toolkit integration boundary. No GLFW/Qt dependency in the graphics device.
// The factory copies extension names and invokes create() synchronously. The
// device owns the resulting VkSurfaceKHR. Keep the native window alive until
// the device is destroyed. Toolkit-owned VkDevices are a separate future adapter.
struct VulkanSurface {
    std::vector<std::string> instance_extensions;
    std::function<VkResult(VkInstance, VkSurfaceKHR*)> create;
};
std::unique_ptr<RenderDevice> create_vulkan_window_device(
        const VulkanSurface& surface, const DeviceOptions& options = {});
} }
#endif
