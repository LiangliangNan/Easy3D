// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_SURFACE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_SURFACE_H
#include <easy3d/renderer/render_device.h>
#include <vulkan/vulkan.h>
#include <functional>

namespace easy3d { namespace graphics {
namespace internal { class VulkanLoader; }
/**
 * \brief Native-window integration data for a Vulkan render device.
 * \details This boundary lets a window toolkit provide a Vulkan surface without
 * adding a GLFW, Qt, or other toolkit dependency to the graphics API. The
 * factory consumes this structure synchronously and does not retain the
 * callback or extension-name storage. The returned device owns and destroys
 * the VkSurfaceKHR created by the callback; the native window itself remains
 * caller-owned and must outlive the device.
 */
struct VulkanSurface {
    /// Instance extensions required by the callback to create the native surface.
    /// Every name must be supported by the Vulkan loader.
    std::vector<std::string> instance_extensions;
    /**
     * \brief Create a VkSurfaceKHR for the supplied instance.
     * \param instance Vulkan instance created by Easy3D with the requested
     *        \ref instance_extensions enabled.
     * \param surface Receives the created surface.
     * \return VK_SUCCESS on success; another VkResult causes device creation
     *         to fail with std::runtime_error.
     * \details The callback is invoked during device creation. It must return
     * VK_SUCCESS and a non-null surface. The callback and any data it captures
     * need remain valid only until the factory call returns.
     */
    std::function<VkResult(VkInstance, VkSurfaceKHR*)> create;
    /**
     * \brief Optional shared Vulkan loader.
     * \details Native hosts can pass the loader used to create their window
     * system's Vulkan surface. If empty, Easy3D opens its own Vulkan loader.
     * The selected loader is retained for the lifetime of the render device.
     */
    std::shared_ptr<internal::VulkanLoader> loader;
};

/**
 * \brief Create a Vulkan render device for an externally created native window.
 * \param surface Required surface extensions, creation callback, and optional
 *        loader. The callback is invoked synchronously.
 * \param validation Enable the Khronos validation layer and debug reporting.
 * \param sync_validation Also enable synchronization validation; this implies
 *        \p validation.
 * \param device_name Optional substring that the selected Vulkan device name
 *        must contain. An empty string allows any compatible device.
 * \return An owning render device. Destroy it before destroying the native
 * window supplied to the callback.
 * \throws std::invalid_argument If the surface callback is empty.
 * \throws std::runtime_error If a required extension, requested validation
 * layer/feature, compatible Vulkan 1.2 device, or presentation queue is
 * unavailable, or if Vulkan initialization or surface creation fails.
 * \details This API is available when Easy3D is built with Vulkan support.
 * Device creation performs Vulkan initialization synchronously. The returned
 * device owns the VkSurfaceKHR, but not the native window or other toolkit
 * resources used by the callback.
 */
std::unique_ptr<RenderDevice> create_vulkan_window_device(
        const VulkanSurface& surface, bool validation = false, bool sync_validation = false,
        const std::string& device_name = {});
} }
#endif
