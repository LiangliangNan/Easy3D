// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_CLIPPING_PLANE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_CLIPPING_PLANE_H
#include <easy3d/graphics/render_device.h>
namespace easy3d {
class Camera;
class ClippingPlane;
namespace vulkan {
// Counterpart of ClippingPlane's OpenGL draw/setup methods. Shared CPU state
// remains in ClippingPlane; the host submits these read-only scene annotations.
class ClippingPlaneRenderer {
public:
    static std::vector<vec4> planes(const ClippingPlane& plane);
    std::vector<graphics::DrawCommand> draw(const ClippingPlane& plane, const Camera& camera);
private:
    float radius_ = 0;
    std::shared_ptr<const graphics::Geometry> face_;
    graphics::DrawCommand wire_;
    mat4 wire_mvp_;
    int width_ = 0, height_ = 0;
};
} }
#endif
