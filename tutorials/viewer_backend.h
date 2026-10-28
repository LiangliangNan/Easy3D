// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIALS_VIEWER_BACKEND_H
#define EASY3D_TUTORIALS_VIEWER_BACKEND_H

// Select the host once for tutorials that support both implementations.
// A dual-backend test can explicitly select the native Vulkan host.
#if defined(EASY3D_NO_OPENGL) || defined(EASY3D_TUTORIAL_VULKAN)
#define EASY3D_TUTORIAL_USES_VULKAN 1
#include <easy3d/viewer/vulkan_viewer.h>
namespace easy3d { using TutorialViewer = VulkanViewer; }
#else
#define EASY3D_TUTORIAL_USES_VULKAN 0
#include <easy3d/viewer/viewer.h>
namespace easy3d { using TutorialViewer = Viewer; }
#endif

#endif // EASY3D_TUTORIALS_VIEWER_BACKEND_H
