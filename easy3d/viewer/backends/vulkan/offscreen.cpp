// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/backends/offscreen.h>
#include <easy3d/viewer/vulkan_offscreen.h>
namespace easy3d { namespace detail {
namespace {
graphics::DeviceOptions device_options(const OffScreenOptions& o) {
    graphics::DeviceOptions result;result.validation=o.validation;
    result.synchronization_validation=o.synchronization_validation;return result;
}
class VulkanOffScreenHost : public VulkanOffScreen {
public:
    VulkanOffScreenHost(int w,int h,const OffScreenOptions& o) : VulkanOffScreen(w,h,device_options(o)) {}
    void activate() const {}
    unsigned validation_errors() const {return device().validation_errors();}
};
}
std::unique_ptr<OffScreenBackend> create_vulkan_offscreen(int w,int h,const OffScreenOptions& o) {
    return std::unique_ptr<OffScreenBackend>(new OffScreenAdapter<VulkanOffScreenHost>(w,h,o));
}
} }
