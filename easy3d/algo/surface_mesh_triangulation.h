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
     *      areas. See the following paper for more details:
     *          - Peter Liepa. Filling holes in meshes. SGP, 2003.
     */
    class SurfaceMeshTriangulation {
    public:
        /**
         * \brief Construct with the surface mesh to be triangulated.
         * \param mesh The surface mesh to be triangulated.
         */
        explicit SurfaceMeshTriangulation(SurfaceMesh *mesh);

        /**
         * \brief Triangulation objective: find the triangulation that minimizes the sum of squared triangle areas,
         *      or the one that maximizes the minimum angle.
         */
        enum Objective { MIN_AREA, MAX_ANGLE};

        /**
         * \brief Triangulate all faces.
         * \param obj The triangulation objective. Default is MIN_AREA.
         */
        void triangulate(Objective obj = MIN_AREA);

        /**
         * \brief Triangulate a particular face.
         * \param f The face to be triangulated.
         * \param obj The triangulation objective. Default is MIN_AREA.
         */
        void triangulate(SurfaceMesh::Face f, Objective obj = MIN_AREA);

    private:
        /**
         * \brief Compute the weight of the triangle (i, j, k).
         * \param i The first vertex index.
         * \param j The second vertex index.
         * \param k The third vertex index.
         * \return The weight of the triangle.
         */
        float compute_weight(int i, int j, int k) const;

        /**
         * \brief Check if the edge (a, b) exists.
         * \param a The first vertex.
         * \param b The second vertex.
         * \return True if the edge exists, false otherwise.
         */
        bool is_edge(SurfaceMesh::Vertex a, SurfaceMesh::Vertex b) const;

        /**
         * \brief Add edges from vertex i to j.
         * \param i The first vertex index.
         * \param j The second vertex index.
         * \return True if the edge was added successfully, false otherwise.
         */
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