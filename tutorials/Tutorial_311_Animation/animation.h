// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#define EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>

namespace easy3d {
// The same CPU animation feeds either rendering backend. Recompute from the
// original positions to avoid accumulating roundoff over repeated cycles.
class BunnyAnimation {
public:
    bool update(TrianglesDrawable& surface) {
        scale_ = scale_ > 1.5f ? 1.0f : scale_ * 1.002f;
        vertices_ = resource::bunny_vertices;
        for (auto& vertex : vertices_) vertex.z *= scale_;
        surface.update_vertex_buffer(vertices_, true);
        return true;
    }
    float scale() const { return scale_; }
private:
    float scale_ = 1.0f;
    std::vector<vec3> vertices_;
};
}
#endif
