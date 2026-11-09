// SPDX-License-Identifier: GPL-3.0-or-later
/********************************************************************
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
 ********************************************************************/
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_DISPATCH_FUNCTIONS_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_DISPATCH_FUNCTIONS_H
// Required backend entry points, grouped by Vulkan dispatch scope.

#define EASY3D_VK_GLOBAL(X)                                                                                            \
    X(vkCreateInstance)                                                                                                \
    X(vkEnumerateInstanceExtensionProperties)                                                                          \
    X(vkEnumerateInstanceLayerProperties)

#define EASY3D_VK_INSTANCE(X)                                                                                          \
    X(vkDestroyInstance)                                                                                               \
    X(vkEnumeratePhysicalDevices)                                                                                      \
    X(vkGetPhysicalDeviceProperties)                                                                                   \
    X(vkGetPhysicalDeviceFeatures)                                                                                     \
    X(vkGetPhysicalDeviceQueueFamilyProperties)                                                                        \
    X(vkGetPhysicalDeviceMemoryProperties)                                                                             \
    X(vkGetPhysicalDeviceFormatProperties)                                                                             \
    X(vkGetPhysicalDeviceImageFormatProperties)                                                                        \
    X(vkEnumerateDeviceExtensionProperties)                                                                            \
    X(vkCreateDevice)                                                                                                  \
    X(vkGetDeviceProcAddr)

#define EASY3D_VK_INSTANCE_WSI(X)                                                                                      \
    X(vkDestroySurfaceKHR)                                                                                             \
    X(vkGetPhysicalDeviceSurfaceSupportKHR)                                                                            \
    X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)                                                                       \
    X(vkGetPhysicalDeviceSurfaceFormatsKHR)                                                                            \
    X(vkGetPhysicalDeviceSurfacePresentModesKHR)

#define EASY3D_VK_DEVICE(X)                                                                                            \
    X(vkAllocateCommandBuffers)                                                                                        \
    X(vkAllocateDescriptorSets)                                                                                        \
    X(vkAllocateMemory)                                                                                                \
    X(vkBeginCommandBuffer)                                                                                            \
    X(vkBindBufferMemory)                                                                                              \
    X(vkBindImageMemory)                                                                                               \
    X(vkCmdBeginRenderPass)                                                                                            \
    X(vkCmdBindDescriptorSets)                                                                                         \
    X(vkCmdBindIndexBuffer)                                                                                            \
    X(vkCmdBindPipeline)                                                                                               \
    X(vkCmdBindVertexBuffers)                                                                                          \
    X(vkCmdBlitImage)                                                                                                  \
    X(vkCmdCopyBuffer)                                                                                                 \
    X(vkCmdCopyBufferToImage)                                                                                          \
    X(vkCmdCopyImageToBuffer)                                                                                          \
    X(vkCmdDispatch)                                                                                                   \
    X(vkCmdDraw)                                                                                                       \
    X(vkCmdDrawIndexed)                                                                                                \
    X(vkCmdEndRenderPass)                                                                                              \
    X(vkCmdFillBuffer)                                                                                                 \
    X(vkCmdNextSubpass)                                                                                                \
    X(vkCmdPipelineBarrier)                                                                                            \
    X(vkCmdPushConstants)                                                                                              \
    X(vkCmdSetDepthBias)                                                                                               \
    X(vkCmdSetScissor)                                                                                                 \
    X(vkCmdSetViewport)                                                                                                \
    X(vkCreateBuffer)                                                                                                  \
    X(vkCreateCommandPool)                                                                                             \
    X(vkCreateComputePipelines)                                                                                        \
    X(vkCreateDescriptorPool)                                                                                          \
    X(vkCreateDescriptorSetLayout)                                                                                     \
    X(vkCreateFence)                                                                                                   \
    X(vkCreateFramebuffer)                                                                                             \
    X(vkCreateGraphicsPipelines)                                                                                       \
    X(vkCreateImage)                                                                                                   \
    X(vkCreateImageView)                                                                                               \
    X(vkCreatePipelineLayout)                                                                                          \
    X(vkCreateRenderPass)                                                                                              \
    X(vkCreateSampler)                                                                                                 \
    X(vkCreateSemaphore)                                                                                               \
    X(vkCreateShaderModule)                                                                                            \
    X(vkDestroyBuffer)                                                                                                 \
    X(vkDestroyCommandPool)                                                                                            \
    X(vkDestroyDescriptorPool)                                                                                         \
    X(vkDestroyDescriptorSetLayout)                                                                                    \
    X(vkDestroyDevice)                                                                                                 \
    X(vkDestroyFence)                                                                                                  \
    X(vkDestroyFramebuffer)                                                                                            \
    X(vkDestroyImage)                                                                                                  \
    X(vkDestroyImageView)                                                                                              \
    X(vkDestroyPipeline)                                                                                               \
    X(vkDestroyPipelineLayout)                                                                                         \
    X(vkDestroyRenderPass)                                                                                             \
    X(vkDestroySampler)                                                                                                \
    X(vkDestroySemaphore)                                                                                              \
    X(vkDestroyShaderModule)                                                                                           \
    X(vkDeviceWaitIdle)                                                                                                \
    X(vkEndCommandBuffer)                                                                                              \
    X(vkFlushMappedMemoryRanges)                                                                                       \
    X(vkFreeDescriptorSets)                                                                                            \
    X(vkFreeMemory)                                                                                                    \
    X(vkGetBufferMemoryRequirements)                                                                                   \
    X(vkGetDeviceQueue)                                                                                                \
    X(vkGetImageMemoryRequirements)                                                                                    \
    X(vkInvalidateMappedMemoryRanges)                                                                                  \
    X(vkMapMemory)                                                                                                     \
    X(vkQueueSubmit)                                                                                                   \
    X(vkResetCommandBuffer)                                                                                            \
    X(vkResetFences)                                                                                                   \
    X(vkUnmapMemory)                                                                                                   \
    X(vkUpdateDescriptorSets)                                                                                          \
    X(vkWaitForFences)

#define EASY3D_VK_DEVICE_WSI(X)                                                                                        \
    X(vkCreateSwapchainKHR)                                                                                            \
    X(vkDestroySwapchainKHR)                                                                                           \
    X(vkGetSwapchainImagesKHR)                                                                                         \
    X(vkAcquireNextImageKHR)                                                                                           \
    X(vkQueuePresentKHR)

#endif
