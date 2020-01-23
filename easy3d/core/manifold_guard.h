/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_MANIFOLD_GUARD_H
#define EASY3D_MANIFOLD_GUARD_H

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * A manifold guard resolves non-manifoldness when constructing a surface mesh.
     *
     * Usage example:
     * ---------------------------------------------------------
     *      ManifoldGuard guard(mesh);
     *      guard.begin_surface();
     *      for (auto p : points)
     *          guard.add_vertex(p);
     *      for (auto f : faces) {
     *          guard.begin_face();
     *          for (auto id : f)
     *              guard.add_vertex_to_face(id);
     *          guard.end_face();
     *      }
     *      guard.end_suface();
     * ---------------------------------------------------------
     */

    class ManifoldGuard {
    public:
        ManifoldGuard(SurfaceMesh* mesh);
        ~ManifoldGuard();

        void begin_surface();
        SurfaceMesh::Vertex add_vertex(const vec3& p);
        void begin_face();
        SurfaceMesh::Vertex add_vertex_to_face(int id);
        SurfaceMesh::Face end_face();   // returns the constructed face
        void end_suface();

    private:
        SurfaceMesh* mesh_;

    };

}

#endif //EASY3D_MANIFOLD_GUARD_H
