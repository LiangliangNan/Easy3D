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
         *        After 'begin', a vertex property has been added to record the original vertex of each vertex.
         *        This property is useful when assigning vertex/edge properties afterwards during the construction.
         *        This property will be destroyed after call to 'finish()'.
         * @original_vertex A string giving the name of the original vertex property.
         * @attention This property may be destroyed after call to 'finish()'.
         * @related finish(bool).
         */
        void begin(const std::string& original_vertex = "v:original_vertex");

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
         * @brief Query the actual vertices of the added face. The order remains the same as those when
         *        constructing the face.
         */
        std::vector<SurfaceMesh::Vertex> face_vertices() const { return face_vertices_; }

        /**
         * @brief Finalize the surface construction. Must be called at the end of the surface construction.
         * @clean True to delete the temporal vertex property.
         * @related begin(const std::string&).
         */
        void finish(bool clean = true);

    private:

        // A face (without duplicating a vertex) cannot be added to a SurfaceMesh if it has less than 3 vertices or it
        // has self duplicated vertices.
        bool face_can_be_added(const std::vector<SurfaceMesh::Vertex> &vertices);

        // A halfedge (s -> t) has duplication if
        //  - there exists a previous halfedge that originates from s and points to t, and
        //  - the previous halfedge is boudary (i.e., its face is NULL).
        bool halfedge_has_duplication(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) const;

        // A halfedge (s -> t) is legal if
        //  - the halfedge does not have duplication, and
        //  - the two end points are not on a closed disk.
        bool halfedge_is_legal(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) const;

        // Copy a vertex v and its attributes.
        // Return the new vertex.
        SurfaceMesh::Vertex copy_vertex(SurfaceMesh::Vertex v);

        // A vertex might have been copied a few times. If copies occurred before, the original vertex will never work.
        // To avoid uncessary duplication, we reuse one of its copy that is not on a closed disk. We test each copy in
        // the order the copies were made. If no valid copy can be found, we make a new copy.
        // If no copy exists and v is on a closed disk, we simply copy it.
        SurfaceMesh::Vertex get(SurfaceMesh::Vertex v);

    private:
        SurfaceMesh* mesh_;

        // faces with less than three vertices
        std::size_t num_faces_less_three_vertices_;

        // faces with duplicated vertices
        std::size_t num_faces_duplicated_vertices_;

        // faces with unknown structure
        std::size_t num_faces_unknown_structure_;

        // non-manifold vertices
        std::size_t num_non_manifold_vertices_;

        // isolated vertices
        std::size_t num_isolated_vertices_;

        // the input vertices of the current face
        std::vector<SurfaceMesh::Vertex> face_original_vertices_ ;
        // the vertices of the current face after resolving complex edges and vertices
        std::vector<SurfaceMesh::Vertex> face_vertices_ ;

        // the copied vertices: vertices in 'second' were copied from 'first'
        std::unordered_map<SurfaceMesh::Vertex, std::vector<SurfaceMesh::Vertex>, SurfaceMesh::Vertex::Hash > copied_vertices_;

        // the copied edges: in each pair, the 'second' was copied from the 'first'.
        struct Edge {
            Edge(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) : source(s), target(t) {}
            SurfaceMesh::Vertex source;
            SurfaceMesh::Vertex target;
        };
        std::vector< std::pair<Edge, Edge> > copied_edges_;

        SurfaceMesh::VertexProperty<SurfaceMesh::Vertex> original_vertex_;
    };

}

#endif //EASY3D_MANIFOLD_GUARD_H
