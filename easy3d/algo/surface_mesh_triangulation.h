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

#ifndef EASY3D_ALGO_SURFACE_MESH_TRIANGULATION_H
#define EASY3D_ALGO_SURFACE_MESH_TRIANGULATION_H


#include <easy3d/core/surface_mesh.h>

#include <vector>
#include <float.h>


namespace easy3d {

    /**
     * \brief Triangulate a polygonal mesh into a pure triangle mesh.
     * \class SurfaceMeshTriangulation easy3d/algo/surface_mesh_triangulation.h
     * \details Tringulate n-gons into n-2 triangles. Find the triangulation that minimizes the sum of squared triangle
     * areas. See the following paper for more details:
     *  - Peter Liepa. Filling holes in meshes. SGP, 2003.
     */
    class SurfaceMeshTriangulation {
    public:

        //! \brief triangulation objective: find the triangulation that minimizes the
        //! sum of squared triangle areas, or the one that maximizes the minimum
        //! angle.
        enum Objective {
            MIN_AREA, MAX_ANGLE
        } objective_;

        SurfaceMeshTriangulation(SurfaceMesh *mesh);

        //! \brief triangulate all faces
        void triangulate(Objective obj = MIN_AREA);

        //! \brief triangulate a particular face f
        void triangulate(SurfaceMesh::Face f, Objective obj = MIN_AREA);

    private:

        // compute the weight of the triangle (i,j,k).
        float compute_weight(int i, int j, int k) const;

        // does edge (a,b) exist?
        bool is_edge(SurfaceMesh::Vertex a, SurfaceMesh::Vertex b) const;

        // does edge (a,b) exist and is non-boundary?
        bool is_interior_edge(SurfaceMesh::Vertex a, SurfaceMesh::Vertex b) const;

        // add edges from vertex i to j
        bool insert_edge(int i, int j);


    private:

        // mesh and properties
        SurfaceMesh *mesh_;
        SurfaceMesh::VertexProperty <vec3> points_;
        std::vector<SurfaceMesh::Halfedge> halfedges_;
        std::vector<SurfaceMesh::Vertex> vertices_;

        // data for computing optimal triangulation
        std::vector<std::vector<float> > weight_;
        std::vector<std::vector<int> > index_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_TRIANGULATION_H