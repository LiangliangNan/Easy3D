// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VIEWER_H
#define EASY3D_VIEWER_VIEWER_H
#if defined(EASY3D_TUTORIAL_VULKAN) && !defined(EASY3D_HAS_VULKAN)
#error "Tutorial backend overrides are unsupported; configure Easy3D_ENABLE_VULKAN=ON"
#endif
#ifdef EASY3D_HAS_VULKAN
#define EASY3D_TUTORIAL_USES_VULKAN 1
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/viewer/vulkan_viewer.h>
namespace easy3d { using Viewer = VulkanViewer; }
namespace easy3d { using OffScreen = VulkanOffScreen; }
#else
#define EASY3D_TUTORIAL_USES_VULKAN 0
#include <easy3d/viewer/opengl_viewer.h>
#include <easy3d/viewer/opengl_multi_viewer.h>
#include <easy3d/viewer/opengl_offscreen.h>
namespace easy3d { using Viewer = OpenGLViewer; }
namespace easy3d { using MultiViewer = OpenGLMultiViewer; }
namespace easy3d { using OffScreen = OpenGLOffScreen; }
#endif
#endif
