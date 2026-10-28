// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_KEY_FRAME_INTERPOLATOR_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_KEY_FRAME_INTERPOLATOR_H
#include <easy3d/graphics/render_device.h>
#include <easy3d/renderer/frame.h>
namespace easy3d {
class Camera;
class KeyFrameInterpolator;
namespace vulkan {
// Native rendering counterpart of KeyFrameInterpolator's path/camera drawing.
// CPU interpolation and playback remain in the shared KeyFrameInterpolator.
class KeyFrameInterpolatorRenderer {
public:
    graphics::DrawCommand draw_path(KeyFrameInterpolator& path,const Camera& camera,float thickness = 2,
                                   const vec4& color = vec4(1,1,.5f,1));
    graphics::DrawCommand draw_cameras(KeyFrameInterpolator& path,const Camera& camera,float camera_width,
                                      const vec4& color = vec4(.2f,.7f,.3f,1));
private:
    std::vector<vec3> path_positions_;
    std::vector<Frame> keyframes_;
    graphics::DrawCommand path_command_,camera_command_;
    mat4 path_mvp_;
    int width_ = 0,height_ = 0;
    float thickness_ = 0,camera_width_ = 0,fov_ = 0,aspect_ = 0;
};
} }
#endif
