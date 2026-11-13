/** \cond LICENSE
* Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */


#ifndef EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#define EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>

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
    bool update(easy3d::TrianglesDrawable &surface) {
        // Grow the Z scale by 0.2% per frame; once it passes 1.5, restart at 1.0.
        scale_ = scale_ > 1.5f ? 1.0f : scale_ * 1.002f;
        // Always start from the pristine bunny vertices, never from last frame's result.
        vertices_ = easy3d::resource::bunny_vertices;
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
    float scale_ = 1.0f; //!< Current Z stretch factor, advanced once per frame.
    std::vector<easy3d::vec3> vertices_; //!< Scratch storage for the deformed positions.
};


#endif