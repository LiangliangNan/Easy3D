// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_OVERLAY_RENDERER_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_OVERLAY_RENDERER_H
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <functional>
namespace easy3d { namespace opengl {
// Per-context realization of immutable triangle annotations. Viewer owns GL state isolation.
class OverlayRenderer {
public:
    ~OverlayRenderer();
    void set(std::vector<graphics::DrawCommand> commands, bool foreground);
    void draw(bool foreground, int source_width, int source_height, const ivec4& tile,
              const std::function<void(int,int)>& draw_depth);
private:
    struct Draw {
        graphics::DrawCommand command;
        unsigned vao=0, vertices=0, indices=0, texture=0;
    };
    void clear();
    std::vector<Draw> draws_;
    std::unique_ptr<ShaderProgram> shader_;
    std::unique_ptr<FramebufferObject> depth_, hud_;
    bool foreground_=false;
};
} }
#endif
