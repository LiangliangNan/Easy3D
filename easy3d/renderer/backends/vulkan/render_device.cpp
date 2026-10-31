// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Liangliang Nan
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/backends/clipping.h>
#include <easy3d/renderer/backends/vulkan/loader.h>
#include <easy3d/renderer/backends/vulkan/surface.h>
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
#include "points_vert.inc"
#include "points_frag.inc"
#include "lines_vert.inc"
#include "lines_frag.inc"
#include "wide_lines_vert.inc"
#include "wide_lines_frag.inc"
#include "overlay_vert.inc"
#include "overlay_frag.inc"
#include "depth_texture_frag.inc"
#include "point_selection_comp.inc"

void check(VkResult result, const char* operation) {
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string(operation) + " failed (VkResult " + std::to_string(result) + ")");
}

void check_initialization(VkResult result,const char* operation) {
    if(result==VK_ERROR_INCOMPATIBLE_DRIVER || result==VK_ERROR_EXTENSION_NOT_PRESENT ||
       result==VK_ERROR_LAYER_NOT_PRESENT || result==VK_ERROR_FEATURE_NOT_PRESENT || result==VK_ERROR_INITIALIZATION_FAILED)
        throw std::runtime_error(std::string(operation)+" failed (VkResult "+std::to_string(result)+")");
    check(result,operation);
}

template<class T> bool named(const std::vector<T>& values, const char* name) {
    for (const auto& v : values) if (std::strcmp(v.extensionName, name) == 0) return true;
    return false;
}
std::vector<VkExtensionProperties> instance_extensions(const internal::VulkanDispatch& dispatch,const char* layer = nullptr) {
    uint32_t count = 0;
    check_initialization(dispatch.vkEnumerateInstanceExtensionProperties(layer, &count, nullptr), "enumerate instance extensions");
    std::vector<VkExtensionProperties> values(count);
    check_initialization(dispatch.vkEnumerateInstanceExtensionProperties(layer, &count, values.data()), "enumerate instance extensions");
    return values;
}

// No mutable global device dispatch or resources: loader dispatch is handle based.
struct Context : internal::VulkanDispatch {
    // Retain the library until all instance/device cleanup has finished.
    std::shared_ptr<internal::VulkanLoader> library;
    bool sample_shading = false;
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
        // A malformed loader can fail before instance dispatch is complete.
        if (instance && vkDestroyInstance) vkDestroyInstance(instance, nullptr);
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
        auto* self = static_cast<Context*>(user);
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ++self->errors;
        std::cerr << "Vulkan validation: " << data->pMessage << '\n';
        return VK_FALSE;
    }
    void init(const DeviceOptions& options, const VulkanSurface* window = nullptr) {
        library=window && window->loader ? window->loader : internal::VulkanLoader::open();
        load_global(*library);
        const bool validation = options.validation || options.synchronization_validation;
        auto available = instance_extensions(*this);
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
            check_initialization(vkEnumerateInstanceLayerProperties(&count, nullptr), "enumerate validation layers");
            std::vector<VkLayerProperties> values(count);
            check_initialization(vkEnumerateInstanceLayerProperties(&count, values.data()), "enumerate validation layers");
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
            const auto layer_extensions = instance_extensions(*this,"VK_LAYER_KHRONOS_validation");
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
        check_initialization(vkCreateInstance(&info, nullptr, &instance), "create Vulkan 1.2 instance");
        load_instance(instance,window!=nullptr);
        if (window) {
            check_initialization(window->create(instance, &surface), "create native Vulkan surface");
            if (!surface) throw std::runtime_error("surface callback returned a null surface");
        }
        if (validation) {
            auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            if (!create) throw std::runtime_error("debug messenger entry point unavailable");
            debug_info.pNext = nullptr;
            check_initialization(create(instance, &debug_info, nullptr, &messenger), "create debug messenger");
        }
        uint32_t count = 0;
        check_initialization(vkEnumeratePhysicalDevices(instance, &count, nullptr), "enumerate Vulkan devices");
        std::vector<VkPhysicalDevice> devices(count);
        check_initialization(vkEnumeratePhysicalDevices(instance, &count, devices.data()), "enumerate Vulkan devices");
        int best_score = -1;
        for (auto candidate : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(candidate, &props);
            if (props.apiVersion < VK_API_VERSION_1_2 ||
                std::string(props.deviceName).find(options.device_name) == std::string::npos) continue;
            const std::pair<VkFormat, VkFormatFeatureFlags> formats[] = {
                {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT |
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
                check_initialization(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extension_count, nullptr), "enumerate WSI extensions");
                std::vector<VkExtensionProperties> supported(extension_count);
                check_initialization(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extension_count, supported.data()), "enumerate WSI extensions");
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
                    check_initialization(vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &supported), "query presentation queue");
                    if (!supported) continue;
                }
                int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 3 :
                            props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2 : 1;
                score=score*2+bool(queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT);
                if (score > best_score) {
                    best_score = score; physical = candidate; family = i; properties = props;
                    compute=(queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT)!=0;
                }
            }
        }
        if (!physical) throw std::runtime_error("no suitable Vulkan 1.2 device/graphics queue for requested targets and surface");
        vkGetPhysicalDeviceMemoryProperties(physical, &memory);
        count = 0;
        check_initialization(vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr), "enumerate device extensions");
        std::vector<VkExtensionProperties> device_extensions(count);
        check_initialization(vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, device_extensions.data()), "enumerate device extensions");
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
        VkPhysicalDeviceFeatures features{}, enabled{};
        vkGetPhysicalDeviceFeatures(physical, &features);
        sample_shading = features.sampleRateShading;
        enabled.sampleRateShading = features.sampleRateShading;
        di.pEnabledFeatures = &enabled;
        // No geometry shaders, large points, wide lines or independent blend.
        check_initialization(vkCreateDevice(physical, &di, nullptr, &device), "create device");
        load_device(device,window!=nullptr);
        vkGetDeviceQueue(device, family, 0, &queue);
        VkCommandPoolCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.queueFamilyIndex = family;
        pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check_initialization(vkCreateCommandPool(device, &pi, nullptr, &pool), "create command pool");
        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = pool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        check_initialization(vkAllocateCommandBuffers(device, &ai, &command), "allocate command buffer");
        VkFenceCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        check_initialization(vkCreateFence(device, &fi, nullptr, &fence), "create submission fence");
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
        if (handle) c.vkDestroyBuffer(c.device, handle, nullptr);
        if (memory) c.vkFreeMemory(c.device, memory, nullptr);
    }
    void init(VkDeviceSize size, VkBufferUsageFlags usage, bool host) {
        VkBufferCreateInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bi.size = size; bi.usage = usage; bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        check(c.vkCreateBuffer(c.device, &bi, nullptr, &handle), "create buffer");
        VkMemoryRequirements requirements;
        c.vkGetBufferMemoryRequirements(c.device, handle, &requirements);
        uint32_t type = c.memory_type(requirements.memoryTypeBits, host ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0,
            host ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        coherent = (c.memory.memoryTypes[type].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = requirements.size; ai.memoryTypeIndex = type;
        check(c.vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate buffer memory");
        check(c.vkBindBufferMemory(c.device, handle, memory, 0), "bind buffer memory");
    }
    void transfer(void* data, size_t size, bool write) {
        void* mapped = nullptr;
        check(c.vkMapMemory(c.device, memory, 0, VK_WHOLE_SIZE, 0, &mapped), "map host buffer");
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = memory; range.size = VK_WHOLE_SIZE;
        VkResult result = VK_SUCCESS;
        if (write) {
            std::memcpy(mapped, data, size);
            if (!coherent) result = c.vkFlushMappedMemoryRanges(c.device, 1, &range);
        } else {
            if (!coherent) result = c.vkInvalidateMappedMemoryRanges(c.device, 1, &range);
            if (result == VK_SUCCESS) std::memcpy(data, mapped, size);
        }
        c.vkUnmapMemory(c.device, memory);
        check(result, write ? "flush host buffer" : "invalidate readback buffer");
    }
};
struct GpuPoint { float position[3]; uint32_t source; };
static_assert(sizeof(GpuPoint)==16,"point vertex/storage layout");
struct GpuPoints {
    std::weak_ptr<const PointData> source;
    Buffer buffer;
    explicit GpuPoints(Context& c):buffer(c) {}
};
// Separate compute resources: no render pass, framebuffer, depth or picking IDs.
struct PointSelector {
    Context& c;
    VkDescriptorSetLayout descriptors=VK_NULL_HANDLE;
    VkDescriptorPool pool=VK_NULL_HANDLE;
    VkDescriptorSet set=VK_NULL_HANDLE;
    VkPipelineLayout layout=VK_NULL_HANDLE;
    VkShaderModule shader=VK_NULL_HANDLE;
    VkPipeline pipeline=VK_NULL_HANDLE;
    explicit PointSelector(Context& context):c(context) {}
    ~PointSelector() {
        if (pipeline) c.vkDestroyPipeline(c.device,pipeline,nullptr);
        if (shader) c.vkDestroyShaderModule(c.device,shader,nullptr);
        if (layout) c.vkDestroyPipelineLayout(c.device,layout,nullptr);
        if (pool) c.vkDestroyDescriptorPool(c.device,pool,nullptr);
        if (descriptors) c.vkDestroyDescriptorSetLayout(c.device,descriptors,nullptr);
    }
    void init() {
        VkDescriptorSetLayoutBinding bindings[3]={};
        for (unsigned i=0;i<3;++i) {
            bindings[i].binding=i; bindings[i].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            bindings[i].descriptorCount=1; bindings[i].stageFlags=VK_SHADER_STAGE_COMPUTE_BIT;
        }
        VkDescriptorSetLayoutCreateInfo di{}; di.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        di.bindingCount=3; di.pBindings=bindings;
        check(c.vkCreateDescriptorSetLayout(c.device,&di,nullptr,&descriptors),"create selection descriptor layout");
        VkDescriptorPoolSize size={VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,3};
        VkDescriptorPoolCreateInfo pi{}; pi.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pi.maxSets=1; pi.poolSizeCount=1; pi.pPoolSizes=&size;
        check(c.vkCreateDescriptorPool(c.device,&pi,nullptr,&pool),"create selection descriptor pool");
        VkDescriptorSetAllocateInfo ai{}; ai.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool=pool; ai.descriptorSetCount=1; ai.pSetLayouts=&descriptors;
        check(c.vkAllocateDescriptorSets(c.device,&ai,&set),"allocate selection descriptor set");
        VkPushConstantRange range={VK_SHADER_STAGE_COMPUTE_BIT,0,128};
        VkPipelineLayoutCreateInfo li{}; li.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        li.setLayoutCount=1; li.pSetLayouts=&descriptors; li.pushConstantRangeCount=1; li.pPushConstantRanges=&range;
        check(c.vkCreatePipelineLayout(c.device,&li,nullptr,&layout),"create selection pipeline layout");
        VkShaderModuleCreateInfo si{}; si.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        si.codeSize=sizeof(point_selection_comp); si.pCode=point_selection_comp;
        check(c.vkCreateShaderModule(c.device,&si,nullptr,&shader),"create selection compute shader");
        VkComputePipelineCreateInfo ci{}; ci.sType=VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        ci.layout=layout; ci.stage.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stage.stage=VK_SHADER_STAGE_COMPUTE_BIT; ci.stage.module=shader; ci.stage.pName="main";
        check(c.vkCreateComputePipelines(c.device,VK_NULL_HANDLE,1,&ci,nullptr,&pipeline),"create selection compute pipeline");
    }
    std::vector<uint32_t> run(const PointSelectionRequest& request,std::vector<uint32_t> words,
                              const std::vector<vec4>& planes,Buffer& positions) {
        const auto count=request.points->positions().size();
        const VkDeviceSize sizes[]={count*sizeof(GpuPoint),request.polygon.size()*sizeof(vec2),words.size()*sizeof(uint32_t)};
        Buffer polygon(c),selection(c); Buffer* buffers[]={&positions,&polygon,&selection};
        VkDescriptorBufferInfo infos[3]={}; VkWriteDescriptorSet writes[3]={};
        for (unsigned i=0;i<3;++i) {
            if (i) buffers[i]->init(sizes[i],VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,true);
            infos[i]={buffers[i]->handle,0,sizes[i]};
            writes[i].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[i].dstSet=set; writes[i].dstBinding=i;
            writes[i].descriptorCount=1; writes[i].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; writes[i].pBufferInfo=&infos[i];
        }
        polygon.transfer(const_cast<vec2*>(request.polygon.data()),size_t(sizes[1]),true);
        selection.transfer(words.data(),size_t(sizes[2]),true);
        c.vkUpdateDescriptorSets(c.device,3,writes,0,nullptr);
        struct Parameters { float mvp[16],planes[8]; uint32_t dimensions[4],options[4]; } push{};
        static_assert(sizeof(Parameters)==128,"compute selection push layout");
        for (unsigned col=0;col<4;++col) for (unsigned row=0;row<4;++row)
            push.mvp[col*4+row]=request.model_view_projection(row,col);
        for (size_t i=0;i<planes.size();++i) for (unsigned j=0;j<4;++j) push.planes[4*i+j]=planes[i][j];
        push.dimensions[0]=request.width; push.dimensions[1]=request.height;
        push.dimensions[2]=uint32_t(count); push.dimensions[3]=uint32_t(request.polygon.size());
        push.options[0]=request.deselect; push.options[1]=uint32_t(planes.size());
        const uint32_t groups=uint32_t((count+63)/64);
        const uint32_t x=std::min(groups,c.properties.limits.maxComputeWorkGroupCount[0]);
        const uint32_t y=(groups+x-1)/x;
        if (y>c.properties.limits.maxComputeWorkGroupCount[1]) throw std::invalid_argument("selection exceeds compute dispatch limits");
        c.execute([&](VkCommandBuffer cmd) {
            VkMemoryBarrier barrier{}; barrier.sType=VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            barrier.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;
            barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;
            c.vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&barrier,0,nullptr,0,nullptr);
            c.vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline);
            c.vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,layout,0,1,&set,0,nullptr);
            c.vkCmdPushConstants(cmd,layout,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(push),&push);
            c.vkCmdDispatch(cmd,x,y,1);
            barrier.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT; barrier.dstAccessMask=VK_ACCESS_HOST_READ_BIT;
            c.vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&barrier,0,nullptr,0,nullptr);
        });
        selection.transfer(words.data(),size_t(sizes[2]),false);
        return words;
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
        if (view) c.vkDestroyImageView(c.device, view, nullptr);
        if (handle) c.vkDestroyImage(c.device, handle, nullptr);
        if (memory) c.vkFreeMemory(c.device, memory, nullptr);
    }
    void init(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect,
              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
        VkImageCreateInfo ii{};
        ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ii.imageType = VK_IMAGE_TYPE_2D; ii.format = format; ii.extent = {width, height, 1};
        ii.mipLevels = 1; ii.arrayLayers = 1; ii.samples = samples;
        ii.tiling = VK_IMAGE_TILING_OPTIMAL; ii.usage = usage;
        ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE; ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        check(c.vkCreateImage(c.device, &ii, nullptr, &handle), "create image");
        VkMemoryRequirements requirements;
        c.vkGetImageMemoryRequirements(c.device, handle, &requirements);
        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = requirements.size;
        ai.memoryTypeIndex = c.memory_type(requirements.memoryTypeBits, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        check(c.vkAllocateMemory(c.device, &ai, nullptr, &memory), "allocate image memory");
        check(c.vkBindImageMemory(c.device, handle, memory, 0), "bind image memory");
        VkImageViewCreateInfo vi{};
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = handle; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = format;
        vi.subresourceRange = {aspect, 0, 1, 0, 1};
        check(c.vkCreateImageView(c.device, &vi, nullptr, &view), "create image view");
    }
};
void image_barrier(const Context& c,VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect,
                   VkImageLayout old_layout, VkImageLayout new_layout,
                   VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                   VkAccessFlags src_access, VkAccessFlags dst_access) {
    VkImageMemoryBarrier b{};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    b.srcAccessMask = src_access; b.dstAccessMask = dst_access;
    b.oldLayout = old_layout; b.newLayout = new_layout;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image; b.subresourceRange = {aspect, 0, 1, 0, 1};
    c.vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &b);
}
enum class PassKind { color_ids, multisample_color, ids };
struct RenderPass {
    Context& c;
    PassKind kind;
    VkSampleCountFlagBits samples;
    VkRenderPass handle = VK_NULL_HANDLE;
    VkPipeline pipelines[6] = {};
    VkPipeline overlay_pipeline = VK_NULL_HANDLE;
    VkPipeline depth_overlay_pipeline = VK_NULL_HANDLE;
    VkPipeline hud_3d_pipeline = VK_NULL_HANDLE;
    VkPipeline depth_texture_pipeline = VK_NULL_HANDLE;
    bool independent_depth = false;
    bool depth_preview = false;
    RenderPass(Context& context, PassKind mode, VkSampleCountFlagBits count) : c(context), kind(mode), samples(count) {}
    ~RenderPass() {
        for (auto p : pipelines) if (p) c.vkDestroyPipeline(c.device, p, nullptr);
        if (overlay_pipeline) c.vkDestroyPipeline(c.device, overlay_pipeline, nullptr);
        if (depth_overlay_pipeline) c.vkDestroyPipeline(c.device, depth_overlay_pipeline, nullptr);
        if (hud_3d_pipeline) c.vkDestroyPipeline(c.device, hud_3d_pipeline, nullptr);
        if (depth_texture_pipeline) c.vkDestroyPipeline(c.device, depth_texture_pipeline, nullptr);
        if (handle) c.vkDestroyRenderPass(c.device, handle, nullptr);
    }
    void init() {
        VkAttachmentDescription attachments[4] = {};
        const bool msaa = kind == PassKind::multisample_color;
        const bool ids_only = kind == PassKind::ids;
        const VkFormat formats[] = {
            ids_only ? VK_FORMAT_R32G32_UINT : VK_FORMAT_R8G8B8A8_UNORM,
            msaa || ids_only ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_R32G32_UINT,
            msaa ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT};
        const unsigned count = ids_only ? 2 : independent_depth ? 4 : 3;
        for (unsigned i = 0; i < count; ++i) {
            auto& a = attachments[i];
            a.format = formats[i]; a.samples = msaa && i < 2 ? samples : VK_SAMPLE_COUNT_1_BIT;
            a.loadOp = msaa && i == 2 ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_CLEAR;
            a.storeOp = msaa && i < 2 ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
            a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            a.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            a.finalLayout = msaa && i < 2 ? (i == 0 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            if (i == 3) {
                a.samples = samples; a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                a.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                a.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }
        VkAttachmentReference colors[] = {
            {ids_only ? VK_ATTACHMENT_UNUSED : 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            {msaa ? VK_ATTACHMENT_UNUSED : ids_only ? 0u : 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
        VkAttachmentReference depth = {msaa || ids_only ? 1u : 2u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkAttachmentReference resolves[] = {{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                           {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
        VkSubpassDescription subpasses[4] = {};
        auto& scene = subpasses[0];
        scene.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        scene.colorAttachmentCount = 2; scene.pColorAttachments = colors;
        scene.pDepthStencilAttachment = &depth;
        // Resolve only after depth-tested annotations and HUD composition.
        VkAttachmentReference overlay_color = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference read_depth = {depth.attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
        const uint32_t preserve[] = {1,2};
        auto& annotations = subpasses[1];
        annotations.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        annotations.colorAttachmentCount = 1; annotations.pColorAttachments = &overlay_color;
        annotations.pDepthStencilAttachment = &read_depth;
        if (!msaa) { annotations.preserveAttachmentCount = 1; annotations.pPreserveAttachments = preserve; }
        const unsigned flat_subpass = independent_depth ? 3 : 2;
        VkAttachmentReference hud_depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        if (independent_depth) {
            auto& hud = subpasses[2];
            hud.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            hud.colorAttachmentCount = 1; hud.pColorAttachments = &overlay_color;
            hud.pDepthStencilAttachment = &hud_depth;
            if (!msaa) { hud.preserveAttachmentCount = 2; hud.pPreserveAttachments = preserve; }
        }
        auto& overlay = subpasses[flat_subpass];
        overlay.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        overlay.colorAttachmentCount = 1; overlay.pColorAttachments = &overlay_color;
        if (msaa) overlay.pResolveAttachments = resolves;
        else {
            overlay.preserveAttachmentCount = depth_preview ? 1 : 2; overlay.pPreserveAttachments = preserve;
            // Read-only attachment + sampled image is supported without a
            // feedback-loop extension: this subpass never writes scene depth.
            if (depth_preview) overlay.pDepthStencilAttachment = &read_depth;
        }
        VkSubpassDependency dependencies[12] = {};
        const auto color_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const auto depth_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencies[0].dstStageMask = color_stage | depth_stages;
        dependencies[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].srcSubpass = 0; dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = color_stage | depth_stages;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dependencies[2].srcSubpass = 0; dependencies[2].dstSubpass = 1;
        dependencies[2].srcStageMask = dependencies[2].dstStageMask = color_stage | depth_stages;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // Store operations remain depth writes for synchronization even though
        // annotation pipelines only read the attachment during fragment tests.
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[3].srcSubpass = 1; dependencies[3].dstSubpass = 2;
        dependencies[3].srcStageMask = dependencies[3].dstStageMask = color_stage;
        dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[4] = dependencies[1]; dependencies[4].srcSubpass = 1;
        dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[5] = dependencies[1]; dependencies[5].srcSubpass = 2;
        dependencies[5].srcStageMask = color_stage;
        dependencies[5].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (independent_depth) {
            dependencies[5].srcStageMask |= depth_stages;
            dependencies[5].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[6] = dependencies[3]; dependencies[6].srcSubpass = 2; dependencies[6].dstSubpass = 3;
            dependencies[7] = dependencies[1]; dependencies[7].srcSubpass = 3;
            dependencies[7].srcStageMask = color_stage;
            dependencies[7].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[8].srcSubpass = VK_SUBPASS_EXTERNAL; dependencies[8].dstSubpass = 2;
            dependencies[8].srcStageMask = dependencies[8].dstStageMask = depth_stages;
            dependencies[8].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[8].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        unsigned dependency_count = ids_only ? 2 : independent_depth ? 9 : 6;
        if (depth_preview) {
            for (unsigned source=0;source<2;++source) {
                auto& d=dependencies[dependency_count++];d.srcSubpass=source;d.dstSubpass=flat_subpass;
                d.srcStageMask=depth_stages;d.dstStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT|depth_stages;
                d.srcAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                d.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                // Attachment stores still count as depth writes even with a
                // read-only layout and depth-write-disabled overlay pipelines.
                // Arbitrary UVs sample outside the destination pixel's region.
            }
            auto& d=dependencies[dependency_count++];d.srcSubpass=flat_subpass;d.dstSubpass=VK_SUBPASS_EXTERNAL;
            d.srcStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT|depth_stages;d.dstStageMask=VK_PIPELINE_STAGE_TRANSFER_BIT;
            d.srcAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            d.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
        }
        VkRenderPassCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        pi.attachmentCount = count; pi.pAttachments = attachments;
        pi.subpassCount = ids_only ? 1 : independent_depth ? 4 : 3; pi.pSubpasses = subpasses;
        pi.dependencyCount = dependency_count; pi.pDependencies = dependencies;
        check(c.vkCreateRenderPass(c.device, &pi, nullptr, &handle), "create scene render pass");
    }
};
struct Target {
    Context& c;
    uint32_t width, height;
    unsigned samples;
    RenderPass& color_pass;
    RenderPass* id_pass;
    GpuImage color, ids, depth, multisample_color, multisample_depth, hud_depth;
    VkFramebuffer framebuffer = VK_NULL_HANDLE, id_framebuffer = VK_NULL_HANDLE;
    VkDescriptorPool depth_pool = VK_NULL_HANDLE;
    VkDescriptorSet depth_set = VK_NULL_HANDLE;
    Buffer readback;
    Target(Context& context, uint32_t w, uint32_t h, RenderPass& pass, RenderPass* picking)
        : c(context), width(w), height(h), samples(unsigned(pass.samples)), color_pass(pass), id_pass(picking),
          color(c), ids(c), depth(c), multisample_color(c), multisample_depth(c), hud_depth(c), readback(c) {}
    ~Target() {
        if (depth_set) c.vkFreeDescriptorSets(c.device,depth_pool,1,&depth_set);
        if (framebuffer) c.vkDestroyFramebuffer(c.device, framebuffer, nullptr);
        if (id_framebuffer) c.vkDestroyFramebuffer(c.device, id_framebuffer, nullptr);
    }
    void create_framebuffer(VkRenderPass pass, const VkImageView* views, unsigned count, VkFramebuffer& result) {
        VkFramebufferCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fi.renderPass = pass; fi.attachmentCount = count; fi.pAttachments = views;
        fi.width = width; fi.height = height; fi.layers = 1;
        check(c.vkCreateFramebuffer(c.device, &fi, nullptr, &result), "create scene framebuffer");
    }
    void init() {
        color.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        ids.init(width, height, VK_FORMAT_R32G32_UINT,
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        depth.init(width, height, VK_FORMAT_D32_SFLOAT,
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                       (color_pass.depth_preview ? VK_IMAGE_USAGE_SAMPLED_BIT : 0), VK_IMAGE_ASPECT_DEPTH_BIT);
        if (color_pass.independent_depth)
            hud_depth.init(width,height,VK_FORMAT_D32_SFLOAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                           VK_IMAGE_ASPECT_DEPTH_BIT,color_pass.samples);
        if (samples == 1) {
            VkImageView views[] = {color.view, ids.view, depth.view, hud_depth.view};
            create_framebuffer(color_pass.handle, views, color_pass.independent_depth ? 4 : 3, framebuffer);
        } else {
            multisample_color.init(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, color_pass.samples);
            multisample_depth.init(width, height, VK_FORMAT_D32_SFLOAT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, color_pass.samples);
            VkImageView color_views[] = {multisample_color.view, multisample_depth.view, color.view, hud_depth.view};
            create_framebuffer(color_pass.handle, color_views, color_pass.independent_depth ? 4 : 3, framebuffer);
            VkImageView picking_views[] = {ids.view, depth.view};
            create_framebuffer(id_pass->handle, picking_views, 2, id_framebuffer);
        }
    }
};

struct GpuVertex {
    float position[3], normal[3], color[4], texcoord[2], highlight[4];
    uint32_t primitive;
};
struct GpuPointAttributes {
    float normal[3],color[4],texcoord[2],highlight[4]; uint32_t primitive;
};
struct GpuGeometry {
    GpuPoints* points=nullptr;
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
    ~GpuTexture() { if (set) c.vkFreeDescriptorSets(c.device, pool, 1, &set); }
};
// Explicit packing: do not rely on public vec3/mat4 alignment for GPU layouts.
struct ImpostorUniform { float model_view[16], projection[16], inverse_projection[16], dimensions[4], light[4], clip_planes[8], clip_info[4];
    float model[16], eye[4], material_light[4], ambient[4], specular[4], backside[4], shading[4]; };
static_assert(sizeof(ImpostorUniform) == 432, "impostor uniform layout changed");
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
        for (auto s : ready) if (s) c.vkDestroySemaphore(c.device, s, nullptr);
        if (acquired) c.vkDestroySemaphore(c.device, acquired, nullptr);
        if (handle) c.vkDestroySwapchainKHR(c.device, handle, nullptr);
    }
    void init(uint32_t width, uint32_t height) {
        requested_width = width; requested_height = height;
        VkSurfaceCapabilitiesKHR caps;
        check(c.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(c.physical, c.surface, &caps), "query surface capabilities");
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
        check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, nullptr), "query surface formats");
        std::vector<VkSurfaceFormatKHR> formats(count);
        check(c.vkGetPhysicalDeviceSurfaceFormatsKHR(c.physical, c.surface, &count, formats.data()), "query surface formats");
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
                c.vkGetPhysicalDeviceFormatProperties(c.physical, format, &properties);
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
        check(c.vkCreateSwapchainKHR(c.device, &si, nullptr, &handle), "create swapchain");
        check(c.vkGetSwapchainImagesKHR(c.device, handle, &count, nullptr), "get presentation images");
        images.resize(count);
        check(c.vkGetSwapchainImagesKHR(c.device, handle, &count, images.data()), "get presentation images");
        VkSemaphoreCreateInfo sem{};
        sem.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        check(c.vkCreateSemaphore(c.device, &sem, nullptr, &acquired), "create acquire semaphore");
        ready.resize(count, VK_NULL_HANDLE);
        for (auto& s : ready) check(c.vkCreateSemaphore(c.device, &sem, nullptr, &s), "create image presentation semaphore");
    }
};

class VulkanDevice final : public RenderDevice {
    Context c_;
    std::string name_;
    std::unique_ptr<PointSelector> point_selector_;
    std::map<const PointData*,std::unique_ptr<GpuPoints>> point_resources_;
    uint64_t point_uploads_=0;
    uint32_t sample_counts_ = 1;
    std::map<unsigned, std::unique_ptr<RenderPass>> passes_;
    std::unique_ptr<RenderPass> id_pass_;
    VkDescriptorSetLayout descriptors_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout impostor_descriptors_ = VK_NULL_HANDLE;
    VkDescriptorSet impostor_set_ = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> impostor_uniforms_;
    size_t impostor_capacity_ = 0;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkSampler samplers_[4] = {};
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    std::unique_ptr<Target> target_;
    std::unique_ptr<Swapchain> swapchain_;
    std::shared_ptr<const Image> white_;
    std::map<const Geometry*, std::unique_ptr<GpuGeometry>> geometry_;
    std::map<const Image*, std::unique_ptr<GpuTexture>> textures_;
    struct ClipEntry {
        std::weak_ptr<const Geometry> source;
        std::vector<vec4> planes;
        mat4 model_view; mat3 normal;
        bool lighting=false, discard=false, analytic=false, baked=false, material=false;
        std::shared_ptr<const Geometry> geometry;
    };
    std::map<const Geometry*,ClipEntry> clipped_;
    DrawCommand clipped(const DrawCommand& input) {
        auto command=input;
        command.clipping_planes=normalize_clipping_planes(input.clipping_planes);
        if (command.overlay || command.clipping_planes.empty()) return command;
        for (unsigned c=0;c<4;++c) for (unsigned r=0;r<4;++r)
            if (!std::isfinite(command.model_view(r,c))) throw std::invalid_argument("non-finite clipping model-view");
        if (command.model_view(3,0)!=0 || command.model_view(3,1)!=0 || command.model_view(3,2)!=0 || command.model_view(3,3)!=1)
            throw std::invalid_argument("clipping requires an affine model-view transform");
        const bool analytic=(command.geometry->topology()==Topology::points && command.point_style!=PointStyle::plain) ||
            (command.geometry->topology()==Topology::lines && command.line_style!=LineStyle::plain);
        if (command.geometry->topology()==Topology::points || (analytic && !command.discard_clipped_primitives)) return command;
        auto& entry=clipped_[input.geometry.get()];
        if (entry.source.lock()!=input.geometry || entry.planes!=command.clipping_planes ||
            entry.model_view!=command.model_view || entry.normal!=command.normal_matrix ||
            entry.lighting!=command.lighting || entry.material!=command.material_lighting || entry.discard!=command.discard_clipped_primitives || entry.analytic!=analytic) {
            auto result=internal::clip_draw(command);
            entry.source=input.geometry; entry.planes=command.clipping_planes; entry.model_view=command.model_view;
            entry.normal=command.normal_matrix; entry.material=command.material_lighting; entry.lighting=command.lighting; entry.discard=command.discard_clipped_primitives;
            entry.analytic=analytic; entry.geometry=result.geometry; entry.baked=command.lighting && !result.lighting;
        }
        command.geometry=entry.geometry;
        if (entry.baked) command.lighting=false;
        return command;
    }
public:
    // Separate initialization ensures all members are destroyed if any Vulkan step throws.
    ~VulkanDevice() override {
        // Presentation may still be using image-owned semaphores after the draw
        // fence completes. Drain at shutdown/recreation only, never per frame.
        if (c_.device) c_.vkDeviceWaitIdle(c_.device);
        swapchain_.reset(); target_.reset(); geometry_.clear(); textures_.clear();
        passes_.clear(); id_pass_.reset();
        if (layout_) c_.vkDestroyPipelineLayout(c_.device, layout_, nullptr);
        for (auto sampler : samplers_) if (sampler) c_.vkDestroySampler(c_.device, sampler, nullptr);
        if (descriptor_pool_) c_.vkDestroyDescriptorPool(c_.device, descriptor_pool_, nullptr);
        if (descriptors_) c_.vkDestroyDescriptorSetLayout(c_.device, descriptors_, nullptr);
        if (impostor_descriptors_) c_.vkDestroyDescriptorSetLayout(c_.device, impostor_descriptors_, nullptr);
    }
    const std::string& name() const override { return name_; }
    uint32_t supported_sample_counts(PointStyle style) const override {
        return style != PointStyle::plain && !c_.sample_shading ? 1 : sample_counts_;
    }
    uint32_t supported_sample_counts(LineStyle style) const override {
        return style != LineStyle::plain && !c_.sample_shading ? 1 : sample_counts_;
    }
    unsigned validation_errors() const override { return c_.errors.load(); }
    uint64_t point_upload_count() const override { return point_uploads_; }
    GpuPoints& point_resource(const std::shared_ptr<const PointData>& data) {
        for (auto it=point_resources_.begin();it!=point_resources_.end();) {
            if (it->second->source.expired()) it=point_resources_.erase(it); else ++it;
        }
        auto found=point_resources_.find(data.get());
        if (found!=point_resources_.end()) return *found->second;
        std::unique_ptr<GpuPoints> gpu(new GpuPoints(c_)); gpu->source=data;
        if (!data->positions().empty()) {
            std::vector<GpuPoint> packed(data->positions().size());
            for (size_t i=0;i<packed.size();++i) {
                for (unsigned j=0;j<3;++j) packed[i].position[j]=data->positions()[i][j];
                packed[i].source=data->source_ids()[i];
            }
            const VkDeviceSize bytes=packed.size()*sizeof(GpuPoint);
            Buffer staging(c_); staging.init(bytes,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,true);
            staging.transfer(packed.data(),size_t(bytes),true);
            gpu->buffer.init(bytes,VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,false);
            c_.execute([&](VkCommandBuffer cmd) {
                VkBufferCopy copy={0,0,bytes}; c_.vkCmdCopyBuffer(cmd,staging.handle,gpu->buffer.handle,1,&copy);
                VkBufferMemoryBarrier barrier{}; barrier.sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask=VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                if (c_.compute) barrier.dstAccessMask|=VK_ACCESS_SHADER_READ_BIT;
                barrier.srcQueueFamilyIndex=barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
                barrier.buffer=gpu->buffer.handle; barrier.size=VK_WHOLE_SIZE;
                c_.vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_VERTEX_INPUT_BIT|
                    (c_.compute?VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT:0),0,0,nullptr,1,&barrier,0,nullptr);
            });
            ++point_uploads_;
        }
        auto* result=gpu.get(); point_resources_.emplace(data.get(),std::move(gpu)); return *result;
    }
    std::vector<uint32_t> select_points(const PointSelectionRequest& request) override {
        if (!request.width || !request.height) throw std::invalid_argument("selection requires positive framebuffer dimensions");
        if (!request.points) throw std::invalid_argument("selection requires a point resource");
        if (request.polygon.size()>UINT32_MAX)
            throw std::invalid_argument("selection input exceeds 32-bit indexing");
        const size_t count=(size_t(request.points->source_count())+31)/32;
        auto words=request.words;
        if (words.empty()) words.resize(count,0);
        else if (words.size()!=count) throw std::invalid_argument("selection word count does not match points");
        for (unsigned i=0;i<16;++i) if (!std::isfinite(request.model_view_projection[i]))
            throw std::invalid_argument("nonfinite selection transform");
        for (const auto& p:request.polygon) if (!std::isfinite(p.x) || !std::isfinite(p.y))
            throw std::invalid_argument("nonfinite selection polygon");
        const auto planes=normalize_clipping_planes(request.local_clipping_planes);
        bool area=false;
        for (size_t i=1;i+1<request.polygon.size();++i) {
            const auto a=request.polygon[i]-request.polygon[0],b=request.polygon[i+1]-request.polygon[0];
            area=area || std::abs(double(a.x)*b.y-double(a.y)*b.x)>1e-8;
        }
        if (request.points->positions().empty() || !area) return words;
        const uint64_t sizes[]={uint64_t(request.points->positions().size())*sizeof(GpuPoint),uint64_t(request.polygon.size())*sizeof(vec2),uint64_t(words.size())*sizeof(uint32_t)};
        for (auto size:sizes) if (size>c_.properties.limits.maxStorageBufferRange)
            throw std::invalid_argument("selection buffer exceeds device storage-buffer range");
        if (!c_.compute) throw std::runtime_error("selected graphics queue does not support compute selection");
        if (!point_selector_) {
            std::unique_ptr<PointSelector> next(new PointSelector(c_)); next->init(); point_selector_=std::move(next);
        }
        return point_selector_->run(request,std::move(words),planes,point_resource(request.points).buffer);
    }
    void init(const DeviceOptions& options, const VulkanSurface* surface = nullptr) {
        c_.init(options, surface); name_ = c_.properties.deviceName;
        white_ = std::make_shared<Image>(1, 1, std::vector<unsigned char>(4, 255));
        sample_counts_ = c_.properties.limits.framebufferColorSampleCounts & c_.properties.limits.framebufferDepthSampleCounts;
        for (auto format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT}) {
            VkImageFormatProperties properties;
            const auto usage = format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            check(c_.vkGetPhysicalDeviceImageFormatProperties(c_.physical, format, VK_IMAGE_TYPE_2D,
                  VK_IMAGE_TILING_OPTIMAL, usage, 0, &properties), "query MSAA target support");
            sample_counts_ &= properties.sampleCounts;
        }
        create_descriptors(); create_layout(); color_pass(1);
    }
    void require_samples(unsigned samples) const {
        if (!samples || samples > 64 || (samples & (samples - 1)))
            throw std::invalid_argument("sample count must be a power of two from 1 to 64");
        if (!(sample_counts_ & samples))
            throw std::invalid_argument("sample count " + std::to_string(samples) + " is unsupported by this device");
    }
    RenderPass& color_pass(unsigned samples, bool independent_depth = false, bool depth_preview = false) {
        const unsigned key = samples | (independent_depth ? 128u : 0u) | (depth_preview ? 256u : 0u);
        auto found = passes_.find(key);
        if (found != passes_.end()) return *found->second;
        std::unique_ptr<RenderPass> pass(new RenderPass(c_, samples == 1 ? PassKind::color_ids : PassKind::multisample_color,
                                                      static_cast<VkSampleCountFlagBits>(samples)));
        pass->independent_depth = independent_depth;
        pass->depth_preview = depth_preview;
        pass->init(); create_pipelines(*pass);
        auto* result = pass.get(); passes_.emplace(key, std::move(pass)); return *result;
    }
    RenderPass* picking_pass(unsigned samples) {
        if (samples == 1) return nullptr;
        if (!id_pass_) {
            std::unique_ptr<RenderPass> pass(new RenderPass(c_, PassKind::ids, VK_SAMPLE_COUNT_1_BIT));
            pass->init(); create_pipelines(*pass); id_pass_ = std::move(pass);
        }
        return id_pass_.get();
    }
    void create_descriptors() {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0; binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1; binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo li{};
        li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        li.bindingCount = 1; li.pBindings = &binding;
        check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &descriptors_), "create descriptor layout");
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        check(c_.vkCreateDescriptorSetLayout(c_.device, &li, nullptr, &impostor_descriptors_), "create impostor descriptor layout");
        VkDescriptorPoolSize sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1025},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}};
        VkDescriptorPoolCreateInfo pi{};
        pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pi.maxSets = 1026; pi.poolSizeCount = 2; pi.pPoolSizes = sizes;
        check(c_.vkCreateDescriptorPool(c_.device, &pi, nullptr, &descriptor_pool_), "create descriptor pool");
        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = descriptor_pool_; ai.descriptorSetCount = 1; ai.pSetLayouts = &impostor_descriptors_;
        check(c_.vkAllocateDescriptorSets(c_.device, &ai, &impostor_set_), "allocate impostor descriptor");
        for (unsigned wrap = 0; wrap < 2; ++wrap) for (unsigned filter = 0; filter < 2; ++filter) {
            VkSamplerCreateInfo si{};
            si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            si.magFilter = si.minFilter = filter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            si.addressModeU = si.addressModeV = si.addressModeW = wrap ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            si.maxLod = 0;
            check(c_.vkCreateSampler(c_.device, &si, nullptr, &samplers_[wrap*2+filter]), "create sampler");
        }
    }
    void create_layout() {
        VkPushConstantRange push = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Push)};
        VkPipelineLayoutCreateInfo li{};
        li.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        const VkDescriptorSetLayout sets[] = {descriptors_, impostor_descriptors_};
        li.setLayoutCount = 2; li.pSetLayouts = sets;
        li.pushConstantRangeCount = 1; li.pPushConstantRanges = &push;
        check(c_.vkCreatePipelineLayout(c_.device, &li, nullptr, &layout_), "create pipeline layout");
    }
    void create_pipelines(RenderPass& pass) {
        VkShaderModule modules[11] = {};
        try {
            VkShaderModuleCreateInfo mi{};
            mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            mi.codeSize = sizeof(scene_vert); mi.pCode = scene_vert;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[0]), "create vertex shader");
            mi.codeSize = sizeof(scene_frag); mi.pCode = scene_frag;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[1]), "create fragment shader");
            mi.codeSize = sizeof(points_vert); mi.pCode = points_vert;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[2]), "create point vertex shader");
            mi.codeSize = sizeof(points_frag); mi.pCode = points_frag;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[3]), "create point fragment shader");
            mi.codeSize = sizeof(lines_vert); mi.pCode = lines_vert;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[4]), "create line vertex shader");
            mi.codeSize = sizeof(lines_frag); mi.pCode = lines_frag;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[5]), "create line fragment shader");
            mi.codeSize = sizeof(wide_lines_vert); mi.pCode = wide_lines_vert;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[6]), "create wide line vertex shader");
            mi.codeSize = sizeof(wide_lines_frag); mi.pCode = wide_lines_frag;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[7]), "create wide line fragment shader");
            mi.codeSize = sizeof(overlay_vert); mi.pCode = overlay_vert;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[8]), "create overlay vertex shader");
            mi.codeSize = sizeof(overlay_frag); mi.pCode = overlay_frag;
            check(c_.vkCreateShaderModule(c_.device, &mi, nullptr, &modules[9]), "create overlay fragment shader");
            if (pass.depth_preview) {
                mi.codeSize = sizeof(depth_texture_frag); mi.pCode = depth_texture_frag;
                check(c_.vkCreateShaderModule(c_.device,&mi,nullptr,&modules[10]), "create depth texture fragment shader");
            }
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
                {4, 0, VK_FORMAT_R32_UINT, offsetof(GpuVertex, primitive)},
                {9, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, highlight)}};
            VkVertexInputAttributeDescription line_attrs[] = {
                attrs[0], attrs[2], attrs[3], attrs[4],
                {5, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, position)},
                {6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, color)},
                {7, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, texcoord)}, attrs[5]};
            VkVertexInputAttributeDescription wide_attrs[] = {
                attrs[0], attrs[1], attrs[2], attrs[3], attrs[4], line_attrs[4], line_attrs[5], line_attrs[6],
                {8, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(GpuVertex) + offsetof(GpuVertex, normal)}, attrs[5]};
            VkPipelineVertexInputStateCreateInfo vertex{};
            vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex.vertexBindingDescriptionCount = 1; vertex.pVertexBindingDescriptions = &binding;
            vertex.vertexAttributeDescriptionCount = 6; vertex.pVertexAttributeDescriptions = attrs;
            VkVertexInputBindingDescription point_bindings[]={
                {0,sizeof(GpuPoint),VK_VERTEX_INPUT_RATE_INSTANCE},
                {1,sizeof(GpuPointAttributes),VK_VERTEX_INPUT_RATE_INSTANCE}};
            VkVertexInputAttributeDescription point_attrs[]={
                {0,0,VK_FORMAT_R32G32B32_SFLOAT,0},
                {1,1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuPointAttributes,normal)},
                {2,1,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(GpuPointAttributes,color)},
                {3,1,VK_FORMAT_R32G32_SFLOAT,offsetof(GpuPointAttributes,texcoord)},
                {4,1,VK_FORMAT_R32_UINT,offsetof(GpuPointAttributes,primitive)},
                {9,1,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(GpuPointAttributes,highlight)}};
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
            ms.rasterizationSamples = pass.samples;
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
            pi.pDynamicState = &dynamic; pi.layout = layout_; pi.renderPass = pass.handle;
            const VkPrimitiveTopology topologies[] = {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST};
            VkBool32 native_point=VK_TRUE;
            VkSpecializationMapEntry point_entry={0,0,sizeof(native_point)};
            VkSpecializationInfo point_specialization={1,&point_entry,sizeof(native_point),&native_point};
            for (unsigned i = 0; i < 6; ++i) {
                if (i==2) {
                    stages[0].module=modules[2]; stages[1].module=modules[3];
                    stages[0].pSpecializationInfo=&point_specialization;
                    point_bindings[0].inputRate=point_bindings[1].inputRate=VK_VERTEX_INPUT_RATE_VERTEX;
                    vertex.vertexBindingDescriptionCount=2; vertex.pVertexBindingDescriptions=point_bindings;
                    vertex.pVertexAttributeDescriptions=point_attrs;
                }
                if (i == 3) {
                    stages[0].pSpecializationInfo=nullptr;
                    point_bindings[0].inputRate=point_bindings[1].inputRate=VK_VERTEX_INPUT_RATE_INSTANCE;
                    stages[0].module = modules[2]; stages[1].module = modules[3];
                    binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                    vertex.vertexBindingDescriptionCount=2; vertex.pVertexBindingDescriptions=point_bindings;
                    vertex.pVertexAttributeDescriptions=point_attrs;
                    ms.sampleShadingEnable = c_.sample_shading && pass.samples != VK_SAMPLE_COUNT_1_BIT;
                    ms.minSampleShading = 1;
                }
                if (i == 4) {
                    vertex.vertexBindingDescriptionCount=1; vertex.pVertexBindingDescriptions=&binding;
                    stages[0].module = modules[4]; stages[1].module = modules[5];
                    binding.stride = 2 * sizeof(GpuVertex);
                    vertex.vertexAttributeDescriptionCount = 8;
                    vertex.pVertexAttributeDescriptions = line_attrs;
                }
                if (i == 5) {
                    stages[0].module = modules[6]; stages[1].module = modules[7];
                    vertex.vertexAttributeDescriptionCount = 10;
                    vertex.pVertexAttributeDescriptions = wide_attrs;
                    ms.sampleShadingEnable = VK_FALSE;
                    ms.minSampleShading = 0;
                }
                assembly.topology = topologies[i >= 3 ? 0 : i];
                check(c_.vkCreateGraphicsPipelines(c_.device, VK_NULL_HANDLE, 1, &pi, nullptr, &pass.pipelines[i]), "create pipeline");
            }
            if (pass.kind != PassKind::ids) {
                stages[0].module = modules[8]; stages[1].module = modules[9];
                binding.stride = sizeof(GpuVertex); binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                VkVertexInputAttributeDescription overlay_attrs[] = {attrs[0],attrs[2],attrs[3]};
                vertex.vertexAttributeDescriptionCount = 3; vertex.pVertexAttributeDescriptions = overlay_attrs;
                assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                ms.sampleShadingEnable = VK_FALSE; ms.minSampleShading = 0;
                depth.depthTestEnable = depth.depthWriteEnable = VK_FALSE;
                VkPipelineColorBlendAttachmentState alpha{};
                alpha.blendEnable = VK_TRUE;
                alpha.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                alpha.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                alpha.colorBlendOp = VK_BLEND_OP_ADD;
                alpha.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                alpha.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                alpha.alphaBlendOp = VK_BLEND_OP_ADD; alpha.colorWriteMask = 0xf;
                blending.attachmentCount = 1; blending.pAttachments = &alpha;
                pi.subpass = 1;
                depth.depthTestEnable = VK_TRUE; depth.depthWriteEnable = VK_FALSE;
                depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                check(c_.vkCreateGraphicsPipelines(c_.device,VK_NULL_HANDLE,1,&pi,nullptr,&pass.depth_overlay_pipeline), "create depth-tested overlay pipeline");
                if (pass.independent_depth) {
                    pi.subpass = 2; depth.depthTestEnable = depth.depthWriteEnable = VK_TRUE;
                    depth.depthCompareOp = VK_COMPARE_OP_LESS;
                    stages[0].module = modules[0]; // lit triangles, color-only fragment output
                    vertex.vertexAttributeDescriptionCount = 6; vertex.pVertexAttributeDescriptions = attrs;
                    check(c_.vkCreateGraphicsPipelines(c_.device,VK_NULL_HANDLE,1,&pi,nullptr,&pass.hud_3d_pipeline), "create independent-depth HUD pipeline");
                    stages[0].module = modules[8];
                    vertex.vertexAttributeDescriptionCount = 3; vertex.pVertexAttributeDescriptions = overlay_attrs;
                }
                pi.subpass = pass.independent_depth ? 3 : 2;
                depth.depthTestEnable = depth.depthWriteEnable = VK_FALSE;
                check(c_.vkCreateGraphicsPipelines(c_.device,VK_NULL_HANDLE,1,&pi,nullptr,&pass.overlay_pipeline), "create overlay pipeline");
                if (pass.depth_preview) {
                    stages[1].module = modules[10];
                    check(c_.vkCreateGraphicsPipelines(c_.device,VK_NULL_HANDLE,1,&pi,nullptr,&pass.depth_texture_pipeline), "create depth texture overlay pipeline");
                }
            }
        } catch (...) {
            for (auto m : modules) if (m) c_.vkDestroyShaderModule(c_.device, m, nullptr);
            throw;
        }
        for (auto m : modules) c_.vkDestroyShaderModule(c_.device, m, nullptr);
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
            for (unsigned j = 0; j < 4; ++j) { p.color[j] = v.color[j]; p.highlight[j] = v.highlight[j]; }
            for (unsigned j = 0; j < 2; ++j) p.texcoord[j] = v.texcoord[j];
            p.primitive = data->primitive_ids().empty() ? static_cast<uint32_t>(i / size) : data->primitive_ids()[i / size];
        }
        if (data->topology()==Topology::points) gpu->points=&point_resource(data->point_data());
        if (count) {
            std::vector<GpuPointAttributes> attributes;
            const void* input=packed.data(); size_t bytes=packed.size()*sizeof(GpuVertex);
            if (gpu->points) {
                attributes.resize(count);
                for (size_t i=0;i<count;++i) {
                    auto& a=attributes[i]; const auto& v=packed[i];
                    std::memcpy(a.normal,v.normal,sizeof(a.normal)); std::memcpy(a.color,v.color,sizeof(a.color));
                    std::memcpy(a.texcoord,v.texcoord,sizeof(a.texcoord)); std::memcpy(a.highlight,v.highlight,sizeof(a.highlight));
                    a.primitive=v.primitive;
                }
                input=attributes.data(); bytes=attributes.size()*sizeof(GpuPointAttributes);
            }
            Buffer staging(c_);
            staging.init(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
            staging.transfer(const_cast<void*>(input), bytes, true);
            gpu->buffer.init(bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false);
            c_.execute([&](VkCommandBuffer cmd) {
                VkBufferCopy copy = {0, 0, bytes};
                c_.vkCmdCopyBuffer(cmd, staging.handle, gpu->buffer.handle, 1, &copy);
                VkBufferMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.buffer = gpu->buffer.handle; barrier.size = VK_WHOLE_SIZE;
                c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
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
        if (data->sampler().filter == TextureFilter::linear) {
            VkFormatProperties properties;
            c_.vkGetPhysicalDeviceFormatProperties(c_.physical, VK_FORMAT_R8G8B8A8_UNORM, &properties);
            if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
                throw std::runtime_error("RGBA8 linear texture filtering is unsupported by this device");
        }
        std::unique_ptr<GpuTexture> gpu(new GpuTexture(c_, descriptor_pool_));
        gpu->source = data;
        gpu->image.init(data->width(), data->height(), VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        Buffer staging(c_);
        staging.init(data->rgba().size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        staging.transfer(const_cast<unsigned char*>(data->rgba().data()), data->rgba().size(), true);
        c_.execute([&](VkCommandBuffer cmd) {
            image_barrier(c_,cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
            VkBufferImageCopy copy{};
            copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.imageExtent = {data->width(), data->height(), 1};
            c_.vkCmdCopyBufferToImage(cmd, staging.handle, gpu->image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
            image_barrier(c_,cmd, gpu->image.handle, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
        });
        VkDescriptorSetAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = descriptor_pool_; ai.descriptorSetCount = 1; ai.pSetLayouts = &descriptors_;
        check(c_.vkAllocateDescriptorSets(c_.device, &ai, &gpu->set), "allocate texture descriptor (maximum 1024 live textures)");
        const unsigned sampler_index = (data->sampler().wrap == TextureWrap::repeat ? 2 : 0) +
                                       (data->sampler().filter == TextureFilter::linear ? 1 : 0);
        VkDescriptorImageInfo image = {samplers_[sampler_index], gpu->image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = gpu->set; write.dstBinding = 0; write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; write.pImageInfo = &image;
        c_.vkUpdateDescriptorSets(c_.device, 1, &write, 0, nullptr);
        auto* result = gpu.get(); textures_.emplace(data.get(), std::move(gpu));
        return *result;
    }
    static bool expanded_lines(const DrawCommand& d) {
        return d.geometry->topology() == Topology::lines &&
               (d.line_style != LineStyle::plain || d.line_width != 1);
    }
    static bool expanded_impostor(const DrawCommand& d) {
        return expanded_lines(d) || (d.geometry->topology()==Topology::points &&
            (d.point_style!=PointStyle::plain || d.point_size!=1));
    }
    struct Prepared { DrawCommand command; GpuGeometry* geometry; GpuTexture* texture; uint32_t uniform_offset; };
    std::vector<Prepared> prepare(uint32_t width, uint32_t height, const std::vector<DrawCommand>& commands,
                                  const vec4& background, unsigned samples) {
        require_samples(samples);
        const auto& limits = c_.properties.limits;
        if (!width || !height || width > limits.maxFramebufferWidth || height > limits.maxFramebufferHeight ||
            width > limits.maxImageDimension2D || height > limits.maxImageDimension2D ||
            uint64_t(width) * height > (std::numeric_limits<size_t>::max() - 8) / 16)
            throw std::invalid_argument("invalid offscreen target dimensions");
        for (unsigned i = 0; i < 4; ++i)
            if (!std::isfinite(background[i])) throw std::invalid_argument("non-finite background color");
        for (const auto& command : commands) {
            if (!command.geometry || (!command.overlay && command.object_id == UINT32_MAX))
                throw std::invalid_argument("scene draw requires geometry and a non-background object ID");
            if (command.overlay_depth_test && !command.overlay)
                throw std::invalid_argument("overlay depth testing requires an overlay command");
            if (command.overlay_independent_depth && (!command.overlay || command.overlay_depth_test))
                throw std::invalid_argument("independent HUD depth requires an overlay without scene depth testing");
            if (command.scene_depth_texture && (!command.overlay || command.overlay_depth_test ||
                command.overlay_independent_depth || command.texture))
                throw std::invalid_argument("scene depth sampling requires a flat overlay without an image texture");
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
            if (command.line_style != LineStyle::plain && (command.model_view(3,0) != 0 ||
                command.model_view(3,1) != 0 || command.model_view(3,2) != 0 || command.model_view(3,3) != 1))
                throw std::invalid_argument("line impostor model-view transform must be affine");
            if (!std::isfinite(command.point_size) || command.point_size <= 0 ||
                !std::isfinite(command.sphere_radius) || command.sphere_radius <= 0)
                throw std::invalid_argument("point size and sphere radius must be finite and positive");
            if (command.geometry->topology() != Topology::points && command.point_style != PointStyle::plain)
                throw std::invalid_argument("point impostor style requires point geometry");
            if ((command.point_style != PointStyle::plain || command.line_style != LineStyle::plain) && samples > 1 && !c_.sample_shading)
                throw std::invalid_argument("multisampled impostors require sampleRateShading for correct coverage/depth");
            for (unsigned col = 0; col < 4; ++col) {
                if (!std::isfinite(command.color[col])) throw std::invalid_argument("non-finite draw color");
                for (unsigned row = 0; row < 4; ++row)
                    if (!std::isfinite(command.model_view_projection(row, col)))
                        throw std::invalid_argument("non-finite transform");
            }
            if (command.material_lighting) {
                if (!std::isfinite(command.shininess) || command.shininess<0)
                    throw std::invalid_argument("material shininess must be finite and nonnegative");
                for (unsigned c=0;c<4;++c) {
                    if (!std::isfinite(command.ambient[c]) || !std::isfinite(command.specular[c]) || !std::isfinite(command.back_color[c]))
                        throw std::invalid_argument("non-finite material color");
                    for (unsigned r=0;r<4;++r) if (!std::isfinite(command.model(r,c)))
                        throw std::invalid_argument("non-finite material model transform");
                }
                for (unsigned c=0;c<3;++c) if (!std::isfinite(command.eye[c]) || !std::isfinite(command.light[c]))
                    throw std::invalid_argument("non-finite material camera/light");
                if (command.lighting && dot(command.light,command.light)==0)
                    throw std::invalid_argument("material lighting requires a nonzero direction");
            }
            if (command.point_style == PointStyle::surfel) {
                if (command.model_view(3,0) != 0 || command.model_view(3,1) != 0 ||
                    command.model_view(3,2) != 0 || command.model_view(3,3) != 1)
                    throw std::invalid_argument("surfel model-view transform must be affine");
                const auto normal_transform = transpose(inverse(mat3(command.model_view)));
                for (unsigned col = 0; col < 3; ++col)
                    for (unsigned row = 0; row < 3; ++row)
                        if (!std::isfinite(normal_transform(row, col)))
                            throw std::invalid_argument("surfel model-view transform must have an invertible linear part");
                const auto& vertices = command.geometry->vertices();
                const auto& indices = command.geometry->indices();
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
        }
        // The preceding call's fence has completed. Retire expired resources now.
        for (auto it=clipped_.begin();it!=clipped_.end();) {
            if (it->second.source.expired()) it=clipped_.erase(it); else ++it;
        }
        for (auto it = geometry_.begin(); it != geometry_.end(); ) {
            if (it->second->source.expired()) it = geometry_.erase(it); else ++it;
        }
        for (auto it=point_resources_.begin();it!=point_resources_.end();) {
            if (it->second->source.expired()) it=point_resources_.erase(it); else ++it;
        }
        for (auto it = textures_.begin(); it != textures_.end(); ) {
            if (it->second->source.expired()) it = textures_.erase(it); else ++it;
        }
        const bool independent_depth = std::any_of(commands.begin(),commands.end(),
            [](const DrawCommand& d) {return d.overlay_independent_depth;});
        const bool depth_preview = std::any_of(commands.begin(),commands.end(),
            [](const DrawCommand& d) {return d.scene_depth_texture;});
        if (!target_ || target_->width != width || target_->height != height || target_->samples != samples ||
            target_->color_pass.independent_depth != independent_depth || target_->color_pass.depth_preview != depth_preview) {
            if (depth_preview) {
                VkFormatProperties properties;c_.vkGetPhysicalDeviceFormatProperties(c_.physical,VK_FORMAT_D32_SFLOAT,&properties);
                if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
                    throw std::runtime_error("scene depth preview requires sampleable D32 depth");
            }
            auto& pass = color_pass(samples,independent_depth,depth_preview);
            auto* picking = picking_pass(samples);
            std::unique_ptr<Target> target(new Target(c_, width, height, pass, picking));
            target->init(); target_ = std::move(target);
        }
        if (depth_preview && !target_->depth_set) {
            target_->depth_pool = descriptor_pool_;
            VkDescriptorSetAllocateInfo ai{};ai.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool=descriptor_pool_;ai.descriptorSetCount=1;ai.pSetLayouts=&descriptors_;
            check(c_.vkAllocateDescriptorSets(c_.device,&ai,&target_->depth_set),"allocate scene depth descriptor");
            VkDescriptorImageInfo image={samplers_[0],target_->depth.view,VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
            VkWriteDescriptorSet write{};write.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet=target_->depth_set;write.descriptorCount=1;
            write.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;write.pImageInfo=&image;
            c_.vkUpdateDescriptorSets(c_.device,1,&write,0,nullptr);
        }
        const size_t alignment = std::max(size_t(16), size_t(limits.minUniformBufferOffsetAlignment));
        const size_t stride = (sizeof(ImpostorUniform) + alignment - 1) / alignment * alignment;
        std::vector<unsigned char> uniforms;
        std::vector<Prepared> draws;
        for (const auto& original : commands) {
            auto command=clipped(original);
            auto& g = geometry(command.geometry);
            if (!g.count) continue;
            auto& t = texture(command.texture ? command.texture : white_);
            uint32_t offset = 0;
            { // Per-draw uniform state is also used by material-lit surfaces.
                if (uniforms.size() > UINT32_MAX - stride)
                    throw std::invalid_argument("too many impostor draw commands");
                offset = static_cast<uint32_t>(uniforms.size());
                ImpostorUniform u{};
                const bool analytic = command.point_style != PointStyle::plain || command.line_style != LineStyle::plain;
                mat4 model_view = mat4::identity(), projection = mat4::identity();
                if (analytic) { model_view = command.model_view; projection = command.projection; }
                else if (command.geometry->topology()==Topology::points && !command.clipping_planes.empty()) model_view=command.model_view;
                const auto inv = analytic ? inverse(projection) : mat4::identity();
                for (unsigned col = 0; col < 4; ++col)
                    for (unsigned row = 0; row < 4; ++row) {
                        u.model_view[col*4+row] = model_view(row,col);
                        u.projection[col*4+row] = projection(row,col);
                        u.inverse_projection[col*4+row] = inv(row,col);
                        if (!std::isfinite(u.model_view[col*4+row]) || !std::isfinite(u.projection[col*4+row]) ||
                            !std::isfinite(u.inverse_projection[col*4+row]))
                            throw std::invalid_argument("invalid impostor transform/projection");
                    }
                u.dimensions[0] = float(width); u.dimensions[1] = float(height);
                u.dimensions[2] = expanded_lines(command) ? command.line_width : command.point_size;
                u.dimensions[3] = expanded_lines(command) ? command.line_radius : command.sphere_radius;
                for (unsigned i = 0; i < 3; ++i) {
                    if (!std::isfinite(command.sphere_light[i]))
                        throw std::invalid_argument("non-finite impostor light");
                    u.light[i] = command.sphere_light[i];
                }
                if (analytic && command.lighting &&
                    dot(command.sphere_light, command.sphere_light) == 0)
                    throw std::invalid_argument("impostor lighting requires a nonzero direction");
                u.light[3] = command.point_style == PointStyle::surfel ? 2 : command.point_style == PointStyle::sphere ? 1 : 0;
                if (expanded_lines(command))
                    u.light[3] = command.line_style == LineStyle::plain ? 5 : command.line_style == LineStyle::cone ? 4 : 3;
                if (analytic || command.geometry->topology()==Topology::points) {
                    u.clip_info[1]=command.discard_clipped_primitives?1.f:0.f;
                    u.clip_info[0]=float(command.clipping_planes.size());
                    for (size_t plane=0;plane<command.clipping_planes.size();++plane)
                        for (unsigned c=0;c<4;++c) u.clip_planes[plane*4+c]=command.clipping_planes[plane][c];
                }
                for (unsigned col=0;col<4;++col) for (unsigned row=0;row<4;++row)
                    u.model[col*4+row]=command.model(row,col);
                for (unsigned i=0;i<3;++i) {
                    u.eye[i]=command.eye[i];u.material_light[i]=command.light[i];
                }
                for (unsigned i=0;i<4;++i) {
                    u.ambient[i]=command.ambient[i];u.specular[i]=command.specular[i];u.backside[i]=command.back_color[i];
                }
                u.shading[0]=command.material_lighting?1.f:0.f;
                u.shading[1]=command.lighting_two_sides?1.f:0.f;
                u.shading[2]=command.distinct_back_color?1.f:0.f;
                u.shading[3]=command.shininess;
                uniforms.resize(uniforms.size() + stride);
                std::memcpy(uniforms.data() + offset, &u, sizeof(u));
            }
            draws.push_back({std::move(command), &g, &t, offset});
        }
        if (!uniforms.empty()) {
            if (uniforms.size() > impostor_capacity_) {
                std::unique_ptr<Buffer> buffer(new Buffer(c_));
                buffer->init(uniforms.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
                VkDescriptorBufferInfo info = {buffer->handle, 0, sizeof(ImpostorUniform)};
                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = impostor_set_; write.descriptorCount = 1;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; write.pBufferInfo = &info;
                c_.vkUpdateDescriptorSets(c_.device, 1, &write, 0, nullptr);
                impostor_uniforms_ = std::move(buffer); impostor_capacity_ = uniforms.size();
            }
            impostor_uniforms_->transfer(uniforms.data(), uniforms.size(), true);
        }
        return draws;
    }
    void encode_pass(VkCommandBuffer cmd, const std::vector<Prepared>& draws, const vec4& background,
                 const RenderPass& pass, VkFramebuffer framebuffer) {
        const auto width = target_->width, height = target_->height;
        VkClearValue clear[4] = {};
        if (pass.kind == PassKind::ids) {
            clear[0].color.uint32[0] = clear[0].color.uint32[1] = UINT32_MAX;
            clear[1].depthStencil.depth = 1;
        } else {
            for (unsigned i = 0; i < 4; ++i) clear[0].color.float32[i] = background[i];
            if (pass.kind == PassKind::multisample_color) clear[1].depthStencil.depth = 1;
            else {
                clear[1].color.uint32[0] = clear[1].color.uint32[1] = UINT32_MAX;
                clear[2].depthStencil.depth = 1;
            }
            clear[3].depthStencil.depth = 1;
        }
        VkRenderPassBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin.renderPass = pass.handle; begin.framebuffer = framebuffer;
        begin.renderArea.extent = {width, height};
        begin.clearValueCount = pass.kind == PassKind::ids ? 2 : pass.independent_depth ? 4 : 3;
        begin.pClearValues = clear;
        c_.vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport = {0, 0, float(width), float(height), 0, 1};
        VkRect2D scissor = {{0, 0}, {width, height}};
        c_.vkCmdSetViewport(cmd, 0, 1, &viewport); c_.vkCmdSetScissor(cmd, 0, 1, &scissor);
        for (const auto& draw : draws) {
            const auto& d = draw.command;
            if (d.overlay) continue;
            unsigned pipeline = d.geometry->topology() == Topology::triangles ? 0 :
                                d.geometry->topology() == Topology::lines ? 1 : 2;
            const bool expanded = expanded_impostor(d);
            if (expanded) pipeline = expanded_lines(d) ? (d.line_style == LineStyle::plain ? 5 : 4) : 3;
            c_.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipelines[pipeline]);
            c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 1, 1, &impostor_set_, 1, &draw.uniform_offset);
            VkDeviceSize offset = 0;
            if (draw.geometry->points) {
                VkBuffer buffers[]={draw.geometry->points->buffer.handle,draw.geometry->buffer.handle};
                VkDeviceSize offsets[]={0,0}; c_.vkCmdBindVertexBuffers(cmd,0,2,buffers,offsets);
            } else c_.vkCmdBindVertexBuffers(cmd, 0, 1, &draw.geometry->buffer.handle, &offset);
            c_.vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_, 0, 1, &draw.texture->set, 0, nullptr);
            Push push{};
            for (unsigned col = 0; col < 4; ++col)
                for (unsigned row = 0; row < 4; ++row) push.mvp[col * 4 + row] = d.model_view_projection(row, col);
            for (unsigned i = 0; i < 4; ++i) push.color[i] = d.color[i];
            push.object = d.object_id; push.lighting = d.lighting ? 1 : 0;
            for (unsigned col = 0; col < 3; ++col)
                for (unsigned row = 0; row < 3; ++row) push.normal[col * 3 + row] = d.normal_matrix(row, col);
            c_.vkCmdPushConstants(cmd, layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
            if (expanded) c_.vkCmdDraw(cmd, 6, expanded_lines(d) ? draw.geometry->count / 2 : draw.geometry->count, 0, 0);
            else c_.vkCmdDraw(cmd, draw.geometry->count, 1, 0, 0);
        }
        if (pass.kind != PassKind::ids) {
            const unsigned phases = pass.independent_depth ? 3 : 2;
            for (unsigned phase=0;phase<phases;++phase) {
                c_.vkCmdNextSubpass(cmd,VK_SUBPASS_CONTENTS_INLINE);
                for (const auto& draw : draws) {
                    const auto& d = draw.command;
                    if (!d.overlay) continue;
                    const unsigned draw_phase = d.overlay_depth_test ? 0 :
                        d.overlay_independent_depth ? 1 : phases-1;
                    if (draw_phase != phase) continue;
                    const auto pipeline=d.scene_depth_texture ? pass.depth_texture_pipeline : phase==0 ? pass.depth_overlay_pipeline :
                        phase==phases-1 ? pass.overlay_pipeline : pass.hud_3d_pipeline;
                    c_.vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);
                    VkDeviceSize offset = 0;
                    c_.vkCmdBindVertexBuffers(cmd,0,1,&draw.geometry->buffer.handle,&offset);
                    const auto texture_set=d.scene_depth_texture ? target_->depth_set : draw.texture->set;
                    c_.vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,layout_,0,1,&texture_set,0,nullptr);
                    c_.vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,layout_,1,1,&impostor_set_,1,&draw.uniform_offset);
                    Push push{};
                    for (unsigned col=0;col<4;++col) for (unsigned row=0;row<4;++row)
                        push.mvp[col*4+row] = d.model_view_projection(row,col);
                    for (unsigned i=0;i<4;++i) push.color[i] = d.color[i];
                    push.lighting = d.lighting ? 1 : 0;
                    for (unsigned col=0;col<3;++col) for (unsigned row=0;row<3;++row)
                        push.normal[col*3+row] = d.normal_matrix(row,col);
                    c_.vkCmdPushConstants(cmd,layout_,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(push),&push);
                    c_.vkCmdDraw(cmd,draw.geometry->count,1,0,0);
                }
            }
        }
        c_.vkCmdEndRenderPass(cmd);
    }
    void encode(VkCommandBuffer cmd, const std::vector<Prepared>& draws, const vec4& background) {
        const bool preview_msaa=target_->color_pass.depth_preview && target_->id_pass;
        if (preview_msaa) {
            // Existing center-sample picking depth is also the preview source.
            // Generate it before color composition; MSAA color still resolves last.
            encode_pass(cmd,draws,background,*target_->id_pass,target_->id_framebuffer);
            image_barrier(c_,cmd,target_->depth.handle,VK_IMAGE_ASPECT_DEPTH_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT|VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT|VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT|VK_ACCESS_TRANSFER_READ_BIT,
                    VK_ACCESS_SHADER_READ_BIT);
        }
        encode_pass(cmd, draws, background, target_->color_pass, target_->framebuffer);
        // Picking uses center-sample IDs/depth independently of antialiased color.
        // This works even when an integer format supports no multisampling, and
        // requires neither depth-resolve extensions nor averaging integer IDs.
        if (target_->id_pass && !preview_msaa) encode_pass(cmd, draws, background, *target_->id_pass, target_->id_framebuffer);
        if (preview_msaa)
            image_barrier(c_,cmd,target_->depth.handle,VK_IMAGE_ASPECT_DEPTH_BIT,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_ACCESS_SHADER_READ_BIT,VK_ACCESS_TRANSFER_READ_BIT);
    }
    FrameImage render(uint32_t width, uint32_t height, const std::vector<DrawCommand>& commands,
                      const vec4& background, unsigned samples) override {
        const auto draws = prepare(width, height, commands, background, samples);
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
                c_.vkCmdCopyImageToBuffer(cmd, images[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target_->readback.handle, 1, &copy);
            }
            VkMemoryBarrier host{};
            host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            c_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
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
                 const vec4& background, unsigned samples) override {
        if (!c_.surface) throw std::logic_error("headless Vulkan device has no presentation surface");
        if (!width || !height) return false;
        if (!swapchain_ || swapchain_->dirty || swapchain_->requested_width != width || swapchain_->requested_height != height) {
            // Rare lifecycle boundary. The normal frame path waits only its draw fence.
            check(c_.vkDeviceWaitIdle(c_.device), "drain presentation before swapchain recreation");
            swapchain_.reset();
            std::unique_ptr<Swapchain> next(new Swapchain(c_));
            next->init(width, height);
            if (!next->handle) return false;
            swapchain_ = std::move(next);
        }
        const auto draws = prepare(width, height, commands, background, samples);
        uint32_t image = 0;
        const auto acquired = c_.vkAcquireNextImageKHR(c_.device, swapchain_->handle, UINT64_MAX,
                                                    swapchain_->acquired, VK_NULL_HANDLE, &image);
        if (acquired == VK_ERROR_OUT_OF_DATE_KHR) { swapchain_->dirty = true; return false; }
        if (acquired != VK_SUBOPTIMAL_KHR) check(acquired, "acquire presentation image");
        c_.execute([&](VkCommandBuffer cmd) {
            encode(cmd, draws, background);
            image_barrier(c_,cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
            VkImageBlit copy{};
            copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copy.srcOffsets[1] = {int32_t(width), int32_t(height), 1};
            copy.dstOffsets[1] = {int32_t(swapchain_->extent.width), int32_t(swapchain_->extent.height), 1};
            c_.vkCmdBlitImage(cmd, target_->color.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                swapchain_->images[image], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy, VK_FILTER_NEAREST);
            image_barrier(c_,cmd, swapchain_->images[image], VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0);
        }, swapchain_->acquired, swapchain_->ready[image]);
        VkPresentInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1; info.pWaitSemaphores = &swapchain_->ready[image];
        info.swapchainCount = 1; info.pSwapchains = &swapchain_->handle; info.pImageIndices = &image;
        const auto presented = c_.vkQueuePresentKHR(c_.queue, &info);
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
