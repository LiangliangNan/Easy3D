// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_CLIPPING_H
#define EASY3D_RENDERER_BACKENDS_CLIPPING_H
#include <easy3d/renderer/render_device.h>
namespace easy3d { namespace graphics { namespace internal {
DrawCommand clip_draw(const DrawCommand& command);
} } }
#endif
