// SPDX-License-Identifier: GPL-3.0-or-later
/********************************************************************
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
 ********************************************************************/
#ifndef EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#define EASY3D_TUTORIAL_BUNNY_ANIMATION_H
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>

namespace easy3d {
    // The same CPU animation feeds either rendering backend. Recompute from the
    // original positions to avoid accumulating roundoff over repeated cycles.
    class BunnyAnimation {
    public:
        bool update(TrianglesDrawable &surface) {
            scale_ = scale_ > 1.5f ? 1.0f : scale_ * 1.002f;
            vertices_ = resource::bunny_vertices;
            for (auto &vertex : vertices_)
                vertex.z *= scale_;
            surface.update_vertex_buffer(vertices_, true);
            return true;
        }
        float scale() const {
            return scale_;
        }

    private:
        float scale_ = 1.0f;
        std::vector<vec3> vertices_;
    };
} // namespace easy3d
#endif
