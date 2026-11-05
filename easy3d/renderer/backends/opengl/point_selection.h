// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shader_program.h>
namespace easy3d {
class Drawable;
namespace opengl {
// OpenGL counterpart of the Vulkan compute point selector. Regions are evaluated
// in a vertex shader with transform feedback: one invocation owns one 32-bit
// selection word, so no atomics or compute shaders are required on GL 3.2.
// Context-owned realizations. GPU snapshots retain the rendering buffer's storage;
// this owner releases GL objects before context destruction, even for retained snapshots.
class PointSelector {
public:
    ~PointSelector();
    // Resources backed by a live drawable (positions alias the rendering VBO).
    // The resource is refreshed automatically when the drawable's buffers change.
    std::shared_ptr<const graphics::PointData> points(const std::shared_ptr<Drawable>& drawable);
    // Resource uploaded from a CPU snapshot, for models that have no rendering
    // buffers. Positions are indexed by source slot; ids mark selectable slots.
    std::shared_ptr<const graphics::PointData> points(std::shared_ptr<const graphics::PointData> data);
    std::vector<uint32_t> select(const graphics::PointSelectionRequest& request);
private:
    struct Points;
    std::vector<std::shared_ptr<Points>> points_;
    std::unique_ptr<ShaderProgram> shader_;
    unsigned vao_=0, feedback_=0, input_=0, output_=0, polygon_=0, polygon_texture_=0;
};
}}
#endif
