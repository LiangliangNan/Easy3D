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
#ifndef EASY3D_VIEWER_BACKENDS_GLFW_RUNTIME_H
#define EASY3D_VIEWER_BACKENDS_GLFW_RUNTIME_H
#include <memory>
#include <string>
#include <vector>
struct GLFWwindow;
namespace easy3d {
    namespace graphics {
        namespace internal {
            class VulkanLoader;
        }
    } // namespace graphics
    namespace detail {
        // UI-thread ownership shared by all Easy3D GLFW hosts, across both backends.
        // Hosts destroy their GPU resources and windows before releasing this lease.
        class GlfwRuntime {
        public:
            static std::shared_ptr<GlfwRuntime> acquire(bool require_vulkan = false);
            ~GlfwRuntime();
            static void poll_events();
            static int modifiers(GLFWwindow *);
            std::shared_ptr<graphics::internal::VulkanLoader> vulkan_loader() const {
                return loader_;
            }
            GLFWwindow *opengl_share_window() const;
            void attach_opengl_window(GLFWwindow *window);
            void detach_opengl_window(GLFWwindow *window);
            bool last_opengl_window() const {
                return opengl_windows_.size() == 1;
            }

        private:
            explicit GlfwRuntime(bool require_vulkan);
            GlfwRuntime(const GlfwRuntime &) = delete;
            GlfwRuntime &operator=(const GlfwRuntime &) = delete;
            std::shared_ptr<graphics::internal::VulkanLoader> loader_;
            std::string vulkan_error_;
            std::vector<GLFWwindow *> opengl_windows_;
        };
    } // namespace detail
} // namespace easy3d
#endif
