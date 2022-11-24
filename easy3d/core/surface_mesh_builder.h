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

#ifndef EASY3D_CORE_SURFACE_MESH_BUILDER_H
#define EASY3D_CORE_SURFACE_MESH_BUILDER_H


#include <unordered_map>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief A helper class for constructing manifold surface mesh models.
     * \class SurfaceMeshBuilder easy3d/core/surface_mesh_builder.h
     * \details SurfaceMeshBuilder resolves non-manifoldness while building a surface mesh.
     *      It is typically used to load a model from a file (because you don't know if the mesh is manifold or not).
     *      For meshes guaranteed to be manifold, you can also use the built-in add_vertex() and
     *      add_[face/triangle/quad]() functions of SurfaceMesh for their construction.
     * Example use:
     * \code
     *      SurfaceMeshBuilder builder(mesh);
     *      builder.begin_surface();
     *      for_each_vertex:
     *          builder.add_vertex(p);
     *      for_each_face:
     *          builder.add_face(ids); // ids: the vertices of the face
     *      builder.end_surface();
     * \endcode
     */

    class SurfaceMeshBuilder {
    public:
        typedef SurfaceMesh::Vertex         Vertex;
        typedef SurfaceMesh::Halfedge       Halfedge;
        typedef SurfaceMesh::Face           Face;
        
    public:
        explicit SurfaceMeshBuilder(SurfaceMesh *mesh);

        ~SurfaceMeshBuilder();

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Begin surface construction. Must be called at the beginning of the surface construction and used in
         *        pair with end_surface() at the end of surface mesh construction.
         * @related end_surface().
        */
        void begin_surface();

        /**
         * @brief Add a vertex to the mesh.
         * @param p The 3D coordinates of the vertex.
         * @return The added vertex on success.
         */
        Vertex add_vertex(const vec3 &p);

        /**
         * @brief Add a face to the mesh.
         * @param vertices The vertices of the face.
         * @return The added face on success.
         * @related add_triangle(), add_quad().
         */
        Face add_face(const std::vector<Vertex> &vertices);

        /**
         * @brief Add a new triangle face connecting vertices v1, v2, and v3.
         * @return The added face on success.
         * @related add_face(), add_quad().
         */
        Face add_triangle(Vertex v1, Vertex v2, Vertex v3);

        /**
         * @brief Add a new quad face connecting vertices v1, v2, v3, and v4.
         * @return The added face on success.
         * @related add_face(), add_triangle().
         */
        Face add_quad(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

        /**
         * @brief Finalize surface construction. Must be called at the end of the surface construction and used in
         *        pair with begin_surface() at the beginning of surface mesh construction.
         * @param log_issues True to log the issues detected and a report on the process of the issues to the log file.
         * @related begin_surface().
         */
        void end_surface(bool log_issues = true);

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief The actual vertices of the previously added face. The order of the vertices are the same as those
         *        provided to add_[face/triangle/quad]() for the construction of the face.
         * @attention The result is valid if the face was successfully added, and it will remain valid until the next
         *            call to add_[face/triangle/quad]() and end_surface().
         */
        const std::vector<Vertex> &face_vertices() const { return face_vertices_; }

    private:

        // A face (without duplicating a vertex) cannot be added to a SurfaceMesh if
        //  - it has less than 3 vertices, or
        //  - it has self duplicated vertices, or
        //  - one of the vertex is out-of-range.
        bool vertices_valid(const std::vector<Vertex> &vertices);

        // Copy a vertex v and its attributes.
        // Return the new vertex.
        Vertex copy_vertex(Vertex v);

        // A vertex might have been copied a few times. If copies occurred before, the original vertex will never work.
        // To avoid unnecessary duplication, we reuse one of its copy that is not on a closed disk. We test each copy in
        // the order the copies were made. If no valid copy can be found, we make a new copy.
        // If no copy exists and v is on a closed disk, we simply copy it.
        Vertex get(Vertex v);

        // Resolve all non-manifold vertices of a mesh.
        // Return the number of non-manifold vertices.
        std::size_t resolve_non_manifold_vertices(SurfaceMesh *mesh);

        // Vertices might be copied, for two reasons:
        //  - resolve non-manifoldness. In two phases: during the construction of the mesh by call to 'add_face()' and
        //    in 'resolve_non_manifold_vertices()'.
        //  - ensure boundary consistency. All happen during the construction of the mesh by call to 'add_face()'.
        //
        // The copied vertices: vertices in 'second' were copied from 'first'.
        // Usually only a small number of vertices will be copied, so no need to use vertex property.
        typedef std::unordered_map<Vertex, std::vector<Vertex>, Vertex::Hash> CopyRecord;

        // Resolve the non-manifoldness of a vertex that is denoted by an incoming halfedge.
        // @param h The halfedge pointing to the non-manifold vertex.
        // Return the number of vertex copies.
        std::size_t resolve_non_manifold_vertex(Halfedge h, SurfaceMesh *mesh, CopyRecord &copy_record);

    private:
        SurfaceMesh *mesh_;

        // faces with less than three vertices
        std::size_t num_faces_less_three_vertices_;

        // faces with duplicate vertices
        std::size_t num_faces_duplicate_vertices;

        // faces with out-of-range vertex indices
        std::size_t num_faces_out_of_range_vertices_;

        // faces with unknown topology
        std::size_t num_faces_unknown_topology_;

        // record for linking a face to the mesh
        CopyRecord copied_vertices_for_linking_;
        // all copy record
        CopyRecord copied_vertices_;

        // The actual vertices after the face was successfully added to the mesh.
        std::vector<Vertex> face_vertices_;

        // A vertex property to record the original vertex of each vertex.
        SurfaceMesh::VertexProperty <Vertex> original_vertex_;

        // The record of all halfedges (each associated with a valid face) originating from a vertex.
        // This is used for fast query of duplicate edges. All vertices are their original indices.
        //  - first: the index of a vertex
        //  - second: the indices of the target vertices
        std::unordered_map<int, std::vector<int> > outgoing_halfedges_;
    };

}

#endif // EASY3D_CORE_SURFACE_MESH_BUILDER_H
