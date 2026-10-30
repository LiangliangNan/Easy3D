// SPDX-License-Identifier: GPL-3.0-or-later
#include "loader.h"
#include <cstdlib>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace easy3d { namespace graphics { namespace internal {
struct VulkanLoader::Impl {
#ifdef _WIN32
    HMODULE module=nullptr;
    ~Impl() {if(module) FreeLibrary(module);}
#else
    void* module=nullptr;
    ~Impl() {if(module) dlclose(module);}
#endif
    PFN_vkGetInstanceProcAddr get=nullptr;
};
VulkanLoader::VulkanLoader() : impl_(new Impl) {}
VulkanLoader::~VulkanLoader() = default;
PFN_vkGetInstanceProcAddr VulkanLoader::get_instance_proc_addr() const {return impl_->get;}
std::shared_ptr<VulkanLoader> VulkanLoader::open() {
    // An explicit override is exclusive: a broken path must not silently load
    // another copy. It also permits deterministic missing-loader process tests.
    const char* override_path=std::getenv("EASY3D_VULKAN_LOADER");
    std::vector<std::string> candidates;
    if(override_path && *override_path) candidates.emplace_back(override_path);
    else {
#ifdef _WIN32
        candidates.emplace_back("vulkan-1.dll");
#elif defined(__APPLE__)
        candidates={"libvulkan.1.dylib","libvulkan.dylib","@loader_path/libvulkan.1.dylib",
                    "/usr/local/lib/libvulkan.1.dylib","/opt/homebrew/lib/libvulkan.1.dylib"};
#else
        candidates={"libvulkan.so.1","libvulkan.so"};
#endif
#ifdef EASY3D_VULKAN_LOADER_HINT
        candidates.emplace_back(EASY3D_VULKAN_LOADER_HINT);
#endif
    }
    std::string errors;
    auto library=std::shared_ptr<VulkanLoader>(new VulkanLoader);
    for(const auto& path:candidates) {
#ifdef _WIN32
        library->impl_->module=LoadLibraryExA(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if(!library->impl_->module) {errors+=path+" (Windows error "+std::to_string(GetLastError())+"); ";continue;}
        library->impl_->get=reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(library->impl_->module,"vkGetInstanceProcAddr"));
#else
        library->impl_->module=dlopen(path.c_str(),RTLD_NOW|RTLD_LOCAL);
        if(!library->impl_->module) {const char* error=dlerror();errors+=path+": "+(error?error:"unknown loader error")+"; ";continue;}
        library->impl_->get=reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(library->impl_->module,"vkGetInstanceProcAddr"));
#endif
        if(!library->impl_->get) throw std::runtime_error("Vulkan loader '"+path+"' does not export vkGetInstanceProcAddr");
        return library;
    }
    throw std::runtime_error("Vulkan loader unavailable: "+errors);
}
namespace {
template<class T> T required(PFN_vkVoidFunction proc,const char* name) {
    if(!proc) throw std::runtime_error(std::string("Vulkan loader/device entry point unavailable: ")+name);
    return reinterpret_cast<T>(proc);
}
}
void VulkanDispatch::load_global(const VulkanLoader& library) {
    vkGetInstanceProcAddr=library.get_instance_proc_addr();
#define EASY3D_VK_LOAD(name) name=required<PFN_##name>(vkGetInstanceProcAddr(VK_NULL_HANDLE,#name),#name);
    EASY3D_VK_GLOBAL(EASY3D_VK_LOAD)
#undef EASY3D_VK_LOAD
}
void VulkanDispatch::load_instance(VkInstance instance,bool surface) {
#define EASY3D_VK_LOAD(name) name=required<PFN_##name>(vkGetInstanceProcAddr(instance,#name),#name);
    EASY3D_VK_INSTANCE(EASY3D_VK_LOAD)
    // Cleanup must be callable even if subsequent device dispatch loading fails.
    EASY3D_VK_LOAD(vkDestroyDevice)
    EASY3D_VK_LOAD(vkDeviceWaitIdle)
    if(surface) {EASY3D_VK_INSTANCE_WSI(EASY3D_VK_LOAD)}
#undef EASY3D_VK_LOAD
}
void VulkanDispatch::load_device(VkDevice device,bool surface) {
#define EASY3D_VK_LOAD(name) name=required<PFN_##name>(vkGetDeviceProcAddr(device,#name),#name);
    EASY3D_VK_DEVICE(EASY3D_VK_LOAD)
    if(surface) {EASY3D_VK_DEVICE_WSI(EASY3D_VK_LOAD)}
#undef EASY3D_VK_LOAD
}
} } }
