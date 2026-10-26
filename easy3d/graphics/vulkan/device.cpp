// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Liangliang Nan
#include <easy3d/graphics/render_device.h>
#include <easy3d/graphics/vulkan/surface.h>
#include <vulkan/vulkan.h>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>

namespace easy3d { namespace graphics { namespace {
#include "scene_vert.inc"
#include "scene_frag.inc"

void check(VkResult result, const char* operation) {
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) + ")");
}

template<class T> bool named(const std::vector<T>& values, const char* name) {
    for (const auto& v : values) if (std::strcmp(v.extensionName, name) == 0) return true;
    return false;
}
std::vector<VkExtensionProperties> instance_extensions(const char* layer = nullptr) {
    uint32_t count = 0;
    check(vkEnumerateInstanceExtensionProperties(layer, &count, nullptr), "enumerate instance extensions");
    std::vector<VkExtensionProperties> values(count);
    check(vkEnumerateInstanceExtensionProperties(layer, &count, values.data()), "enumerate instance extensions");
    return values;
}

// No mutable global device dispatch or resources: loader dispatch is handle based.
struct Context {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t family = 0;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer command = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceMemoryProperties memory{};
    std::atomic<unsigned> errors{0};
    ~Context() {
        if (device) {
            // Shutdown only. Normal rendering waits on its submission fence.
            vkDeviceWaitIdle(device);
            if (fence) vkDestroyFence(device, fence, nullptr);
            if (pool) vkDestroyCommandPool(device, pool, nullptr);
            vkDestroyDevice(device, nullptr);
        }
        if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);
        if (messenger) {
            auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroy) destroy(instance, messenger, nullptr);
        }
        if (instance) vkDestroyInstance(instance, nullptr);
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
        auto* self = static_cast<Context*>(user);
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ++self->errors;
        std::cerr << "Vulkan validation: " << data->pMessage << '\n';
        return VK_FALSE;
    }
    void init(const DeviceOptions& options, const VulkanSurface* window = nullptr) {
        const bool validation = options.validation || options.synchronization_validation;
        auto available = instance_extensions();
        std::vector<const char*> extensions, layers;
        if (window) {
            if (!window->create) throw std::invalid_argument("surface creation callback is required");
            for (const auto& name : window->instance_extensions) {
                if (!named(available, name.c_str()))
                    throw std::runtime_error("required surface extension unavailable: " + name);
                if (std::find_if(extensions.begin(), extensions.end(), [&](const char* e) { return name == e; }) == extensions.end())
                    extensions.push_back(name.c_str());
            }
        }
        VkDebugUtilsMessengerCreateInfoEXT debug_info{};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pfnUserCallback = debug;
        debug_info.pUserData = this;
        if (validation) {
            uint32_t count = 0;
            check(vkEnumerateInstanceLayerProperties(&count, nullptr), "enumerate validation layers");
            std::vector<VkLayerProperties> values(count);
            check(vkEnumerateInstanceLayerProperties(&count, values.data()), "enumerate validation layers");
            bool found = false;
            for (const auto& v : values) if (!std::strcmp(v.layerName, "VK_LAYER_KHRONOS_validation")) found = true;
            if (!found) throw std::runtime_error("VK_LAYER_KHRONOS_validation was requested but is unavailable");
            if (!named(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
                throw std::runtime_error("VK_EXT_debug_utils is required for validation reporting");
            layers.push_back("VK_LAYER_KHRONOS_validation");
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        VkValidationFeaturesEXT validation_features{};
        VkValidationFeatureEnableEXT sync = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
        if (options.synchronization_validation) {
            const auto layer_extensions = instance_extensions("VK_LAYER_KHRONOS_validation");
            if (!named(available, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) &&
                !named(layer_extensions, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
                throw std::runtime_error("synchronization validation is unavailable");
            extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
            validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            validation_features.enabledValidationFeatureCount = 1;
            validation_features.pEnabledValidationFeatures = &sync;
            debug_info.pNext = &validation_features;
        }
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = "Easy3D";
        app.apiVersion = VK_API_VERSION_1_2;
        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &app;
        info.enabledLayerCount = static_cast<uint32_t>(layers.size());
        info.ppEnabledLayerNames = layers.data();
        info.pNext = validation ? &debug_info : nullptr;
        // MoltenVK devices may otherwise be omitted by the loader.
        if (named(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
        std::vector<const char*> unique_extensions;
        for (const char* name : extensions)
            if (std::find_if(unique_extensions.begin(), unique_extensions.end(), [&](const char* other) {
                    return std::strcmp(name, other) == 0;
                }) == unique_extensions.end()) unique_extensions.push_back(name);
        extensions = std::move(unique_extensions);
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        check(vkCreateInstance(&info, nullptr, &instance), "create Vulkan 1.2 instance");
        if (window) {
            check(window->create(instance, &surface), "create native Vulkan surface");
            if (!surface) throw std::runtime_error("surface callback returned a null surface");
        }
        if (validation) {
            auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            if (!create) throw std::runtime_error("debug messenger entry point unavailable");
            debug_info.pNext = nullptr;
            check(create(instance, &debug_info, nullptr, &messenger), "create debug messenger");
        }
        uint32_t count = 0;
        check(vkEnumeratePhysicalDevices(instance, &count, nullptr), "enumerate Vulkan devices");
        std::vector<VkPhysicalDevice> devices(count);
        check(vkEnumeratePhysicalDevices(instance, &count, devices.data()), "enumerate Vulkan devices");
        int best_score = -1;
        for (auto candidate : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(candidate, &props);
            if (props.apiVersion < VK_API_VERSION_1_2 ||
                std::string(props.deviceName).find(options.device_name) == std::string::npos) continue;
            const std::pair<VkFormat, VkFormatFeatureFlags> formats[] = {
                {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT},
                {VK_FORMAT_R32G32_UINT, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT},
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT}
            };
            bool suitable = true;
            for (const auto& format : formats) {
                VkFormatProperties p;
                vkGetPhysicalDeviceFormatProperties(candidate, format.first, &p);
                suitable &= (p.optimalTilingFeatures & format.second) == format.second;
            }
            if (!suitable) continue;
            if (surface) {
                VkFormatProperties source;
                vkGetPhysicalDeviceFormatProperties(candidate, VK_FORMAT_R8G8B8A8_UNORM, &source);
                if (!(source.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) continue;
                uint32_t extension_count = 0;
                check(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extension_count, nullptr), "enumerate WSI extensions");
                std::vector<VkExtensionProperties> supported(extension_count);
                check(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extension_count, supported.data()), "enumerate WSI extensions");
                if (!named(supported, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;
            }
            uint32_t families = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &families, nullptr);
            std::vector<VkQueueFamilyProperties> queues(families);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &families, queues.data());
            for (uint32_t i = 0; i < families; ++i) {
                if (!(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) continue;
                if (surface) {
                    VkBool32 supported = VK_FALSE;
                    check(vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &supported), "query presentation queue");
                    if (!supported) continue;
                }
                int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 3 :
                            props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2 : 1;
                if (score > best_score) {
                    best_score = score; physical = candidate; family = i; properties = props;
                }
                break;
            }
        }
        if (!physical) throw std::runtime_error("no suitable Vulkan 1.2 device/graphics queue for requested targets and surface");
        vkGetPhysicalDeviceMemoryProperties(physical, &memory);
        count = 0;
        check(vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr), "enumerate device extensions");
        std::vector<VkExtensionProperties> device_extensions(count);
        check(vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, device_extensions.data()), "enumerate device extensions");
        extensions.clear();
        if (surface) extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        // This extension is mandatory when advertised by a portability device.
        if (named(device_extensions, "VK_KHR_portability_subset")) extensions.push_back("VK_KHR_portability_subset");
        float priority = 1;
        VkDeviceQueueCreateInfo qi{};
        qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = family; qi.queueCount = 1; qi.pQueuePriorities = &priority;
        VkDeviceCreateInfo di{};
        di.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        di.queueCreateInfoCount = 1; di.pQueueCreateInfos = &qi;
        di.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        di.ppEnabledExtensionNames = extensions.data();
        // No geometry shaders, wide lines, independent blend or other optional features.
        check(vkCreateDevice(physical, &di, nullptr, &device), "create device");
        vkGetDeviceQueue(device, family, 0, &queue);
        VkCommandPoolCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.queueFamilyIndex = family;
        pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check(vkCreateCommandPool(device, &pi, nullptr, &pool), "create command pool");
        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = pool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        check(vkAllocateCommandBuffers(device, &ai, &command), "allocate command buffer");
        VkFenceCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        check(vkCreateFence(device, &fi, nullptr, &fence), "create submission fence");
    }
    uint32_t memory_type(uint32_t mask, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const {
        uint32_t fallback = UINT32_MAX;
        for (uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
            if (!(mask & (1u << i)) || (memory.memoryTypes[i].propertyFlags & required) != required) continue;
            if ((memory.memoryTypes[i].propertyFlags & preferred) == preferred) return i;
            fallback = i;
        }
        if (fallback == UINT32_MAX) throw std::runtime_error("no compatible Vulkan memory type");
        return fallback;
    }
    void execute(const std::function<void(VkCommandBuffer)>& record,
                 VkSemaphore wait = VK_NULL_HANDLE, VkSemaphore signal = VK_NULL_HANDLE) {
        check(vkResetCommandBuffer(command, 0), "reset command buffer");
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check(vkBeginCommandBuffer(command, &begin), "begin command buffer");
        record(command);
        check(vkEndCommandBuffer(command), "end command buffer");
        check(vkResetFences(device, 1, &fence), "reset fence");
        VkSubmitInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1; si.pCommandBuffers = &command;
        const VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        if (wait) { si.waitSemaphoreCount = 1; si.pWaitSemaphores = &wait; si.pWaitDstStageMask = &stage; }
        if (signal) { si.signalSemaphoreCount = 1; si.pSignalSemaphores = &signal; }
        check(vkQueueSubmit(queue, 1, &si, fence), "submit graphics commands");
        check(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), "wait for capture");
    }
};

struct Buffer {
    Context& c;
    VkBuffer handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    bool coherent = false;
    explicit Buffer(Context& context) : c(context) {}
    Buffer(const Buffer&) = delete;
    ~Buffer() {
        if (handle) vkDestroyBuffer(c.device, handle, nullptr);
        if (memory) vkFreeMemory(c.device, memory, nullptr);
    }
    void init(VkDeviceSize size, VkBufferUsageFlags usage, bool host) {
        VkBufferCreateInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bi.size = size; bi.usage = usage; bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        check(vkCreateBuffer(c.device, &bi, nullptr, &handle), "create buffer");
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(c.device, handle, &requirements);
        uint32_t type = c.memory_type(requirements.memoryTypeBits, host ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0,
            host ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        coherent = (c.memory.memoryTypes[type].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = requirements.size; ai.memoryTypeIndex = type;
        check(vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate buffer memory");
        check(vkBindBufferMemory(c.device, handle, memory, 0), "bind buffer memory");
    }
    void transfer(void* data, size_t size, bool write) {
        void* mapped = nullptr;
        check(vkMapMemory(c.device, memory, 0, VK_WHOLE_SIZE, 0, &mapped), "map host buffer");
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = memory; range.size = VK_WHOLE_SIZE;
        VkResult result = VK_SUCCESS;
        if (write) {
            std::memcpy(mapped, data, size);
            if (!coherent) result = vkFlushMappedMemoryRanges(c.device, 1, &range);
        } else {
            if (!coherent) result = vkInvalidateMappedMemoryRanges(c.device, 1, &range);
            if (result == VK_SUCCESS) std::memcpy(data, mapped, size);
        }
        vkUnmapMemory(c.device, memory);
        check(result, write ? "flush host buffer" : "invalidate readback buffer");
    }
};
struct GpuImage {
    Context& c;
    VkImage handle = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    explicit GpuImage(Context& context) : c(context) {}
    GpuImage(const GpuImage&) = delete;
    ~GpuImage() {
        if (view) vkDestroyImageView(c.device, view, nullptr);
        if (handle) vkDestroyImage(c.device, handle, nullptr);
        if (memory) vkFreeMemory(c.device, memory, nullptr);
    }
    void init(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
        VkImageCreateInfo ii{};
        ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ii.imageType = VK_IMAGE_TYPE_2D; ii.format = format; ii.extent = {width, height, 1};
        ii.mipLevels = 1; ii.arrayLayers = 1; ii.samples = VK_SAMPLE_COUNT_1_BIT;
        ii.tiling = VK_IMAGE_TILING_OPTIMAL; ii.usage = usage;
        ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE; ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        check(vkCreateImage(c.device, &ii, nullptr, &handle), "create image");
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(c.device, handle, &requirements);
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = requirements.size;
        ai.memoryTypeIndex = c.memory_type(requirements.memoryTypeBits, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        check(vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate image memory");
        check(vkBindImageMemory(c.device, handle, memory, 0), "bind image memory");
        VkImageViewCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = handle; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = format;
        vi.subresourceRange = {aspect, 0, 1, 0, 1};
        check(vkCreateImageView(c.device, &vi, nullptr, &view), "create image view");
    }
};
void image_barrier(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
                   VkImageLayout old_layout, VkImageLayout new_layout,
                   VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                   VkAccessFlags src_access, VkAccessFlags dst_access) {
    VkImageMemoryBarrier b{};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    b.srcAccessMask = src_access; b.dstAccessMask = dst_access;
    b.oldLayout = old_layout; b.newLayout = new_layout;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image; b.subresourceRange = {aspect, 0, 1, 0, 1};
    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &b);
}
struct Target {
    Context& c;
    uint32_t width, height;
    GpuImage color, ids, depth;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    Buffer readback;
    Target(Context& context, uint32_t w, uint32_t h) : c(context), width(w), height(h),
        color(c), ids(c), depth(c), readback(c) {}
    ~Target() { if (framebuffer) vkDestroyFramebuffer(c.device, framebuffer, nullptr); }
    void init(VkRenderPass pass) {
        color.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        ids.init(width, height, VK_FORMAT_R32G32_UINT,
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        depth.init(width, height, VK_FORMAT_D32_SFLOAT,
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        VkImageView views[] = {color.view, ids.view, depth.view};
        VkFramebufferCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fi.renderPass = pass; fi.attachmentCount = 3; fi.pAttachments = views;
        fi.width = width; fi.height = height; fi.layers = 1;
        check(vkCreateFramebuffer(c.device, &fi, nullptr, &framebuffer), "create framebuffer");
    }
};
struct GpuVertex {
    float position[3], normal[3], color[4], texcoord[2];
    uint32_t primitive;
};
struct GpuGeometry {
    std::weak_ptr<const Geometry> source;
    Buffer buffer;
    uint32_t count = 0;
    explicit GpuGeometry(Context& c) : buffer(c) {}
};
struct GpuTexture {
    Context& c;
    std::weak_ptr<const Image> source;
    GpuImage image;
    VkDescriptorPool pool;
    VkDescriptorSet set = VK_NULL_HANDLE;
    GpuTexture(Context& context, VkDescriptorPool p) : c(context), image(c), pool(p) {}
    ~GpuTexture() { if (set) vkFreeDescriptorSets(c.device, pool, 1, &set); }
};
// Explicit packing: do not rely on public vec3/mat4 alignment for GPU layouts.
struct Push { float mvp[16], color[4]; uint32_t object, lighting; float normal[9]; };
static_assert(sizeof(Push) == 124, "shader push-constant layout changed");

// Presentation semaphores belong to swapchain images, not CPU frame slots. An
// image is reacquired only after its previous presentation wait has completed.
struct Swapchain {
    Context& c;
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkExtent2D extent{};
    uint32_t requested_width = 0, requested_height = 0;
    std::vector<VkImage> images;
    std::vector<VkSemaphore> ready;
    VkSemaphore acquired = VK_NULL_HANDLE;
    bool dirty = false;
    explicit Swapchain(Context& context) : c(context) {}
    ~Swapchain() {
        for (auto s : ready) if (s) vkDestroySemaphore(c.device, s, nullptr);
        if (acquired) vkDestroySemaphore(c.device, acquired, nullptr);
        if (handle) vkDestroySwapchainKHR(c.device, handle, nullptr);
    }
    void init(uint32_t width, uint32_t height) {
        requested_width = width; requested_height = height;
        VkSurfaceCapabilitiesKHR caps;
        check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(c.physical, c.surface, &caps), "query surface capabilities");
        if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
            throw std::runtime_error("surface does not support transfer-destination presentation images");
        if (!(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR))
            throw std::runtime_error("rotated Vulkan presentation surfaces are not yet supported");
        extent = caps.currentExtent;
        if (extent.width == UINT32_MAX) {
            extent.width = std::max(caps.minImageExtent.width, std::min(width, caps.maxImageExtent.width));
            extent.height = std::max(caps.minImageExtent.height, std::min(height, caps.maxImageExtent.height));
        }
        if (!extent.width || !extent.height) return;
        uint32_t count = 0;
        check(vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, nullptr), "query surface formats");
        std::vector<VkSurfaceFormatKHR> formats(count);
        check(vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, formats.data()), "query surface formats");
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
            formats[0].format = VK_FORMAT_B8G8R8A8_UNORM;
        VkSurfaceFormatKHR chosen{};
        bool found = false;
        const VkFormat preferred[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                     VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB};
        for (auto format : preferred) {
            for (const auto& candidate : formats) {
                if (candidate.format != format || candidate.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) continue;
                VkFormatProperties properties;
                vkGetPhysicalDeviceFormatProperties(c.physical, format, &properties);
                if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) continue;
                chosen = candidate; found = true; break;
            }
            if (found) break;
        }
        if (!found) throw std::runtime_error("no supported RGBA/BGRA presentation format with blit support");
        uint32_t image_count = caps.minImageCount + 1;
        if (caps.maxImageCount && image_count > caps.maxImageCount) image_count = caps.maxImageCount;
        VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        const VkCompositeAlphaFlagBitsKHR modes[] = {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};
        for (auto mode : modes) if (caps.supportedCompositeAlpha & mode) { alpha = mode; break; }
        VkSwapchainCreateInfoKHR si{};
        si.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        si.surface = c.surface; si.minImageCount = image_count;
        si.imageFormat = chosen.format; si.imageColorSpace = chosen.colorSpace;
        si.imageExtent = extent; si.imageArrayLayers = 1;
        si.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        si.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        si.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        si.compositeAlpha = alpha; si.presentMode = VK_PRESENT_MODE_FIFO_KHR; si.clipped = VK_TRUE;
        check(vkCreateSwapchainKHR(c.device, &si, nullptr, &handle), "create swapchain");
        check(vkGetSwapchainImagesKHR(c.device, handle, &count, nullptr), "get presentation images");
        images.resize(count);
        check(vkGetSwapchainImagesKHR(c.device, handle, &count, images.data()), "get presentation images");
        VkSemaphoreCreateInfo sem{};
        sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        check(vkCreateSemaphore(c.device, &sem, nullptr, &acquired), "create acquire semaphore");
        ready.resize(count, VK_NULL_HANDLE);
        for (auto& s : ready) check(vkCreateSemaphore(c.device, &sem, nullptr, &s), "create image presentation semaphore");
    }
};

class VulkanDevice final : public RenderDevice {
    Context c_;
    std::string name_;
    VkRenderPass pass_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptors_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkSampler sampler_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkPipeline pipelines_[3] = {};
    std::unique_ptr<Target> target_;
    std::unique_ptr<Swapchain> swapchain_;
    std::shared_ptr<const Image> white_;
    std::map<const Geometry*, std::unique_ptr<GpuGeometry>> geometry_;
    std::map<const Image*, std::unique_ptr<GpuTexture>> textures_;
public:
    // Separate initialization ensures all members are destroyed if any Vulkan step throws.
    ~VulkanDevice() override {
        // Presentation may still be using image-owned semaphores after the draw
        // fence completes. Drain at shutdown/recreation only, never per frame.
        if (c_.device) vkDeviceWaitIdle(c_.device);
        swapchain_.reset(); target_.reset(); geometry_.clear(); textures_.clear();
        for (auto p : pipelines_) if (p) vkDestroyPipeline(c_.device, p, nullptr);
        if (layout_) vkDestroyPipelineLayout(c_.device, layout_, nullptr);
        if (sampler_) vkDestroySampler(c_.device, sampler_, nullptr);
        if (descriptor_pool_) vkDestroyDescriptorPool(c_.device, descriptor_pool_, nullptr);
        if (descriptors_) vkDestroyDescriptorSetLayout(c_.device, descriptors_, nullptr);
        if (pass_) vkDestroyRenderPass(c_.device, pass_, nullptr);
    }
    const std::string& name() const override { return name_; }
    unsigned validation_errors() const override { return c_.errors.load(); }
    void init(const DeviceOptions& options, const VulkanSurface* surface = nullptr) {
        c_.init(options, surface); name_ = c_.properties.deviceName;
        white_ = std::make_shared<Image>(1, 1, std::vector<unsigned char>(4, 255));
        create_pass(); create_descriptors(); create_pipelines();
    }
    void create_pass() {
        VkAttachmentDescription attachments[3] = {};
        const VkFormat formats[] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R32G32_UINT, VK_FORMAT_D32_SFLOAT};
        for (unsigned i = 0; i < 3; ++i) {
            attachments[i].format = formats[i]; attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[i].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        VkAttachmentReference colors[] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                          {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
        VkAttachmentReference depth = {2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 2; subpass.pColorAttachments = colors;
        subpass.pDepthStencilAttachment = &depth;
        VkSubpassDependency dependencies[2] = {};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].srcSubpass = 0; dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        VkRenderPassCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        pi.attachmentCount = 3; pi.pAttachments = attachments;
        pi.subpassCount = 1; pi.pSubpasses = &subpass;
        pi.dependencyCount = 2; pi.pDependencies = dependencies;
        check(vkCreateRenderPass(c_.device, &pi, nullptr, &pass_), "create render pass");
    }
    void create_descriptors() {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0; binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1; binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo li{};
        li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        li.bindingCount = 1; li.pBindings = &binding;
        check(vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &descriptors_), "create descriptor layout");
        VkDescriptorPoolSize size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024};
        VkDescriptorPoolCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pi.maxSets = 1024; pi.poolSizeCount = 1; pi.pPoolSizes = &size;
        check(vkCreateDescriptorPool(c_.device, &pi, nullptr, &descriptor_pool_), "create descriptor pool");
        VkSamplerCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        si.magFilter = VK_FILTER_NEAREST; si.minFilter = VK_FILTER_NEAREST;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.maxLod = 0;
        check(vkCreateSampler(c_.device, &si, nullptr, &sampler_), "create sampler");
    }
    void create_pipelines() {
        VkPushConstantRange push = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Push)};
        VkPipelineLayoutCreateInfo li{};
        li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        li.setLayoutCount = 1; li.pSetLayouts = &descriptors_;
        li.pushConstantRangeCount = 1; li.pPushConstantRanges = &push;
        check(vkCreatePipelineLayout(c_.device, &li, nullptr, &layout_), "create pipeline layout");
        VkShaderModule modules[2] = {};
        try {
            VkShaderModuleCreateInfo mi{};
            mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            mi.codeSize = sizeof(scene_vert); mi.pCode = scene_vert;
            check(vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]), "create vertex shader");
            mi.codeSize = sizeof(scene_frag); mi.pCode = scene_frag;
            check(vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]), "create fragment shader");
            VkPipelineShaderStageCreateInfo stages[2] = {};
            for (unsigned i = 0; i < 2; ++i) {
                stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
                stages[i].module = modules[i]; stages[i].pName = "main";
            }
            VkVertexInputBindingDescription binding = {0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
            VkVertexInputAttributeDescription attrs[] = {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, position)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, normal)},
                {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, color)},
                {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuVertex, texcoord)},
                {4, 0, VK_FORMAT_R32_UINT, offsetof(GpuVertex, primitive)}};
            VkPipelineVertexInputStateCreateInfo vertex{};
            vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex.vertexBindingDescriptionCount = 1; vertex.pVertexBindingDescriptions = &binding;
            vertex.vertexAttributeDescriptionCount = 5; vertex.pVertexAttributeDescriptions = attrs;
            VkPipelineInputAssemblyStateCreateInfo assembly{};
            assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            VkPipelineViewportStateCreateInfo viewport{};
            viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport.viewportCount = 1; viewport.scissorCount = 1;
            VkPipelineRasterizationStateCreateInfo raster{};
            raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            raster.polygonMode = VK_POLYGON_MODE_FILL; raster.cullMode = VK_CULL_MODE_NONE;
            raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; raster.lineWidth = 1;
            VkPipelineMultisampleStateCreateInfo ms{};
            ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            VkPipelineDepthStencilStateCreateInfo depth{};
            depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth.depthTestEnable = VK_TRUE; depth.depthWriteEnable = VK_TRUE; depth.depthCompareOp = VK_COMPARE_OP_LESS;
            VkPipelineColorBlendAttachmentState blend[2] = {};
            // Both attachments have identical state; no independentBlend feature required.
            for (auto& b : blend) b.colorWriteMask = 0xf;
            VkPipelineColorBlendStateCreateInfo blending{};
            blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            blending.attachmentCount = 2; blending.pAttachments = blend;
            VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo dynamic{};
            dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic.dynamicStateCount = 2; dynamic.pDynamicStates = dynamic_states;
            VkGraphicsPipelineCreateInfo pi{};
            pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pi.stageCount = 2; pi.pStages = stages; pi.pVertexInputState = &vertex;
            pi.pInputAssemblyState = &assembly; pi.pViewportState = &viewport;
            pi.pRasterizationState = &raster; pi.pMultisampleState = &ms;
            pi.pDepthStencilState = &depth; pi.pColorBlendState = &blending;
            pi.pDynamicState = &dynamic; pi.layout = layout_; pi.renderPass = pass_;
            const VkPrimitiveTopology topologies[] = {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST};
            for (unsigned i = 0; i < 3; ++i) {
                assembly.topology = topologies[i];
                check(vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr, &pipelines_[i]), "create pipeline");
            }
        } catch (...) {
            for (auto m : modules) if (m) vkDestroyShaderModule(c_.device, m, nullptr);
            throw;
        }
        for (auto m : modules) vkDestroyShaderModule(c_.device, m, nullptr);
    }
    GpuGeometry& geometry(const std::shared_ptr<const Geometry>& data) {
        auto it = geometry_.find(data.get());
        if (it != geometry_.end()) return *it->second;
        std::unique_ptr<GpuGeometry> gpu(new GpuGeometry(c_));
        gpu->source = data;
        size_t count = data->indices().empty() ? data->vertices().size() : data->indices().size();
        gpu->count = static_cast<uint32_t>(count);
        const unsigned size = data->topology() == Topology::triangles ? 3 : data->topology() == Topology::lines ? 2 : 1;
        std::vector<GpuVertex> packed(count);
        for (size_t i = 0; i < count; ++i) {
            const auto& v = data->vertices()[data->indices().empty() ? i : data->indices()[i]];
            auto& p = packed[i];
            for (unsigned j = 0; j < 3; ++j) { p.position[j] = v.position[j]; p.normal[j] = v.normal[j]; }
            for (unsigned j = 0; j < 4; ++j) p.color[j] = v.color[j];
            for (unsigned j = 0; j < 2; ++j) p.texcoord[j] = v.texcoord[j];
            p.primitive = data->primitive_ids().empty() ? static_cast<uint32_t>(i / size) : data->primitive_ids()[i / size];
        }
        if (count) {
            const VkDeviceSize bytes = packed.size() * sizeof(GpuVertex);
            Buffer staging(c_);
            staging.init(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
            staging.transfer(packed.data(), static_cast<size_t>(bytes), true);
            gpu->buffer.init(bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false);
            c_.execute([&](VkCommandBuffer cmd) {
                VkBufferCopy copy = {0, 0, bytes};
                vkCmdCopyBuffer(cmd, staging.handle, gpu->buffer.handle, 1, &copy);
                VkBufferMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.buffer = gpu->buffer.handle; barrier.size = VK_WHOLE_SIZE;
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                     0, 0, nullptr, 1, &barrier, 0, nullptr);
            });
        }
        auto* result = gpu.get(); geometry_.emplace(data.get(), std::move(gpu));
        return *result;
    }
    GpuTexture& texture(const std::shared_ptr<const Image>& data) {
        auto it = textures_.find(data.get());
        if (it != textures_.end()) return *it->second;
        if (data->width() > c_.properties.limits.maxImageDimension2D || data->height() > c_.properties.limits.maxImageDimension2D)
            throw std::invalid_argument("texture exceeds device image limits");
        std::unique_ptr<GpuTexture> gpu(new GpuTexture(c_, descriptor_pool_));
        gpu->source = data;
        gpu->image.init(data->width(), data->height(), VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        Buffer staging(c_);
        staging.init(data->rgba().size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        staging.transfer(const_cast<unsigned char*>(data->rgba().data()), data->rgba().size(), true);
        c_.execute([&](VkCommandBuffer cmd) {
            image_barrier(cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
            VkBufferImageCopy copy{};
            copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.imageExtent = {data->width(), data->height(), 1};
            vkCmdCopyBufferToImage(cmd, staging.handle, gpu->image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
            image_barrier(cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
        });
        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = descriptor_pool_; ai.descriptorSetCount = 1; ai.pSetLayouts = &descriptors_;
        check(vkAllocateDescriptorSets(c_.device, &ai, &gpu->set), "allocate texture descriptor (maximum 1024 live textures)");
        VkDescriptorImageInfo image = {sampler_, gpu->image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = gpu->set; write.dstBinding = 0; write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; write.pImageInfo = &image;
        vkUpdateDescriptorSets(c_.device, 1, &write, 0, nullptr);
        auto* result = gpu.get(); textures_.emplace(data.get(), std::move(gpu));
        return *result;
    }
    struct Prepared { const DrawCommand* command; GpuGeometry* geometry; GpuTexture* texture; };
    std::vector<Prepared> prepare(uint32_t width, uint32_t height, const std::vector<DrawCommand>& commands,
                                  const vec4& background) {
        const auto& limits = c_.properties.limits;
        if (!width || !height || width > limits.maxFramebufferWidth || height > limits.maxFramebufferHeight ||
            width > limits.maxImageDimension2D || height > limits.maxImageDimension2D ||
            uint64_t(width) * height > (std::numeric_limits<size_t>::max() - 8) / 16)
            throw std::invalid_argument("invalid offscreen target dimensions");
        for (unsigned i = 0; i < 4; ++i)
            if (!std::isfinite(background[i])) throw std::invalid_argument("non-finite background color");
        for (const auto& command : commands) {
            if (!command.geometry || command.object_id == UINT32_MAX)
                throw std::invalid_argument("draw requires geometry and a non-background object ID");
            for (unsigned col = 0; col < 4; ++col) {
                if (!std::isfinite(command.color[col])) throw std::invalid_argument("non-finite draw color");
                for (unsigned row = 0; row < 4; ++row)
                    if (!std::isfinite(command.model_view_projection(row, col)))
                        throw std::invalid_argument("non-finite transform");
            }
            for (unsigned col = 0; col < 3; ++col)
                for (unsigned row = 0; row < 3; ++row)
                    if (!std::isfinite(command.normal_matrix(row, col)))
                        throw std::invalid_argument("non-finite normal transform");
        }
        // The preceding call's fence has completed. Retire expired resources now.
        for (auto it = geometry_.begin(); it != geometry_.end(); ) {
            if (it->second->source.expired()) it = geometry_.erase(it); else ++it;
        }
        for (auto it = textures_.begin(); it != textures_.end(); ) {
            if (it->second->source.expired()) it = textures_.erase(it); else ++it;
        }
        if (!target_ || target_->width != width || target_->height != height) {
            std::unique_ptr<Target> target(new Target(c_, width, height));
            target->init(pass_); target_ = std::move(target);
        }
        std::vector<Prepared> draws;
        for (const auto& command : commands) {
            auto& g = geometry(command.geometry);
            if (!g.count) continue;
            auto& t = texture(command.texture ? command.texture : white_);
            draws.push_back({&command, &g, &t});
        }
        return draws;
    }
    void encode(VkCommandBuffer cmd, const std::vector<Prepared>& draws, const vec4& background) {
        const auto width = target_->width, height = target_->height;
            VkClearValue clear[3] = {};
            for (unsigned i = 0; i < 4; ++i) clear[0].color.float32[i] = background[i];
            clear[1].color.uint32[0] = clear[1].color.uint32[1] = UINT32_MAX;
            clear[2].depthStencil.depth = 1;
            VkRenderPassBeginInfo begin{};
            begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            begin.renderPass = pass_; begin.framebuffer = target_->framebuffer;
            begin.renderArea.extent = {width, height}; begin.clearValueCount = 3; begin.pClearValues = clear;
            vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport viewport = {0, 0, float(width), float(height), 0, 1};
            VkRect2D scissor = {{0, 0}, {width, height}};
            vkCmdSetViewport(cmd, 0, 1, &viewport); vkCmdSetScissor(cmd, 0, 1, &scissor);
            for (const auto& draw : draws) {
                const auto& d = *draw.command;
                unsigned pipeline = d.geometry->topology() == Topology::triangles ? 0 :
                                    d.geometry->topology() == Topology::lines ? 1 : 2;
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[pipeline]);
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 0, 1, &draw.texture->set, 0, nullptr);
                Push push{};
                for (unsigned col = 0; col < 4; ++col)
                    for (unsigned row = 0; row < 4; ++row) push.mvp[col * 4 + row] = d.model_view_projection(row, col);
                for (unsigned i = 0; i < 4; ++i) push.color[i] = d.color[i];
                push.object = d.object_id; push.lighting = d.lighting ? 1 : 0;
                for (unsigned col = 0; col < 3; ++col)
                    for (unsigned row = 0; row < 3; ++row) push.normal[col * 3 + row] = d.normal_matrix(row, col);
                vkCmdPushConstants(cmd, layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
                vkCmdDraw(cmd, draw.geometry->count, 1, 0, 0);
            }
            vkCmdEndRenderPass(cmd);
    }
    FrameImage render(uint32_t width, uint32_t height, const std::vector<DrawCommand>& commands,
                      const vec4& background) override {
        const auto draws = prepare(width, height, commands, background);
        FrameImage result;
        result.width = width; result.height = height;
        const size_t pixels = size_t(width) * height;
        // Presentation never allocates host readback storage. Allocate only for
        // an explicitly requested capture, including eight-byte ID alignment.
        if (!target_->readback.handle)
            target_->readback.init(VkDeviceSize(pixels) * 16 + 8, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
        const size_t id_offset = (pixels * 4 + 7) & ~size_t(7);
        const size_t depth_offset = id_offset + pixels * 8;
        result.rgba.resize(pixels * 4); result.depth.resize(pixels); result.ids.resize(pixels);
        static_assert(sizeof(FrameImage::Id) == 8, "ID readback must be tightly packed");
        c_.execute([&](VkCommandBuffer cmd) {
            encode(cmd, draws, background);
            const VkImage images[] = {target_->color.handle, target_->ids.handle, target_->depth.handle};
            const VkDeviceSize offsets[] = {0, VkDeviceSize(id_offset), VkDeviceSize(depth_offset)};
            for (unsigned i = 0; i < 3; ++i) {
                VkBufferImageCopy copy{};
                copy.bufferOffset = offsets[i];
                copy.imageSubresource = {i == 2 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                copy.imageExtent = {width, height, 1};
                vkCmdCopyImageToBuffer(cmd, images[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target_->readback.handle, 1, &copy);
            }
            VkMemoryBarrier host{};
            host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                                 0, 1, &host, 0, nullptr, 0, nullptr);
        });
        std::vector<unsigned char> bytes(depth_offset + pixels * 4);
        target_->readback.transfer(bytes.data(), bytes.size(), false);
        std::memcpy(result.rgba.data(), bytes.data(), pixels * 4);
        std::memcpy(result.ids.data(), bytes.data() + id_offset, pixels * 8);
        std::memcpy(result.depth.data(), bytes.data() + depth_offset, pixels * 4);
        return result;
    }
    bool present(uint32_t width, uint32_t height, const std::vector<DrawCommand>& commands,
                 const vec4& background) override {
        if (!c_.surface) throw std::logic_error("headless Vulkan device has no presentation surface");
        if (!width || !height) return false;
        if (!swapchain_ || swapchain_->dirty || swapchain_->requested_width != width || swapchain_->requested_height != height) {
            // Rare lifecycle boundary. The normal frame path waits only its draw fence.
            check(vkDeviceWaitIdle(c_.device), "drain presentation before swapchain recreation");
            swapchain_.reset();
            std::unique_ptr<Swapchain> next(new Swapchain(c_));
            next->init(width, height);
            if (!next->handle) return false;
            swapchain_ = std::move(next);
        }
        const auto draws = prepare(width, height, commands, background);
        uint32_t image = 0;
        const auto acquired = vkAcquireNextImageKHR(c_.device, swapchain_->handle, UINT64_MAX,
                                                    swapchain_->acquired, VK_NULL_HANDLE, &image);
        if (acquired == VK_ERROR_OUT_OF_DATE_KHR) { swapchain_->dirty = true; return false; }
        if (acquired != VK_SUBOPTIMAL_KHR) check(acquired, "acquire presentation image");
        c_.execute([&](VkCommandBuffer cmd) {
            encode(cmd, draws, background);
            image_barrier(cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
            VkImageBlit copy{};
            copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.srcOffsets[1] = {int32_t(width), int32_t(height), 1};
            copy.dstOffsets[1] = {int32_t(swapchain_->extent.width), int32_t(swapchain_->extent.height), 1};
            vkCmdBlitImage(cmd, target_->color.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                swapchain_->images[image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy, VK_FILTER_NEAREST);
            image_barrier(cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0);
        }, swapchain_->acquired, swapchain_->ready[image]);
        VkPresentInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1; info.pWaitSemaphores = &swapchain_->ready[image];
        info.swapchainCount = 1; info.pSwapchains = &swapchain_->handle; info.pImageIndices = &image;
        const auto presented = vkQueuePresentKHR(c_.queue, &info);
        if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR || acquired == VK_SUBOPTIMAL_KHR)
            swapchain_->dirty = true;
        if (presented != VK_ERROR_OUT_OF_DATE_KHR && presented != VK_SUBOPTIMAL_KHR)
            check(presented, "present frame");
        return presented != VK_ERROR_OUT_OF_DATE_KHR;
    }

};
} // namespace
std::unique_ptr<RenderDevice> create_vulkan_device(const DeviceOptions& options) {
    std::unique_ptr<VulkanDevice> device(new VulkanDevice);
    device->init(options);
    return std::unique_ptr<RenderDevice>(device.release());
}
std::unique_ptr<RenderDevice> create_vulkan_window_device(const VulkanSurface& surface, const DeviceOptions& options) {
    std::unique_ptr<VulkanDevice> device(new VulkanDevice);
    device->init(options, &surface);
    return std::unique_ptr<RenderDevice>(device.release());
}

} } // namespace easy3d::graphics
