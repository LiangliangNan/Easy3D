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
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_LOADER_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_LOADER_H
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>
#include <memory>
#include "dispatch_functions.h"
namespace easy3d {
    namespace graphics {
        namespace internal {
            // Immutable library/global entry point. Owned by contexts and native window
            // runtimes, never unloaded while Vulkan or toolkit callbacks can still use it.
            class VulkanLoader {
            public:
                static std::shared_ptr<VulkanLoader> open();
                ~VulkanLoader();
                VulkanLoader(const VulkanLoader &) = delete;
                VulkanLoader &operator=(const VulkanLoader &) = delete;
                PFN_vkGetInstanceProcAddr get_instance_proc_addr() const;

            private:
                VulkanLoader();
                struct Impl;
                std::unique_ptr<Impl> impl_;
            };
            // Each context has its own instance/device dispatch. No global device table is
            // overwritten when a second window/device is created or destroyed.
            struct VulkanDispatch {
                PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
#define EASY3D_VK_DECLARE(name) PFN_##name name = nullptr;
                EASY3D_VK_GLOBAL(EASY3D_VK_DECLARE)
                EASY3D_VK_INSTANCE(EASY3D_VK_DECLARE)
                EASY3D_VK_INSTANCE_WSI(EASY3D_VK_DECLARE)
                EASY3D_VK_DEVICE(EASY3D_VK_DECLARE)
                EASY3D_VK_DEVICE_WSI(EASY3D_VK_DECLARE)
#undef EASY3D_VK_DECLARE
                void load_global(const VulkanLoader &library);
                void load_instance(VkInstance instance, bool surface);
                void load_device(VkDevice device, bool surface);
            };
        } // namespace internal
    } // namespace graphics
} // namespace easy3d
#endif
