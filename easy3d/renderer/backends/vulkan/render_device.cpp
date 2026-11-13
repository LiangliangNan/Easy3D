/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */
// Copyright (C) 2026 Liangliang Nan
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/backends/clipping.h>
#include <easy3d/renderer/backends/vulkan/loader.h>
#include <easy3d/renderer/backends/vulkan/surface.h>
#include <vulkan/vulkan.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>

namespace easy3d {
    namespace graphics {
        namespace {
#include "scene_vert.inc"
#include "scene_frag.inc"
#include "points_vert.inc"
#include "points_frag.inc"
#include "points_plain_frag.inc"
#include "lines_vert.inc"
#include "lines_frag.inc"
#include "wide_lines_vert.inc"
#include "wide_lines_frag.inc"
#include "overlay_vert.inc"
#include "overlay_frag.inc"
#include "hud_frag.inc"
#include "depth_texture_frag.inc"
#include "edl_frag.inc"
#include "ssao_frag.inc"
#include "ao_gtao_comp.inc"
#include "ao_blur_comp.inc"
#include "shadow_vert.inc"
#include "shadow_frag.inc"
#include "shadow_depth_frag.inc"
#include "point_selection_comp.inc"
#include "oit_quad_vert.inc"
#include "oit_average_frag.inc"
#include "oit_peel_frag.inc"
#include "oit_average_composite_frag.inc"
#include "oit_peel_composite_frag.inc"
#include "oit_linked_lists_build_frag.inc"
#include "oit_linked_lists_resolve_frag.inc"

            void check(VkResult result, const char *operation) {
                if (result != VK_SUCCESS)
                    throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) +
                                             ")");
            }

            void check_initialization(VkResult result, const char *operation) {
                if (result == VK_ERROR_INCOMPATIBLE_DRIVER || result == VK_ERROR_EXTENSION_NOT_PRESENT ||
                    result == VK_ERROR_LAYER_NOT_PRESENT || result == VK_ERROR_FEATURE_NOT_PRESENT ||
                    result == VK_ERROR_INITIALIZATION_FAILED)
                    throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) +
                                             ")");
                check(result, operation);
            }

            template <class T> bool named(const std::vector<T> &values, const char *name) {
                for (const auto &v : values)
                    if (std::strcmp(v.extensionName, name) == 0)
                        return true;
                return false;
            }
            std::vector<VkExtensionProperties> instance_extensions(const internal::VulkanDispatch &dispatch,
                                                                   const char *layer = nullptr) {
                uint32_t count = 0;
                check_initialization(dispatch.vkEnumerateInstanceExtensionProperties(layer, &count, nullptr),
                                     "enumerate instance extensions");
                std::vector<VkExtensionProperties> values(count);
                check_initialization(dispatch.vkEnumerateInstanceExtensionProperties(layer, &count, values.data()),
                                     "enumerate instance extensions");
                return values;
            }

            // No mutable global device dispatch or resources: loader dispatch is handle based.
            struct Context : internal::VulkanDispatch {
                // Retain the library until all instance/device cleanup has finished.
                std::shared_ptr<internal::VulkanLoader> library;
                bool sample_shading = false;
                bool large_points = false;
                bool independent_blend = false;
                float min_point_size = 1.0f;
                float max_point_size = 1.0f;
                bool compute = false;
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
                bool profile_enabled = [] {
                    const char *value = std::getenv("EASY3D_VULKAN_PROFILE");
                    return value && *value;
                }();
                std::uint64_t profile_submissions = 0;
                std::uint64_t profile_staging_allocations = 0;
                std::uint64_t profile_staging_bytes = 0;
                std::chrono::nanoseconds profile_execute_time{};
                std::chrono::nanoseconds profile_record_time{};
                std::chrono::nanoseconds profile_submit_time{};
                std::chrono::nanoseconds profile_fence_wait_time{};
                std::chrono::nanoseconds profile_staging_create_time{};
                std::chrono::nanoseconds profile_staging_write_time{};
                ~Context() {
                    if (device) {
                        // Shutdown only. Normal rendering waits on its submission fence.
                        vkDeviceWaitIdle(device);
                        if (profile_enabled) {
                            const auto milliseconds = [](std::chrono::nanoseconds value) {
                                return std::chrono::duration<double, std::milli>(value).count();
                            };
                            std::clog << "Vulkan profile: " << profile_submissions << " submissions, "
                                      << milliseconds(profile_execute_time) << " ms execute wall, "
                                      << milliseconds(profile_record_time) << " ms record CPU, "
                                      << milliseconds(profile_submit_time) << " ms queue-submit CPU, "
                                      << milliseconds(profile_fence_wait_time) << " ms fence wait, "
                                      << profile_staging_allocations << " staging allocations, "
                                      << profile_staging_bytes << " staging bytes, "
                                      << milliseconds(profile_staging_create_time) << " ms staging create CPU, "
                                      << milliseconds(profile_staging_write_time) << " ms staging write CPU, "
                                      << "largePoints=" << (large_points ? "enabled" : "disabled") << ", device="
                                      << properties.deviceName << '\n';
                        }
                        if (fence)
                            vkDestroyFence(device, fence, nullptr);
                        if (pool)
                            vkDestroyCommandPool(device, pool, nullptr);
                        vkDestroyDevice(device, nullptr);
                    }
                    if (surface)
                        vkDestroySurfaceKHR(instance, surface, nullptr);
                    if (messenger) {
                        auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
                        if (destroy)
                            destroy(instance, messenger, nullptr);
                    }
                    // A malformed loader can fail before instance dispatch is complete.
                    if (instance && vkDestroyInstance)
                        vkDestroyInstance(instance, nullptr);
                }
                static VKAPI_ATTR VkBool32 VKAPI_CALL debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT,
                                                            const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                            void *user) {
                    auto *self = static_cast<Context *>(user);
                    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                        ++self->errors;
                    std::cerr << "Vulkan validation: " << data->pMessage << '\n';
                    return VK_FALSE;
                }
                void init(bool validation,
                          bool sync_validation,
                          bool profiling,
                          bool use_native_large_points,
                          const std::string &device_name,
                          const VulkanSurface *window = nullptr) {
                    profile_enabled = profile_enabled || profiling;
                    library = window && window->loader ? window->loader : internal::VulkanLoader::open();
                    load_global(*library);
                    validation = validation || sync_validation;
                    auto available = instance_extensions(*this);
                    std::vector<const char *> extensions, layers;
                    if (window) {
                        if (!window->create)
                            throw std::invalid_argument("surface creation callback is required");
                        for (const auto &name : window->instance_extensions) {
                            if (!named(available, name.c_str()))
                                throw std::runtime_error("required surface extension unavailable: " + name);
                            if (std::find_if(extensions.begin(), extensions.end(),
                                             [&](const char *e) { return name == e; }) == extensions.end())
                                extensions.push_back(name.c_str());
                        }
                    }
                    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
                    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    debug_info.messageSeverity =
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    debug_info.pfnUserCallback = debug;
                    debug_info.pUserData = this;
                    if (validation) {
                        uint32_t count = 0;
                        check_initialization(vkEnumerateInstanceLayerProperties(&count, nullptr),
                                             "enumerate validation layers");
                        std::vector<VkLayerProperties> values(count);
                        check_initialization(vkEnumerateInstanceLayerProperties(&count, values.data()),
                                             "enumerate validation layers");
                        bool found = false;
                        for (const auto &v : values)
                            if (!std::strcmp(v.layerName, "VK_LAYER_KHRONOS_validation"))
                                found = true;
                        if (!found)
                            throw std::runtime_error("VK_LAYER_KHRONOS_validation was requested but is unavailable");
                        if (!named(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
                            throw std::runtime_error("VK_EXT_debug_utils is required for validation reporting");
                        layers.push_back("VK_LAYER_KHRONOS_validation");
                        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    }
                    VkValidationFeaturesEXT validation_features{};
                    VkValidationFeatureEnableEXT sync = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
                    if (sync_validation) {
                        const auto layer_extensions = instance_extensions(*this, "VK_LAYER_KHRONOS_validation");
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
                    std::vector<const char *> unique_extensions;
                    for (const char *name : extensions)
                        if (std::find_if(unique_extensions.begin(), unique_extensions.end(), [&](const char *other) {
                                return std::strcmp(name, other) == 0;
                            }) == unique_extensions.end())
                            unique_extensions.push_back(name);
                    extensions = std::move(unique_extensions);
                    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                    info.ppEnabledExtensionNames = extensions.data();
                    check_initialization(vkCreateInstance(&info, nullptr, &instance), "create Vulkan 1.2 instance");
                    load_instance(instance, window != nullptr);
                    if (window) {
                        check_initialization(window->create(instance, &surface), "create native Vulkan surface");
                        if (!surface)
                            throw std::runtime_error("surface callback returned a null surface");
                    }
                    if (validation) {
                        auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
                        if (!create)
                            throw std::runtime_error("debug messenger entry point unavailable");
                        debug_info.pNext = nullptr;
                        check_initialization(create(instance, &debug_info, nullptr, &messenger),
                                             "create debug messenger");
                    }
                    uint32_t count = 0;
                    check_initialization(vkEnumeratePhysicalDevices(instance, &count, nullptr),
                                         "enumerate Vulkan devices");
                    std::vector<VkPhysicalDevice> devices(count);
                    check_initialization(vkEnumeratePhysicalDevices(instance, &count, devices.data()),
                                         "enumerate Vulkan devices");
                    int best_score = -1;
                    for (auto candidate : devices) {
                        VkPhysicalDeviceProperties props;
                        vkGetPhysicalDeviceProperties(candidate, &props);
                        if (props.apiVersion < VK_API_VERSION_1_2 ||
                            std::string(props.deviceName).find(device_name) == std::string::npos)
                            continue;
                        const std::pair<VkFormat, VkFormatFeatureFlags> formats[] = {
                            {VK_FORMAT_R8G8B8A8_UNORM,
                             VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                                 VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                 VK_FORMAT_FEATURE_TRANSFER_DST_BIT},
                            {VK_FORMAT_R32G32_UINT,
                             VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT},
                            {VK_FORMAT_D32_SFLOAT,
                             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT}};
                        bool suitable = true;
                        for (const auto &format : formats) {
                            VkFormatProperties p;
                            vkGetPhysicalDeviceFormatProperties(candidate, format.first, &p);
                            suitable &= (p.optimalTilingFeatures & format.second) == format.second;
                        }
                        if (!suitable)
                            continue;
                        if (surface) {
                            VkFormatProperties source;
                            vkGetPhysicalDeviceFormatProperties(candidate, VK_FORMAT_R8G8B8A8_UNORM, &source);
                            if (!(source.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
                                continue;
                            uint32_t extension_count = 0;
                            check_initialization(
                                vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extension_count, nullptr),
                                "enumerate WSI extensions");
                            std::vector<VkExtensionProperties> supported(extension_count);
                            check_initialization(vkEnumerateDeviceExtensionProperties(
                                                     candidate, nullptr, &extension_count, supported.data()),
                                                 "enumerate WSI extensions");
                            if (!named(supported, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                                continue;
                        }
                        uint32_t families = 0;
                        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &families, nullptr);
                        std::vector<VkQueueFamilyProperties> queues(families);
                        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &families, queues.data());
                        for (uint32_t i = 0; i < families; ++i) {
                            if (!(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                                continue;
                            if (surface) {
                                VkBool32 supported = VK_FALSE;
                                check_initialization(
                                    vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &supported),
                                    "query presentation queue");
                                if (!supported)
                                    continue;
                            }
                            int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 3
                                        : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2
                                                                                                     : 1;
                            score = score * 2 + bool(queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT);
                            if (score > best_score) {
                                best_score = score;
                                physical = candidate;
                                family = i;
                                properties = props;
                                compute = (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
                            }
                        }
                    }
                    if (!physical)
                        throw std::runtime_error(
                            "no suitable Vulkan 1.2 device/graphics queue for requested targets and surface");
                    vkGetPhysicalDeviceMemoryProperties(physical, &memory);
                    count = 0;
                    check_initialization(vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr),
                                         "enumerate device extensions");
                    std::vector<VkExtensionProperties> device_extensions(count);
                    check_initialization(
                        vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, device_extensions.data()),
                        "enumerate device extensions");
                    extensions.clear();
                    if (surface)
                        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                    // This extension is mandatory when advertised by a portability device.
                    if (named(device_extensions, "VK_KHR_portability_subset"))
                        extensions.push_back("VK_KHR_portability_subset");
                    float priority = 1;
                    VkDeviceQueueCreateInfo qi{};
                    qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    qi.queueFamilyIndex = family;
                    qi.queueCount = 1;
                    qi.pQueuePriorities = &priority;
                    VkDeviceCreateInfo di{};
                    di.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                    di.queueCreateInfoCount = 1;
                    di.pQueueCreateInfos = &qi;
                    di.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                    di.ppEnabledExtensionNames = extensions.data();
                    VkPhysicalDeviceFeatures features{}, enabled{};
                    vkGetPhysicalDeviceFeatures(physical, &features);
                    sample_shading = features.sampleRateShading;
                    large_points = features.largePoints && use_native_large_points;
                    independent_blend = features.independentBlend;
                    min_point_size = large_points ? properties.limits.pointSizeRange[0] : 1.0f;
                    max_point_size = large_points ? properties.limits.pointSizeRange[1] : 1.0f;
                    enabled.sampleRateShading = features.sampleRateShading;
                    enabled.largePoints = large_points;
                    enabled.independentBlend = features.independentBlend;
                    // Per-pixel linked lists OIT writes and atomically updates storage buffers
                    // from the fragment stage (build and resolve passes).
                    enabled.fragmentStoresAndAtomics = features.fragmentStoresAndAtomics;
                    di.pEnabledFeatures = &enabled;
                    // No geometry shaders or wide lines. Large points are an optional
                    // optimization for plain points; independent blending lets transparent
                    // scene triangles blend color without blending the integer ID target.
                    check_initialization(vkCreateDevice(physical, &di, nullptr, &device), "create device");
                    load_device(device, window != nullptr);
                    vkGetDeviceQueue(device, family, 0, &queue);
                    VkCommandPoolCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    pi.queueFamilyIndex = family;
                    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    check_initialization(vkCreateCommandPool(device, &pi, nullptr, &pool), "create command pool");
                    VkCommandBufferAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    ai.commandPool = pool;
                    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    ai.commandBufferCount = 1;
                    check_initialization(vkAllocateCommandBuffers(device, &ai, &command), "allocate command buffer");
                    VkFenceCreateInfo fi{};
                    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    check_initialization(vkCreateFence(device, &fi, nullptr, &fence), "create submission fence");
                }
                uint32_t
                memory_type(uint32_t mask, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const {
                    uint32_t fallback = UINT32_MAX;
                    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
                        if (!(mask & (1u << i)) || (memory.memoryTypes[i].propertyFlags & required) != required)
                            continue;
                        if ((memory.memoryTypes[i].propertyFlags & preferred) == preferred)
                            return i;
                        fallback = i;
                    }
                    if (fallback == UINT32_MAX)
                        throw std::runtime_error("no compatible Vulkan memory type");
                    return fallback;
                }
                void execute(const std::function<void(VkCommandBuffer)> &record,
                             VkSemaphore wait = VK_NULL_HANDLE,
                             VkSemaphore signal = VK_NULL_HANDLE) {
                    const auto execute_start = profile_enabled ? std::chrono::steady_clock::now()
                                                               : std::chrono::steady_clock::time_point{};
                    check(vkResetCommandBuffer(command, 0), "reset command buffer");
                    VkCommandBufferBeginInfo begin{};
                    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    check(vkBeginCommandBuffer(command, &begin), "begin command buffer");
                    const auto record_start = profile_enabled ? std::chrono::steady_clock::now()
                                                              : std::chrono::steady_clock::time_point{};
                    record(command);
                    if (profile_enabled)
                        profile_record_time += std::chrono::steady_clock::now() - record_start;
                    check(vkEndCommandBuffer(command), "end command buffer");
                    check(vkResetFences(device, 1, &fence), "reset fence");
                    VkSubmitInfo si{};
                    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    si.commandBufferCount = 1;
                    si.pCommandBuffers = &command;
                    const VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                    if (wait) {
                        si.waitSemaphoreCount = 1;
                        si.pWaitSemaphores = &wait;
                        si.pWaitDstStageMask = &stage;
                    }
                    if (signal) {
                        si.signalSemaphoreCount = 1;
                        si.pSignalSemaphores = &signal;
                    }
                    const auto submit_start = profile_enabled ? std::chrono::steady_clock::now()
                                                              : std::chrono::steady_clock::time_point{};
                    check(vkQueueSubmit(queue, 1, &si, fence), "submit graphics commands");
                    if (profile_enabled)
                        profile_submit_time += std::chrono::steady_clock::now() - submit_start;
                    const auto wait_start = profile_enabled ? std::chrono::steady_clock::now()
                                                            : std::chrono::steady_clock::time_point{};
                    check(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), "wait for capture");
                    if (profile_enabled) {
                        profile_fence_wait_time += std::chrono::steady_clock::now() - wait_start;
                        profile_execute_time += std::chrono::steady_clock::now() - execute_start;
                        ++profile_submissions;
                    }
                }
            };

            struct Buffer {
                Context &c;
                VkBuffer handle = VK_NULL_HANDLE;
                VkDeviceMemory memory = VK_NULL_HANDLE;
                bool coherent = false;
                explicit Buffer(Context &context) : c(context) {}
                Buffer(const Buffer &) = delete;
                ~Buffer() {
                    if (handle)
                        c.vkDestroyBuffer(c.device, handle, nullptr);
                    if (memory)
                        c.vkFreeMemory(c.device, memory, nullptr);
                }
                void init(VkDeviceSize size, VkBufferUsageFlags usage, bool host) {
                    VkBufferCreateInfo bi{};
                    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                    bi.size = size;
                    bi.usage = usage;
                    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    check(c.vkCreateBuffer(c.device, &bi, nullptr, &handle), "create buffer");
                    VkMemoryRequirements requirements;
                    c.vkGetBufferMemoryRequirements(c.device, handle, &requirements);
                    uint32_t type = c.memory_type(
                        requirements.memoryTypeBits, host ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0,
                        host ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    coherent = (c.memory.memoryTypes[type].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
                    VkMemoryAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    ai.allocationSize = requirements.size;
                    ai.memoryTypeIndex = type;
                    check(c.vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate buffer memory");
                    check(c.vkBindBufferMemory(c.device, handle, memory, 0), "bind buffer memory");
                }
                void transfer(void *data, size_t size, bool write) {
                    void *mapped = nullptr;
                    check(c.vkMapMemory(c.device, memory, 0, VK_WHOLE_SIZE, 0, &mapped), "map host buffer");
                    VkMappedMemoryRange range{};
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.memory = memory;
                    range.size = VK_WHOLE_SIZE;
                    VkResult result = VK_SUCCESS;
                    if (write) {
                        std::memcpy(mapped, data, size);
                        if (!coherent)
                            result = c.vkFlushMappedMemoryRanges(c.device, 1, &range);
                    } else {
                        if (!coherent)
                            result = c.vkInvalidateMappedMemoryRanges(c.device, 1, &range);
                        if (result == VK_SUCCESS)
                            std::memcpy(data, mapped, size);
                    }
                    c.vkUnmapMemory(c.device, memory);
                    check(result, write ? "flush host buffer" : "invalidate readback buffer");
                }
            };
            struct GpuPoint {
                float position[3];
                uint32_t source;
            };
            static_assert(sizeof(GpuPoint) == 16, "point vertex/storage layout");
            struct GpuPoints {
                std::weak_ptr<const PointData> source;
                Buffer buffer;
                explicit GpuPoints(Context &c) : buffer(c) {}
            };
            // Separate compute resources: no render pass, framebuffer, depth or picking IDs.
            struct PointSelector {
                Context &c;
                VkDescriptorSetLayout descriptors = VK_NULL_HANDLE;
                VkDescriptorPool pool = VK_NULL_HANDLE;
                VkDescriptorSet set = VK_NULL_HANDLE;
                VkPipelineLayout layout = VK_NULL_HANDLE;
                VkShaderModule shader = VK_NULL_HANDLE;
                VkPipeline pipeline = VK_NULL_HANDLE;
                explicit PointSelector(Context &context) : c(context) {}
                ~PointSelector() {
                    if (pipeline)
                        c.vkDestroyPipeline(c.device, pipeline, nullptr);
                    if (shader)
                        c.vkDestroyShaderModule(c.device, shader, nullptr);
                    if (layout)
                        c.vkDestroyPipelineLayout(c.device, layout, nullptr);
                    if (pool)
                        c.vkDestroyDescriptorPool(c.device, pool, nullptr);
                    if (descriptors)
                        c.vkDestroyDescriptorSetLayout(c.device, descriptors, nullptr);
                }
                void init() {
                    VkDescriptorSetLayoutBinding bindings[3] = {};
                    for (unsigned i = 0; i < 3; ++i) {
                        bindings[i].binding = i;
                        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        bindings[i].descriptorCount = 1;
                        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                    }
                    VkDescriptorSetLayoutCreateInfo di{};
                    di.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    di.bindingCount = 3;
                    di.pBindings = bindings;
                    check(c.vkCreateDescriptorSetLayout(c.device, &di, nullptr, &descriptors),
                          "create selection descriptor layout");
                    VkDescriptorPoolSize size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3};
                    VkDescriptorPoolCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    pi.maxSets = 1;
                    pi.poolSizeCount = 1;
                    pi.pPoolSizes = &size;
                    check(c.vkCreateDescriptorPool(c.device, &pi, nullptr, &pool), "create selection descriptor pool");
                    VkDescriptorSetAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ai.descriptorPool = pool;
                    ai.descriptorSetCount = 1;
                    ai.pSetLayouts = &descriptors;
                    check(c.vkAllocateDescriptorSets(c.device, &ai, &set), "allocate selection descriptor set");
                    VkPushConstantRange range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 128};
                    VkPipelineLayoutCreateInfo li{};
                    li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    li.setLayoutCount = 1;
                    li.pSetLayouts = &descriptors;
                    li.pushConstantRangeCount = 1;
                    li.pPushConstantRanges = &range;
                    check(c.vkCreatePipelineLayout(c.device, &li, nullptr, &layout),
                          "create selection pipeline layout");
                    VkShaderModuleCreateInfo si{};
                    si.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    si.codeSize = sizeof(point_selection_comp);
                    si.pCode = point_selection_comp;
                    check(c.vkCreateShaderModule(c.device, &si, nullptr, &shader), "create selection compute shader");
                    VkComputePipelineCreateInfo ci{};
                    ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                    ci.layout = layout;
                    ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                    ci.stage.module = shader;
                    ci.stage.pName = "main";
                    check(c.vkCreateComputePipelines(c.device, VK_NULL_HANDLE, 1, &ci, nullptr, &pipeline),
                          "create selection compute pipeline");
                }
                std::vector<uint32_t> run(const PointSelectionRequest &request,
                                          std::vector<uint32_t> words,
                                          const std::vector<vec4> &planes,
                                          Buffer &positions) {
                    const auto count = request.points->positions().size();
                    const VkDeviceSize sizes[] = {count * sizeof(GpuPoint), request.polygon.size() * sizeof(vec2),
                                                  words.size() * sizeof(uint32_t)};
                    Buffer polygon(c), selection(c);
                    Buffer *buffers[] = {&positions, &polygon, &selection};
                    VkDescriptorBufferInfo infos[3] = {};
                    VkWriteDescriptorSet writes[3] = {};
                    for (unsigned i = 0; i < 3; ++i) {
                        if (i)
                            buffers[i]->init(sizes[i], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
                        infos[i] = {buffers[i]->handle, 0, sizes[i]};
                        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writes[i].dstSet = set;
                        writes[i].dstBinding = i;
                        writes[i].descriptorCount = 1;
                        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        writes[i].pBufferInfo = &infos[i];
                    }
                    polygon.transfer(const_cast<vec2 *>(request.polygon.data()), size_t(sizes[1]), true);
                    selection.transfer(words.data(), size_t(sizes[2]), true);
                    c.vkUpdateDescriptorSets(c.device, 3, writes, 0, nullptr);
                    struct Parameters {
                        float mvp[16], planes[8];
                        uint32_t dimensions[4], options[4];
                    } push{};
                    static_assert(sizeof(Parameters) == 128, "compute selection push layout");
                    for (unsigned col = 0; col < 4; ++col)
                        for (unsigned row = 0; row < 4; ++row)
                            push.mvp[col * 4 + row] = request.model_view_projection(row, col);
                    for (size_t i = 0; i < planes.size(); ++i)
                        for (unsigned j = 0; j < 4; ++j)
                            push.planes[4 * i + j] = planes[i][j];
                    push.dimensions[0] = request.width;
                    push.dimensions[1] = request.height;
                    push.dimensions[2] = uint32_t(count);
                    push.dimensions[3] = uint32_t(request.polygon.size());
                    push.options[0] = request.deselect;
                    push.options[1] = uint32_t(planes.size());
                    const uint32_t groups = uint32_t((count + 63) / 64);
                    const uint32_t x = std::min(groups, c.properties.limits.maxComputeWorkGroupCount[0]);
                    const uint32_t y = (groups + x - 1) / x;
                    if (y > c.properties.limits.maxComputeWorkGroupCount[1])
                        throw std::invalid_argument("selection exceeds compute dispatch limits");
                    c.execute([&](VkCommandBuffer cmd) {
                        VkMemoryBarrier barrier{};
                        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        c.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                               1, &barrier, 0, nullptr, 0, nullptr);
                        c.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
                        c.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &set, 0, nullptr);
                        c.vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
                        c.vkCmdDispatch(cmd, x, y, 1);
                        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                        barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
                        c.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                                               1, &barrier, 0, nullptr, 0, nullptr);
                    });
                    selection.transfer(words.data(), size_t(sizes[2]), false);
                    return words;
                }
            };
            struct GpuImage {
                Context &c;
                VkImage handle = VK_NULL_HANDLE;
                VkImageView view = VK_NULL_HANDLE;
                VkDeviceMemory memory = VK_NULL_HANDLE;
                explicit GpuImage(Context &context) : c(context) {}
                GpuImage(const GpuImage &) = delete;
                ~GpuImage() {
                    if (view)
                        c.vkDestroyImageView(c.device, view, nullptr);
                    if (handle)
                        c.vkDestroyImage(c.device, handle, nullptr);
                    if (memory)
                        c.vkFreeMemory(c.device, memory, nullptr);
                }
                void init(uint32_t width,
                          uint32_t height,
                          VkFormat format,
                          VkImageUsageFlags usage,
                          VkImageAspectFlags aspect,
                          VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
                    VkImageCreateInfo ii{};
                    ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                    ii.imageType = VK_IMAGE_TYPE_2D;
                    ii.format = format;
                    ii.extent = {width, height, 1};
                    ii.mipLevels = 1;
                    ii.arrayLayers = 1;
                    ii.samples = samples;
                    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
                    ii.usage = usage;
                    ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    check(c.vkCreateImage(c.device, &ii, nullptr, &handle), "create image");
                    VkMemoryRequirements requirements;
                    c.vkGetImageMemoryRequirements(c.device, handle, &requirements);
                    VkMemoryAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    ai.allocationSize = requirements.size;
                    ai.memoryTypeIndex =
                        c.memory_type(requirements.memoryTypeBits, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    check(c.vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate image memory");
                    check(c.vkBindImageMemory(c.device, handle, memory, 0), "bind image memory");
                    VkImageViewCreateInfo vi{};
                    vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    vi.image = handle;
                    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    vi.format = format;
                    vi.subresourceRange = {aspect, 0, 1, 0, 1};
                    check(c.vkCreateImageView(c.device, &vi, nullptr, &view), "create image view");
                }
            };
            void image_barrier(const Context &c,
                               VkCommandBuffer cmd,
                               VkImage image,
                               VkImageAspectFlags aspect,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout,
                               VkPipelineStageFlags src_stage,
                               VkPipelineStageFlags dst_stage,
                               VkAccessFlags src_access,
                               VkAccessFlags dst_access) {
                VkImageMemoryBarrier b{};
                b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                b.srcAccessMask = src_access;
                b.dstAccessMask = dst_access;
                b.oldLayout = old_layout;
                b.newLayout = new_layout;
                b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                b.image = image;
                b.subresourceRange = {aspect, 0, 1, 0, 1};
                c.vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &b);
            }
            enum class PassKind { color_ids, multisample_color, ids };
            struct RenderPass {
                Context &c;
                PassKind kind;
                VkSampleCountFlagBits samples;
                VkRenderPass handle = VK_NULL_HANDLE;
                VkPipeline pipelines[7] = {};
                VkPipeline overlay_pipeline = VK_NULL_HANDLE;
                VkPipeline depth_overlay_pipeline = VK_NULL_HANDLE;
                VkPipeline hud_3d_pipeline = VK_NULL_HANDLE;
                VkPipeline depth_texture_pipeline = VK_NULL_HANDLE;
                VkPipeline edl_pipeline = VK_NULL_HANDLE;
                VkPipeline ssao_pipeline = VK_NULL_HANDLE;
                VkPipeline shadow_pipeline = VK_NULL_HANDLE;
                VkPipeline transparent_pipeline = VK_NULL_HANDLE;
                // Depth-tested picking pass for order-independent transparent surfaces.
                VkPipeline oit_id_pipeline = VK_NULL_HANDLE;
                // Full-screen order-independent transparency composites (scene subpass).
                VkPipeline oit_average_pipeline = VK_NULL_HANDLE;
                VkPipeline oit_peel_pipeline = VK_NULL_HANDLE;
                bool independent_depth = false;
                bool depth_preview = false;
                RenderPass(Context &context, PassKind mode, VkSampleCountFlagBits count)
                    : c(context), kind(mode), samples(count) {}
                ~RenderPass() {
                    for (auto p : pipelines)
                        if (p)
                            c.vkDestroyPipeline(c.device, p, nullptr);
                    if (overlay_pipeline)
                        c.vkDestroyPipeline(c.device, overlay_pipeline, nullptr);
                    if (depth_overlay_pipeline)
                        c.vkDestroyPipeline(c.device, depth_overlay_pipeline, nullptr);
                    if (hud_3d_pipeline)
                        c.vkDestroyPipeline(c.device, hud_3d_pipeline, nullptr);
                    if (depth_texture_pipeline)
                        c.vkDestroyPipeline(c.device, depth_texture_pipeline, nullptr);
                    if (edl_pipeline)
                        c.vkDestroyPipeline(c.device, edl_pipeline, nullptr);
                    if (ssao_pipeline)
                        c.vkDestroyPipeline(c.device, ssao_pipeline, nullptr);
                    if (shadow_pipeline)
                        c.vkDestroyPipeline(c.device, shadow_pipeline, nullptr);
                    if (transparent_pipeline)
                        c.vkDestroyPipeline(c.device, transparent_pipeline, nullptr);
                    if (oit_id_pipeline)
                        c.vkDestroyPipeline(c.device, oit_id_pipeline, nullptr);
                    if (oit_average_pipeline)
                        c.vkDestroyPipeline(c.device, oit_average_pipeline, nullptr);
                    if (oit_peel_pipeline)
                        c.vkDestroyPipeline(c.device, oit_peel_pipeline, nullptr);
                    if (handle)
                        c.vkDestroyRenderPass(c.device, handle, nullptr);
                }
                void init() {
                    VkAttachmentDescription attachments[4] = {};
                    const bool msaa = kind == PassKind::multisample_color;
                    const bool ids_only = kind == PassKind::ids;
                    const VkFormat formats[] = {ids_only ? VK_FORMAT_R32G32_UINT : VK_FORMAT_R8G8B8A8_UNORM,
                                                msaa || ids_only ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_R32G32_UINT,
                                                msaa ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_D32_SFLOAT,
                                                VK_FORMAT_D32_SFLOAT};
                    const unsigned count = ids_only ? 2 : independent_depth ? 4 : 3;
                    for (unsigned i = 0; i < count; ++i) {
                        auto &a = attachments[i];
                        a.format = formats[i];
                        a.samples = msaa && i < 2 ? samples : VK_SAMPLE_COUNT_1_BIT;
                        a.loadOp = msaa && i == 2 ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_CLEAR;
                        a.storeOp = msaa && i < 2 ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
                        a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        a.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        a.finalLayout = msaa && i < 2 ? (i == 0 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                                : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                                                      : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                        if (i == 3) {
                            a.samples = samples;
                            a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                            a.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                            a.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        }
                    }
                    VkAttachmentReference colors[] = {
                        {ids_only ? VK_ATTACHMENT_UNUSED : 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                        {msaa       ? VK_ATTACHMENT_UNUSED
                         : ids_only ? 0u
                                    : 1u,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
                    VkAttachmentReference depth = {msaa || ids_only ? 1u : 2u,
                                                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                    VkAttachmentReference resolves[] = {
                        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
                    VkSubpassDescription subpasses[4] = {};
                    auto &scene = subpasses[0];
                    scene.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    scene.colorAttachmentCount = 2;
                    scene.pColorAttachments = colors;
                    scene.pDepthStencilAttachment = &depth;
                    // Resolve only after depth-tested annotations and HUD composition.
                    VkAttachmentReference overlay_color = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
                    VkAttachmentReference read_depth = {depth.attachment,
                                                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
                    const uint32_t preserve[] = {1, 2};
                    auto &annotations = subpasses[1];
                    annotations.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    annotations.colorAttachmentCount = 1;
                    annotations.pColorAttachments = &overlay_color;
                    annotations.pDepthStencilAttachment = &read_depth;
                    if (!msaa) {
                        annotations.preserveAttachmentCount = 1;
                        annotations.pPreserveAttachments = preserve;
                    }
                    const unsigned flat_subpass = independent_depth ? 3 : 2;
                    VkAttachmentReference hud_depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                    if (independent_depth) {
                        auto &hud = subpasses[2];
                        hud.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        hud.colorAttachmentCount = 1;
                        hud.pColorAttachments = &overlay_color;
                        hud.pDepthStencilAttachment = &hud_depth;
                        if (!msaa) {
                            hud.preserveAttachmentCount = 2;
                            hud.pPreserveAttachments = preserve;
                        }
                    }
                    auto &overlay = subpasses[flat_subpass];
                    overlay.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    overlay.colorAttachmentCount = 1;
                    overlay.pColorAttachments = &overlay_color;
                    if (msaa)
                        overlay.pResolveAttachments = resolves;
                    else {
                        // Scene depth is sampled here by commands such as EDL, AO, shadows
                        // and depth textures. Referencing it would make the implicit
                        // attachment store a write racing with those samples, so keep the
                        // read-only layout from the annotation subpass and preserve it.
                        overlay.preserveAttachmentCount = 2;
                        overlay.pPreserveAttachments = preserve;
                    }
                    VkSubpassDependency dependencies[12] = {};
                    const auto color_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    const auto depth_stages =
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[0].dstSubpass = 0;
                    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    dependencies[0].dstStageMask = color_stage | depth_stages;
                    dependencies[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    dependencies[0].dstAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[1].srcSubpass = 0;
                    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[1].srcStageMask = color_stage | depth_stages;
                    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    dependencies[1].srcAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    dependencies[2].srcSubpass = 0;
                    dependencies[2].dstSubpass = 1;
                    dependencies[2].srcStageMask = dependencies[2].dstStageMask = color_stage | depth_stages;
                    dependencies[2].srcAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[2].dstAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    // Store operations remain depth writes for synchronization even though
                    // annotation pipelines only read the attachment during fragment tests.
                    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                    dependencies[3].srcSubpass = 1;
                    dependencies[3].dstSubpass = 2;
                    dependencies[3].srcStageMask = dependencies[3].dstStageMask = color_stage;
                    dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependencies[3].dstAccessMask =
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                    dependencies[4] = dependencies[1];
                    dependencies[4].srcSubpass = 1;
                    dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[5] = dependencies[1];
                    dependencies[5].srcSubpass = 2;
                    dependencies[5].srcStageMask = color_stage;
                    dependencies[5].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    if (independent_depth) {
                        dependencies[5].srcStageMask |= depth_stages;
                        dependencies[5].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        dependencies[6] = dependencies[3];
                        dependencies[6].srcSubpass = 2;
                        dependencies[6].dstSubpass = 3;
                        dependencies[7] = dependencies[1];
                        dependencies[7].srcSubpass = 3;
                        dependencies[7].srcStageMask = color_stage;
                        dependencies[7].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[8].srcSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[8].dstSubpass = 2;
                        dependencies[8].srcStageMask = dependencies[8].dstStageMask = depth_stages;
                        dependencies[8].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        dependencies[8].dstAccessMask =
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    }
                    unsigned dependency_count = ids_only ? 2 : independent_depth ? 9 : 6;
                    if (depth_preview) {
                        for (unsigned source = 0; source < 2; ++source) {
                            auto &d = dependencies[dependency_count++];
                            d.srcSubpass = source;
                            d.dstSubpass = flat_subpass;
                            d.srcStageMask = depth_stages;
                            d.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | depth_stages;
                            d.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                            d.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                            // Attachment stores still count as depth writes even with a
                            // read-only layout and depth-write-disabled overlay pipelines.
                            // Arbitrary UVs sample outside the destination pixel's region.
                        }
                        auto &d = dependencies[dependency_count++];
                        d.srcSubpass = flat_subpass;
                        d.dstSubpass = VK_SUBPASS_EXTERNAL;
                        d.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | depth_stages;
                        d.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        d.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        d.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    }
                    VkRenderPassCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    pi.attachmentCount = count;
                    pi.pAttachments = attachments;
                    pi.subpassCount = ids_only ? 1 : independent_depth ? 4 : 3;
                    pi.pSubpasses = subpasses;
                    pi.dependencyCount = dependency_count;
                    pi.pDependencies = dependencies;
                    check(c.vkCreateRenderPass(c.device, &pi, nullptr, &handle), "create scene render pass");
                }
            };
            struct PassCache {
                std::map<unsigned, std::unique_ptr<RenderPass>> color;
                std::unique_ptr<RenderPass> picking;

                template <typename CreatePipelines>
                RenderPass &get_color(Context &c,
                                      unsigned samples,
                                      bool independent_depth,
                                      bool depth_preview,
                                      const CreatePipelines &create_pipelines) {
                    const unsigned key = samples | (independent_depth ? 128u : 0u) | (depth_preview ? 256u : 0u);
                    auto found = color.find(key);
                    if (found != color.end())
                        return *found->second;
                    std::unique_ptr<RenderPass> pass(
                        new RenderPass(c, samples == 1 ? PassKind::color_ids : PassKind::multisample_color,
                                       static_cast<VkSampleCountFlagBits>(samples)));
                    pass->independent_depth = independent_depth;
                    pass->depth_preview = depth_preview;
                    pass->init();
                    create_pipelines(*pass);
                    auto *result = pass.get();
                    color.emplace(key, std::move(pass));
                    return *result;
                }

                template <typename CreatePipelines>
                RenderPass &get_picking(Context &c, const CreatePipelines &create_pipelines) {
                    if (!picking) {
                        std::unique_ptr<RenderPass> pass(new RenderPass(c, PassKind::ids, VK_SAMPLE_COUNT_1_BIT));
                        pass->init();
                        create_pipelines(*pass);
                        picking = std::move(pass);
                    }
                    return *picking;
                }

                void clear() {
                    color.clear();
                    picking.reset();
                }
            };
            struct Target {
                Context &c;
                uint32_t width, height;
                unsigned samples;
                RenderPass &color_pass;
                RenderPass *id_pass;
                GpuImage color, ids, depth, multisample_color, multisample_depth, hud_depth;
                VkFramebuffer framebuffer = VK_NULL_HANDLE, id_framebuffer = VK_NULL_HANDLE;
                VkDescriptorPool depth_pool = VK_NULL_HANDLE;
                VkDescriptorSet depth_set = VK_NULL_HANDLE;
                Buffer readback;
                Target(Context &context, uint32_t w, uint32_t h, RenderPass &pass, RenderPass *picking)
                    : c(context), width(w), height(h), samples(unsigned(pass.samples)), color_pass(pass),
                      id_pass(picking), color(c), ids(c), depth(c), multisample_color(c), multisample_depth(c),
                      hud_depth(c), readback(c) {}
                ~Target() {
                    if (depth_set)
                        c.vkFreeDescriptorSets(c.device, depth_pool, 1, &depth_set);
                    if (framebuffer)
                        c.vkDestroyFramebuffer(c.device, framebuffer, nullptr);
                    if (id_framebuffer)
                        c.vkDestroyFramebuffer(c.device, id_framebuffer, nullptr);
                }
                void
                create_framebuffer(VkRenderPass pass, const VkImageView *views, unsigned count, VkFramebuffer &result) {
                    VkFramebufferCreateInfo fi{};
                    fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    fi.renderPass = pass;
                    fi.attachmentCount = count;
                    fi.pAttachments = views;
                    fi.width = width;
                    fi.height = height;
                    fi.layers = 1;
                    check(c.vkCreateFramebuffer(c.device, &fi, nullptr, &result), "create scene framebuffer");
                }
                void init() {
                    ids.init(width, height, VK_FORMAT_R32G32_UINT,
                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                             VK_IMAGE_ASPECT_COLOR_BIT);
                    depth.init(width, height, VK_FORMAT_D32_SFLOAT,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   (color_pass.depth_preview ? VK_IMAGE_USAGE_SAMPLED_BIT : 0),
                               VK_IMAGE_ASPECT_DEPTH_BIT);
                    if (color_pass.kind == PassKind::ids) {
                        VkImageView views[] = {ids.view, depth.view};
                        create_framebuffer(color_pass.handle, views, 2, framebuffer);
                        return;
                    }
                    color.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                               VK_IMAGE_ASPECT_COLOR_BIT);
                    if (color_pass.independent_depth)
                        hud_depth.init(width, height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                       VK_IMAGE_ASPECT_DEPTH_BIT, color_pass.samples);
                    if (samples == 1) {
                        VkImageView views[] = {color.view, ids.view, depth.view, hud_depth.view};
                        create_framebuffer(color_pass.handle, views, color_pass.independent_depth ? 4 : 3, framebuffer);
                        if (id_pass) { // ambient occlusion resolves the center-sample depth first
                            VkImageView picking_views[] = {ids.view, depth.view};
                            create_framebuffer(id_pass->handle, picking_views, 2, id_framebuffer);
                        }
                    } else {
                        multisample_color.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
                                               color_pass.samples);
                        multisample_depth.init(width, height, VK_FORMAT_D32_SFLOAT,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                                               color_pass.samples);
                        VkImageView color_views[] = {multisample_color.view, multisample_depth.view, color.view,
                                                     hud_depth.view};
                        create_framebuffer(color_pass.handle, color_views, color_pass.independent_depth ? 4 : 3,
                                           framebuffer);
                        VkImageView picking_views[] = {ids.view, depth.view};
                        create_framebuffer(id_pass->handle, picking_views, 2, id_framebuffer);
                    }
                }
            };
            template <class Record>
            FrameImage capture_frame(Context &c, Target &target, const Record &record) {
                const size_t pixels = size_t(target.width) * target.height;
                if (!target.readback.handle)
                    target.readback.init(VkDeviceSize(pixels) * 16 + 8, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
                const size_t id_offset = (pixels * 4 + 7) & ~size_t(7);
                const size_t depth_offset = id_offset + pixels * 8;
                FrameImage result;
                result.width = target.width;
                result.height = target.height;
                result.rgba.resize(pixels * 4);
                result.depth.resize(pixels);
                result.ids.resize(pixels);
                static_assert(sizeof(FrameImage::Id) == 8, "ID readback must be tightly packed");
                c.execute([&](VkCommandBuffer cmd) {
                    record(cmd);
                    const VkImage images[] = {target.color.handle, target.ids.handle, target.depth.handle};
                    const VkDeviceSize offsets[] = {0, VkDeviceSize(id_offset), VkDeviceSize(depth_offset)};
                    for (unsigned i = 0; i < 3; ++i) {
                        VkBufferImageCopy copy{};
                        copy.bufferOffset = offsets[i];
                        copy.imageSubresource = {
                            static_cast<VkImageAspectFlags>(i == 2 ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                                  : VK_IMAGE_ASPECT_COLOR_BIT),
                            0, 0, 1};
                        copy.imageExtent = {target.width, target.height, 1};
                        c.vkCmdCopyImageToBuffer(cmd, images[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                 target.readback.handle, 1, &copy);
                    }
                    VkMemoryBarrier host{};
                    host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
                    c.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1,
                                           &host, 0, nullptr, 0, nullptr);
                });
                std::vector<unsigned char> bytes(depth_offset + pixels * 4);
                target.readback.transfer(bytes.data(), bytes.size(), false);
                std::memcpy(result.rgba.data(), bytes.data(), pixels * 4);
                std::memcpy(result.ids.data(), bytes.data() + id_offset, pixels * 8);
                std::memcpy(result.depth.data(), bytes.data() + depth_offset, pixels * 4);
                return result;
            }
            template <class Record>
            ScenePixel capture_pixel(Context &c, Target &target, uint32_t x, uint32_t y, const Record &record) {
                const VkOffset2D pixel{int32_t(x), int32_t(y)};
                c.execute([&](VkCommandBuffer cmd) {
                    record(cmd, pixel);
                    VkBufferImageCopy copy{};
                    copy.imageOffset = {pixel.x, pixel.y, 0};
                    copy.imageExtent = {1, 1, 1};
                    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                    c.vkCmdCopyImageToBuffer(cmd, target.ids.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                             target.readback.handle, 1, &copy);
                    copy.bufferOffset = 8;
                    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    c.vkCmdCopyImageToBuffer(cmd, target.depth.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                             target.readback.handle, 1, &copy);
                    VkMemoryBarrier host{};
                    host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                    host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
                    c.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1,
                                           &host, 0, nullptr, 0, nullptr);
                });
                unsigned char bytes[12];
                target.readback.transfer(bytes, sizeof(bytes), false);
                ScenePixel result;
                std::memcpy(&result.id, bytes, 8);
                std::memcpy(&result.depth, bytes + 8, 4);
                return result;
            }

            struct AoParameters {
                float projection[16], inverse_projection[16], shape[4];
            };
            // Screen-space ambient occlusion computed from a depth prepass: two compute
            // dispatches produce the horizon-based term and its bilateral denoise.
            struct Ao {
                Context &c;
                std::unique_ptr<GpuImage> raw, blurred;
                Buffer parameters;
                VkDescriptorSet set = VK_NULL_HANDLE;
                uint32_t width = 0, height = 0;
                bool initialized = false;
                explicit Ao(Context &context) : c(context), parameters(context) {}

                void ensure(VkDescriptorPool pool,
                            VkDescriptorSetLayout descriptors,
                            uint32_t target_width,
                            uint32_t target_height) {
                    if (!set) {
                        VkDescriptorSetAllocateInfo ai{};
                        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                        ai.descriptorPool = pool;
                        ai.descriptorSetCount = 1;
                        ai.pSetLayouts = &descriptors;
                        check(c.vkAllocateDescriptorSets(c.device, &ai, &set),
                              "allocate ambient-occlusion descriptors");
                        parameters.init(sizeof(AoParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
                    }
                    if (width == target_width && height == target_height)
                        return;
                    VkFormatProperties properties;
                    c.vkGetPhysicalDeviceFormatProperties(c.physical, VK_FORMAT_R32_SFLOAT, &properties);
                    if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
                        throw std::runtime_error("ambient occlusion requires R32F storage images");
                    raw.reset(new GpuImage(c));
                    raw->init(target_width, target_height, VK_FORMAT_R32_SFLOAT,
                              VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
                    blurred.reset(new GpuImage(c));
                    blurred->init(target_width, target_height, VK_FORMAT_R32_SFLOAT,
                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_IMAGE_ASPECT_COLOR_BIT);
                    width = target_width;
                    height = target_height;
                    initialized = false;
                }

                void update(VkSampler sampler,
                            VkImageView depth_view,
                            VkDescriptorSet composite_set,
                            const mat4 &projection,
                            float radius,
                            float bias) {
                    AoParameters values{};
                    const mat4 inverse_projection = inverse(projection);
                    for (unsigned col = 0; col < 4; ++col)
                        for (unsigned row = 0; row < 4; ++row) {
                            values.projection[col * 4 + row] = projection(row, col);
                            values.inverse_projection[col * 4 + row] = inverse_projection(row, col);
                        }
                    values.shape[0] = radius;
                    values.shape[1] = bias;
                    values.shape[2] = float(width);
                    values.shape[3] = float(height);
                    parameters.transfer(&values, sizeof(values), true);

                    // The scene composite samples the denoised term from its depth set.
                    VkDescriptorImageInfo composite{sampler, blurred->view,
                                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = composite_set;
                    write.dstBinding = 2;
                    write.descriptorCount = 1;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write.pImageInfo = &composite;
                    c.vkUpdateDescriptorSets(c.device, 1, &write, 0, nullptr);

                    VkDescriptorImageInfo depth{sampler, depth_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                    VkDescriptorImageInfo raw_sampled{sampler, raw->view, VK_IMAGE_LAYOUT_GENERAL};
                    VkDescriptorImageInfo raw_storage{VK_NULL_HANDLE, raw->view, VK_IMAGE_LAYOUT_GENERAL};
                    VkDescriptorImageInfo blurred_storage{VK_NULL_HANDLE, blurred->view, VK_IMAGE_LAYOUT_GENERAL};
                    VkDescriptorBufferInfo buffer{parameters.handle, 0, sizeof(AoParameters)};
                    VkWriteDescriptorSet writes[5] = {};
                    for (unsigned i = 0; i < 5; ++i) {
                        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writes[i].dstSet = set;
                        writes[i].dstBinding = i;
                        writes[i].descriptorCount = 1;
                    }
                    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writes[0].pImageInfo = &depth;
                    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    writes[1].pImageInfo = &raw_storage;
                    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writes[2].pImageInfo = &raw_sampled;
                    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    writes[3].pImageInfo = &blurred_storage;
                    writes[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    writes[4].pBufferInfo = &buffer;
                    c.vkUpdateDescriptorSets(c.device, 5, writes, 0, nullptr);
                }
            };
            struct GpuVertex {
                float position[3], normal[3], color[4], texcoord[2], highlight[4];
                uint32_t primitive;
            };
            struct ShadowMap {
                Context &c;
                uint32_t size = 0;
                GpuImage image;
                VkRenderPass pass = VK_NULL_HANDLE;
                VkFramebuffer framebuffer = VK_NULL_HANDLE;
                VkPipeline pipeline = VK_NULL_HANDLE;
                explicit ShadowMap(Context &context) : c(context), image(context) {}
                ~ShadowMap() {
                    if (pipeline)
                        c.vkDestroyPipeline(c.device, pipeline, nullptr);
                    if (framebuffer)
                        c.vkDestroyFramebuffer(c.device, framebuffer, nullptr);
                    if (pass)
                        c.vkDestroyRenderPass(c.device, pass, nullptr);
                }
                void init(uint32_t s, VkPipelineLayout layout) {
                    size = s;
                    image.init(size, size, VK_FORMAT_D32_SFLOAT,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                               VK_IMAGE_ASPECT_DEPTH_BIT);
                    VkAttachmentDescription attachment{};
                    attachment.format = VK_FORMAT_D32_SFLOAT;
                    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    VkAttachmentReference depth = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                    VkSubpassDescription subpass{};
                    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpass.pDepthStencilAttachment = &depth;
                    const auto tests =
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    VkSubpassDependency dependencies[2] = {};
                    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[0].dstSubpass = 0;
                    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | tests;
                    dependencies[0].dstStageMask = tests;
                    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[1].srcSubpass = 0;
                    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[1].srcStageMask = tests;
                    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    VkRenderPassCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    pi.attachmentCount = 1;
                    pi.pAttachments = &attachment;
                    pi.subpassCount = 1;
                    pi.pSubpasses = &subpass;
                    pi.dependencyCount = 2;
                    pi.pDependencies = dependencies;
                    check(c.vkCreateRenderPass(c.device, &pi, nullptr, &pass), "create shadow render pass");
                    VkImageView views[] = {image.view};
                    VkFramebufferCreateInfo fi{};
                    fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    fi.renderPass = pass;
                    fi.attachmentCount = 1;
                    fi.pAttachments = views;
                    fi.width = size;
                    fi.height = size;
                    fi.layers = 1;
                    check(c.vkCreateFramebuffer(c.device, &fi, nullptr, &framebuffer), "create shadow framebuffer");
                    VkShaderModule modules[2] = {};
                    try {
                        VkShaderModuleCreateInfo mi{};
                        mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        mi.codeSize = sizeof(shadow_vert);
                        mi.pCode = shadow_vert;
                        check(c.vkCreateShaderModule(c.device, &mi, nullptr, &modules[0]),
                              "create shadow vertex shader");
                        mi.codeSize = sizeof(shadow_depth_frag);
                        mi.pCode = shadow_depth_frag;
                        check(c.vkCreateShaderModule(c.device, &mi, nullptr, &modules[1]),
                              "create shadow depth fragment shader");
                        VkPipelineShaderStageCreateInfo stages[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                            stages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
                            stages[i].module = modules[i];
                            stages[i].pName = "main";
                        }
                        VkVertexInputBindingDescription binding = {0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
                        VkVertexInputAttributeDescription attributes[] = {
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, position)}};
                        VkPipelineVertexInputStateCreateInfo vertex{};
                        vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        vertex.vertexBindingDescriptionCount = 1;
                        vertex.pVertexBindingDescriptions = &binding;
                        vertex.vertexAttributeDescriptionCount = 1;
                        vertex.pVertexAttributeDescriptions = attributes;
                        VkPipelineInputAssemblyStateCreateInfo assembly{};
                        assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                        VkPipelineViewportStateCreateInfo viewport{};
                        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                        viewport.viewportCount = 1;
                        viewport.scissorCount = 1;
                        VkPipelineRasterizationStateCreateInfo raster{};
                        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                        raster.polygonMode = VK_POLYGON_MODE_FILL;
                        raster.cullMode = VK_CULL_MODE_NONE;
                        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        raster.lineWidth = 1;
                        VkPipelineMultisampleStateCreateInfo ms{};
                        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                        VkPipelineDepthStencilStateCreateInfo depth_state{};
                        depth_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                        depth_state.depthTestEnable = VK_TRUE;
                        depth_state.depthWriteEnable = VK_TRUE;
                        depth_state.depthCompareOp = VK_COMPARE_OP_LESS;
                        VkPipelineColorBlendStateCreateInfo blending{};
                        blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
                        VkPipelineDynamicStateCreateInfo dynamic{};
                        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                        dynamic.dynamicStateCount = 2;
                        dynamic.pDynamicStates = dynamic_states;
                        VkGraphicsPipelineCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                        info.stageCount = 2;
                        info.pStages = stages;
                        info.pVertexInputState = &vertex;
                        info.pInputAssemblyState = &assembly;
                        info.pViewportState = &viewport;
                        info.pRasterizationState = &raster;
                        info.pMultisampleState = &ms;
                        info.pDepthStencilState = &depth_state;
                        info.pColorBlendState = &blending;
                        info.pDynamicState = &dynamic;
                        info.layout = layout;
                        info.renderPass = pass;
                        check(c.vkCreateGraphicsPipelines(c.device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline),
                              "create shadow depth pipeline");
                    } catch (...) {
                        for (auto m : modules)
                            if (m)
                                c.vkDestroyShaderModule(c.device, m, nullptr);
                        throw;
                    }
                    for (auto m : modules)
                        if (m)
                            c.vkDestroyShaderModule(c.device, m, nullptr);
                }
            };

            struct GpuPointAttributes {
                float normal[3], color[4], texcoord[2], highlight[4];
                uint32_t primitive;
            };
            class AmbientOcclusionPipelineBuilder {
                Context &c_;
                VkDescriptorSetLayout &descriptors_;
                VkPipelineLayout &layout_;
                VkPipeline &horizons_;
                VkPipeline &denoise_;

            public:
                AmbientOcclusionPipelineBuilder(Context &context,
                                                VkDescriptorSetLayout &descriptors,
                                                VkPipelineLayout &layout,
                                                VkPipeline &horizons,
                                                VkPipeline &denoise)
                    : c_(context), descriptors_(descriptors), layout_(layout), horizons_(horizons),
                      denoise_(denoise) {}

                void create() {
                    VkDescriptorSetLayoutBinding binding[5] = {};
                    const VkDescriptorType types[5] = {
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};
                    for (unsigned i = 0; i < 5; ++i) {
                        binding[i].binding = i;
                        binding[i].descriptorType = types[i];
                        binding[i].descriptorCount = 1;
                        binding[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                    }
                    VkDescriptorSetLayoutCreateInfo li{};
                    li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    li.bindingCount = 5;
                    li.pBindings = binding;
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &descriptors_),
                          "create ambient-occlusion descriptor layout");
                    VkPipelineLayoutCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    pi.setLayoutCount = 1;
                    pi.pSetLayouts = &descriptors_;
                    check(c_.vkCreatePipelineLayout(c_.device, &pi, nullptr, &layout_),
                          "create ambient-occlusion pipeline layout");
                    VkShaderModule modules[2] = {};
                    try {
                        VkShaderModuleCreateInfo mi{};
                        mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        mi.codeSize = sizeof(ao_gtao_comp);
                        mi.pCode = ao_gtao_comp;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]),
                              "create ambient-occlusion horizon shader");
                        mi.codeSize = sizeof(ao_blur_comp);
                        mi.pCode = ao_blur_comp;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]),
                              "create ambient-occlusion blur shader");
                        VkComputePipelineCreateInfo ci{};
                        ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                        ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                        ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                        ci.stage.pName = "main";
                        ci.layout = layout_;
                        ci.stage.module = modules[0];
                        check(c_.vkCreateComputePipelines(c_.device, VK_NULL_HANDLE, 1, &ci, nullptr, &horizons_),
                              "create ambient-occlusion horizon pipeline");
                        ci.stage.module = modules[1];
                        check(c_.vkCreateComputePipelines(c_.device, VK_NULL_HANDLE, 1, &ci, nullptr, &denoise_),
                              "create ambient-occlusion blur pipeline");
                    } catch (...) {
                        for (auto module : modules)
                            if (module)
                                c_.vkDestroyShaderModule(c_.device, module, nullptr);
                        throw;
                    }
                    for (auto module : modules)
                        if (module)
                            c_.vkDestroyShaderModule(c_.device, module, nullptr);
                }
            };
            class OitDescriptorLayoutBuilder {
                Context &c_;
                VkDescriptorSetLayout &scene_;
                VkDescriptorSetLayout &composite_;
                VkDescriptorSetLayout &lists_;

            public:
                OitDescriptorLayoutBuilder(Context &context,
                                           VkDescriptorSetLayout &scene,
                                           VkDescriptorSetLayout &composite,
                                           VkDescriptorSetLayout &lists)
                    : c_(context), scene_(scene), composite_(composite), lists_(lists) {}

                void create() {
                    VkDescriptorSetLayoutCreateInfo li{};
                    li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    VkDescriptorSetLayoutBinding peel_bindings[2] = {};
                    peel_bindings[0].binding = 0;
                    peel_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    peel_bindings[0].descriptorCount = 1;
                    peel_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    peel_bindings[1] = peel_bindings[0];
                    peel_bindings[1].binding = 3;
                    li.bindingCount = 2;
                    li.pBindings = peel_bindings;
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &scene_),
                          "create transparency peel descriptor layout");

                    VkDescriptorSetLayoutBinding composite_bindings[2] = {};
                    composite_bindings[0].binding = 0;
                    composite_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    composite_bindings[0].descriptorCount = 1;
                    composite_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    composite_bindings[1] = composite_bindings[0];
                    composite_bindings[1].binding = 1;
                    li.bindingCount = 2;
                    li.pBindings = composite_bindings;
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &composite_),
                          "create transparency composite descriptor layout");

                    VkDescriptorSetLayoutBinding lists_bindings[4] = {};
                    for (unsigned i = 0; i < 3; ++i) {
                        lists_bindings[i].binding = i;
                        lists_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        lists_bindings[i].descriptorCount = 1;
                        lists_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    }
                    lists_bindings[3].binding = 3;
                    lists_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    lists_bindings[3].descriptorCount = 1;
                    lists_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    li.bindingCount = 4;
                    li.pBindings = lists_bindings;
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &lists_),
                          "create linked lists descriptor layout");
                }
            };
            class PipelineBuilder {
                Context &c_;
                VkPipelineLayout layout_;
                VkPipelineLayout oit_composite_layout_;

            public:
                PipelineBuilder(Context &context, VkPipelineLayout layout, VkPipelineLayout oit_composite_layout)
                    : c_(context), layout_(layout), oit_composite_layout_(oit_composite_layout) {}

                void create(RenderPass &pass) {
                    VkShaderModule modules[19] = {};
                    try {
                        VkShaderModuleCreateInfo mi{};
                        mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        mi.codeSize = sizeof(scene_vert);
                        mi.pCode = scene_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]), "create vertex shader");
                        mi.codeSize = sizeof(scene_frag);
                        mi.pCode = scene_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]), "create fragment shader");
                        mi.codeSize = sizeof(points_vert);
                        mi.pCode = points_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[2]),
                              "create point vertex shader");
                        mi.codeSize = sizeof(points_frag);
                        mi.pCode = points_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[3]),
                              "create point fragment shader");
                        mi.codeSize = sizeof(points_plain_frag);
                        mi.pCode = points_plain_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[18]),
                              "create plain point fragment shader");
                        mi.codeSize = sizeof(lines_vert);
                        mi.pCode = lines_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[4]),
                              "create line vertex shader");
                        mi.codeSize = sizeof(lines_frag);
                        mi.pCode = lines_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[5]),
                              "create line fragment shader");
                        mi.codeSize = sizeof(wide_lines_vert);
                        mi.pCode = wide_lines_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[6]),
                              "create wide line vertex shader");
                        mi.codeSize = sizeof(wide_lines_frag);
                        mi.pCode = wide_lines_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[7]),
                              "create wide line fragment shader");
                        mi.codeSize = sizeof(overlay_vert);
                        mi.pCode = overlay_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[8]),
                              "create overlay vertex shader");
                        mi.codeSize = sizeof(overlay_frag);
                        mi.pCode = overlay_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[9]),
                              "create overlay fragment shader");
                        if (pass.depth_preview) {
                            mi.codeSize = sizeof(depth_texture_frag);
                            mi.pCode = depth_texture_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[10]),
                                  "create depth texture fragment shader");
                            mi.codeSize = sizeof(edl_frag);
                            mi.pCode = edl_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[12]),
                                  "create eye-dome-lighting fragment shader");
                            mi.codeSize = sizeof(ssao_frag);
                            mi.pCode = ssao_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[13]),
                                  "create ambient-occlusion fragment shader");
                            mi.codeSize = sizeof(shadow_frag);
                            mi.pCode = shadow_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[14]),
                                  "create shadow fragment shader");
                        }
                        if (pass.independent_depth) {
                            mi.codeSize = sizeof(hud_frag);
                            mi.pCode = hud_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[11]),
                                  "create independent HUD fragment shader");
                        }
                        if (pass.kind != PassKind::ids) {
                            mi.codeSize = sizeof(oit_quad_vert);
                            mi.pCode = oit_quad_vert;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[15]),
                                  "create transparency composite vertex shader");
                            mi.codeSize = sizeof(oit_average_composite_frag);
                            mi.pCode = oit_average_composite_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[16]),
                                  "create average transparency composite shader");
                            mi.codeSize = sizeof(oit_peel_composite_frag);
                            mi.pCode = oit_peel_composite_frag;
                            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[17]),
                                  "create peeling transparency composite shader");
                        }
                        VkPipelineShaderStageCreateInfo stages[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                            stages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
                            stages[i].module = modules[i];
                            stages[i].pName = "main";
                        }
                        VkVertexInputBindingDescription binding = {0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
                        VkVertexInputAttributeDescription attrs[] = {
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, position)},
                            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, normal)},
                            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, color)},
                            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuVertex, texcoord)},
                            {4, 0, VK_FORMAT_R32_UINT, offsetof(GpuVertex, primitive)},
                            {9, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, highlight)}};
                        VkVertexInputAttributeDescription line_attrs[] = {
                            attrs[0],
                            attrs[2],
                            attrs[3],
                            attrs[4],
                            {5, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, position)},
                            {6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, color)},
                            {7, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, texcoord)},
                            attrs[5]};
                        VkVertexInputAttributeDescription wide_attrs[] = {
                            attrs[0],
                            attrs[1],
                            attrs[2],
                            attrs[3],
                            attrs[4],
                            line_attrs[4],
                            line_attrs[5],
                            line_attrs[6],
                            {8, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, normal)},
                            attrs[5]};
                        VkPipelineVertexInputStateCreateInfo vertex{};
                        vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        vertex.vertexBindingDescriptionCount = 1;
                        vertex.pVertexBindingDescriptions = &binding;
                        vertex.vertexAttributeDescriptionCount = 6;
                        vertex.pVertexAttributeDescriptions = attrs;
                        VkVertexInputBindingDescription point_bindings[] = {
                            {0, sizeof(GpuPoint), VK_VERTEX_INPUT_RATE_INSTANCE},
                            {1, sizeof(GpuPointAttributes), VK_VERTEX_INPUT_RATE_INSTANCE}};
                        VkVertexInputAttributeDescription point_attrs[] = {
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
                            {1, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuPointAttributes, normal)},
                            {2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuPointAttributes, color)},
                            {3, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuPointAttributes, texcoord)},
                            {4, 1, VK_FORMAT_R32_UINT, offsetof(GpuPointAttributes, primitive)},
                            {9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuPointAttributes, highlight)}};
                        VkPipelineInputAssemblyStateCreateInfo assembly{};
                        assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                        VkPipelineViewportStateCreateInfo viewport{};
                        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                        viewport.viewportCount = 1;
                        viewport.scissorCount = 1;
                        VkPipelineRasterizationStateCreateInfo raster{};
                        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                        raster.polygonMode = VK_POLYGON_MODE_FILL;
                        raster.cullMode = VK_CULL_MODE_NONE;
                        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        raster.lineWidth = 1;
                        raster.depthBiasEnable = VK_TRUE;
                        VkPipelineMultisampleStateCreateInfo ms{};
                        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                        ms.rasterizationSamples = pass.samples;
                        VkPipelineDepthStencilStateCreateInfo depth{};
                        depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                        depth.depthTestEnable = VK_TRUE;
                        depth.depthWriteEnable = VK_TRUE;
                        depth.depthCompareOp = VK_COMPARE_OP_LESS;
                        VkPipelineColorBlendAttachmentState blend[2] = {};
                        // Both attachments have identical state; no independentBlend feature required.
                        for (auto &b : blend)
                            b.colorWriteMask = 0xf;
                        VkPipelineColorBlendStateCreateInfo blending{};
                        blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                        blending.attachmentCount = 2;
                        blending.pAttachments = blend;
                        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                           VK_DYNAMIC_STATE_DEPTH_BIAS};
                        VkPipelineDynamicStateCreateInfo dynamic{};
                        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                        dynamic.dynamicStateCount = 3;
                        dynamic.pDynamicStates = dynamic_states;
                        VkGraphicsPipelineCreateInfo pi{};
                        pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                        pi.stageCount = 2;
                        pi.pStages = stages;
                        pi.pVertexInputState = &vertex;
                        pi.pInputAssemblyState = &assembly;
                        pi.pViewportState = &viewport;
                        pi.pRasterizationState = &raster;
                        pi.pMultisampleState = &ms;
                        pi.pDepthStencilState = &depth;
                        pi.pColorBlendState = &blending;
                        pi.pDynamicState = &dynamic;
                        pi.layout = layout_;
                        pi.renderPass = pass.handle;
                        const VkPrimitiveTopology topologies[] = {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                                                  VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                                                  VK_PRIMITIVE_TOPOLOGY_POINT_LIST};
                        VkBool32 native_point = VK_TRUE;
                        VkSpecializationMapEntry point_entry = {0, 0, sizeof(native_point)};
                        VkSpecializationInfo point_specialization = {1, &point_entry, sizeof(native_point),
                                                                     &native_point};
                        for (unsigned i = 0; i < 7; ++i) {
                            if (i == 2) {
                                stages[0].module = modules[2];
                                stages[1].module = modules[18];
                                stages[0].pSpecializationInfo = &point_specialization;
                                point_bindings[0].inputRate = point_bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                                vertex.vertexBindingDescriptionCount = 2;
                                vertex.pVertexBindingDescriptions = point_bindings;
                                vertex.pVertexAttributeDescriptions = point_attrs;
                            }
                            if (i == 3) {
                                stages[0].pSpecializationInfo = nullptr;
                                point_bindings[0].inputRate = point_bindings[1].inputRate =
                                    VK_VERTEX_INPUT_RATE_INSTANCE;
                                stages[0].module = modules[2];
                                stages[1].module = modules[18];
                                binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                                vertex.vertexBindingDescriptionCount = 2;
                                vertex.pVertexBindingDescriptions = point_bindings;
                                vertex.pVertexAttributeDescriptions = point_attrs;
                                ms.sampleShadingEnable = c_.sample_shading && pass.samples != VK_SAMPLE_COUNT_1_BIT;
                                ms.minSampleShading = 1;
                            }
                            if (i == 4) {
                                vertex.vertexBindingDescriptionCount = 1;
                                vertex.pVertexBindingDescriptions = &binding;
                                stages[0].module = modules[4];
                                stages[1].module = modules[5];
                                binding.stride = 2 * sizeof(GpuVertex);
                                vertex.vertexAttributeDescriptionCount = 8;
                                vertex.pVertexAttributeDescriptions = line_attrs;
                            }
                            if (i == 5) {
                                stages[0].module = modules[6];
                                stages[1].module = modules[7];
                                vertex.vertexAttributeDescriptionCount = 10;
                                vertex.pVertexAttributeDescriptions = wide_attrs;
                                ms.sampleShadingEnable = VK_FALSE;
                                ms.minSampleShading = 0;
                            }
                            if (i == 6) {
                                // Expanded impostor points (spheres and surfels). Only this
                                // point path writes gl_FragDepth, because the impostor surface
                                // depth differs from the proxy geometry depth, so only it loses
                                // early depth testing.
                                stages[0].module = modules[2];
                                stages[1].module = modules[3];
                                stages[0].pSpecializationInfo = nullptr;
                                point_bindings[0].inputRate = point_bindings[1].inputRate =
                                    VK_VERTEX_INPUT_RATE_INSTANCE;
                                binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                                vertex.vertexBindingDescriptionCount = 2;
                                vertex.pVertexBindingDescriptions = point_bindings;
                                vertex.vertexAttributeDescriptionCount = 6;
                                vertex.pVertexAttributeDescriptions = point_attrs;
                                ms.sampleShadingEnable = c_.sample_shading && pass.samples != VK_SAMPLE_COUNT_1_BIT;
                                ms.minSampleShading = 1;
                            }
                            assembly.topology = topologies[i >= 3 ? 0 : i];
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.pipelines[i]),
                                  "create pipeline");
                        }
                        if (pass.kind != PassKind::ids && c_.independent_blend) {
                            // Scene triangles with straight-alpha blending. Only the color
                            // attachment blends; the integer ID attachment keeps replacing
                            // its value so picking still returns the nearest surface.
                            stages[0].module = modules[0];
                            stages[1].module = modules[1];
                            stages[0].pSpecializationInfo = nullptr;
                            binding.stride = sizeof(GpuVertex);
                            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                            vertex.vertexBindingDescriptionCount = 1;
                            vertex.pVertexBindingDescriptions = &binding;
                            vertex.vertexAttributeDescriptionCount = 6;
                            vertex.pVertexAttributeDescriptions = attrs;
                            assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                            ms.sampleShadingEnable = VK_FALSE;
                            ms.minSampleShading = 0;
                            depth.depthTestEnable = depth.depthWriteEnable = VK_TRUE;
                            depth.depthCompareOp = VK_COMPARE_OP_LESS;
                            VkPipelineColorBlendAttachmentState transparent_blend[2] = {};
                            transparent_blend[0].blendEnable = VK_TRUE;
                            transparent_blend[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                            transparent_blend[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            transparent_blend[0].colorBlendOp = VK_BLEND_OP_ADD;
                            transparent_blend[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                            transparent_blend[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            transparent_blend[0].alphaBlendOp = VK_BLEND_OP_ADD;
                            transparent_blend[1].colorWriteMask = 0xf;
                            transparent_blend[0].colorWriteMask = 0xf;
                            blending.attachmentCount = 2;
                            blending.pAttachments = transparent_blend;
                            pi.subpass = 0;
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.transparent_pipeline),
                                  "create transparent scene pipeline");
                            if (pass.kind == PassKind::color_ids) {
                                // Order-independent methods blend the transparent surfaces in
                                // their own targets, so the scene color stays masked here while
                                // the depth test still records their IDs and depth for picking.
                                VkPipelineColorBlendAttachmentState id_only[2] = {};
                                id_only[0].colorWriteMask = 0;
                                id_only[1].colorWriteMask = 0xf;
                                blending.pAttachments = id_only;
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.oit_id_pipeline),
                                      "create transparency picking pipeline");
                                blending.pAttachments = blend;
                            }
                        }
                        if (pass.kind != PassKind::ids) {
                            stages[0].module = modules[8];
                            stages[1].module = modules[9];
                            binding.stride = sizeof(GpuVertex);
                            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                            VkVertexInputAttributeDescription overlay_attrs[] = {attrs[0], attrs[2], attrs[3]};
                            vertex.vertexAttributeDescriptionCount = 3;
                            vertex.pVertexAttributeDescriptions = overlay_attrs;
                            assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                            ms.sampleShadingEnable = VK_FALSE;
                            ms.minSampleShading = 0;
                            depth.depthTestEnable = depth.depthWriteEnable = VK_FALSE;
                            VkPipelineColorBlendAttachmentState alpha{};
                            alpha.blendEnable = VK_TRUE;
                            alpha.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                            alpha.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            alpha.colorBlendOp = VK_BLEND_OP_ADD;
                            alpha.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                            alpha.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            alpha.alphaBlendOp = VK_BLEND_OP_ADD;
                            alpha.colorWriteMask = 0xf;
                            blending.attachmentCount = 1;
                            blending.pAttachments = &alpha;
                            pi.subpass = 1;
                            depth.depthTestEnable = VK_TRUE;
                            depth.depthWriteEnable = VK_FALSE;
                            depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.depth_overlay_pipeline),
                                  "create depth-tested overlay pipeline");
                            if (pass.independent_depth) {
                                pi.subpass = 2;
                                depth.depthTestEnable = depth.depthWriteEnable = VK_TRUE;
                                depth.depthCompareOp = VK_COMPARE_OP_LESS;
                                stages[0].module = modules[0];
                                stages[1].module = modules[11]; // lit triangles, color only
                                vertex.vertexAttributeDescriptionCount = 6;
                                vertex.pVertexAttributeDescriptions = attrs;
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.hud_3d_pipeline),
                                      "create independent-depth HUD pipeline");
                                stages[0].module = modules[8];
                                stages[1].module = modules[9];
                                vertex.vertexAttributeDescriptionCount = 3;
                                vertex.pVertexAttributeDescriptions = overlay_attrs;
                            }
                            pi.subpass = pass.independent_depth ? 3 : 2;
                            depth.depthTestEnable = depth.depthWriteEnable = VK_FALSE;
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.overlay_pipeline),
                                  "create overlay pipeline");
                            if (pass.depth_preview) {
                                stages[1].module = modules[10];
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.depth_texture_pipeline),
                                      "create depth texture overlay pipeline");
                                // Multiply existing color by the shaded luminance from depth.
                                // A black source with SRC_ALPHA/ONE_MINUS_SRC_ALPHA yields
                                // dst*(1-alpha); alpha itself keeps the destination value.
                                VkPipelineColorBlendAttachmentState edl{};
                                edl.blendEnable = VK_TRUE;
                                edl.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                                edl.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                                edl.colorBlendOp = VK_BLEND_OP_ADD;
                                edl.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                                edl.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                                edl.alphaBlendOp = VK_BLEND_OP_ADD;
                                edl.colorWriteMask = 0xf;
                                blending.attachmentCount = 1;
                                blending.pAttachments = &edl;
                                stages[1].module = modules[12];
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.edl_pipeline),
                                      "create eye-dome-lighting overlay pipeline");
                                stages[1].module = modules[13];
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.ssao_pipeline),
                                      "create ambient-occlusion overlay pipeline");
                                stages[1].module = modules[14];
                                blending.attachmentCount = 1;
                                blending.pAttachments = &alpha;
                                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                                   &pass.shadow_pipeline),
                                      "create shadow overlay pipeline");
                            }
                            // Order-independent transparency composites cover the scene and
                            // leave the integer picking attachment untouched.
                            stages[0].module = modules[15];
                            stages[1].module = modules[16];
                            stages[0].pSpecializationInfo = nullptr;
                            vertex.vertexBindingDescriptionCount = 0;
                            vertex.pVertexBindingDescriptions = nullptr;
                            vertex.vertexAttributeDescriptionCount = 0;
                            vertex.pVertexAttributeDescriptions = nullptr;
                            assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                            ms.sampleShadingEnable = VK_FALSE;
                            ms.minSampleShading = 0;
                            depth.depthTestEnable = depth.depthWriteEnable = VK_FALSE;
                            VkPipelineColorBlendAttachmentState oit[2] = {};
                            oit[0].blendEnable = VK_TRUE;
                            oit[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                            oit[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            oit[0].colorBlendOp = VK_BLEND_OP_ADD;
                            oit[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                            oit[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                            oit[0].alphaBlendOp = VK_BLEND_OP_ADD;
                            oit[0].colorWriteMask = 0xf;
                            oit[1].colorWriteMask = 0;
                            blending.attachmentCount = 2;
                            blending.pAttachments = oit;
                            pi.subpass = 0;
                            pi.layout = oit_composite_layout_;
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.oit_average_pipeline),
                                  "create average transparency composite pipeline");
                            stages[1].module = modules[17];
                            check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr,
                                                               &pass.oit_peel_pipeline),
                                  "create peeling transparency composite pipeline");
                        }
                    } catch (...) {
                        for (auto m : modules)
                            if (m)
                                c_.vkDestroyShaderModule(c_.device, m, nullptr);
                        throw;
                    }
                    for (auto m : modules)
                        c_.vkDestroyShaderModule(c_.device, m, nullptr);
                }
            };

            struct GpuGeometry {
                GpuPoints *points = nullptr;
                std::weak_ptr<const Geometry> source;
                Buffer buffer;
                uint32_t count = 0;
                vec3 center = vec3(0); // vertex average, used to order transparent draws
                explicit GpuGeometry(Context &c) : buffer(c) {}
            };
            struct GpuTexture {
                Context &c;
                std::weak_ptr<const Image> source;
                GpuImage image;
                VkDescriptorPool pool;
                VkDescriptorSet set = VK_NULL_HANDLE;
                GpuTexture(Context &context, VkDescriptorPool p) : c(context), image(c), pool(p) {}
                ~GpuTexture() {
                    if (set)
                        c.vkFreeDescriptorSets(c.device, pool, 1, &set);
                }
            };
            class GpuResources {
                Context &c_;
                VkDescriptorPool descriptor_pool_;
                VkDescriptorSetLayout descriptors_;
                const VkSampler *samplers_;
                std::map<const PointData *, std::unique_ptr<GpuPoints>> points_;
                std::map<const Geometry *, std::unique_ptr<GpuGeometry>> geometry_;
                std::map<const Image *, std::unique_ptr<GpuTexture>> textures_;
                uint64_t point_uploads_ = 0;

            public:
                GpuResources(Context &context,
                             VkDescriptorPool descriptor_pool,
                             VkDescriptorSetLayout descriptors,
                             const VkSampler *samplers)
                    : c_(context), descriptor_pool_(descriptor_pool), descriptors_(descriptors), samplers_(samplers) {}
                GpuResources(const GpuResources &) = delete;
                GpuResources &operator=(const GpuResources &) = delete;
                ~GpuResources() {
                    clear();
                }
                void clear() {
                    geometry_.clear();
                    textures_.clear();
                    points_.clear();
                }
                void prune() {
                    for (auto it = geometry_.begin(); it != geometry_.end();) {
                        if (it->second->source.expired())
                            it = geometry_.erase(it);
                        else
                            ++it;
                    }
                    for (auto it = textures_.begin(); it != textures_.end();) {
                        if (it->second->source.expired())
                            it = textures_.erase(it);
                        else
                            ++it;
                    }
                    for (auto it = points_.begin(); it != points_.end();) {
                        if (it->second->source.expired())
                            it = points_.erase(it);
                        else
                            ++it;
                    }
                }
                uint64_t point_upload_count() const {
                    return point_uploads_;
                }
                GpuPoints &point(const std::shared_ptr<const PointData> &data) {
                    for (auto it = points_.begin(); it != points_.end();) {
                        if (it->second->source.expired())
                            it = points_.erase(it);
                        else
                            ++it;
                    }
                    auto found = points_.find(data.get());
                    if (found != points_.end()) {
                        if (found->second->source.lock() == data)
                            return *found->second;
                        points_.erase(found);
                    }
                    std::unique_ptr<GpuPoints> gpu(new GpuPoints(c_));
                    gpu->source = data;
                    if (!data->positions().empty()) {
                        std::vector<GpuPoint> packed(data->positions().size());
                        for (size_t i = 0; i < packed.size(); ++i) {
                            for (unsigned j = 0; j < 3; ++j)
                                packed[i].position[j] = data->positions()[i][j];
                            packed[i].source = data->source_ids()[i];
                        }
                        const VkDeviceSize bytes = packed.size() * sizeof(GpuPoint);
                        Buffer staging(c_);
                        const auto staging_create_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                             : std::chrono::steady_clock::time_point{};
                        staging.init(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
                        if (c_.profile_enabled) {
                            ++c_.profile_staging_allocations;
                            c_.profile_staging_bytes += bytes;
                            c_.profile_staging_create_time += std::chrono::steady_clock::now() - staging_create_start;
                        }
                        const auto staging_write_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                            : std::chrono::steady_clock::time_point{};
                        staging.transfer(packed.data(), size_t(bytes), true);
                        if (c_.profile_enabled)
                            c_.profile_staging_write_time += std::chrono::steady_clock::now() - staging_write_start;
                        gpu->buffer.init(bytes,
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                         false);
                        c_.execute([&](VkCommandBuffer cmd) {
                            VkBufferCopy copy = {0, 0, bytes};
                            c_.vkCmdCopyBuffer(cmd, staging.handle, gpu->buffer.handle, 1, &copy);
                            VkBufferMemoryBarrier barrier{};
                            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                            if (c_.compute)
                                barrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
                            barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            barrier.buffer = gpu->buffer.handle;
                            barrier.size = VK_WHOLE_SIZE;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
                                                        (c_.compute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : 0),
                                                    0, 0, nullptr, 1, &barrier, 0, nullptr);
                        });
                        ++point_uploads_;
                    }
                    auto *result = gpu.get();
                    points_.emplace(data.get(), std::move(gpu));
                    return *result;
                }
                GpuGeometry &geometry(const std::shared_ptr<const Geometry> &data) {
                    auto found = geometry_.find(data.get());
                    if (found != geometry_.end()) {
                        if (found->second->source.lock() == data)
                            return *found->second;
                        geometry_.erase(found);
                    }
                    std::unique_ptr<GpuGeometry> gpu(new GpuGeometry(c_));
                    gpu->source = data;
                    const size_t count = data->indices().empty() ? data->vertices().size() : data->indices().size();
                    gpu->count = static_cast<uint32_t>(count);
                    const unsigned primitive_size = data->topology() == Topology::triangles ? 3
                                                    : data->topology() == Topology::lines   ? 2
                                                                                           : 1;
                    std::vector<GpuVertex> packed(count);
                    vec3 center(0);
                    for (size_t i = 0; i < count; ++i) {
                        const auto &v = data->vertices()[data->indices().empty() ? i : data->indices()[i]];
                        auto &p = packed[i];
                        for (unsigned j = 0; j < 3; ++j) {
                            p.position[j] = v.position[j];
                            p.normal[j] = v.normal[j];
                        }
                        for (unsigned j = 0; j < 4; ++j) {
                            p.color[j] = v.color[j];
                            p.highlight[j] = v.highlight[j];
                        }
                        for (unsigned j = 0; j < 2; ++j)
                            p.texcoord[j] = v.texcoord[j];
                        p.primitive = data->primitive_ids().empty() ? static_cast<uint32_t>(i / primitive_size)
                                                                    : data->primitive_ids()[i / primitive_size];
                        center += v.position;
                    }
                    if (count)
                        gpu->center = center / float(count);
                    if (data->topology() == Topology::points)
                        gpu->points = &point(data->point_data());
                    if (count) {
                        std::vector<GpuPointAttributes> attributes;
                        const void *input = packed.data();
                        size_t bytes = packed.size() * sizeof(GpuVertex);
                        if (gpu->points) {
                            attributes.resize(count);
                            for (size_t i = 0; i < count; ++i) {
                                auto &a = attributes[i];
                                const auto &v = packed[i];
                                std::memcpy(a.normal, v.normal, sizeof(a.normal));
                                std::memcpy(a.color, v.color, sizeof(a.color));
                                std::memcpy(a.texcoord, v.texcoord, sizeof(a.texcoord));
                                std::memcpy(a.highlight, v.highlight, sizeof(a.highlight));
                                a.primitive = v.primitive;
                            }
                            input = attributes.data();
                            bytes = attributes.size() * sizeof(GpuPointAttributes);
                        }
                        Buffer staging(c_);
                        const auto staging_create_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                             : std::chrono::steady_clock::time_point{};
                        staging.init(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
                        if (c_.profile_enabled) {
                            ++c_.profile_staging_allocations;
                            c_.profile_staging_bytes += bytes;
                            c_.profile_staging_create_time += std::chrono::steady_clock::now() - staging_create_start;
                        }
                        const auto staging_write_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                            : std::chrono::steady_clock::time_point{};
                        staging.transfer(const_cast<void *>(input), bytes, true);
                        if (c_.profile_enabled)
                            c_.profile_staging_write_time += std::chrono::steady_clock::now() - staging_write_start;
                        gpu->buffer.init(bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                         false);
                        c_.execute([&](VkCommandBuffer cmd) {
                            VkBufferCopy copy = {0, 0, bytes};
                            c_.vkCmdCopyBuffer(cmd, staging.handle, gpu->buffer.handle, 1, &copy);
                            VkBufferMemoryBarrier barrier{};
                            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                            barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            barrier.buffer = gpu->buffer.handle;
                            barrier.size = VK_WHOLE_SIZE;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0,
                                                    nullptr);
                        });
                    }
                    auto *result = gpu.get();
                    geometry_.emplace(data.get(), std::move(gpu));
                    return *result;
                }
                GpuTexture &texture(const std::shared_ptr<const Image> &data) {
                    auto found = textures_.find(data.get());
                    if (found != textures_.end()) {
                        if (found->second->source.lock() == data)
                            return *found->second;
                        textures_.erase(found);
                    }
                    if (data->width() > c_.properties.limits.maxImageDimension2D ||
                        data->height() > c_.properties.limits.maxImageDimension2D)
                        throw std::invalid_argument("texture exceeds device image limits");
                    if (data->sampler().filter == TextureFilter::linear) {
                        VkFormatProperties properties;
                        c_.vkGetPhysicalDeviceFormatProperties(c_.physical, VK_FORMAT_R8G8B8A8_UNORM, &properties);
                        if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
                            throw std::runtime_error("RGBA8 linear texture filtering is unsupported by this device");
                    }
                    std::unique_ptr<GpuTexture> gpu(new GpuTexture(c_, descriptor_pool_));
                    gpu->source = data;
                    gpu->image.init(data->width(), data->height(), VK_FORMAT_R8G8B8A8_UNORM,
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                    VK_IMAGE_ASPECT_COLOR_BIT);
                    Buffer staging(c_);
                    const auto staging_create_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                         : std::chrono::steady_clock::time_point{};
                    staging.init(data->rgba().size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
                    if (c_.profile_enabled) {
                        ++c_.profile_staging_allocations;
                        c_.profile_staging_bytes += data->rgba().size();
                        c_.profile_staging_create_time += std::chrono::steady_clock::now() - staging_create_start;
                    }
                    const auto staging_write_start = c_.profile_enabled ? std::chrono::steady_clock::now()
                                                                        : std::chrono::steady_clock::time_point{};
                    staging.transfer(const_cast<unsigned char *>(data->rgba().data()), data->rgba().size(), true);
                    if (c_.profile_enabled)
                        c_.profile_staging_write_time += std::chrono::steady_clock::now() - staging_write_start;
                    c_.execute([&](VkCommandBuffer cmd) {
                        image_barrier(c_, cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
                        VkBufferImageCopy copy{};
                        copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                        copy.imageExtent = {data->width(), data->height(), 1};
                        c_.vkCmdCopyBufferToImage(cmd, staging.handle, gpu->image.handle,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
                        image_barrier(c_, cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                    });
                    VkDescriptorSetAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ai.descriptorPool = descriptor_pool_;
                    ai.descriptorSetCount = 1;
                    ai.pSetLayouts = &descriptors_;
                    check(c_.vkAllocateDescriptorSets(c_.device, &ai, &gpu->set),
                          "allocate texture descriptor (maximum 1024 live textures)");
                    const unsigned sampler_index = (data->sampler().wrap == TextureWrap::repeat ? 2 : 0) +
                                                   (data->sampler().filter == TextureFilter::linear ? 1 : 0);
                    VkDescriptorImageInfo image{samplers_[sampler_index], gpu->image.view,
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                    VkDescriptorImageInfo images[2] = {image, image};
                    VkWriteDescriptorSet write[2] = {};
                    for (unsigned i = 0; i < 2; ++i) {
                        write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write[i].dstSet = gpu->set;
                        write[i].dstBinding = i;
                        write[i].descriptorCount = 1;
                        write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write[i].pImageInfo = &images[i];
                    }
                    c_.vkUpdateDescriptorSets(c_.device, 2, write, 0, nullptr);
                    auto *result = gpu.get();
                    textures_.emplace(data.get(), std::move(gpu));
                    return *result;
                }
            };
            // Explicit packing: do not rely on public vec3/mat4 alignment for GPU layouts.
            struct ImpostorUniform {
                float model_view[16], projection[16], inverse_projection[16], dimensions[4], viewport_origin[4],
                    light[4], clip_planes[8], clip_info[4];
                // line_options[0] is the shader's line_screen_space_width flag (0 or 1); a bool
                // in a uniform block is stored as an integer. It follows clip_info so that the
                // shaders reading only the prefix of this buffer see it at the same offset.
                uint32_t line_options[4];
                float model[16], eye[4], material_light[4], ambient[4], specular[4], backside[4], shading[4],
                    shadow[16];
            };
            static_assert(sizeof(ImpostorUniform) == 528, "impostor uniform layout changed");
            struct Push {
                float mvp[16], color[4];
                uint32_t object, lighting;
                float normal[9];
            };
            static_assert(sizeof(Push) == 124, "shader push-constant layout changed");

            // Order-independent transparency targets. Both effects are single sampled and
            // ignore opaque depth, matching the legacy OpenGL transparency classes.
            // Average accumulation uses two additive attachments (premultiplied color and
            // fragment count). Peeling resolves one front layer per pass into its own
            // target, because a depth-tested pass with blending would keep every fragment
            // that passes; a fullscreen pass then blends that layer over the accumulation.
            struct Oit {
                Context &c;
                uint32_t width = 0, height = 0;
                VkFormat color_format = VK_FORMAT_R16G16B16A16_SFLOAT;
                VkFormat count_format = VK_FORMAT_R16_SFLOAT;
                GpuImage color, count, layer, start_depth;
                // Two peel layers alternate between these single-sampled depth targets.
                GpuImage depth_a, depth_b;
                GpuImage &depth(unsigned index) {
                    return index ? depth_b : depth_a;
                }
                VkRenderPass average_pass = VK_NULL_HANDLE, layer_pass = VK_NULL_HANDLE, start_pass = VK_NULL_HANDLE;
                VkRenderPass reset_pass = VK_NULL_HANDLE, accumulate_pass = VK_NULL_HANDLE;
                VkFramebuffer average_framebuffer = VK_NULL_HANDLE, layer_framebuffers[2] = {},
                              start_framebuffer = VK_NULL_HANDLE;
                VkFramebuffer reset_framebuffer = VK_NULL_HANDLE, accumulate_framebuffer = VK_NULL_HANDLE;
                VkPipeline average_pipeline = VK_NULL_HANDLE, layer_pipeline = VK_NULL_HANDLE;
                VkPipeline accumulate_pipeline = VK_NULL_HANDLE;
                VkDescriptorSet composite_set = VK_NULL_HANDLE, layer_set = VK_NULL_HANDLE;
                VkDescriptorPool pool = VK_NULL_HANDLE;
                bool start_ready = false;
                VkImageLayout color_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkImageLayout count_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkImageLayout layer_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkImageLayout depth_layout[2] = {VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED};
                std::map<std::pair<const GpuTexture *, unsigned>, VkDescriptorSet> peel_sets;
                explicit Oit(Context &context)
                    : c(context), color(context), count(context), layer(context), start_depth(context),
                      depth_a(context), depth_b(context) {}
                ~Oit() {
                    if (average_pipeline)
                        c.vkDestroyPipeline(c.device, average_pipeline, nullptr);
                    if (layer_pipeline)
                        c.vkDestroyPipeline(c.device, layer_pipeline, nullptr);
                    if (accumulate_pipeline)
                        c.vkDestroyPipeline(c.device, accumulate_pipeline, nullptr);
                    if (average_framebuffer)
                        c.vkDestroyFramebuffer(c.device, average_framebuffer, nullptr);
                    for (auto framebuffer : layer_framebuffers)
                        if (framebuffer)
                            c.vkDestroyFramebuffer(c.device, framebuffer, nullptr);
                    if (start_framebuffer)
                        c.vkDestroyFramebuffer(c.device, start_framebuffer, nullptr);
                    if (reset_framebuffer)
                        c.vkDestroyFramebuffer(c.device, reset_framebuffer, nullptr);
                    if (accumulate_framebuffer)
                        c.vkDestroyFramebuffer(c.device, accumulate_framebuffer, nullptr);
                    if (average_pass)
                        c.vkDestroyRenderPass(c.device, average_pass, nullptr);
                    if (layer_pass)
                        c.vkDestroyRenderPass(c.device, layer_pass, nullptr);
                    if (start_pass)
                        c.vkDestroyRenderPass(c.device, start_pass, nullptr);
                    if (reset_pass)
                        c.vkDestroyRenderPass(c.device, reset_pass, nullptr);
                    if (accumulate_pass)
                        c.vkDestroyRenderPass(c.device, accumulate_pass, nullptr);
                    if (composite_set)
                        c.vkFreeDescriptorSets(c.device, pool, 1, &composite_set);
                    if (layer_set)
                        c.vkFreeDescriptorSets(c.device, pool, 1, &layer_set);
                    for (auto &entry : peel_sets)
                        c.vkFreeDescriptorSets(c.device, pool, 1, &entry.second);
                }
            };

            // Per-pixel linked lists OIT resources (Yang et al., NVIDIA 2010). Unlike the
            // image-attachment average/peel targets, the build pass appends fragments to
            // storage buffers: a per-pixel head pointer, a node array, and an atomic counter.
            // The resolve pass reads those buffers and writes the premultiplied result into
            // the shared Oit accumulation image, which the scene pass composites like peeling.
            struct LinkedLists {
                Context &c;
                uint32_t width = 0, height = 0, node_count = 0;
                Buffer head, nodes, counter, params, counter_readback;
                GpuImage build_target;
                VkRenderPass build_pass = VK_NULL_HANDLE;
                VkFramebuffer build_framebuffer = VK_NULL_HANDLE;
                VkPipeline build_pipeline = VK_NULL_HANDLE, resolve_pipeline = VK_NULL_HANDLE;
                VkDescriptorSet set = VK_NULL_HANDLE;
                VkDescriptorPool pool = VK_NULL_HANDLE;
                explicit LinkedLists(Context &context)
                    : c(context), head(context), nodes(context), counter(context), params(context),
                      counter_readback(context), build_target(context) {}
                ~LinkedLists() {
                    if (build_pipeline)
                        c.vkDestroyPipeline(c.device, build_pipeline, nullptr);
                    if (resolve_pipeline)
                        c.vkDestroyPipeline(c.device, resolve_pipeline, nullptr);
                    if (build_framebuffer)
                        c.vkDestroyFramebuffer(c.device, build_framebuffer, nullptr);
                    if (build_pass)
                        c.vkDestroyRenderPass(c.device, build_pass, nullptr);
                    if (set)
                        c.vkFreeDescriptorSets(c.device, pool, 1, &set);
                }
            };

            // Presentation semaphores belong to swapchain images, not CPU frame slots. An
            // image is reacquired only after its previous presentation wait has completed.
            struct Swapchain {
                Context &c;
                VkSwapchainKHR handle = VK_NULL_HANDLE;
                VkExtent2D extent{};
                uint32_t requested_width = 0, requested_height = 0;
                std::vector<VkImage> images;
                std::vector<VkSemaphore> ready;
                VkSemaphore acquired = VK_NULL_HANDLE;
                bool dirty = false;
                explicit Swapchain(Context &context) : c(context) {}
                ~Swapchain() {
                    for (auto s : ready)
                        if (s)
                            c.vkDestroySemaphore(c.device, s, nullptr);
                    if (acquired)
                        c.vkDestroySemaphore(c.device, acquired, nullptr);
                    if (handle)
                        c.vkDestroySwapchainKHR(c.device, handle, nullptr);
                }
                void init(uint32_t width, uint32_t height) {
                    requested_width = width;
                    requested_height = height;
                    VkSurfaceCapabilitiesKHR caps;
                    check(c.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(c.physical, c.surface, &caps),
                          "query surface capabilities");
                    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
                        throw std::runtime_error("surface does not support transfer-destination presentation images");
                    if (!(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR))
                        throw std::runtime_error("rotated Vulkan presentation surfaces are not yet supported");
                    extent = caps.currentExtent;
                    if (extent.width == UINT32_MAX) {
                        extent.width = std::max(caps.minImageExtent.width, std::min(width, caps.maxImageExtent.width));
                        extent.height =
                            std::max(caps.minImageExtent.height, std::min(height, caps.maxImageExtent.height));
                    }
                    if (!extent.width || !extent.height)
                        return;
                    uint32_t count = 0;
                    check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, nullptr),
                          "query surface formats");
                    std::vector<VkSurfaceFormatKHR> formats(count);
                    check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, formats.data()),
                          "query surface formats");
                    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
                        formats[0].format = VK_FORMAT_B8G8R8A8_UNORM;
                    VkSurfaceFormatKHR chosen{};
                    bool found = false;
                    const VkFormat preferred[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB};
                    for (auto format : preferred) {
                        for (const auto &candidate : formats) {
                            if (candidate.format != format || candidate.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                                continue;
                            VkFormatProperties properties;
                            c.vkGetPhysicalDeviceFormatProperties(c.physical, format, &properties);
                            if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
                                continue;
                            chosen = candidate;
                            found = true;
                            break;
                        }
                        if (found)
                            break;
                    }
                    if (!found)
                        throw std::runtime_error("no supported RGBA/BGRA presentation format with blit support");
                    uint32_t mode_count = 0;
                    check(c.vkGetPhysicalDeviceSurfacePresentModesKHR(c.physical, c.surface, &mode_count, nullptr),
                          "query presentation modes");
                    std::vector<VkPresentModeKHR> present_modes(mode_count);
                    check(c.vkGetPhysicalDeviceSurfacePresentModesKHR(c.physical, c.surface, &mode_count,
                                                                      present_modes.data()),
                          "query presentation modes");
                    const VkPresentModeKHR present_mode = std::find(present_modes.begin(), present_modes.end(),
                                                                    VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end()
                                                              ? VK_PRESENT_MODE_MAILBOX_KHR
                                                              : VK_PRESENT_MODE_FIFO_KHR;
                    uint32_t image_count = caps.minImageCount + 1;
                    if (caps.maxImageCount && image_count > caps.maxImageCount)
                        image_count = caps.maxImageCount;
                    VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                    const VkCompositeAlphaFlagBitsKHR modes[] = {
                        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};
                    for (auto mode : modes)
                        if (caps.supportedCompositeAlpha & mode) {
                            alpha = mode;
                            break;
                        }
                    VkSwapchainCreateInfoKHR si{};
                    si.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                    si.surface = c.surface;
                    si.minImageCount = image_count;
                    si.imageFormat = chosen.format;
                    si.imageColorSpace = chosen.colorSpace;
                    si.imageExtent = extent;
                    si.imageArrayLayers = 1;
                    si.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    si.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    si.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                    si.compositeAlpha = alpha;
                    si.presentMode = present_mode;
                    si.clipped = VK_TRUE;
                    check(c.vkCreateSwapchainKHR(c.device, &si, nullptr, &handle), "create swapchain");
                    check(c.vkGetSwapchainImagesKHR(c.device, handle, &count, nullptr), "get presentation images");
                    images.resize(count);
                    check(c.vkGetSwapchainImagesKHR(c.device, handle, &count, images.data()),
                          "get presentation images");
                    VkSemaphoreCreateInfo sem{};
                    sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    check(c.vkCreateSemaphore(c.device, &sem, nullptr, &acquired), "create acquire semaphore");
                    ready.resize(count, VK_NULL_HANDLE);
                    for (auto &s : ready)
                        check(c.vkCreateSemaphore(c.device, &sem, nullptr, &s), "create image presentation semaphore");
                }
            };

            class OitResourcesBuilder {
                Context &c_;
                std::unique_ptr<Oit> &oit_;
                VkDescriptorPool descriptor_pool_;
                VkPipelineLayout layout_;
                VkPipelineLayout oit_peel_layout_;
                VkPipelineLayout oit_composite_layout_;
                VkDescriptorSetLayout oit_composite_descriptors_;
                VkSampler sampler_;

            public:
                OitResourcesBuilder(Context &context,
                                    std::unique_ptr<Oit> &oit,
                                    VkDescriptorPool descriptor_pool,
                                    VkPipelineLayout layout,
                                    VkPipelineLayout peel_layout,
                                    VkPipelineLayout composite_layout,
                                    VkDescriptorSetLayout composite_descriptors,
                                    VkSampler sampler)
                    : c_(context), oit_(oit), descriptor_pool_(descriptor_pool), layout_(layout),
                      oit_peel_layout_(peel_layout), oit_composite_layout_(composite_layout),
                      oit_composite_descriptors_(composite_descriptors), sampler_(sampler) {}

                void ensure(uint32_t width, uint32_t height) {
                    if (oit_ && oit_->width == width && oit_->height == height)
                        return;
                    const auto supports = [&](VkFormat format, VkFormatFeatureFlags flags) {
                        VkFormatProperties properties;
                        c_.vkGetPhysicalDeviceFormatProperties(c_.physical, format, &properties);
                        return (properties.optimalTilingFeatures & flags) == flags;
                    };
                    const VkFormatFeatureFlags color_features = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                                                VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
                                                                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
                    VkFormat color_format = VK_FORMAT_UNDEFINED, count_format = VK_FORMAT_UNDEFINED;
                    for (auto format : {VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT})
                        if (supports(format, color_features)) {
                            color_format = format;
                            break;
                        }
                    for (auto format : {VK_FORMAT_R16_SFLOAT, VK_FORMAT_R32_SFLOAT})
                        if (supports(format, color_features)) {
                            count_format = format;
                            break;
                        }
                    if (color_format == VK_FORMAT_UNDEFINED || count_format == VK_FORMAT_UNDEFINED)
                        throw std::runtime_error(
                            "order-independent transparency requires blended floating-point color attachments");
                    if (!supports(VK_FORMAT_D32_SFLOAT,
                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
                        throw std::runtime_error("depth peeling requires sampleable D32 depth");

                    oit_.reset(new Oit(c_));
                    oit_->width = width;
                    oit_->height = height;
                    oit_->color_format = color_format;
                    oit_->count_format = count_format;
                    oit_->pool = descriptor_pool_;
                    oit_->color.init(width, height, color_format,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
                    oit_->count.init(width, height, count_format,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
                    oit_->layer.init(width, height, color_format,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
                    for (unsigned i = 0; i < 2; ++i)
                        oit_->depth(i).init(width, height, VK_FORMAT_D32_SFLOAT,
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                            VK_IMAGE_ASPECT_DEPTH_BIT);
                    oit_->start_depth.init(1, 1, VK_FORMAT_D32_SFLOAT,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                           VK_IMAGE_ASPECT_DEPTH_BIT);

                    const auto make_pass = [&](VkRenderPass &result, const VkAttachmentDescription *attachments,
                                               unsigned count, const VkSubpassDescription &subpass,
                                               const VkSubpassDependency *dependencies, unsigned dependency_count) {
                        VkRenderPassCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                        info.attachmentCount = count;
                        info.pAttachments = attachments;
                        info.subpassCount = 1;
                        info.pSubpasses = &subpass;
                        info.dependencyCount = dependency_count;
                        info.pDependencies = dependencies;
                        check(c_.vkCreateRenderPass(c_.device, &info, nullptr, &result),
                              "create transparency render pass");
                    };
                    const auto make_framebuffer = [&](VkRenderPass pass, const VkImageView *views, unsigned count,
                                                      uint32_t w, uint32_t h, VkFramebuffer &framebuffer) {
                        VkFramebufferCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                        info.renderPass = pass;
                        info.attachmentCount = count;
                        info.pAttachments = views;
                        info.width = w;
                        info.height = h;
                        info.layers = 1;
                        check(c_.vkCreateFramebuffer(c_.device, &info, nullptr, &framebuffer),
                              "create transparency framebuffer");
                    };
                    {
                        // Additive premultiplied color plus fragment count.
                        VkAttachmentDescription attachments[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            auto &a = attachments[i];
                            a.format = i == 0 ? color_format : count_format;
                            a.samples = VK_SAMPLE_COUNT_1_BIT;
                            a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                            a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                            a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                            a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                            a.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                            a.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        }
                        VkAttachmentReference colors[2] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                           {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
                        VkSubpassDescription subpass{};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.colorAttachmentCount = 2;
                        subpass.pColorAttachments = colors;
                        VkSubpassDependency dependencies[2] = {};
                        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[0].dstSubpass = 0;
                        dependencies[0].srcStageMask = dependencies[0].dstStageMask =
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].srcSubpass = 0;
                        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        make_pass(oit_->average_pass, attachments, 2, subpass, dependencies, 2);
                        VkImageView views[] = {oit_->color.view, oit_->count.view};
                        make_framebuffer(oit_->average_pass, views, 2, width, height, oit_->average_framebuffer);
                    }
                    {
                        // A resolved front layer: premultiplied color plus the layer depth.
                        VkAttachmentDescription layer_attachments[2] = {};
                        auto &layer_color = layer_attachments[0];
                        layer_color.format = color_format;
                        layer_color.samples = VK_SAMPLE_COUNT_1_BIT;
                        layer_color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        layer_color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        layer_color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        layer_color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        layer_color.initialLayout = layer_color.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        auto &layer_depth = layer_attachments[1];
                        layer_depth.format = VK_FORMAT_D32_SFLOAT;
                        layer_depth.samples = VK_SAMPLE_COUNT_1_BIT;
                        layer_depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        layer_depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        layer_depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        layer_depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        layer_depth.initialLayout = layer_depth.finalLayout =
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        VkAttachmentReference color_reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
                        VkAttachmentReference depth_reference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                        VkSubpassDescription subpass{};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.colorAttachmentCount = 1;
                        subpass.pColorAttachments = &color_reference;
                        subpass.pDepthStencilAttachment = &depth_reference;
                        VkSubpassDependency layer_dependencies[2] = {};
                        layer_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                        layer_dependencies[0].dstSubpass = 0;
                        layer_dependencies[0].srcStageMask = layer_dependencies[0].dstStageMask =
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        layer_dependencies[0].dstAccessMask =
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        layer_dependencies[1].srcSubpass = 0;
                        layer_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                        layer_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        layer_dependencies[1].dstStageMask =
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        layer_dependencies[1].srcAccessMask =
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        layer_dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        make_pass(oit_->layer_pass, layer_attachments, 2, subpass, layer_dependencies, 2);
                        for (unsigned i = 0; i < 2; ++i) {
                            VkImageView views[] = {oit_->layer.view, oit_->depth(i).view};
                            make_framebuffer(oit_->layer_pass, views, 2, width, height, oit_->layer_framebuffers[i]);
                        }
                    }
                    {
                        // The premultiplied accumulation: cleared once per frame, then loaded
                        // by every layer blend and finally sampled by the scene composite.
                        VkAttachmentDescription attachment{};
                        attachment.format = color_format;
                        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        attachment.initialLayout = attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        VkAttachmentReference color_reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
                        VkSubpassDescription subpass{};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.colorAttachmentCount = 1;
                        subpass.pColorAttachments = &color_reference;
                        VkSubpassDependency dependencies[2] = {};
                        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[0].dstSubpass = 0;
                        dependencies[0].srcStageMask =
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[0].srcAccessMask =
                            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[0].dstAccessMask =
                            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].srcSubpass = 0;
                        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[1].dstStageMask =
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].dstAccessMask =
                            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        make_pass(oit_->reset_pass, &attachment, 1, subpass, dependencies, 2);
                        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                        make_pass(oit_->accumulate_pass, &attachment, 1, subpass, dependencies, 2);
                        VkImageView views[] = {oit_->color.view};
                        make_framebuffer(oit_->reset_pass, views, 1, width, height, oit_->reset_framebuffer);
                        make_framebuffer(oit_->accumulate_pass, views, 1, width, height, oit_->accumulate_framebuffer);
                    }
                    {
                        // One-shot 1x1 image holding the peeled depth before the first layer. It is
                        // cleared to zero, so the first layer keeps every fragment.
                        VkAttachmentDescription attachment{};
                        attachment.format = VK_FORMAT_D32_SFLOAT;
                        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        // The image starts uninitialized; the clear defines its contents.
                        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        VkAttachmentReference depth_reference = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
                        VkSubpassDescription subpass{};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.pDepthStencilAttachment = &depth_reference;
                        VkSubpassDependency dependencies[2] = {};
                        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[0].dstSubpass = 0;
                        dependencies[0].srcStageMask = dependencies[0].dstStageMask =
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        dependencies[1].srcSubpass = 0;
                        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[1].srcStageMask =
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        make_pass(oit_->start_pass, &attachment, 1, subpass, dependencies, 2);
                        VkImageView views[] = {oit_->start_depth.view};
                        make_framebuffer(oit_->start_pass, views, 1, 1, 1, oit_->start_framebuffer);
                    }

                    VkShaderModule modules[5] = {};
                    try {
                        VkShaderModuleCreateInfo mi{};
                        mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        mi.codeSize = sizeof(scene_vert);
                        mi.pCode = scene_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]),
                              "create transparency vertex shader");
                        mi.codeSize = sizeof(oit_average_frag);
                        mi.pCode = oit_average_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]),
                              "create average transparency shader");
                        mi.codeSize = sizeof(oit_peel_frag);
                        mi.pCode = oit_peel_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[2]),
                              "create peeling transparency shader");
                        mi.codeSize = sizeof(oit_quad_vert);
                        mi.pCode = oit_quad_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[3]),
                              "create transparency quad shader");
                        mi.codeSize = sizeof(oit_peel_composite_frag);
                        mi.pCode = oit_peel_composite_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[4]),
                              "create transparency layer blend shader");
                        VkPipelineShaderStageCreateInfo stages[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                            stages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
                            stages[i].pName = "main";
                        }
                        VkVertexInputBindingDescription binding = {0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
                        VkVertexInputAttributeDescription attributes[] = {
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, position)},
                            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, normal)},
                            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, color)},
                            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuVertex, texcoord)},
                            {4, 0, VK_FORMAT_R32_UINT, offsetof(GpuVertex, primitive)},
                            {9, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, highlight)}};
                        VkPipelineVertexInputStateCreateInfo vertex{};
                        vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        vertex.vertexBindingDescriptionCount = 1;
                        vertex.pVertexBindingDescriptions = &binding;
                        vertex.vertexAttributeDescriptionCount = 6;
                        vertex.pVertexAttributeDescriptions = attributes;
                        VkPipelineInputAssemblyStateCreateInfo assembly{};
                        assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                        VkPipelineViewportStateCreateInfo viewport{};
                        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                        viewport.viewportCount = 1;
                        viewport.scissorCount = 1;
                        VkPipelineRasterizationStateCreateInfo raster{};
                        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                        raster.polygonMode = VK_POLYGON_MODE_FILL;
                        raster.cullMode = VK_CULL_MODE_NONE;
                        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        raster.lineWidth = 1;
                        raster.depthBiasEnable = VK_TRUE;
                        VkPipelineMultisampleStateCreateInfo multisample{};
                        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                           VK_DYNAMIC_STATE_DEPTH_BIAS};
                        VkPipelineDynamicStateCreateInfo dynamic{};
                        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                        dynamic.dynamicStateCount = 3;
                        dynamic.pDynamicStates = dynamic_states;
                        VkPipelineDepthStencilStateCreateInfo depth_state{};
                        depth_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                        VkPipelineColorBlendStateCreateInfo blending{};
                        blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                        VkGraphicsPipelineCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                        info.stageCount = 2;
                        info.pStages = stages;
                        info.pVertexInputState = &vertex;
                        info.pInputAssemblyState = &assembly;
                        info.pViewportState = &viewport;
                        info.pRasterizationState = &raster;
                        info.pMultisampleState = &multisample;
                        info.pDepthStencilState = &depth_state;
                        info.pColorBlendState = &blending;
                        info.pDynamicState = &dynamic;

                        stages[0].module = modules[0];
                        stages[1].module = modules[1];
                        VkPipelineColorBlendAttachmentState additive[2] = {};
                        for (auto &blend : additive) {
                            blend.blendEnable = VK_TRUE;
                            blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                            blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                            blend.colorBlendOp = VK_BLEND_OP_ADD;
                            blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                            blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                            blend.alphaBlendOp = VK_BLEND_OP_ADD;
                            blend.colorWriteMask = 0xf;
                        }
                        blending.attachmentCount = 2;
                        blending.pAttachments = additive;
                        info.renderPass = oit_->average_pass;
                        info.layout = layout_;
                        check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &info, nullptr,
                                                           &oit_->average_pipeline),
                              "create average transparency pipeline");

                        // One front layer: depth tested, replacing the layer target so only the
                        // nearest surviving fragment of each pixel is kept.
                        stages[1].module = modules[2];
                        depth_state.depthTestEnable = depth_state.depthWriteEnable = VK_TRUE;
                        depth_state.depthCompareOp = VK_COMPARE_OP_LESS;
                        VkPipelineColorBlendAttachmentState replace{};
                        replace.colorWriteMask = 0xf;
                        blending.attachmentCount = 1;
                        blending.pAttachments = &replace;
                        info.renderPass = oit_->layer_pass;
                        info.layout = oit_peel_layout_;
                        check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &info, nullptr,
                                                           &oit_->layer_pipeline),
                              "create depth-peel pipeline");

                        // Blend the resolved layer into the premultiplied accumulation. Layers
                        // are peeled front to back, so each new one lies *behind* everything
                        // already accumulated and must go under it, exactly like the
                        // under-blending of the OpenGL dual_depth_peeling_peel_rendering
                        // shader: dst = src * (1 - dst.a) + dst. Over-blending here would
                        // invert the depth order and let the farthest peeled layer win.
                        stages[0].module = modules[3];
                        stages[1].module = modules[4];
                        vertex.vertexBindingDescriptionCount = 0;
                        vertex.pVertexBindingDescriptions = nullptr;
                        vertex.vertexAttributeDescriptionCount = 0;
                        vertex.pVertexAttributeDescriptions = nullptr;
                        depth_state.depthTestEnable = depth_state.depthWriteEnable = VK_FALSE;
                        VkPipelineColorBlendAttachmentState under{};
                        under.blendEnable = VK_TRUE;
                        under.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                        under.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                        under.colorBlendOp = VK_BLEND_OP_ADD;
                        under.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                        under.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                        under.alphaBlendOp = VK_BLEND_OP_ADD;
                        under.colorWriteMask = 0xf;
                        blending.attachmentCount = 1;
                        blending.pAttachments = &under;
                        info.renderPass = oit_->accumulate_pass;
                        info.layout = oit_composite_layout_;
                        check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &info, nullptr,
                                                           &oit_->accumulate_pipeline),
                              "create transparency layer blend pipeline");
                    } catch (...) {
                        for (auto module : modules)
                            if (module)
                                c_.vkDestroyShaderModule(c_.device, module, nullptr);
                        throw;
                    }
                    for (auto module : modules)
                        if (module)
                            c_.vkDestroyShaderModule(c_.device, module, nullptr);

                    VkDescriptorSetAllocateInfo allocation{};
                    allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocation.descriptorPool = descriptor_pool_;
                    allocation.descriptorSetCount = 2;
                    const VkDescriptorSetLayout composite_layouts[2] = {oit_composite_descriptors_,
                                                                        oit_composite_descriptors_};
                    allocation.pSetLayouts = composite_layouts;
                    VkDescriptorSet composite_sets[2] = {};
                    check(c_.vkAllocateDescriptorSets(c_.device, &allocation, composite_sets),
                          "allocate transparency composite descriptors");
                    oit_->composite_set = composite_sets[0];
                    oit_->layer_set = composite_sets[1];
                    // The layer blend only reads the resolved front layer.
                    const VkDescriptorImageInfo layer_image{sampler_, oit_->layer.view,
                                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                    VkWriteDescriptorSet layer_writes[2] = {};
                    for (unsigned i = 0; i < 2; ++i) {
                        layer_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        layer_writes[i].dstSet = oit_->layer_set;
                        layer_writes[i].dstBinding = i;
                        layer_writes[i].descriptorCount = 1;
                        layer_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        layer_writes[i].pImageInfo = &layer_image;
                    }
                    c_.vkUpdateDescriptorSets(c_.device, 2, layer_writes, 0, nullptr);
                }
            };

            class LinkedListsBuilder {
                Context &c_;
                std::unique_ptr<LinkedLists> &lists_;
                std::unique_ptr<Oit> &oit_;
                VkDescriptorPool descriptor_pool_;
                VkDescriptorSetLayout oit_lists_descriptors_;
                VkPipelineLayout oit_lists_layout_;
                VkPipelineLayout oit_resolve_layout_;

            public:
                LinkedListsBuilder(Context &context,
                                   std::unique_ptr<LinkedLists> &lists,
                                   std::unique_ptr<Oit> &oit,
                                   VkDescriptorPool descriptor_pool,
                                   VkDescriptorSetLayout lists_descriptors,
                                   VkPipelineLayout lists_layout,
                                   VkPipelineLayout resolve_layout)
                    : c_(context), lists_(lists), oit_(oit), descriptor_pool_(descriptor_pool),
                      oit_lists_descriptors_(lists_descriptors), oit_lists_layout_(lists_layout),
                      oit_resolve_layout_(resolve_layout) {}

                void ensure(uint32_t width, uint32_t height, unsigned max_fragments) {
                    const uint32_t node_count = width * height * std::max(1u, max_fragments);
                    if (lists_ && lists_->width == width && lists_->height == height &&
                        lists_->node_count == node_count)
                        return;
                    lists_.reset(new LinkedLists(c_));
                    lists_->width = width;
                    lists_->height = height;
                    lists_->node_count = node_count;
                    lists_->pool = descriptor_pool_;
                    const VkDeviceSize head_bytes = VkDeviceSize(width) * height * 4;
                    const VkDeviceSize node_bytes = VkDeviceSize(node_count) * 32;
                    lists_->head.init(head_bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      false);
                    lists_->nodes.init(node_bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false);
                    lists_->counter.init(4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         false);
                    lists_->counter_readback.init(4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
                    lists_->params.init(16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
                    uint32_t parameters[4] = {node_count, width, height, 0};
                    lists_->params.transfer(parameters, sizeof(parameters), true);

                    {
                        // The build fragment shader appends its fragments to storage buffers and
                        // discards its color output. The dummy attachment keeps the pass equal to
                        // the OpenGL build FBO (which also allocates a color buffer): a pass with
                        // no attachments at all makes lavapipe drop the masked color-attachment
                        // writes of the picking draws that follow it.
                        lists_->build_target.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
                        VkAttachmentDescription attachment{};
                        attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
                        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        VkAttachmentReference build_color = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
                        VkSubpassDescription subpass{};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.colorAttachmentCount = 1;
                        subpass.pColorAttachments = &build_color;
                        VkSubpassDependency dependencies[2] = {};
                        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[0].dstSubpass = 0;
                        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT |
                                                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
                                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].srcSubpass = 0;
                        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dependencies[1].srcAccessMask =
                            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        VkRenderPassCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                        info.attachmentCount = 1;
                        info.pAttachments = &attachment;
                        info.subpassCount = 1;
                        info.pSubpasses = &subpass;
                        info.dependencyCount = 2;
                        info.pDependencies = dependencies;
                        check(c_.vkCreateRenderPass(c_.device, &info, nullptr, &lists_->build_pass),
                              "create linked lists build pass");
                        VkFramebufferCreateInfo fi{};
                        fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                        fi.renderPass = lists_->build_pass;
                        fi.attachmentCount = 1;
                        fi.pAttachments = &lists_->build_target.view;
                        fi.width = width;
                        fi.height = height;
                        fi.layers = 1;
                        check(c_.vkCreateFramebuffer(c_.device, &fi, nullptr, &lists_->build_framebuffer),
                              "create linked lists framebuffer");
                    }

                    VkDescriptorSetAllocateInfo allocation{};
                    allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    allocation.descriptorPool = descriptor_pool_;
                    allocation.descriptorSetCount = 1;
                    allocation.pSetLayouts = &oit_lists_descriptors_;
                    check(c_.vkAllocateDescriptorSets(c_.device, &allocation, &lists_->set),
                          "allocate linked lists descriptors");
                    const VkDescriptorBufferInfo buffers[4] = {{lists_->head.handle, 0, head_bytes},
                                                               {lists_->nodes.handle, 0, node_bytes},
                                                               {lists_->counter.handle, 0, 4},
                                                               {lists_->params.handle, 0, 16}};
                    VkWriteDescriptorSet writes[4] = {};
                    for (unsigned i = 0; i < 4; ++i) {
                        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writes[i].dstSet = lists_->set;
                        writes[i].dstBinding = i;
                        writes[i].descriptorCount = 1;
                        writes[i].descriptorType =
                            i == 3 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        writes[i].pBufferInfo = &buffers[i];
                    }
                    c_.vkUpdateDescriptorSets(c_.device, 4, writes, 0, nullptr);

                    VkShaderModule modules[4] = {};
                    try {
                        VkShaderModuleCreateInfo mi{};
                        mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                        mi.codeSize = sizeof(scene_vert);
                        mi.pCode = scene_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]),
                              "create linked lists vertex shader");
                        mi.codeSize = sizeof(oit_linked_lists_build_frag);
                        mi.pCode = oit_linked_lists_build_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]),
                              "create linked lists build shader");
                        mi.codeSize = sizeof(oit_quad_vert);
                        mi.pCode = oit_quad_vert;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[2]),
                              "create linked lists quad shader");
                        mi.codeSize = sizeof(oit_linked_lists_resolve_frag);
                        mi.pCode = oit_linked_lists_resolve_frag;
                        check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[3]),
                              "create linked lists resolve shader");
                        VkPipelineShaderStageCreateInfo stages[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                            stages[i].stage = i == 0 ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
                            stages[i].pName = "main";
                        }
                        VkVertexInputBindingDescription binding = {0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
                        VkVertexInputAttributeDescription attributes[] = {
                            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, position)},
                            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, normal)},
                            {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, color)},
                            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuVertex, texcoord)},
                            {4, 0, VK_FORMAT_R32_UINT, offsetof(GpuVertex, primitive)},
                            {9, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, highlight)}};
                        VkPipelineVertexInputStateCreateInfo vertex{};
                        vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        vertex.vertexBindingDescriptionCount = 1;
                        vertex.pVertexBindingDescriptions = &binding;
                        vertex.vertexAttributeDescriptionCount = 6;
                        vertex.pVertexAttributeDescriptions = attributes;
                        VkPipelineInputAssemblyStateCreateInfo assembly{};
                        assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                        VkPipelineViewportStateCreateInfo viewport{};
                        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                        viewport.viewportCount = 1;
                        viewport.scissorCount = 1;
                        VkPipelineRasterizationStateCreateInfo raster{};
                        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                        raster.polygonMode = VK_POLYGON_MODE_FILL;
                        raster.cullMode = VK_CULL_MODE_NONE;
                        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                        raster.lineWidth = 1;
                        raster.depthBiasEnable = VK_TRUE;
                        VkPipelineMultisampleStateCreateInfo multisample{};
                        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                           VK_DYNAMIC_STATE_DEPTH_BIAS};
                        VkPipelineDynamicStateCreateInfo dynamic{};
                        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                        dynamic.dynamicStateCount = 3;
                        dynamic.pDynamicStates = dynamic_states;
                        VkPipelineDepthStencilStateCreateInfo depth_state{};
                        depth_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                        VkPipelineColorBlendStateCreateInfo blending{};
                        blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                        VkGraphicsPipelineCreateInfo info{};
                        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                        info.stageCount = 2;
                        info.pStages = stages;
                        info.pVertexInputState = &vertex;
                        info.pInputAssemblyState = &assembly;
                        info.pViewportState = &viewport;
                        info.pRasterizationState = &raster;
                        info.pMultisampleState = &multisample;
                        info.pDepthStencilState = &depth_state;
                        info.pColorBlendState = &blending;
                        info.pDynamicState = &dynamic;

                        // Build: scene geometry, every fragment appends a node. The color
                        // attachment only receives the shader's discarded dummy output.
                        stages[0].module = modules[0];
                        stages[1].module = modules[1];
                        VkPipelineColorBlendAttachmentState build_attachment{};
                        build_attachment.colorWriteMask = 0xf;
                        blending.attachmentCount = 1;
                        blending.pAttachments = &build_attachment;
                        info.renderPass = lists_->build_pass;
                        info.layout = oit_lists_layout_;
                        check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &info, nullptr,
                                                           &lists_->build_pipeline),
                              "create linked lists build pipeline");

                        // Resolve: full-screen triangle replacing the accumulation image.
                        stages[0].module = modules[2];
                        stages[1].module = modules[3];
                        vertex.vertexBindingDescriptionCount = 0;
                        vertex.pVertexBindingDescriptions = nullptr;
                        vertex.vertexAttributeDescriptionCount = 0;
                        vertex.pVertexAttributeDescriptions = nullptr;
                        VkPipelineColorBlendAttachmentState replace{};
                        replace.colorWriteMask = 0xf;
                        blending.attachmentCount = 1;
                        blending.pAttachments = &replace;
                        info.renderPass = oit_->reset_pass;
                        info.layout = oit_resolve_layout_;
                        check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &info, nullptr,
                                                           &lists_->resolve_pipeline),
                              "create linked lists resolve pipeline");
                    } catch (...) {
                        for (auto module : modules)
                            if (module)
                                c_.vkDestroyShaderModule(c_.device, module, nullptr);
                        throw;
                    }
                    for (auto module : modules)
                        if (module)
                            c_.vkDestroyShaderModule(c_.device, module, nullptr);
                }
            };

            class Presentation {
                Context &c_;
                std::unique_ptr<Swapchain> swapchain_;

            public:
                explicit Presentation(Context &context) : c_(context) {}

                bool prepare(uint32_t width, uint32_t height) {
                    if (!swapchain_ || swapchain_->dirty || swapchain_->requested_width != width ||
                        swapchain_->requested_height != height) {
                        // Swapchain recreation is a lifecycle boundary; normal frames wait only their draw fence.
                        check(c_.vkDeviceWaitIdle(c_.device), "drain presentation before swapchain recreation");
                        swapchain_.reset();
                        std::unique_ptr<Swapchain> next(new Swapchain(c_));
                        next->init(width, height);
                        if (!next->handle)
                            return false;
                        swapchain_ = std::move(next);
                    }
                    return true;
                }

                template <typename Record, typename Submitted>
                bool present(uint32_t width,
                             uint32_t height,
                             VkImage source,
                             const Record &record,
                             const Submitted &submitted) {
                    uint32_t image = 0;
                    const auto acquired = c_.vkAcquireNextImageKHR(c_.device, swapchain_->handle, UINT64_MAX,
                                                                   swapchain_->acquired, VK_NULL_HANDLE, &image);
                    if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
                        swapchain_->dirty = true;
                        return false;
                    }
                    if (acquired != VK_SUBOPTIMAL_KHR)
                        check(acquired, "acquire presentation image");
                    c_.execute(
                        [&](VkCommandBuffer cmd) {
                            record(cmd);
                            image_barrier(c_, cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                          VK_ACCESS_TRANSFER_WRITE_BIT);
                            VkImageBlit copy{};
                            copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                            copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                            copy.srcOffsets[1] = {int32_t(width), int32_t(height), 1};
                            copy.dstOffsets[1] = {int32_t(swapchain_->extent.width),
                                                  int32_t(swapchain_->extent.height), 1};
                            c_.vkCmdBlitImage(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                              swapchain_->images[image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy,
                                              VK_FILTER_NEAREST);
                            image_barrier(c_, cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                          VK_ACCESS_TRANSFER_WRITE_BIT, 0);
                        },
                        swapchain_->acquired, swapchain_->ready[image]);
                    submitted();
                    VkPresentInfoKHR info{};
                    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                    info.waitSemaphoreCount = 1;
                    info.pWaitSemaphores = &swapchain_->ready[image];
                    info.swapchainCount = 1;
                    info.pSwapchains = &swapchain_->handle;
                    info.pImageIndices = &image;
                    const auto presented = c_.vkQueuePresentKHR(c_.queue, &info);
                    if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR ||
                        acquired == VK_SUBOPTIMAL_KHR)
                        swapchain_->dirty = true;
                    if (presented != VK_ERROR_OUT_OF_DATE_KHR && presented != VK_SUBOPTIMAL_KHR)
                        check(presented, "present frame");
                    return presented != VK_ERROR_OUT_OF_DATE_KHR;
                }
            };

            bool expanded_lines(const DrawCommand &d) {
                return d.geometry->topology() == Topology::lines &&
                       (d.line_style != LineStyle::plain || d.line_width != 1);
            }

            bool native_point(const Context &c, const DrawCommand &d) {
                return d.geometry->topology() == Topology::points && d.point_style == PointStyle::plain &&
                       c.large_points && d.point_size >= c.min_point_size && d.point_size <= c.max_point_size;
            }

            bool expanded_impostor(const Context &c, const DrawCommand &d) {
                return expanded_lines(d) || (d.geometry->topology() == Topology::points &&
                                             (d.point_style != PointStyle::plain || !native_point(c, d)));
            }

            class ScenePassEncoder {
                Context &c_;
                VkPipelineLayout layout_;
                VkDescriptorSet impostor_set_;
                VkPipelineLayout oit_composite_layout_;
                Oit *oit_;
                bool oit_active_;
                TransparencyMethod oit_method_;
                VkBuffer transparent_indices_;

            public:
                ScenePassEncoder(Context &context,
                                 VkPipelineLayout layout,
                                 VkDescriptorSet impostor_set,
                                 VkPipelineLayout oit_composite_layout,
                                 Oit *oit,
                                 bool oit_active,
                                 TransparencyMethod oit_method,
                                 VkBuffer transparent_indices)
                    : c_(context), layout_(layout), impostor_set_(impostor_set),
                      oit_composite_layout_(oit_composite_layout), oit_(oit), oit_active_(oit_active),
                      oit_method_(oit_method), transparent_indices_(transparent_indices) {}

                template <typename Prepared>
                void encode(VkCommandBuffer cmd,
                            const std::vector<Prepared> &draws,
                            const vec4 &background,
                            const Target &target,
                            const RenderPass &pass,
                            VkFramebuffer framebuffer,
                            const VkOffset2D *pixel = nullptr) {
                    const auto width = target.width, height = target.height;
                    VkClearValue clear[4] = {};
                    if (pass.kind == PassKind::ids) {
                        clear[0].color.uint32[0] = clear[0].color.uint32[1] = UINT32_MAX;
                        clear[1].depthStencil.depth = 1;
                    } else {
                        for (unsigned i = 0; i < 4; ++i)
                            clear[0].color.float32[i] = background[i];
                        if (pass.kind == PassKind::multisample_color)
                            clear[1].depthStencil.depth = 1;
                        else {
                            clear[1].color.uint32[0] = clear[1].color.uint32[1] = UINT32_MAX;
                            clear[2].depthStencil.depth = 1;
                        }
                        clear[3].depthStencil.depth = 1;
                    }
                    VkRenderPassBeginInfo begin{};
                    begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    begin.renderPass = pass.handle;
                    begin.framebuffer = framebuffer;
                    begin.renderArea = pixel ? VkRect2D{*pixel, {1, 1}} : VkRect2D{{0, 0}, {width, height}};
                    begin.clearValueCount = pass.kind == PassKind::ids ? 2 : pass.independent_depth ? 4 : 3;
                    begin.pClearValues = clear;
                    c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
                    VkViewport viewport = {0, 0, float(width), float(height), 0, 1};
                    const VkRect2D scissor = begin.renderArea;
                    c_.vkCmdSetViewport(cmd, 0, 1, &viewport);
                    c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
                    const auto set_view = [&](const DrawCommand &d) {
                        const auto &r = d.viewport;
                        VkViewport v{float(r.x), float(r.y), float(r.z ? r.z : width), float(r.w ? r.w : height), 0, 1};
                        VkRect2D s{{r.x, r.y}, {uint32_t(r.z ? r.z : width), uint32_t(r.w ? r.w : height)}};
                        if (pixel) {
                            if (pixel->x < s.offset.x || pixel->y < s.offset.y ||
                                uint32_t(pixel->x - s.offset.x) >= s.extent.width ||
                                uint32_t(pixel->y - s.offset.y) >= s.extent.height)
                                return false;
                            s = scissor;
                        }
                        c_.vkCmdSetViewport(cmd, 0, 1, &v);
                        c_.vkCmdSetScissor(cmd, 0, 1, &s);
                        return true;
                    };
                    const auto bind_scene = [&](const Prepared &draw, VkPipeline pipeline) {
                        const auto &d = draw.command;
                        if (!set_view(d))
                            return false;
                        c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                        // Vulkan depth bias mirrors glPolygonOffset(0.5, -0.0001): the slope
                        // factor matches and the constant factor takes the OpenGL unit value.
                        // Picking keeps the unbiased surfaces, as the OpenGL picker does.
                        if (d.depth_bias && !d.transparent && pass.kind != PassKind::ids)
                            c_.vkCmdSetDepthBias(cmd, -0.0001f, 0.0f, 0.5f);
                        else
                            c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                        c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 1, 1, &impostor_set_,
                                                   1, &draw.uniform_offset);
                        VkDeviceSize offset = 0;
                        if (draw.geometry->points) {
                            VkBuffer buffers[] = {draw.geometry->points->buffer.handle, draw.geometry->buffer.handle};
                            VkDeviceSize offsets[] = {0, 0};
                            c_.vkCmdBindVertexBuffers(cmd, 0, 2, buffers, offsets);
                        } else
                            c_.vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
                        c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 0, 1,
                                                   &draw.texture->set, 0, nullptr);
                        Push push{};
                        for (unsigned col = 0; col < 4; ++col)
                            for (unsigned row = 0; row < 4; ++row)
                                push.mvp[col * 4 + row] = d.model_view_projection(row, col);
                        for (unsigned i = 0; i < 4; ++i)
                            push.color[i] = d.color[i];
                        push.object = d.object_id;
                        push.lighting = d.lighting ? 1 : 0;
                        for (unsigned col = 0; col < 3; ++col)
                            for (unsigned row = 0; row < 3; ++row)
                                push.normal[col * 3 + row] = d.normal_matrix(row, col);
                        c_.vkCmdPushConstants(cmd, layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
                        return true;
                    };
                    const auto draw_scene = [&](const Prepared &draw, VkPipeline pipeline, bool expanded) {
                        if (!bind_scene(draw, pipeline))
                            return;
                        if (expanded)
                            c_.vkCmdDraw(cmd, 6,
                                         expanded_lines(draw.command) ? draw.geometry->count / 2 : draw.geometry->count,
                                         0, 0);
                        else
                            c_.vkCmdDraw(cmd, draw.geometry->count, 1, 0, 0);
                    };
                    for (const auto &draw : draws) {
                        const auto &d = draw.command;
                        if (d.overlay)
                            continue;
                        // Transparent surfaces use the blended pipeline after all opaque geometry.
                        if (d.transparent && pass.kind != PassKind::ids)
                            continue;
                        unsigned pipeline = d.geometry->topology() == Topology::triangles ? 0
                                            : d.geometry->topology() == Topology::lines   ? 1
                                                                                          : 2;
                        const bool expanded = expanded_impostor(c_, d) && !d.transparent;
                        if (expanded)
                            pipeline = expanded_lines(d) ? (d.line_style == LineStyle::plain ? 5 : 4)
                                          : (d.point_style == PointStyle::plain ? 3 : 6);
                        draw_scene(draw, pass.pipelines[pipeline], expanded);
                    }
                    // Order-independent methods already resolved the transparent surfaces into
                    // their own targets; the scene pass then only records their IDs and depth.
                    const VkPipeline transparent_pass = oit_active_ ? pass.oit_id_pipeline : pass.transparent_pipeline;
                    if (transparent_pass) {
                        std::vector<const Prepared *> transparent;
                        for (const auto &draw : draws)
                            if (draw.command.transparent)
                                transparent.push_back(&draw);
                        // Far to near, so straight-alpha compositing matches the visible surface order.
                        std::stable_sort(
                            transparent.begin(), transparent.end(),
                            [](const Prepared *a, const Prepared *b) { return a->sort_depth > b->sort_depth; });
                        for (const Prepared *draw : transparent) {
                            if (!bind_scene(*draw, transparent_pass))
                                continue;
                            if (draw->index_count) {
                                c_.vkCmdBindIndexBuffer(cmd, transparent_indices_,
                                                        VkDeviceSize(draw->index_offset) * sizeof(uint32_t),
                                                        VK_INDEX_TYPE_UINT32);
                                c_.vkCmdDrawIndexed(cmd, draw->index_count, 1, 0, 0, 0);
                            } else
                                c_.vkCmdDraw(cmd, draw->geometry->count, 1, 0, 0);
                        }
                    }
                    // Composite the accumulated transparency over the opaque scene. Both
                    // methods store premultiplied color and over-blend it here, so the
                    // picking attachment keeps the nearest surface.
                    if (oit_active_ && pass.kind != PassKind::ids && pass.oit_average_pipeline) {
                        VkViewport viewport = {0, 0, float(width), float(height), 0, 1};
                        const VkRect2D scissor = {{0, 0}, {width, height}};
                        c_.vkCmdSetViewport(cmd, 0, 1, &viewport);
                        c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
                        c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                        c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             oit_method_ == TransparencyMethod::average ? pass.oit_average_pipeline
                                                                                        : pass.oit_peel_pipeline);
                        c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, oit_composite_layout_, 0, 1,
                                                   &oit_->composite_set, 0, nullptr);
                        c_.vkCmdDraw(cmd, 3, 1, 0, 0);
                    }
                    if (pass.kind != PassKind::ids) {
                        const unsigned phases = pass.independent_depth ? 3 : 2;
                        for (unsigned phase = 0; phase < phases; ++phase) {
                            c_.vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
                            for (const auto &draw : draws) {
                                const auto &d = draw.command;
                                if (!d.overlay)
                                    continue;
                                const unsigned draw_phase = d.overlay_depth_test          ? 0
                                                            : d.overlay_independent_depth ? 1
                                                                                          : phases - 1;
                                if (draw_phase != phase)
                                    continue;
                                set_view(d);
                                c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                                const auto pipeline = d.shadow                ? pass.shadow_pipeline
                                                      : d.ambient_occlusion   ? pass.ssao_pipeline
                                                      : d.eye_dome_lighting   ? pass.edl_pipeline
                                                      : d.scene_depth_texture ? pass.depth_texture_pipeline
                                                      : phase == 0            ? pass.depth_overlay_pipeline
                                                      : phase == phases - 1   ? pass.overlay_pipeline
                                                                              : pass.hud_3d_pipeline;
                                c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                                VkDeviceSize offset = 0;
                                c_.vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
                                const auto texture_set =
                                    (d.scene_depth_texture || d.eye_dome_lighting || d.ambient_occlusion || d.shadow)
                                        ? target.depth_set
                                        : draw.texture->set;
                                c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 0, 1,
                                                           &texture_set, 0, nullptr);
                                c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 1, 1,
                                                           &impostor_set_, 1, &draw.uniform_offset);
                                Push push{};
                                for (unsigned col = 0; col < 4; ++col)
                                    for (unsigned row = 0; row < 4; ++row)
                                        push.mvp[col * 4 + row] = d.model_view_projection(row, col);
                                for (unsigned i = 0; i < 4; ++i)
                                    push.color[i] = d.color[i];
                                push.lighting = d.lighting ? 1 : 0;
                                for (unsigned col = 0; col < 3; ++col)
                                    for (unsigned row = 0; row < 3; ++row)
                                        push.normal[col * 3 + row] = d.normal_matrix(row, col);
                                c_.vkCmdPushConstants(cmd, layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
                                c_.vkCmdDraw(cmd, draw.geometry->count, 1, 0, 0);
                            }
                        }
                    }
                    c_.vkCmdEndRenderPass(cmd);
                }
            };

            class ShadowPassEncoder {
                Context &c_;
                VkPipelineLayout layout_;
                const ShadowMap *shadow_map_;

            public:
                ShadowPassEncoder(Context &context, VkPipelineLayout layout, const ShadowMap *shadow_map)
                    : c_(context), layout_(layout), shadow_map_(shadow_map) {}

                template <typename Prepared>
                void encode(VkCommandBuffer cmd, const std::vector<Prepared> &draws) {
                    if (!shadow_map_)
                        return;
                    VkClearValue clear{};
                    clear.depthStencil.depth = 1;
                    VkRenderPassBeginInfo begin{};
                    begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    begin.renderPass = shadow_map_->pass;
                    begin.framebuffer = shadow_map_->framebuffer;
                    begin.renderArea = {{0, 0}, {shadow_map_->size, shadow_map_->size}};
                    begin.clearValueCount = 1;
                    begin.pClearValues = &clear;
                    c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
                    VkViewport viewport = {0, 0, float(shadow_map_->size), float(shadow_map_->size), 0, 1};
                    const VkRect2D scissor = begin.renderArea;
                    c_.vkCmdSetViewport(cmd, 0, 1, &viewport);
                    c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
                    c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_map_->pipeline);
                    for (const auto &draw : draws) {
                        const auto &d = draw.command;
                        if (!d.shadow_caster)
                            continue;
                        VkDeviceSize offset = 0;
                        c_.vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
                        Push push{};
                        const auto mvp = d.shadow_matrix * d.model_view_projection;
                        for (unsigned col = 0; col < 4; ++col)
                            for (unsigned row = 0; row < 4; ++row)
                                push.mvp[col * 4 + row] = mvp(row, col);
                        c_.vkCmdPushConstants(cmd, layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
                        c_.vkCmdDraw(cmd, draw.geometry->count, 1, 0, 0);
                    }
                    c_.vkCmdEndRenderPass(cmd);
                }
            };

            class AmbientOcclusionPassEncoder {
                Context &c_;
                Ao &ao_;
                VkPipelineLayout layout_;
                VkPipeline horizons_;
                VkPipeline denoise_;
                Target &target_;

            public:
                AmbientOcclusionPassEncoder(Context &context,
                                            Ao &ao,
                                            VkPipelineLayout layout,
                                            VkPipeline horizons,
                                            VkPipeline denoise,
                                            Target &target)
                    : c_(context), ao_(ao), layout_(layout), horizons_(horizons), denoise_(denoise),
                      target_(target) {}

                void dispatch(VkCommandBuffer cmd) {
                    const auto color = VK_IMAGE_ASPECT_COLOR_BIT;
                    const auto general = VK_IMAGE_LAYOUT_GENERAL;
                    if (!ao_.initialized) {
                        image_barrier(c_, cmd, ao_.raw->handle, color, VK_IMAGE_LAYOUT_UNDEFINED, general,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                      VK_ACCESS_SHADER_WRITE_BIT);
                        image_barrier(c_, cmd, ao_.blurred->handle, color, VK_IMAGE_LAYOUT_UNDEFINED, general,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                                      VK_ACCESS_SHADER_WRITE_BIT);
                        ao_.initialized = true;
                    } else {
                        image_barrier(c_, cmd, ao_.raw->handle, color, general, general,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT);
                        // The previous frame left the denoised image readable by the composite.
                        image_barrier(c_, cmd, ao_.blurred->handle, color, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      general, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                      VK_ACCESS_SHADER_WRITE_BIT);
                    }
                    const uint32_t groups_x = (ao_.width + 7) / 8, groups_y = (ao_.height + 7) / 8;
                    c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout_, 0, 1, &ao_.set, 0, nullptr);
                    c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, horizons_);
                    c_.vkCmdDispatch(cmd, groups_x, groups_y, 1);
                    image_barrier(c_, cmd, ao_.raw->handle, color, general, general,
                                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                    c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, denoise_);
                    c_.vkCmdDispatch(cmd, groups_x, groups_y, 1);
                    image_barrier(c_, cmd, ao_.blurred->handle, color, general,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                  VK_ACCESS_SHADER_READ_BIT);
                    if (target_.samples == 1) {
                        // The color pass rewrites the depth attachment; order it after the
                        // compute reads instead of discarding pending work.
                        image_barrier(
                            c_, cmd, target_.depth.handle, VK_IMAGE_ASPECT_DEPTH_BIT,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            VK_ACCESS_SHADER_READ_BIT,
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
                    }
                }
            };

            class ClippedGeometryCache {
                struct Entry {
                    std::weak_ptr<const Geometry> source;
                    std::vector<internal::ClipPlane> object_planes;
                    mat3 normal;
                    bool lighting = false, discard = false, analytic = false, baked = false, material = false;
                    std::shared_ptr<const Geometry> geometry;
                };
                std::map<const Geometry *, Entry> entries_;

            public:
                void prune() {
                    for (auto it = entries_.begin(); it != entries_.end();) {
                        if (it->second.source.expired())
                            it = entries_.erase(it);
                        else
                            ++it;
                    }
                }

                DrawCommand apply(const DrawCommand &input) {
                    auto command = input;
                    command.clipping_planes = normalize_clipping_planes(input.clipping_planes);
                    if (command.overlay || command.clipping_planes.empty())
                        return command;
                    for (unsigned c = 0; c < 4; ++c)
                        for (unsigned r = 0; r < 4; ++r)
                            if (!std::isfinite(command.model_view(r, c)))
                                throw std::invalid_argument("non-finite clipping model-view");
                    if (command.model_view(3, 0) != 0 || command.model_view(3, 1) != 0 ||
                        command.model_view(3, 2) != 0 || command.model_view(3, 3) != 1)
                        throw std::invalid_argument("clipping requires an affine model-view transform");
                    const bool analytic =
                        (command.geometry->topology() == Topology::points &&
                         command.point_style != PointStyle::plain) ||
                        (command.geometry->topology() == Topology::lines && command.line_style != LineStyle::plain);
                    if (command.geometry->topology() == Topology::points ||
                        (analytic && !command.discard_clipped_primitives))
                        return command;
                    // Keying on the object-space halfspaces instead of the view-space planes and
                    // model_view keeps an orbit from discarding the cut: the camera cancels out of
                    // them exactly, and clip_draw() tests these same values.
                    const auto object_planes = internal::object_clip_planes(command);
                    auto &entry = entries_[input.geometry.get()];
                    if (entry.source.lock() != input.geometry || entry.object_planes != object_planes ||
                        entry.normal != command.normal_matrix ||
                        entry.lighting != command.lighting || entry.material != command.material_lighting ||
                        entry.discard != command.discard_clipped_primitives || entry.analytic != analytic) {
                        auto result = internal::clip_draw(command);
                        entry.source = input.geometry;
                        entry.object_planes = object_planes;
                        entry.normal = command.normal_matrix;
                        entry.material = command.material_lighting;
                        entry.lighting = command.lighting;
                        entry.discard = command.discard_clipped_primitives;
                        entry.analytic = analytic;
                        entry.geometry = result.geometry;
                        entry.baked = command.lighting && !result.lighting;
                    }
                    command.geometry = entry.geometry;
                    if (entry.baked)
                        command.lighting = false;
                    return command;
                }
            };

            class DrawCommandValidator {
                const Context &c_;

            public:
                explicit DrawCommandValidator(const Context &context) : c_(context) {}

                void validate(uint32_t width,
                              uint32_t height,
                              unsigned samples,
                              bool pixel_query,
                              const std::vector<DrawCommand> &commands) const {
                    const auto &limits = c_.properties.limits;
                    for (const auto &command : commands) {
                        if (pixel_query && command.overlay)
                            continue;
                        const auto &vp = command.viewport;
                        if (vp != ivec4(0) &&
                            (vp.x < 0 || vp.y < 0 || vp.z <= 0 || vp.w <= 0 ||
                             uint64_t(vp.x) + uint64_t(vp.z) > width || uint64_t(vp.y) + uint64_t(vp.w) > height))
                            throw std::invalid_argument("draw viewport is outside the render target");
                        if (!command.geometry ||
                            (!command.overlay && command.object_id == UINT32_MAX && !command.shadow_background))
                            throw std::invalid_argument("scene draw requires geometry and a non-background object ID");
                        if (command.overlay_depth_test && !command.overlay)
                            throw std::invalid_argument("overlay depth testing requires an overlay command");
                        if (command.overlay_independent_depth && (!command.overlay || command.overlay_depth_test))
                            throw std::invalid_argument(
                                "independent HUD depth requires an overlay without scene depth testing");
                        if (command.scene_depth_texture && (!command.overlay || command.overlay_depth_test ||
                                                            command.overlay_independent_depth || command.texture))
                            throw std::invalid_argument(
                                "scene depth sampling requires a flat overlay without an image texture");
                        if (command.eye_dome_lighting && (!command.overlay || command.overlay_depth_test ||
                                                          command.overlay_independent_depth || command.texture))
                            throw std::invalid_argument(
                                "eye-dome lighting requires a flat overlay without an image texture");
                        if (command.ambient_occlusion && (!command.overlay || command.overlay_depth_test ||
                                                          command.overlay_independent_depth || command.texture))
                            throw std::invalid_argument(
                                "ambient occlusion requires a flat overlay without an image texture");
                        if (command.ambient_occlusion && (!std::isfinite(command.color.x) || command.color.x <= 0 ||
                                                          !std::isfinite(command.color.y) || command.color.y < 0))
                            throw std::invalid_argument(
                                "ambient occlusion requires a positive radius and a non-negative bias");
                        if (command.shadow && (!command.overlay || command.overlay_depth_test ||
                                               command.overlay_independent_depth || command.texture))
                            throw std::invalid_argument("shadow requires a flat overlay without an image texture");
                        if (command.shadow &&
                            (!std::isfinite(command.color.x) || command.color.x < 0 || command.color.x > 1 ||
                             !std::isfinite(command.color.y) || command.color.y < 0))
                            throw std::invalid_argument("shadow requires darkness in [0, 1] and non-negative softness");
                        if (command.shadow && (!std::isfinite(command.color.z) || !std::isfinite(command.color.w) ||
                                               command.color.z <= 0 || command.color.w <= command.color.z))
                            throw std::invalid_argument("shadow requires a positive light near/far range");
                        if ((command.shadow || command.shadow_caster || command.shadow_background) &&
                            (!command.shadow_map_size || (command.shadow_map_size & (command.shadow_map_size - 1))))
                            throw std::invalid_argument("shadow map size must be a nonzero power of two");
                        if ((command.shadow_caster || command.shadow_background) &&
                            command.geometry->topology() != Topology::triangles)
                            throw std::invalid_argument("shadow casters and receivers require triangle geometry");
                        if (command.shadow_background && (command.overlay || command.object_id != UINT32_MAX))
                            throw std::invalid_argument(
                                "shadow background requires a scene draw with the background ID");
                        if (command.transparent && command.overlay)
                            throw std::invalid_argument(
                                "transparent blending is a scene option, not an overlay option");
                        if (command.transparent && command.geometry->topology() != Topology::triangles)
                            throw std::invalid_argument("transparent blending requires triangle geometry");
                        if (command.transparent && (command.color.w < 0 || command.color.w > 1))
                            throw std::invalid_argument("transparent color alpha must be in [0, 1]");
                        if (command.transparent && !c_.independent_blend && !pixel_query)
                            throw std::runtime_error("transparent scene blending requires independentBlend");
                        if (command.overlay && command.geometry->topology() != Topology::triangles)
                            throw std::invalid_argument("overlay draws require triangle geometry");
                        if (command.point_style != PointStyle::plain && command.point_style != PointStyle::sphere &&
                            command.point_style != PointStyle::surfel)
                            throw std::invalid_argument("unknown point style");
                        if (command.line_style != LineStyle::plain && command.line_style != LineStyle::cylinder &&
                            command.line_style != LineStyle::cone)
                            throw std::invalid_argument("unknown line style");
                        if (!std::isfinite(command.line_width) || command.line_width <= 0)
                            throw std::invalid_argument("line width must be finite and positive");
                        if (!std::isfinite(command.line_radius) || command.line_radius <= 0)
                            throw std::invalid_argument("line radius must be finite and positive");
                        if (command.geometry->topology() != Topology::lines && command.line_style != LineStyle::plain)
                            throw std::invalid_argument("line impostor style requires line geometry");
                        if (command.line_style != LineStyle::plain &&
                            (command.model_view(3, 0) != 0 || command.model_view(3, 1) != 0 ||
                             command.model_view(3, 2) != 0 || command.model_view(3, 3) != 1))
                            throw std::invalid_argument("line impostor model-view transform must be affine");
                        if (!std::isfinite(command.point_size) || command.point_size <= 0 ||
                            !std::isfinite(command.sphere_radius) || command.sphere_radius <= 0)
                            throw std::invalid_argument("point size and sphere radius must be finite and positive");
                        if (command.geometry->topology() != Topology::points &&
                            command.point_style != PointStyle::plain)
                            throw std::invalid_argument("point impostor style requires point geometry");
                        if ((command.point_style != PointStyle::plain || command.line_style != LineStyle::plain) &&
                            samples > 1 && !c_.sample_shading)
                            throw std::invalid_argument(
                                "multisampled impostors require sampleRateShading for correct coverage/depth");
                        if (command.shadow || command.shadow_caster || command.shadow_background) {
                            for (unsigned col = 0; col < 4; ++col)
                                for (unsigned row = 0; row < 4; ++row)
                                    if (!std::isfinite(command.shadow_matrix(row, col)))
                                        throw std::invalid_argument("non-finite shadow matrix");
                        }
                        for (unsigned col = 0; col < 4; ++col) {
                            if (!std::isfinite(command.color[col]))
                                throw std::invalid_argument("non-finite draw color");
                            for (unsigned row = 0; row < 4; ++row)
                                if (!std::isfinite(command.model_view_projection(row, col)))
                                    throw std::invalid_argument("non-finite transform");
                        }
                        if (command.material_lighting) {
                            if (!std::isfinite(command.shininess) || command.shininess < 0)
                                throw std::invalid_argument("material shininess must be finite and nonnegative");
                            for (unsigned c = 0; c < 4; ++c) {
                                if (!std::isfinite(command.ambient[c]) || !std::isfinite(command.specular[c]) ||
                                    !std::isfinite(command.back_color[c]))
                                    throw std::invalid_argument("non-finite material color");
                                for (unsigned r = 0; r < 4; ++r)
                                    if (!std::isfinite(command.model(r, c)))
                                        throw std::invalid_argument("non-finite material model transform");
                            }
                            for (unsigned c = 0; c < 3; ++c)
                                if (!std::isfinite(command.eye[c]) || !std::isfinite(command.light[c]))
                                    throw std::invalid_argument("non-finite material camera/light");
                            if (command.lighting && dot(command.light, command.light) == 0)
                                throw std::invalid_argument("material lighting requires a nonzero direction");
                        }
                        if (command.point_style == PointStyle::surfel) {
                            if (command.model_view(3, 0) != 0 || command.model_view(3, 1) != 0 ||
                                command.model_view(3, 2) != 0 || command.model_view(3, 3) != 1)
                                throw std::invalid_argument("surfel model-view transform must be affine");
                            const auto normal_transform = transpose(inverse(mat3(command.model_view)));
                            for (unsigned col = 0; col < 3; ++col)
                                for (unsigned row = 0; row < 3; ++row)
                                    if (!std::isfinite(normal_transform(row, col)))
                                        throw std::invalid_argument(
                                            "surfel model-view transform must have an invertible linear part");
                            const auto &vertices = command.geometry->vertices();
                            const auto &indices = command.geometry->indices();
                            const size_t count = indices.empty() ? vertices.size() : indices.size();
                            for (size_t i = 0; i < count; ++i) {
                                const auto n = normal_transform * vertices[indices.empty() ? i : indices[i]].normal;
                                const float length_squared = dot(n, n);
                                if (!std::isfinite(length_squared) || length_squared <= 0)
                                    throw std::invalid_argument("surfels require finite nonzero transformed normals");
                            }
                        }
                        for (unsigned col = 0; col < 3; ++col)
                            for (unsigned row = 0; row < 3; ++row)
                                if (!std::isfinite(command.normal_matrix(row, col)))
                                    throw std::invalid_argument("non-finite normal transform");
                        if (command.ambient_occlusion)
                            for (unsigned col = 0; col < 4; ++col)
                                for (unsigned row = 0; row < 4; ++row)
                                    if (!std::isfinite(command.projection(row, col)))
                                        throw std::invalid_argument("non-finite ambient-occlusion projection");
                    }
                }
            };

            class TransparentTriangleOrderBuilder {
            public:
                struct Range {
                    uint32_t offset = 0;
                    uint32_t count = 0;
                };

                static Range append(const DrawCommand &command, std::vector<uint32_t> &indices) {
                    const auto &vertices = command.geometry->vertices();
                    const auto &source_indices = command.geometry->indices();
                    const size_t count = source_indices.empty() ? vertices.size() : source_indices.size();
                    std::vector<std::pair<float, uint32_t>> order(count / 3);
                    for (size_t t = 0; t < order.size(); ++t) {
                        vec3 centroid(0);
                        for (unsigned j = 0; j < 3; ++j) {
                            const size_t vertex = source_indices.empty() ? 3 * t + j : source_indices[3 * t + j];
                            centroid += vertices[vertex].position;
                        }
                        const auto clip = command.model_view_projection * vec4(centroid / 3.0f, 1);
                        order[t] = {0.5f * (clip.z + clip.w), uint32_t(t)};
                    }
                    std::stable_sort(order.begin(), order.end(),
                                     [](const std::pair<float, uint32_t> &a,
                                        const std::pair<float, uint32_t> &b) { return a.first > b.first; });
                    const size_t added = order.size() * 3;
                    if (added > UINT32_MAX || indices.size() > UINT32_MAX - added)
                        throw std::invalid_argument("too many transparent triangles");
                    Range range;
                    range.offset = uint32_t(indices.size());
                    range.count = uint32_t(added);
                    for (const auto &entry : order) {
                        const uint32_t first = entry.second * 3;
                        indices.push_back(first);
                        indices.push_back(first + 1);
                        indices.push_back(first + 2);
                    }
                    return range;
                }
            };

            class ImpostorUniformBuilder {
            public:
                static ImpostorUniform build(const DrawCommand &command, uint32_t width, uint32_t height) {
                    ImpostorUniform uniform{};
                    const bool analytic =
                        command.point_style != PointStyle::plain || command.line_style != LineStyle::plain;
                    const bool camera_space_line = command.geometry->topology() == Topology::lines &&
                                                   command.line_style == LineStyle::plain &&
                                                   !command.line_screen_space_width;
                    mat4 model_view = mat4::identity(), projection = mat4::identity();
                    if (analytic || camera_space_line) {
                        model_view = command.model_view;
                        projection = command.projection;
                    } else if (command.ambient_occlusion)
                        projection = command.projection;
                    else if (command.geometry->topology() == Topology::points &&
                             !command.clipping_planes.empty())
                        model_view = command.model_view;
                    const auto inverse_projection =
                        (analytic || command.ambient_occlusion) ? inverse(projection) : mat4::identity();
                    for (unsigned col = 0; col < 4; ++col)
                        for (unsigned row = 0; row < 4; ++row) {
                            uniform.model_view[col * 4 + row] = model_view(row, col);
                            uniform.projection[col * 4 + row] = projection(row, col);
                            uniform.inverse_projection[col * 4 + row] = inverse_projection(row, col);
                            if (!std::isfinite(uniform.model_view[col * 4 + row]) ||
                                !std::isfinite(uniform.projection[col * 4 + row]) ||
                                !std::isfinite(uniform.inverse_projection[col * 4 + row]))
                                throw std::invalid_argument("invalid impostor transform/projection");
                        }
                    uniform.dimensions[0] = float(command.viewport.z ? command.viewport.z : width);
                    uniform.dimensions[1] = float(command.viewport.w ? command.viewport.w : height);
                    uniform.viewport_origin[0] = float(command.viewport.x);
                    uniform.viewport_origin[1] = float(command.viewport.y);
                    // z = logical pixel size for plain styles; w = world-space
                    // impostor radius (diameter * world_pixel_ratio * 0.5).
                    uniform.dimensions[2] = expanded_lines(command) ? command.line_width : command.point_size;
                    uniform.dimensions[3] = expanded_lines(command) ? command.line_radius : command.sphere_radius;
                    for (unsigned i = 0; i < 3; ++i) {
                        if (!std::isfinite(command.sphere_light[i]))
                            throw std::invalid_argument("non-finite impostor light");
                        uniform.light[i] = command.sphere_light[i];
                    }
                    if (analytic && command.lighting && dot(command.sphere_light, command.sphere_light) == 0)
                        throw std::invalid_argument("impostor lighting requires a nonzero direction");
                    uniform.light[3] = command.point_style == PointStyle::surfel   ? 2.f
                                       : command.point_style == PointStyle::sphere ? 1.f
                                                                                   : 0.f;
                    if (expanded_lines(command))
                        uniform.light[3] = command.line_style == LineStyle::plain  ? 5.f
                                           : command.line_style == LineStyle::cone ? 4.f
                                                                                   : 3.f;
                    if (analytic || command.geometry->topology() == Topology::points) {
                        uniform.clip_info[1] = command.discard_clipped_primitives ? 1.f : 0.f;
                        uniform.clip_info[0] = float(command.clipping_planes.size());
                        for (size_t plane = 0; plane < command.clipping_planes.size(); ++plane)
                            for (unsigned c = 0; c < 4; ++c)
                                uniform.clip_planes[plane * 4 + c] = command.clipping_planes[plane][c];
                    }
                    if (command.geometry->topology() == Topology::lines && command.line_style == LineStyle::plain) {
                        // 1: expand the plain line in screen space, 0: in camera space.
                        uniform.line_options[0] = command.line_screen_space_width ? 1u : 0u;
                    }
                    for (unsigned col = 0; col < 4; ++col)
                        for (unsigned row = 0; row < 4; ++row)
                            uniform.model[col * 4 + row] = command.model(row, col);
                    for (unsigned i = 0; i < 3; ++i) {
                        uniform.eye[i] = command.eye[i];
                        uniform.material_light[i] = command.light[i];
                    }
                    for (unsigned i = 0; i < 4; ++i) {
                        uniform.ambient[i] = command.ambient[i];
                        uniform.specular[i] = command.specular[i];
                        uniform.backside[i] = command.back_color[i];
                    }
                    uniform.shading[0] = command.material_lighting ? 1.f : 0.f;
                    uniform.shading[1] = command.lighting_two_sides ? 1.f : 0.f;
                    uniform.shading[2] = command.distinct_back_color ? 1.f : 0.f;
                    uniform.shading[3] = command.shininess;
                    for (unsigned col = 0; col < 4; ++col)
                        for (unsigned row = 0; row < 4; ++row)
                            uniform.shadow[col * 4 + row] = command.shadow_matrix(row, col);
                    return uniform;
                }
            };

            class DrawBufferCache {
                Context &c_;
                VkDescriptorSet impostor_set_;
                std::unique_ptr<Buffer> uniforms_, transparent_indices_;
                size_t uniform_capacity_ = 0, index_capacity_ = 0;

            public:
                DrawBufferCache(Context &context, VkDescriptorSet impostor_set)
                    : c_(context), impostor_set_(impostor_set) {}

                void upload(std::vector<unsigned char> &uniforms, std::vector<uint32_t> &transparent_indices) {
                    if (!uniforms.empty()) {
                        if (uniforms.size() > uniform_capacity_) {
                            std::unique_ptr<Buffer> buffer(new Buffer(c_));
                            buffer->init(uniforms.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
                            VkDescriptorBufferInfo info = {buffer->handle, 0, sizeof(ImpostorUniform)};
                            VkWriteDescriptorSet write{};
                            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write.dstSet = impostor_set_;
                            write.descriptorCount = 1;
                            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                            write.pBufferInfo = &info;
                            c_.vkUpdateDescriptorSets(c_.device, 1, &write, 0, nullptr);
                            uniforms_ = std::move(buffer);
                            uniform_capacity_ = uniforms.size();
                        }
                        uniforms_->transfer(uniforms.data(), uniforms.size(), true);
                    }
                    if (!transparent_indices.empty()) {
                        const size_t bytes = transparent_indices.size() * sizeof(uint32_t);
                        if (bytes > index_capacity_) {
                            std::unique_ptr<Buffer> buffer(new Buffer(c_));
                            buffer->init(bytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true);
                            transparent_indices_ = std::move(buffer);
                            index_capacity_ = bytes;
                        }
                        transparent_indices_->transfer(transparent_indices.data(), bytes, true);
                    }
                }

                VkBuffer transparent_index_buffer() const {
                    return transparent_indices_ ? transparent_indices_->handle : VK_NULL_HANDLE;
                }
            };

            class FrameTargetCoordinator {
                Context &c_;
                PassCache &passes_;
                std::unique_ptr<Target> &target_;
                std::unique_ptr<ShadowMap> &shadow_map_;
                uint32_t &shadow_bound_size_;
                VkDescriptorPool descriptor_pool_;
                VkDescriptorSetLayout descriptors_;
                const VkSampler *samplers_;
                VkPipelineLayout shadow_layout_;

            public:
                FrameTargetCoordinator(Context &context,
                                       PassCache &passes,
                                       std::unique_ptr<Target> &target,
                                       std::unique_ptr<ShadowMap> &shadow_map,
                                       uint32_t &shadow_bound_size,
                                       VkDescriptorPool descriptor_pool,
                                       VkDescriptorSetLayout descriptors,
                                       const VkSampler *samplers,
                                       VkPipelineLayout shadow_layout)
                    : c_(context), passes_(passes), target_(target), shadow_map_(shadow_map),
                      shadow_bound_size_(shadow_bound_size), descriptor_pool_(descriptor_pool),
                      descriptors_(descriptors), samplers_(samplers), shadow_layout_(shadow_layout) {}

                template <typename CreatePipelines>
                void prepare(uint32_t width,
                             uint32_t height,
                             unsigned samples,
                             const std::vector<DrawCommand> &commands,
                             bool ao_active,
                             const CreatePipelines &create_pipelines) {
                    const auto &limits = c_.properties.limits;
                    unsigned shadow_size = 0;
                    for (const auto &command : commands)
                        if ((command.shadow || command.shadow_caster || command.shadow_background) &&
                            command.shadow_map_size > shadow_size)
                            shadow_size = command.shadow_map_size;
                    if (shadow_size) {
                        if (shadow_size > limits.maxImageDimension2D || shadow_size > limits.maxFramebufferWidth ||
                            shadow_size > limits.maxFramebufferHeight)
                            throw std::invalid_argument("shadow map size exceeds device limits");
                        if (!shadow_map_ || shadow_map_->size != shadow_size) {
                            shadow_map_.reset();
                            std::unique_ptr<ShadowMap> map(new ShadowMap(c_));
                            map->init(shadow_size, shadow_layout_);
                            shadow_map_ = std::move(map);
                        }
                    }
                    const bool independent_depth =
                        std::any_of(commands.begin(), commands.end(),
                                    [](const DrawCommand &d) { return d.overlay_independent_depth; });
                    const bool depth_preview =
                        std::any_of(commands.begin(), commands.end(), [](const DrawCommand &d) {
                            return d.scene_depth_texture || d.eye_dome_lighting || d.ambient_occlusion || d.shadow;
                        });
                    // Ambient occlusion resolves depth in a pre-pass, so it needs the
                    // single-sample picking target even without multisampling.
                    const bool needs_ids = samples > 1 || ao_active;
                    if (!target_ || target_->width != width || target_->height != height ||
                        target_->samples != samples || target_->color_pass.independent_depth != independent_depth ||
                        target_->color_pass.depth_preview != depth_preview || (target_->id_pass != nullptr) != needs_ids) {
                        if (depth_preview) {
                            VkFormatProperties properties;
                            c_.vkGetPhysicalDeviceFormatProperties(c_.physical, VK_FORMAT_D32_SFLOAT, &properties);
                            if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
                                throw std::runtime_error("scene depth preview requires sampleable D32 depth");
                        }
                        auto &pass =
                            passes_.get_color(c_, samples, independent_depth, depth_preview, create_pipelines);
                        auto *picking = needs_ids ? &passes_.get_picking(c_, create_pipelines) : nullptr;
                        std::unique_ptr<Target> target(new Target(c_, width, height, pass, picking));
                        target->init();
                        target_ = std::move(target);
                    }
                    if (depth_preview && !target_->depth_set) {
                        target_->depth_pool = descriptor_pool_;
                        VkDescriptorSetAllocateInfo ai{};
                        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                        ai.descriptorPool = descriptor_pool_;
                        ai.descriptorSetCount = 1;
                        ai.pSetLayouts = &descriptors_;
                        check(c_.vkAllocateDescriptorSets(c_.device, &ai, &target_->depth_set),
                              "allocate scene depth descriptor");
                        VkDescriptorImageInfo images[3] = {
                            {samplers_[0], target_->depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
                            {samplers_[1], shadow_map_ ? shadow_map_->image.view : target_->depth.view,
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
                            {samplers_[0], target_->depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL}};
                        VkWriteDescriptorSet write[3] = {};
                        for (unsigned i = 0; i < 3; ++i) {
                            write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write[i].dstSet = target_->depth_set;
                            write[i].dstBinding = i;
                            write[i].descriptorCount = 1;
                            write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            write[i].pImageInfo = &images[i];
                        }
                        c_.vkUpdateDescriptorSets(c_.device, 3, write, 0, nullptr);
                        shadow_bound_size_ = shadow_map_ ? shadow_map_->size : 0;
                    } else if (depth_preview && shadow_map_ && shadow_bound_size_ != shadow_map_->size) {
                        VkDescriptorImageInfo image = {samplers_[1], shadow_map_->image.view,
                                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
                        VkWriteDescriptorSet write{};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.dstSet = target_->depth_set;
                        write.dstBinding = 1;
                        write.descriptorCount = 1;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &image;
                        c_.vkUpdateDescriptorSets(c_.device, 1, &write, 0, nullptr);
                        shadow_bound_size_ = shadow_map_->size;
                    }
                }
            };

            class VulkanDevice final : public RenderDevice {
                Context c_;
                std::string name_;
                std::unique_ptr<PointSelector> point_selector_;
                std::unique_ptr<GpuResources> resources_;
                uint32_t sample_counts_ = 1;
                PassCache passes_;
                VkDescriptorSetLayout descriptors_ = VK_NULL_HANDLE;
                VkDescriptorSetLayout impostor_descriptors_ = VK_NULL_HANDLE;
                VkDescriptorSet impostor_set_ = VK_NULL_HANDLE;
                std::unique_ptr<DrawBufferCache> draw_buffers_;
                VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
                VkSampler samplers_[4] = {};
                VkPipelineLayout layout_ = VK_NULL_HANDLE;
                VkDescriptorSetLayout ao_descriptors_ = VK_NULL_HANDLE;
                VkPipelineLayout ao_layout_ = VK_NULL_HANDLE;
                VkPipeline ao_horizons_ = VK_NULL_HANDLE, ao_denoise_ = VK_NULL_HANDLE;
                std::unique_ptr<Ao> ao_;
                bool ao_active_ = false;
                mat4 ao_projection_ = mat4::identity();
                float ao_radius_ = 0.5f, ao_bias_ = 0.005f;
                VkDescriptorSetLayout oit_scene_descriptors_ = VK_NULL_HANDLE;
                VkDescriptorSetLayout oit_composite_descriptors_ = VK_NULL_HANDLE;
                VkPipelineLayout oit_peel_layout_ = VK_NULL_HANDLE, oit_composite_layout_ = VK_NULL_HANDLE;
                VkDescriptorSetLayout oit_lists_descriptors_ = VK_NULL_HANDLE;
                VkPipelineLayout oit_lists_layout_ = VK_NULL_HANDLE, oit_resolve_layout_ = VK_NULL_HANDLE;
                std::unique_ptr<Oit> oit_;
                std::unique_ptr<LinkedLists> lists_;
                TransparencyMethod oit_method_ = TransparencyMethod::sorted;
                unsigned oit_layers_ = 4;
                bool oit_active_ = false;
                uint32_t oit_dropped_ = 0;
                std::unique_ptr<Target> target_, pixel_target_;
                std::unique_ptr<ShadowMap> shadow_map_;
                uint32_t shadow_bound_size_ = 0;
                std::unique_ptr<Presentation> presentation_;
                std::shared_ptr<const Image> white_;
                ClippedGeometryCache clipped_;

            public:
                // Separate initialization ensures all members are destroyed if any Vulkan step throws.
                ~VulkanDevice() override {
                    // Presentation may still be using image-owned semaphores after the draw
                    // fence completes. Drain at shutdown/recreation only, never per frame.
                    if (c_.device)
                        c_.vkDeviceWaitIdle(c_.device);
                    presentation_.reset();
                    target_.reset();
                    pixel_target_.reset();
                    shadow_map_.reset();
                    point_selector_.reset();
                    // Free transparency descriptor sets before the textures they reference.
                    lists_.reset();
                    oit_.reset();
                    resources_.reset();
                    passes_.clear();
                    ao_.reset();
                    if (ao_horizons_)
                        c_.vkDestroyPipeline(c_.device, ao_horizons_, nullptr);
                    if (ao_denoise_)
                        c_.vkDestroyPipeline(c_.device, ao_denoise_, nullptr);
                    if (ao_layout_)
                        c_.vkDestroyPipelineLayout(c_.device, ao_layout_, nullptr);
                    if (ao_descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, ao_descriptors_, nullptr);
                    if (oit_peel_layout_)
                        c_.vkDestroyPipelineLayout(c_.device, oit_peel_layout_, nullptr);
                    if (oit_composite_layout_)
                        c_.vkDestroyPipelineLayout(c_.device, oit_composite_layout_, nullptr);
                    if (oit_lists_layout_)
                        c_.vkDestroyPipelineLayout(c_.device, oit_lists_layout_, nullptr);
                    if (oit_resolve_layout_)
                        c_.vkDestroyPipelineLayout(c_.device, oit_resolve_layout_, nullptr);
                    if (oit_lists_descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, oit_lists_descriptors_, nullptr);
                    if (oit_scene_descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, oit_scene_descriptors_, nullptr);
                    if (oit_composite_descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, oit_composite_descriptors_, nullptr);
                    if (layout_)
                        c_.vkDestroyPipelineLayout(c_.device, layout_, nullptr);
                    for (auto sampler : samplers_)
                        if (sampler)
                            c_.vkDestroySampler(c_.device, sampler, nullptr);
                    if (descriptor_pool_)
                        c_.vkDestroyDescriptorPool(c_.device, descriptor_pool_, nullptr);
                    if (descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, descriptors_, nullptr);
                    if (impostor_descriptors_)
                        c_.vkDestroyDescriptorSetLayout(c_.device, impostor_descriptors_, nullptr);
                }
                const std::string &name() const override {
                    return name_;
                }
                uint32_t supported_sample_counts(PointStyle style) const override {
                    return style != PointStyle::plain && !c_.sample_shading ? 1 : sample_counts_;
                }
                uint32_t supported_sample_counts(LineStyle style) const override {
                    return style != LineStyle::plain && !c_.sample_shading ? 1 : sample_counts_;
                }
                unsigned validation_errors() const override {
                    return c_.errors.load();
                }
                uint64_t point_upload_count() const override {
                    return resources_ ? resources_->point_upload_count() : 0;
                }
                uint32_t oit_dropped_fragments() const override {
                    return oit_dropped_;
                }
                GpuPoints &point_resource(const std::shared_ptr<const PointData> &data) {
                    return resources_->point(data);
                }
                std::vector<uint32_t> select_points(const PointSelectionRequest &request) override {
                    if (!request.width || !request.height)
                        throw std::invalid_argument("selection requires positive framebuffer dimensions");
                    if (!request.points)
                        throw std::invalid_argument("selection requires a point resource");
                    if (request.polygon.size() > UINT32_MAX)
                        throw std::invalid_argument("selection input exceeds 32-bit indexing");
                    const size_t count = (size_t(request.points->source_count()) + 31) / 32;
                    auto words = request.words;
                    if (words.empty())
                        words.resize(count, 0);
                    else if (words.size() != count)
                        throw std::invalid_argument("selection word count does not match points");
                    for (unsigned i = 0; i < 16; ++i)
                        if (!std::isfinite(request.model_view_projection[i]))
                            throw std::invalid_argument("nonfinite selection transform");
                    for (const auto &p : request.polygon)
                        if (!std::isfinite(p.x) || !std::isfinite(p.y))
                            throw std::invalid_argument("nonfinite selection polygon");
                    const auto planes = normalize_clipping_planes(request.local_clipping_planes);
                    bool area = false;
                    for (size_t i = 1; i + 1 < request.polygon.size(); ++i) {
                        const auto a = request.polygon[i] - request.polygon[0],
                                   b = request.polygon[i + 1] - request.polygon[0];
                        area = area || std::abs(double(a.x) * b.y - double(a.y) * b.x) > 1e-8;
                    }
                    if (request.points->positions().empty() || !area)
                        return words;
                    const uint64_t sizes[] = {uint64_t(request.points->positions().size()) * sizeof(GpuPoint),
                                              uint64_t(request.polygon.size()) * sizeof(vec2),
                                              uint64_t(words.size()) * sizeof(uint32_t)};
                    for (auto size : sizes)
                        if (size > c_.properties.limits.maxStorageBufferRange)
                            throw std::invalid_argument("selection buffer exceeds device storage-buffer range");
                    if (!c_.compute)
                        throw std::runtime_error("selected graphics queue does not support compute selection");
                    if (!point_selector_) {
                        std::unique_ptr<PointSelector> next(new PointSelector(c_));
                        next->init();
                        point_selector_ = std::move(next);
                    }
                    return point_selector_->run(request, std::move(words), planes,
                                                point_resource(request.points).buffer);
                }
                void init(bool validation,
                          bool sync_validation,
                          bool profiling,
                          bool use_native_large_points,
                          const std::string &device_name,
                          const VulkanSurface *surface = nullptr) {
                    c_.init(validation, sync_validation, profiling, use_native_large_points, device_name, surface);
                    name_ = c_.properties.deviceName;
                    white_ = std::make_shared<Image>(1, 1, std::vector<unsigned char>(4, 255));
                    sample_counts_ = c_.properties.limits.framebufferColorSampleCounts &
                                     c_.properties.limits.framebufferDepthSampleCounts;
                    for (auto format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT}) {
                        VkImageFormatProperties properties;
                        const auto usage = format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                                          : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                        check(c_.vkGetPhysicalDeviceImageFormatProperties(c_.physical, format, VK_IMAGE_TYPE_2D,
                                                                          VK_IMAGE_TILING_OPTIMAL, usage, 0,
                                                                          &properties),
                              "query MSAA target support");
                        sample_counts_ &= properties.sampleCounts;
                    }
                    create_descriptors();
                    draw_buffers_.reset(new DrawBufferCache(c_, impostor_set_));
                    resources_.reset(new GpuResources(c_, descriptor_pool_, descriptors_, samplers_));
                    create_layout();
                    AmbientOcclusionPipelineBuilder(c_, ao_descriptors_, ao_layout_, ao_horizons_, ao_denoise_)
                        .create();
                    color_pass(1);
                }
                void require_samples(unsigned samples) const {
                    if (!samples || samples > 64 || (samples & (samples - 1)))
                        throw std::invalid_argument("sample count must be a power of two from 1 to 64");
                    if (!(sample_counts_ & samples))
                        throw std::invalid_argument("sample count " + std::to_string(samples) +
                                                    " is unsupported by this device");
                }
                RenderPass &color_pass(unsigned samples, bool independent_depth = false, bool depth_preview = false) {
                    return passes_.get_color(c_, samples, independent_depth, depth_preview,
                                             [this](RenderPass &pass) { create_pipelines(pass); });
                }
                RenderPass &picking_pass() {
                    return passes_.get_picking(c_, [this](RenderPass &pass) { create_pipelines(pass); });
                }
                void ensure_ao(uint32_t width, uint32_t height) {
                    if (!ao_) {
                        ao_.reset(new Ao(c_));
                    }
                    ao_->ensure(descriptor_pool_, ao_descriptors_, width, height);
                }
                void create_descriptors() {
                    VkDescriptorSetLayoutBinding binding[3] = {};
                    for (unsigned i = 0; i < 3; ++i) {
                        binding[i].binding = i;
                        binding[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        binding[i].descriptorCount = 1;
                        binding[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                    }
                    VkDescriptorSetLayoutCreateInfo li{};
                    li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    li.bindingCount = 3;
                    li.pBindings = binding;
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &descriptors_),
                          "create descriptor layout");
                    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                    li.bindingCount = 1;
                    li.pBindings = &binding[0];
                    check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &impostor_descriptors_),
                          "create impostor descriptor layout");
                    OitDescriptorLayoutBuilder(c_, oit_scene_descriptors_, oit_composite_descriptors_,
                                               oit_lists_descriptors_)
                        .create();
                    VkDescriptorPoolSize sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8200},
                                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1},
                                                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4},
                                                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8},
                                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6}};
                    VkDescriptorPoolCreateInfo pi{};
                    pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                    pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                    pi.maxSets = 4096;
                    pi.poolSizeCount = 5;
                    pi.pPoolSizes = sizes;
                    check(c_.vkCreateDescriptorPool(c_.device, &pi, nullptr, &descriptor_pool_),
                          "create descriptor pool");
                    VkDescriptorSetAllocateInfo ai{};
                    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ai.descriptorPool = descriptor_pool_;
                    ai.descriptorSetCount = 1;
                    ai.pSetLayouts = &impostor_descriptors_;
                    check(c_.vkAllocateDescriptorSets(c_.device, &ai, &impostor_set_), "allocate impostor descriptor");
                    for (unsigned wrap = 0; wrap < 2; ++wrap)
                        for (unsigned filter = 0; filter < 2; ++filter) {
                            VkSamplerCreateInfo si{};
                            si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                            si.magFilter = si.minFilter = filter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
                            si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                            si.addressModeU = si.addressModeV = si.addressModeW =
                                wrap ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                            si.maxLod = 0;
                            check(c_.vkCreateSampler(c_.device, &si, nullptr, &samplers_[wrap * 2 + filter]),
                                  "create sampler");
                        }
                }
                void create_layout() {
                    VkPushConstantRange push = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Push)};
                    VkPipelineLayoutCreateInfo li{};
                    li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    const VkDescriptorSetLayout sets[] = {descriptors_, impostor_descriptors_};
                    li.setLayoutCount = 2;
                    li.pSetLayouts = sets;
                    li.pushConstantRangeCount = 1;
                    li.pPushConstantRanges = &push;
                    check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &layout_), "create pipeline layout");
                    const VkDescriptorSetLayout peel_sets[] = {oit_scene_descriptors_, impostor_descriptors_};
                    li.pSetLayouts = peel_sets;
                    check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &oit_peel_layout_),
                          "create transparency peel pipeline layout");
                    li.setLayoutCount = 1;
                    li.pSetLayouts = &oit_composite_descriptors_;
                    li.pushConstantRangeCount = 0;
                    li.pPushConstantRanges = nullptr;
                    check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &oit_composite_layout_),
                          "create transparency composite pipeline layout");
                    // The build pass reuses the scene texture/material sets and adds the linked
                    // lists buffers at set 2; the resolve pass reads only the lists buffers.
                    const VkDescriptorSetLayout lists_sets[] = {descriptors_, impostor_descriptors_,
                                                                oit_lists_descriptors_};
                    li.setLayoutCount = 3;
                    li.pSetLayouts = lists_sets;
                    li.pushConstantRangeCount = 1;
                    li.pPushConstantRanges = &push;
                    check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &oit_lists_layout_),
                          "create linked lists build pipeline layout");
                    li.setLayoutCount = 1;
                    li.pSetLayouts = &oit_lists_descriptors_;
                    li.pushConstantRangeCount = 0;
                    li.pPushConstantRanges = nullptr;
                    check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &oit_resolve_layout_),
                          "create linked lists resolve pipeline layout");
                }
                void create_pipelines(RenderPass &pass) {
                    PipelineBuilder(c_, layout_, oit_composite_layout_).create(pass);
                }
                GpuGeometry &geometry(const std::shared_ptr<const Geometry> &data) {
                    return resources_->geometry(data);
                }
                GpuTexture &texture(const std::shared_ptr<const Image> &data) {
                    return resources_->texture(data);
                }
                struct Prepared {
                    DrawCommand command;
                    GpuGeometry *geometry;
                    GpuTexture *texture;
                    uint32_t uniform_offset;
                    float sort_depth;                   // NDC depth of the geometry center; larger is farther
                    uint32_t index_offset, index_count; // transparent far-to-near triangle order
                };
                std::vector<Prepared> prepare(uint32_t width,
                                              uint32_t height,
                                              const std::vector<DrawCommand> &commands,
                                              const vec4 &background,
                                              unsigned samples,
                                              bool pixel_query = false) {
                    require_samples(samples);
                    const auto &limits = c_.properties.limits;
                    if (!width || !height || width > limits.maxFramebufferWidth ||
                        height > limits.maxFramebufferHeight || width > limits.maxImageDimension2D ||
                        height > limits.maxImageDimension2D ||
                        uint64_t(width) * height > (std::numeric_limits<size_t>::max() - 8) / 16)
                        throw std::invalid_argument("invalid offscreen target dimensions");
                    for (unsigned i = 0; i < 4; ++i)
                        if (!std::isfinite(background[i]))
                            throw std::invalid_argument("non-finite background color");
                    DrawCommandValidator(c_).validate(width, height, samples, pixel_query, commands);
                    // The preceding call's fence has completed. Retire expired resources now.
                    clipped_.prune();
                    resources_->prune();
                    if (!pixel_query) {
                        ao_active_ = false;
                        for (const auto &command : commands) {
                            if (!command.ambient_occlusion)
                                continue;
                            ao_active_ = true;
                            ao_projection_ = command.projection;
                            ao_radius_ = command.color.x;
                            ao_bias_ = command.color.y;
                            break;
                        }
                        if (ao_active_)
                            ensure_ao(width, height);
                        FrameTargetCoordinator(c_, passes_, target_, shadow_map_, shadow_bound_size_, descriptor_pool_,
                                               descriptors_, samplers_, layout_)
                            .prepare(width, height, samples, commands, ao_active_,
                                     [this](RenderPass &pass) { create_pipelines(pass); });
                        if (ao_active_) {
                            ao_->update(samplers_[0], target_->depth.view, target_->depth_set, ao_projection_,
                                        ao_radius_, ao_bias_);
                        }
                    }
                    const size_t alignment = std::max(size_t(16), size_t(limits.minUniformBufferOffsetAlignment));
                    const size_t stride = (sizeof(ImpostorUniform) + alignment - 1) / alignment * alignment;
                    std::vector<unsigned char> uniforms;
                    std::vector<Prepared> draws;
                    std::vector<uint32_t> transparent_indices;
                    for (const auto &original : commands) {
                        if (pixel_query && original.overlay)
                            continue;
                        auto command = clipped_.apply(original);
                        auto &g = geometry(command.geometry);
                        if (!g.count)
                            continue;
                        auto &t = texture(command.texture ? command.texture : white_);
                        uint32_t offset = 0;
                        { // Per-draw uniform state is also used by material-lit surfaces.
                            if (uniforms.size() > UINT32_MAX - stride)
                                throw std::invalid_argument("too many impostor draw commands");
                            offset = static_cast<uint32_t>(uniforms.size());
                            const auto uniform = ImpostorUniformBuilder::build(command, width, height);
                            uniforms.resize(uniforms.size() + stride);
                            std::memcpy(uniforms.data() + offset, &uniform, sizeof(uniform));
                        }
                        float sort_depth = 0;
                        uint32_t index_offset = 0, index_count = 0;
                        if (command.transparent) {
                            const auto center = command.model_view_projection * vec4(g.center, 1);
                            sort_depth = 0.5f * (center.z + center.w);
                            // Straight-alpha blending needs every triangle of a drawable in
                            // far-to-near order, not only the drawables themselves.
                            if (!pixel_query) {
                                const auto range = TransparentTriangleOrderBuilder::append(command, transparent_indices);
                                index_offset = range.offset;
                                index_count = range.count;
                            }
                        }
                        draws.push_back({std::move(command), &g, &t, offset, sort_depth, index_offset, index_count});
                    }
                    draw_buffers_->upload(uniforms, transparent_indices);
                    return draws;
                }
                void encode_pass(VkCommandBuffer cmd,
                                 const std::vector<Prepared> &draws,
                                 const vec4 &background,
                                 const Target &target,
                                 const RenderPass &pass,
                                 VkFramebuffer framebuffer,
                                 const VkOffset2D *pixel = nullptr) {
                    ScenePassEncoder encoder(
                        c_, layout_, impostor_set_, oit_composite_layout_, oit_.get(), oit_active_, oit_method_,
                        draw_buffers_->transparent_index_buffer());
                    encoder.encode(cmd, draws, background, target, pass, framebuffer, pixel);
                }
                // Order-independent transparency targets are created on first use and
                // recreated with the scene target. Both methods stay single sampled.
                class OitDescriptorCache {
                    Context &c_;
                    std::unique_ptr<Oit> &oit_;
                    VkDescriptorPool descriptor_pool_;
                    VkDescriptorSetLayout oit_scene_descriptors_;
                    VkSampler *samplers_;

                public:
                    OitDescriptorCache(Context &context,
                                       std::unique_ptr<Oit> &oit,
                                       VkDescriptorPool descriptor_pool,
                                       VkDescriptorSetLayout scene_descriptors,
                                       VkSampler *samplers)
                        : c_(context), oit_(oit), descriptor_pool_(descriptor_pool),
                          oit_scene_descriptors_(scene_descriptors), samplers_(samplers) {}

                    void update_composite(TransparencyMethod method) {
                        VkDescriptorImageInfo images[2] = {
                            {samplers_[0], oit_->color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                            {samplers_[0], method == TransparencyMethod::average ? oit_->count.view : oit_->color.view,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
                        VkWriteDescriptorSet writes[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            writes[i].dstSet = oit_->composite_set;
                            writes[i].dstBinding = i;
                            writes[i].descriptorCount = 1;
                            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            writes[i].pImageInfo = &images[i];
                        }
                        c_.vkUpdateDescriptorSets(c_.device, 2, writes, 0, nullptr);
                    }

                    VkDescriptorSet peel_set(const Prepared &draw, unsigned previous) {
                        const auto key = std::make_pair(draw.texture, previous);
                        const auto found = oit_->peel_sets.find(key);
                        if (found != oit_->peel_sets.end())
                            return found->second;
                        VkDescriptorSet set = VK_NULL_HANDLE;
                        VkDescriptorSetAllocateInfo allocation{};
                        allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                        allocation.descriptorPool = descriptor_pool_;
                        allocation.descriptorSetCount = 1;
                        allocation.pSetLayouts = &oit_scene_descriptors_;
                        check(c_.vkAllocateDescriptorSets(c_.device, &allocation, &set),
                              "allocate transparency peel descriptors");
                        unsigned sampler_index = 0;
                        if (auto source = draw.texture->source.lock())
                            sampler_index = (source->sampler().wrap == TextureWrap::repeat ? 2 : 0) +
                                            (source->sampler().filter == TextureFilter::linear ? 1 : 0);
                        const VkImageView previous_view =
                            previous == 2 ? oit_->start_depth.view : oit_->depth(previous).view;
                        VkDescriptorImageInfo images[2] = {
                            {samplers_[sampler_index], draw.texture->image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                            {samplers_[0], previous_view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL}};
                        VkWriteDescriptorSet writes[2] = {};
                        for (unsigned i = 0; i < 2; ++i) {
                            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            writes[i].dstSet = set;
                            writes[i].dstBinding = i == 0 ? 0 : 3;
                            writes[i].descriptorCount = 1;
                            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                            writes[i].pImageInfo = &images[i];
                        }
                        c_.vkUpdateDescriptorSets(c_.device, 2, writes, 0, nullptr);
                        oit_->peel_sets.emplace(key, set);
                        return set;
                    }
                };

                void ensure_oit(uint32_t width, uint32_t height) {
                    OitResourcesBuilder(c_, oit_, descriptor_pool_, layout_, oit_peel_layout_,
                                        oit_composite_layout_, oit_composite_descriptors_, samplers_[0])
                        .ensure(width, height);
                }
                // Linked lists OIT resources are created on first use and recreated when the
                // viewport or the per-pixel fragment capacity changes. The resolve pipeline
                // renders into the shared Oit accumulation image (reset_pass), so the scene
                // composite over-blends it exactly like a peeled result.
                void ensure_linked_lists(uint32_t width, uint32_t height, unsigned max_fragments) {
                    LinkedListsBuilder(c_, lists_, oit_, descriptor_pool_, oit_lists_descriptors_,
                                       oit_lists_layout_, oit_resolve_layout_)
                        .ensure(width, height, max_fragments);
                }
                void update_oit_composite(TransparencyMethod method) {
                    OitDescriptorCache(c_, oit_, descriptor_pool_, oit_scene_descriptors_, samplers_)
                        .update_composite(method);
                }
                VkDescriptorSet oit_peel_set(const Prepared &draw, unsigned previous) {
                    return OitDescriptorCache(c_, oit_, descriptor_pool_, oit_scene_descriptors_, samplers_)
                        .peel_set(draw, previous);
                }

                class OitPassEncoder {
                    VulkanDevice &device_;
                    Context &c_;
                    std::unique_ptr<Oit> &oit_;
                    std::unique_ptr<LinkedLists> &lists_;
                    std::unique_ptr<Target> &target_;
                    VkPipelineLayout &layout_;
                    VkDescriptorSet &impostor_set_;
                    VkPipelineLayout &oit_peel_layout_;
                    VkPipelineLayout &oit_composite_layout_;
                    VkPipelineLayout &oit_lists_layout_;
                    VkPipelineLayout &oit_resolve_layout_;
                    TransparencyMethod &oit_method_;
                    unsigned &oit_layers_;

                public:
                    explicit OitPassEncoder(VulkanDevice &device)
                        : device_(device), c_(device.c_), oit_(device.oit_), lists_(device.lists_),
                          target_(device.target_), layout_(device.layout_), impostor_set_(device.impostor_set_),
                          oit_peel_layout_(device.oit_peel_layout_),
                          oit_composite_layout_(device.oit_composite_layout_),
                          oit_lists_layout_(device.oit_lists_layout_), oit_resolve_layout_(device.oit_resolve_layout_),
                          oit_method_(device.oit_method_), oit_layers_(device.oit_layers_) {}

                    void encode(VkCommandBuffer cmd, const std::vector<Prepared> &draws) {
                        std::vector<const Prepared *> transparent;
                        for (const auto &draw : draws)
                            if (draw.command.transparent)
                                transparent.push_back(&draw);
                        if (transparent.empty())
                            return;
                        device_.ensure_oit(target_->width, target_->height);
                        const uint32_t width = target_->width, height = target_->height;
                        const auto full_view = [&] {
                            VkViewport viewport = {0, 0, float(width), float(height), 0, 1};
                            const VkRect2D scissor = {{0, 0}, {width, height}};
                            c_.vkCmdSetViewport(cmd, 0, 1, &viewport);
                            c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
                        };
                        const auto set_view = [&](const DrawCommand &d) {
                            const auto &r = d.viewport;
                            VkViewport viewport = {
                                float(r.x), float(r.y), float(r.z ? r.z : width), float(r.w ? r.w : height), 0, 1};
                            VkRect2D scissor = {
                                {r.x, r.y}, {uint32_t(r.z ? r.z : width), uint32_t(r.w ? r.w : height)}};
                            c_.vkCmdSetViewport(cmd, 0, 1, &viewport);
                            c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
                        };
                        const auto bind_draw = [&](const Prepared &draw,
                                                   VkPipeline pipeline,
                                                   VkPipelineLayout pipeline_layout,
                                                   VkDescriptorSet texture_set) {
                            set_view(draw.command);
                            c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                            c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                            c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                                       &texture_set, 0, nullptr);
                            c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1,
                                                       &impostor_set_, 1, &draw.uniform_offset);
                            VkDeviceSize offset = 0;
                            c_.vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
                            Push push{};
                            for (unsigned col = 0; col < 4; ++col)
                                for (unsigned row = 0; row < 4; ++row)
                                    push.mvp[col * 4 + row] = draw.command.model_view_projection(row, col);
                            for (unsigned i = 0; i < 4; ++i)
                                push.color[i] = draw.command.color[i];
                            push.object = draw.command.object_id;
                            push.lighting = draw.command.lighting ? 1 : 0;
                            for (unsigned col = 0; col < 3; ++col)
                                for (unsigned row = 0; row < 3; ++row)
                                    push.normal[col * 3 + row] = draw.command.normal_matrix(row, col);
                            c_.vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push),
                                                  &push);
                        };
                        if (oit_method_ == TransparencyMethod::average) {
                            for (auto *image : {&oit_->color, &oit_->count}) {
                                auto &layout = image == &oit_->color ? oit_->color_layout : oit_->count_layout;
                                const bool fresh = layout == VK_IMAGE_LAYOUT_UNDEFINED;
                                image_barrier(c_, cmd, image->handle, VK_IMAGE_ASPECT_COLOR_BIT, layout,
                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              fresh ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                    : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              fresh ? 0 : VK_ACCESS_SHADER_READ_BIT,
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
                                layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            }
                            VkClearValue clears[2] = {};
                            VkRenderPassBeginInfo begin{};
                            begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                            begin.renderPass = oit_->average_pass;
                            begin.framebuffer = oit_->average_framebuffer;
                            begin.renderArea = {{0, 0}, {width, height}};
                            begin.clearValueCount = 2;
                            begin.pClearValues = clears;
                            c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
                            full_view();
                            for (const Prepared *draw : transparent) {
                                bind_draw(*draw, oit_->average_pipeline, layout_, draw->texture->set);
                                c_.vkCmdDraw(cmd, draw->geometry->count, 1, 0, 0);
                            }
                            c_.vkCmdEndRenderPass(cmd);
                            for (auto *image : {&oit_->color, &oit_->count}) {
                                auto &layout = image == &oit_->color ? oit_->color_layout : oit_->count_layout;
                                image_barrier(c_, cmd, image->handle, VK_IMAGE_ASPECT_COLOR_BIT,
                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                                layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            }
                        } else if (oit_method_ == TransparencyMethod::linked_lists) {
                            device_.ensure_linked_lists(width, height, oit_layers_);
                            // Reset the head pointers to -1 (empty list) and the node counter to 0.
                            VkMemoryBarrier clear_barrier{};
                            clear_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                            clear_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                            clear_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &clear_barrier, 0, nullptr, 0,
                                                    nullptr);
                            c_.vkCmdFillBuffer(cmd, lists_->head.handle, 0, VkDeviceSize(width) * height * 4,
                                               0xFFFFFFFFu);
                            c_.vkCmdFillBuffer(cmd, lists_->counter.handle, 0, 4, 0u);
                            // Make the clears (transfer) and the host-written params visible to the
                            // build fragment shader before it appends nodes.
                            VkMemoryBarrier build_barrier{};
                            build_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                            build_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT;
                            build_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
                                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &build_barrier, 0,
                                                    nullptr, 0, nullptr);
                            VkRenderPassBeginInfo build{};
                            build.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                            build.renderPass = lists_->build_pass;
                            build.framebuffer = lists_->build_framebuffer;
                            build.renderArea = {{0, 0}, {width, height}};
                            c_.vkCmdBeginRenderPass(cmd, &build, VK_SUBPASS_CONTENTS_INLINE);
                            full_view();
                            for (const Prepared *draw : transparent) {
                                bind_draw(*draw, lists_->build_pipeline, oit_lists_layout_, draw->texture->set);
                                c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, oit_lists_layout_, 2,
                                                           1, &lists_->set, 0, nullptr);
                                c_.vkCmdDraw(cmd, draw->geometry->count, 1, 0, 0);
                            }
                            c_.vkCmdEndRenderPass(cmd);
                            // Copy the allocation counter to host-visible memory so the host can
                            // report pool overflow. It counts every append attempt, including
                            // fragments dropped once the pool was full.
                            VkMemoryBarrier counter_barrier{};
                            counter_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                            counter_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                            counter_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                    VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &counter_barrier, 0, nullptr,
                                                    0, nullptr);
                            VkBufferCopy counter_copy{};
                            counter_copy.size = 4;
                            c_.vkCmdCopyBuffer(cmd, lists_->counter.handle, lists_->counter_readback.handle, 1,
                                               &counter_copy);
                            // Order the linked-list writes before the resolve pass reads them.
                            VkMemoryBarrier resolve_barrier{};
                            resolve_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                            resolve_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                            resolve_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &resolve_barrier, 0,
                                                    nullptr, 0, nullptr);
                            // Resolve into the shared accumulation image, then expose it to the
                            // scene composite (which over-blends it like a peeled result).
                            const bool fresh = oit_->color_layout == VK_IMAGE_LAYOUT_UNDEFINED;
                            image_barrier(c_, cmd, oit_->color.handle, VK_IMAGE_ASPECT_COLOR_BIT, oit_->color_layout,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          fresh ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          fresh ? 0 : VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
                            oit_->color_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            VkClearValue clear{};
                            VkRenderPassBeginInfo resolve{};
                            resolve.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                            resolve.renderPass = oit_->reset_pass;
                            resolve.framebuffer = oit_->reset_framebuffer;
                            resolve.renderArea = {{0, 0}, {width, height}};
                            resolve.clearValueCount = 1;
                            resolve.pClearValues = &clear;
                            c_.vkCmdBeginRenderPass(cmd, &resolve, VK_SUBPASS_CONTENTS_INLINE);
                            full_view();
                            c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                            c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lists_->resolve_pipeline);
                            c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, oit_resolve_layout_, 0, 1,
                                                       &lists_->set, 0, nullptr);
                            c_.vkCmdDraw(cmd, 3, 1, 0, 0);
                            c_.vkCmdEndRenderPass(cmd);
                            image_barrier(c_, cmd, oit_->color.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                            oit_->color_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        } else {
                            const bool fresh = oit_->color_layout == VK_IMAGE_LAYOUT_UNDEFINED;
                            image_barrier(c_, cmd, oit_->color.handle, VK_IMAGE_ASPECT_COLOR_BIT, oit_->color_layout,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          fresh ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          fresh ? 0 : VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
                            oit_->color_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            // Zero the accumulation; every layer pass loads it back.
                            VkClearValue clear{};
                            VkRenderPassBeginInfo begin{};
                            begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                            begin.renderPass = oit_->reset_pass;
                            begin.framebuffer = oit_->reset_framebuffer;
                            begin.renderArea = {{0, 0}, {width, height}};
                            begin.clearValueCount = 1;
                            begin.pClearValues = &clear;
                            c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
                            c_.vkCmdEndRenderPass(cmd);
                            if (!oit_->start_ready) {
                                // Zero is the sentinel "nothing peeled yet" depth.
                                VkClearValue start_clear{};
                                VkRenderPassBeginInfo start{};
                                start.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                                start.renderPass = oit_->start_pass;
                                start.framebuffer = oit_->start_framebuffer;
                                start.renderArea = {{0, 0}, {1, 1}};
                                start.clearValueCount = 1;
                                start.pClearValues = &start_clear;
                                c_.vkCmdBeginRenderPass(cmd, &start, VK_SUBPASS_CONTENTS_INLINE);
                                c_.vkCmdEndRenderPass(cmd);
                                image_barrier(c_, cmd, oit_->start_depth.handle, VK_IMAGE_ASPECT_DEPTH_BIT,
                                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                                oit_->start_ready = true;
                            }
                            for (unsigned layer = 0; layer < oit_layers_; ++layer) {
                                const unsigned target_index = layer & 1u;
                                const unsigned previous = layer == 0 ? 2u : 1u - target_index;
                                auto &depth_image = oit_->depth(target_index);
                                auto &depth_layout = oit_->depth_layout[target_index];
                                const bool fresh_depth = depth_layout == VK_IMAGE_LAYOUT_UNDEFINED;
                                image_barrier(
                                    c_, cmd, depth_image.handle, VK_IMAGE_ASPECT_DEPTH_BIT, depth_layout,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    fresh_depth ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                    fresh_depth ? 0 : VK_ACCESS_SHADER_READ_BIT,
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
                                depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                                // The layer target is replaced every pass, so its previous
                                // contents (last read by the layer blend) are discarded.
                                const bool fresh_layer = oit_->layer_layout == VK_IMAGE_LAYOUT_UNDEFINED;
                                image_barrier(
                                    c_, cmd, oit_->layer.handle, VK_IMAGE_ASPECT_COLOR_BIT, oit_->layer_layout,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    fresh_layer ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    fresh_layer ? 0 : VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
                                oit_->layer_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                                VkClearValue clears[2] = {};
                                clears[1].depthStencil.depth = 1;
                                VkRenderPassBeginInfo begin{};
                                begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                                begin.renderPass = oit_->layer_pass;
                                begin.framebuffer = oit_->layer_framebuffers[target_index];
                                begin.renderArea = {{0, 0}, {width, height}};
                                begin.clearValueCount = 2;
                                begin.pClearValues = clears;
                                c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
                                full_view();
                                for (const Prepared *draw : transparent) {
                                    bind_draw(*draw, oit_->layer_pipeline, oit_peel_layout_,
                                              device_.oit_peel_set(*draw, previous));
                                    c_.vkCmdDraw(cmd, draw->geometry->count, 1, 0, 0);
                                }
                                c_.vkCmdEndRenderPass(cmd);
                                image_barrier(
                                    c_, cmd, depth_image.handle, VK_IMAGE_ASPECT_DEPTH_BIT,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_SHADER_READ_BIT);
                                depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                                image_barrier(
                                    c_, cmd, oit_->layer.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                                oit_->layer_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                // Blend the resolved layer under the running accumulation.
                                VkRenderPassBeginInfo blend{};
                                blend.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                                blend.renderPass = oit_->accumulate_pass;
                                blend.framebuffer = oit_->accumulate_framebuffer;
                                blend.renderArea = {{0, 0}, {width, height}};
                                c_.vkCmdBeginRenderPass(cmd, &blend, VK_SUBPASS_CONTENTS_INLINE);
                                full_view();
                                c_.vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
                                c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, oit_->accumulate_pipeline);
                                c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, oit_composite_layout_,
                                                           0, 1, &oit_->layer_set, 0, nullptr);
                                c_.vkCmdDraw(cmd, 3, 1, 0, 0);
                                c_.vkCmdEndRenderPass(cmd);
                            }
                            image_barrier(c_, cmd, oit_->color.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
                            oit_->color_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        }
                        device_.update_oit_composite(oit_method_);
                    }
                };

                void encode_oit(VkCommandBuffer cmd, const std::vector<Prepared> &draws) {
                    OitPassEncoder(*this).encode(cmd, draws);
                }
                void encode_shadow(VkCommandBuffer cmd, const std::vector<Prepared> &draws) {
                    ShadowPassEncoder(c_, layout_, shadow_map_.get()).encode(cmd, draws);
                }
                void dispatch_ao(VkCommandBuffer cmd) {
                    AmbientOcclusionPassEncoder(c_, *ao_, ao_layout_, ao_horizons_, ao_denoise_, *target_)
                        .dispatch(cmd);
                }
                class OitFrameCoordinator {
                    VulkanDevice &device_;

                public:
                    explicit OitFrameCoordinator(VulkanDevice &device) : device_(device) {}

                    void encode(VkCommandBuffer cmd, const std::vector<Prepared> &draws) {
                        device_.oit_dropped_ = 0;
                        // Accumulate or peel transparent surfaces before the scene pass composites them.
                        device_.oit_active_ = device_.oit_method_ != TransparencyMethod::sorted &&
                                              std::any_of(draws.begin(), draws.end(),
                                                          [](const Prepared &draw) { return draw.command.transparent; });
                        if (device_.oit_active_)
                            device_.encode_oit(cmd, draws);
                    }

                    void capture_overflow() {
                        // The submission fence has already waited, so the counter copy is complete.
                        if (device_.oit_active_ && device_.oit_method_ == TransparencyMethod::linked_lists &&
                            device_.lists_) {
                            uint32_t attempts = 0;
                            device_.lists_->counter_readback.transfer(&attempts, sizeof(attempts), false);
                            device_.oit_dropped_ =
                                attempts > device_.lists_->node_count ? attempts - device_.lists_->node_count : 0;
                        }
                    }
                };
                class FrameEncoder {
                    VulkanDevice &device_;

                public:
                    explicit FrameEncoder(VulkanDevice &device) : device_(device) {}

                    void encode(VkCommandBuffer cmd,
                                const std::vector<Prepared> &draws,
                                const vec4 &background,
                                bool capture) {
                        device_.encode_shadow(cmd, draws);
                        OitFrameCoordinator(device_).encode(cmd, draws);
                        // Ambient occlusion and multisampled previews both read the center-sample
                        // depth, which only the single-sample picking pass produces.
                        const bool prepass = device_.target_->id_pass &&
                                             (device_.ao_active_ ||
                                              (device_.target_->color_pass.depth_preview &&
                                               device_.target_->samples > 1));
                        if (prepass) {
                            device_.encode_pass(cmd, draws, background, *device_.target_,
                                                *device_.target_->id_pass, device_.target_->id_framebuffer);
                            image_barrier(
                                device_.c_, cmd, device_.target_->depth.handle, VK_IMAGE_ASPECT_DEPTH_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                device_.ao_active_ ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                   : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                device_.ao_active_ ? (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
                                                   : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
                                VK_ACCESS_SHADER_READ_BIT);
                        }
                        if (device_.ao_active_)
                            device_.dispatch_ao(cmd);
                        device_.encode_pass(cmd, draws, background, *device_.target_,
                                            device_.target_->color_pass, device_.target_->framebuffer);
                        // Picking uses center-sample IDs/depth independently of antialiased color.
                        // This works even when an integer format supports no multisampling, and
                        // requires neither depth-resolve extensions nor averaging integer IDs.
                        if (capture && device_.target_->id_pass && !prepass)
                            device_.encode_pass(cmd, draws, background, *device_.target_,
                                                *device_.target_->id_pass, device_.target_->id_framebuffer);
                        if (prepass && device_.target_->samples > 1)
                            image_barrier(
                                device_.c_, cmd, device_.target_->depth.handle, VK_IMAGE_ASPECT_DEPTH_BIT,
                                device_.ao_active_ ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                   : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                VK_ACCESS_TRANSFER_READ_BIT);
                    }
                };
                void encode(VkCommandBuffer cmd,
                            const std::vector<Prepared> &draws,
                            const vec4 &background,
                            bool capture = true) {
                    FrameEncoder(*this).encode(cmd, draws, background, capture);
                }
                FrameImage render(uint32_t width,
                                  uint32_t height,
                                  const std::vector<DrawCommand> &commands,
                                  const vec4 &background,
                                  unsigned samples,
                                  TransparencyMethod transparency,
                                  unsigned peel_layers) override {
                    oit_method_ = transparency;
                    oit_layers_ = peel_layers ? peel_layers : 16u;
                    const auto draws = prepare(width, height, commands, background, samples);
                    // Presentation never allocates host readback storage. Allocate only for
                    // an explicitly requested capture, including eight-byte ID alignment.
                    auto result = capture_frame(c_, *target_, [&](VkCommandBuffer cmd) {
                        encode(cmd, draws, background);
                    });
                    OitFrameCoordinator(*this).capture_overflow();
                    return result;
                }
                ScenePixel sample_pixel(uint32_t width,
                                        uint32_t height,
                                        const std::vector<DrawCommand> &commands,
                                        uint32_t x,
                                        uint32_t y) override {
                    if (x >= width || y >= height)
                        throw std::out_of_range("scene sample pixel is outside the target");
                    const vec4 background(0);
                    const auto draws = prepare(width, height, commands, background, 1, true);
                    if (draws.empty())
                        return {};
                    auto &pass = picking_pass();
                    if (!pixel_target_ || pixel_target_->width != width || pixel_target_->height != height) {
                        std::unique_ptr<Target> target(new Target(c_, width, height, pass, nullptr));
                        target->init();
                        target->readback.init(12, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
                        pixel_target_ = std::move(target);
                    }
                    auto &target = *pixel_target_;
                    return capture_pixel(c_, target, x, y, [&](VkCommandBuffer cmd, const VkOffset2D &pixel) {
                        encode_pass(cmd, draws, background, target, pass, target.framebuffer, &pixel);
                    });
                }
                bool present(uint32_t width,
                             uint32_t height,
                             const std::vector<DrawCommand> &commands,
                             const vec4 &background,
                             unsigned samples,
                             TransparencyMethod transparency,
                             unsigned peel_layers) override {
                    oit_method_ = transparency;
                    oit_layers_ = peel_layers ? peel_layers : 16u;
                    if (!c_.surface)
                        throw std::logic_error("headless Vulkan device has no presentation surface");
                    if (!width || !height)
                        return false;
                    if (!presentation_)
                        presentation_.reset(new Presentation(c_));
                    if (!presentation_->prepare(width, height))
                        return false;
                    const auto draws = prepare(width, height, commands, background, samples);
                    return presentation_->present(
                        width, height, target_->color.handle,
                        [&](VkCommandBuffer cmd) { encode(cmd, draws, background, false); },
                        [&] { OitFrameCoordinator(*this).capture_overflow(); });
                }
            };
        } // namespace
        std::unique_ptr<RenderDevice>
        create_vulkan_device(bool validation, bool sync_validation, bool profiling,
                             bool use_native_large_points, const std::string &device_name) {
            std::unique_ptr<VulkanDevice> device(new VulkanDevice);
            device->init(validation, sync_validation, profiling, use_native_large_points, device_name);
            return std::unique_ptr<RenderDevice>(device.release());
        }
        std::unique_ptr<RenderDevice> create_vulkan_window_device(const VulkanSurface &surface,
                                                                  bool validation,
                                                                  bool sync_validation,
                                                                  bool profiling,
                                                                  bool use_native_large_points,
                                                                  const std::string &device_name) {
            std::unique_ptr<VulkanDevice> device(new VulkanDevice);
            device->init(validation, sync_validation, profiling, use_native_large_points, device_name, &surface);
            return std::unique_ptr<RenderDevice>(device.release());
        }

    } // namespace graphics
} // namespace easy3d
