// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_BACKENDS_VIEWER_H
#define EASY3D_VIEWER_BACKENDS_VIEWER_H
#include <easy3d/viewer/viewer_base.h>
namespace easy3d { namespace detail {
std::unique_ptr<ViewerBase> create_opengl_viewer(const std::string&, const ViewerOptions&);
std::unique_ptr<ViewerBase> create_vulkan_viewer(const std::string&, const ViewerOptions&);
} }
#endif
