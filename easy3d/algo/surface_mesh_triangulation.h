/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_TRIANGULATION_H
#define EASY3D_ALGO_SURFACE_MESH_TRIANGULATION_H


#include <easy3d/core/surface_mesh.h>

#include <vector>
#include <cfloat>


namespace easy3d {

    /**
     * \brief Triangulate a polygonal mesh into a pure triangle mesh.
     * \class SurfaceMeshTriangulation easy3d/algo/surface_mesh_triangulation.h
     * \details Triangulate n-gons into n-2 triangles. Find the triangulation that minimizes the sum of squared triangle
     * areas. See the following paper for more details:
     *  - Peter Liepa. Filling holes in meshes. SGP, 2003.
     */
    class SurfaceMeshTriangulation {
    public:
        explicit SurfaceMeshTriangulation(SurfaceMesh *mesh);

        //! \brief triangulation objective: find the triangulation that minimizes the
        //! sum of squared triangle areas, or the one that maximizes the minimum
        //! angle.
        enum Objective { MIN_AREA, MAX_ANGLE};

        //! \brief triangulate all faces
        void triangulate(Objective obj = MIN_AREA);

        //! \brief triangulate a particular face f
        void triangulate(SurfaceMesh::Face f, Objective obj = MIN_AREA);

    private:

        // compute the weight of the triangle (i,j,k).
        float compute_weight(int i, int j, int k) const;

        // does edge (a,b) exist?
        bool is_edge(SurfaceMesh::Vertex a, SurfaceMesh::Vertex b) const;

        // add edges from vertex i to j
        bool insert_edge(int i, int j);

    private:
        Objective objective_;

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