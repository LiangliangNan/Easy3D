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

#ifndef EASY3D_MANIFOLD_BUILDER_H
#define EASY3D_MANIFOLD_BUILDER_H

#include <easy3d/core/surface_mesh.h>

#include <map>


namespace easy3d {

    /**
     * ManifoldBuilder is a class that resolves non-manifoldness of a surface mesh. It is typical used to ensure a
     * manifold surface mesh loaded from a file (because you don't know if the mesh is manifold or not).
     * For meshes guaranteed to be manifold (for example, using your algorithm), using ManifoldBuilder is optional.
     * In this case, you can use the built-in 'add_vertex()' and 'add_face()' functions of SurfaceMesh.
     *
     * Example use:
     * ---------------------------------------------------------
     *      ManifoldBuilder builder(mesh);
     *      builder.begin();
     *      for (auto p : points)
     *          builder.add_vertex(p);
     *      for (auto ids : faces)
     *          builder.add_face(ids);
     *      builder.end();
     * ---------------------------------------------------------
     */

    class ManifoldBuilder {
    public:
        ManifoldBuilder(SurfaceMesh* mesh);
        ~ManifoldBuilder();

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
         * @related add_triangle(), add_quad().
         */
        SurfaceMesh::Face add_face(const std::vector<SurfaceMesh::Vertex>& vertices);

        /**
         * @brief Add a new triangle face connecting vertices v1, v2, and v3.
         * @return The added face on success.
         * @related add_face(), add_quad().
         */
        SurfaceMesh::Face add_triangle(SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, SurfaceMesh::Vertex v3);

        /**
         * @brief Add a new quad face connecting vertices v1, v2, v3, and v4.
         * @return The added face on success.
         * @related add_face(), add_triangle().
         */
        SurfaceMesh::Face add_quad(SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, SurfaceMesh::Vertex v3, SurfaceMesh::Vertex v4);

        /**
         * @brief Query the actual vertices of the added face. The order remains the same as those when
         *        constructing the face.
         * @attention You must query the vertices after add_face() and before the next call to add_face().
         */
        const std::vector<SurfaceMesh::Vertex>& face_vertices() const { return face_vertices_; }

    private:

        // A face (without duplicating a vertex) cannot be added to a SurfaceMesh if
        //  - it has less than 3 vertices, or
        //  - it has self duplicated vertices, or
        //  - one of the vertex is out-of-range.
        bool vertices_valid(const std::vector<SurfaceMesh::Vertex> &vertices);

        // Copy a vertex v and its attributes.
        // Return the new vertex.
        SurfaceMesh::Vertex copy_vertex(SurfaceMesh::Vertex v);

        // A vertex might have been copied a few times. If copies occurred before, the original vertex will never work.
        // To avoid unnecessary duplication, we reuse one of its copy that is not on a closed disk. We test each copy in
        // the order the copies were made. If no valid copy can be found, we make a new copy.
        // If no copy exists and v is on a closed disk, we simply copy it.
        SurfaceMesh::Vertex get(SurfaceMesh::Vertex v);

        // Resolve all non-manifold vertices of a mesh.
        // Return the number of non-manifold vertices.
        std::size_t resolve_non_manifold_vertices(SurfaceMesh* mesh);

		// The copied vertices: vertices in 'second' were copied from 'first'.
		// Usually only a small number of vertices will be copied, so no need to use vertex property.
		typedef std::map<SurfaceMesh::Vertex, std::vector<SurfaceMesh::Vertex> > CopyRecord;

        // Resolve the non-manifoldness of a vertex that is denoted by an incoming halfedge.
        // @param h The halfedge pointing to the non-manifold vertex.
        // Return the number of vertex copies.
        std::size_t resolve_non_manifold_vertex(SurfaceMesh::Halfedge h, SurfaceMesh* mesh, CopyRecord& dmap);

    private:
        SurfaceMesh* mesh_;

        // faces with less than three vertices
        std::size_t num_faces_less_three_vertices_;

        // faces with duplicated vertices
        std::size_t num_faces_duplicated_vertices_;

        // faces with out-of-range vertex indices
        std::size_t num_faces_out_of_range_vertices_;

        // faces with unknown topology
        std::size_t num_faces_unknown_topology_;

        // the vertices of the current face after resolving complex edges and vertices
        std::vector<SurfaceMesh::Vertex> face_vertices_ ;

        // A vertex property to record the original vertex of each vertex.
        SurfaceMesh::VertexProperty<SurfaceMesh::Vertex> original_vertex_;

        // The copied vertices: vertices in 'second' were copied from 'first'.
        // Usually only a small number of vertices will be copied, so no need to use vertex property.
        CopyRecord copied_vertices_;

		// The records of the existing halfedges (each associated with a valid face) used
		// for fast query of duplicated edges. All vertices are their original indices.
		// A halfedge is denoted as: v(i) -> v(outgoing_halfedges_[i][j]).
		std::vector< std::vector<int> > outgoing_halfedges_;
    };

}

#endif //EASY3D_MANIFOLD_BUILDER_H
