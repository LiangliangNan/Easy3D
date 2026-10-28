// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_DRAWABLE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_DRAWABLE_H

#include <easy3d/graphics/render_device.h>
#include <memory>
#include <vector>

namespace easy3d {
class Camera;
class Drawable;
namespace vulkan {

// Internal rendering implementation for the shared Drawable classes. Owns CPU
// command/texture caches, not scene objects, cameras, devices or GPU resources.
// One instance per host; immutable command snapshots may outlive the cache.
class DrawableRenderer {
public:
    DrawableRenderer();
    ~DrawableRenderer();
    DrawableRenderer(const DrawableRenderer&) = delete;
    DrawableRenderer& operator=(const DrawableRenderer&) = delete;

    graphics::DrawCommand command(const std::shared_ptr<Drawable>& drawable,
                                  uint32_t id, const Camera& camera,
                                  const std::vector<vec4>& clipping_planes,
                                  bool model_selected = false) const;
    void prune() const;
    void clear();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace vulkan
} // namespace easy3d
#endif
