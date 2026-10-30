// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_MULTI_VIEWER_H
#define EASY3D_VIEWER_MULTI_VIEWER_H
#ifdef EASY3D_NO_OPENGL
#error "VulkanMultiViewer is pending migration step 3"
#else
#include <easy3d/viewer/viewer.h>
namespace easy3d { using MultiViewer = OpenGLMultiViewer; }
#endif
#endif
