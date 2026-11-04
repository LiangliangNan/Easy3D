// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shader_program.h>
namespace easy3d {
class Drawable;
namespace opengl {
// Context-owned realizations. GPU snapshots retain the rendering buffer's storage;
// this owner releases GL objects before context destruction, even for retained snapshots.
class PointSelector {
public:
    ~PointSelector();
    std::shared_ptr<const graphics::PointData> points(const std::shared_ptr<Drawable>& drawable,
                                                    unsigned long long revision);
    std::vector<uint32_t> select(const graphics::PointSelectionRequest& request);
private:
    struct Points;
    std::vector<std::shared_ptr<Points>> points_;
    std::unique_ptr<ShaderProgram> shader_;
    unsigned vao_=0, feedback_=0, input_=0, output_=0, polygon_=0, polygon_texture_=0;
};
}}
#endif
