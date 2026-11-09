/** \cond LICENSE
 * SPDX-License-Identifier: GPL-3.0-or-later
 \endcond */
#ifndef EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#define EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>

namespace easy3d {
    /**
     * \brief CPU-side animation state for the stretching-bunny demo.
     *
     * The Viewer calls update() once per animated frame (via animation_func_). The helper
     * owns the current scale factor and a scratch copy of the vertex positions, deforms the
     * bunny, and republishes the positions through the drawable's dynamic vertex buffer.
     */
    // The same CPU animation feeds either rendering backend. Recompute from the
    // original positions to avoid accumulating roundoff over repeated cycles.
    class BunnyAnimation {
    public:
        // One animation step: advance the scale, rebuild the deformed vertices from the
        // rest pose, and upload them. Returns true so the viewer redraws the frame.
        bool update(TrianglesDrawable &surface) {
            // Grow the Z scale by 0.2% per frame; once it passes 1.5, restart at 1.0.
            scale_ = scale_ > 1.5f ? 1.0f : scale_ * 1.002f;
            // Always start from the pristine bunny vertices, never from last frame's result.
            vertices_ = resource::bunny_vertices;
            for (auto &vertex : vertices_)
                vertex.z *= scale_;
            // Publish the new positions as a dynamic buffer snapshot; the active backend
            // (OpenGL or Vulkan) uploads it before rendering the next frame.
            surface.update_vertex_buffer(vertices_, true);
            return true;
        }
        // The current stretch factor (1.0 = rest pose).
        float scale() const {
            return scale_;
        }

    private:
        float scale_ = 1.0f;         //!< Current Z stretch factor, advanced once per frame.
        std::vector<vec3> vertices_; //!< Scratch storage for the deformed positions.
    };
} // namespace easy3d
#endif
