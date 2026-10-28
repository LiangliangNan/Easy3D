// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_SHAPE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_SHAPE_H
#include <easy3d/core/rect.h>
#include <easy3d/graphics/render_device.h>

namespace easy3d { namespace vulkan { namespace shape {
// Native counterparts of the screen-space shape drawing helpers. Return
// immutable triangle overlay commands; submission is the host's responsibility.
graphics::DrawCommand draw_quad_filled(const Rect& rect, const vec4& color, unsigned width, unsigned height);
graphics::DrawCommand draw_quad_wire(const Rect& rect, const vec4& color, unsigned width, unsigned height,
                                     float line_width = 1);
graphics::DrawCommand draw_lines_2d(const std::vector<vec2>& segments, const vec4& color,
                                   unsigned width, unsigned height, float line_width = 1);
graphics::DrawCommand draw_lines_3d(const std::vector<vec3>& segments,const std::vector<vec3>& colors,
                                   const mat4& mvp,unsigned width,unsigned height,float line_width = 1);
// Depth-tested RGB box guide. Uses the same create_box() corner geometry as
// OpenGL, expanding homogeneously clipped segments into triangle ribbons.
graphics::DrawCommand draw_box_wire(const mat4& mvp,unsigned width,unsigned height,
                                    float line_width = 1,bool abstracted = false);
} } }
#endif
