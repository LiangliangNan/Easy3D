// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/backends/vulkan/surface.h>

#include <memory>
#include <string>
#include <type_traits>

namespace {

    using LegacySurface = easy3d::graphics::VulkanSurface;
    using InteropSurface = easy3d::graphics::backends::vulkan::VulkanSurface;
    using LegacyFactory = std::unique_ptr<easy3d::graphics::RenderDevice> (*)(
        const LegacySurface &, bool, bool, bool, bool, const std::string &);
    using InteropFactory = std::unique_ptr<easy3d::graphics::RenderDevice> (*)(
        const InteropSurface &, const easy3d::graphics::DeviceOptions &);

    static_assert(std::is_base_of<LegacySurface, InteropSurface>::value,
                  "Vulkan interop surface no longer preserves the legacy compatibility base");
    static_assert(std::is_same<decltype(&easy3d::graphics::create_vulkan_window_device), LegacyFactory>::value,
                  "legacy Vulkan window factory signature changed");
    static_assert(std::is_same<decltype(&easy3d::graphics::backends::vulkan::create_vulkan_window_device),
                               InteropFactory>::value,
                  "Vulkan interop window factory signature changed");

} // namespace

int main() {
    LegacyFactory volatile legacy_factory = &easy3d::graphics::create_vulkan_window_device;
    (void) legacy_factory;
    return 0;
}
