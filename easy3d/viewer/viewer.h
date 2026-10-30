// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VIEWER_H
#define EASY3D_VIEWER_VIEWER_H
#if defined(EASY3D_TUTORIAL_VULKAN) && !defined(EASY3D_NO_OPENGL)
#error "Tutorial backend overrides are unsupported; configure Easy3D_RENDERER_BACKEND=VULKAN"
#endif
#ifdef EASY3D_NO_OPENGL
#define EASY3D_TUTORIAL_USES_VULKAN 1
#ifndef EASY3D_HAS_VULKAN_VIEWER
#error "Interactive Viewer requires Easy3D_ENABLE_VULKAN_VIEWER=ON"
#endif
#include <easy3d/viewer/vulkan_viewer.h>
namespace easy3d { using Viewer = VulkanViewer; }
#else
#define EASY3D_TUTORIAL_USES_VULKAN 0
#include <easy3d/viewer/opengl_viewer.h>
namespace easy3d { using Viewer = OpenGLViewer; }
#endif
#endif
