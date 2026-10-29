// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_LOADER_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_LOADER_H
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>
#include <memory>
#include "dispatch_functions.h"
namespace easy3d { namespace graphics { namespace internal {
// Immutable library/global entry point. Owned by contexts and native window
// runtimes, never unloaded while Vulkan or toolkit callbacks can still use it.
class VulkanLoader {
public:
    static std::shared_ptr<VulkanLoader> open();
    ~VulkanLoader();
    VulkanLoader(const VulkanLoader&)=delete;
    VulkanLoader& operator=(const VulkanLoader&)=delete;
    PFN_vkGetInstanceProcAddr get_instance_proc_addr() const;
private:
    VulkanLoader();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Each context has its own instance/device dispatch. No global device table is
// overwritten when a second window/device is created or destroyed.
struct VulkanDispatch {
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr=nullptr;
#define EASY3D_VK_DECLARE(name) PFN_##name name=nullptr;
    EASY3D_VK_GLOBAL(EASY3D_VK_DECLARE)
    EASY3D_VK_INSTANCE(EASY3D_VK_DECLARE)
    EASY3D_VK_INSTANCE_WSI(EASY3D_VK_DECLARE)
    EASY3D_VK_DEVICE(EASY3D_VK_DECLARE)
    EASY3D_VK_DEVICE_WSI(EASY3D_VK_DECLARE)
#undef EASY3D_VK_DECLARE
    void load_global(const VulkanLoader& library);
    void load_instance(VkInstance instance,bool surface);
    void load_device(VkDevice device,bool surface);
};
} } }
#endif
