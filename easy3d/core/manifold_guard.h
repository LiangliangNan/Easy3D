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

#include <unordered_map>


namespace easy3d {

    /**
     * A manifold guard resolves non-manifoldness when constructing a surface mesh.
     *
     * TODO: check duplicated faces?
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
         * @brief Set the mesh to be reconstructed. This method allow constructing multiple meshes using the same
         *        manifold guard.
         * @param mesh The mesh to be reconstructed.
         */
        void set_mesh(SurfaceMesh* mesh) { mesh_ = mesh; }

        /**
         * @brief Begin surface construction. Must be called at the beginning of the surface construction.
         */
        void begin();

        /**
         * @brief Add a vertex to the mesh.
         * @param p The 3D coordinates of the vertex.
         * @return The added vertex on success.
         */
        SurfaceMesh::Vertex add_vertex(const vec3& p);

        /**
         * @brief Add a face to the mesh.
         * @param vertices The vertices of the face.
         * @return The added face on success.
         */
        SurfaceMesh::Face   add_face(const std::vector<SurfaceMesh::Vertex>& vertices);

        /**
         * @brief Query the actual vertices of the added face. The order remains the same as the 'ids' when
         *        constructing this face.
         */
        std::vector<SurfaceMesh::Vertex> face_vertices() const { return face_vertices_; }

        /**
         * @brief Finalize surface construction. Must be called at the end of the surface construction.
         */
        void finish();

    private:

        SurfaceMesh::Vertex copy_vertex(SurfaceMesh::Vertex v);

        // The edge (face_vertices_[s] -> face_vertices_[t]) of the current face causes a non-manifold edge.
        // This method tries to resolve it by using the copied vertices. If it work, assign the good edge vertices.
        // If existing copies still don't work, it makes new copies.
        void resolve_non_manifold_edge(std::size_t s, std::size_t t);

        // Will adding the halfedge (s -> t) result in a complex edge?
        // Return true if the edge does not exist or if it is a boundary (i.e., the face is NULL).
        bool halfedge_is_legel(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) const;

    private:
        SurfaceMesh* mesh_;

        // faces with less than three vertices
        std::size_t num_faces_less_three_vertices_;

        // faces with duplicated vertices
        std::size_t num_faces_duplicated_vertices_;

        // non-manifold vertices
        std::size_t num_non_manifold_vertices_;

        // isolated vertices
        std::size_t num_isolated_vertices_;

        // complex faces that couldn't be added to the mesh (unknow known reason)
        std::size_t num_unknown_complex_faces_;

        // the input vertices of the current face
        std::vector<SurfaceMesh::Vertex> input_face_vertices_ ;
        // the vertices of the current face after resolving complex edges and vertices
        std::vector<SurfaceMesh::Vertex> face_vertices_ ;

        // the copied vertices: vertices in 'second' were copied from 'first'
        std::unordered_map<SurfaceMesh::Vertex, std::vector<SurfaceMesh::Vertex>, SurfaceMesh::Vertex::Hash > copied_vertices_;

        // the copied edges: vertices in 'second' were copied from 'first'
        std::vector< std::pair<SurfaceMesh::Halfedge, SurfaceMesh::Halfedge> > copied_edges_;
    };

}

#endif //EASY3D_MANIFOLD_GUARD_H
