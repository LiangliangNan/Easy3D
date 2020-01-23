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
     *      guard.begin();
     *      for (auto p : points)
     *          guard.add_vertex(p);
     *      for (auto ids : faces)
     *          guard.add_face(ids);
     *      guard.finish();
     * ---------------------------------------------------------
     */

    class ManifoldGuard {
    public:
        ManifoldGuard(SurfaceMesh* mesh = nullptr);
        ~ManifoldGuard();

        /**
         * @brief Set the mesh to be reconstructed. This method allow to construct multiple meshes using the same
         *        manifold guard.
         * @param mesh The mesh to be reconstructed.
         */
        void set_mesh(SurfaceMesh* mesh);

        /**
         * @brief Begin surface construction. Must be called at the beginning of the surface construction.
         */
        void begin();

        /**
         * @brief Add a vertex to the mesh.
         * @param p The 3D coordinates of the vertex.
         * @return The vertex on success.
         */
        SurfaceMesh::Vertex add_vertex(const vec3& p);

        /**
         * @brief Add a face to the mesh.
         * @param ids The vertex indices of the mesh.
         * @return The face on success.
         */
        SurfaceMesh::Face   add_face(const std::vector<int>& ids);

        /**
         * @brief Finalize surface construction. Must be called at the end of the surface construction.
         */
        void finish();

    private:
        void begin_face();
        void end_face();

        SurfaceMesh::Vertex copy_vertex(SurfaceMesh::Vertex v);

    private:
        SurfaceMesh* mesh_;

        // faces with less than three vertices
        std::size_t num_faces_less_three_vertices_;

        // faces with duplicated vertices
        std::size_t num_faces_duplicated_vertices_;

        // complex edges (i.e., edges connecting more than two faces)
        std::size_t num_complex_edges_;

        // non-manifold vertices
        std::size_t num_non_manifold_vertices_;

        // isolated vertices
        std::size_t num_isolated_vertices_;

        // the vertices of the current face
        std::vector<SurfaceMesh::Vertex> face_vertices_ ;
    };

}

#endif //EASY3D_MANIFOLD_GUARD_H
