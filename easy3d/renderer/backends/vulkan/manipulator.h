// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_MANIPULATOR_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_MANIPULATOR_H
#include <easy3d/graphics/render_device.h>
namespace easy3d {
class Model;
class Camera;
namespace vulkan {
// Rendering counterpart of Manipulator::draw_frame(). The model's shared
// Manipulator owns its transform; this helper only caches the guide command.
class ManipulatorRenderer {
public:
    graphics::DrawCommand draw_frame(const Model& model,const Camera& camera,float line_width = 1);
private:
    graphics::DrawCommand command_;
    mat4 mvp_;
    int width_ = 0, height_ = 0;
    float line_width_ = 0;
};
} }
#endif
