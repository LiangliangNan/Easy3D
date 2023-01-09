#pragma once

#ifndef NDEBUG  // enable VK_EXT_debug_utils in the debug mode
#define VULKAN_DEBUG 1
#endif

#define DEFAULT_FENCE_TIMEOUT 100000000000        // Default fence timeout in nanoseconds

#include <vulkan/vulkan.h>


void checkVulkanResult(VkResult result, const char *file, int line_number, const char *function);

// Macro to check and log Vulkan return results, and it throws an exception if fails.
#define VK_CHECK(result) { checkVulkanResult(result, __FILE__, __LINE__, __FUNCTION__); }
