// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/backends/viewer.h>
#include <easy3d/viewer/vulkan_viewer.h>
namespace easy3d { namespace detail {
std::unique_ptr<ViewerBase> create_vulkan_viewer(const std::string& title, const ViewerOptions& o) {
    VulkanViewerOptions options;
    options.width = o.width; options.height = o.height; options.visible = false;
    options.resizable = o.resizable; options.samples = 1;
    options.device.validation = o.validation;
    options.device.synchronization_validation = o.synchronization_validation;
    std::unique_ptr<VulkanViewer> host(new VulkanViewer(title, options));
    if (!(host->device().supported_sample_counts() & o.samples))
        throw BackendUnavailable("Vulkan viewer does not support the requested MSAA sample count");
    host->set_samples(o.samples);
    return std::unique_ptr<ViewerBase>(host.release());
}
} }
