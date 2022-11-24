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

#ifndef EASY3D_ALGO_SURFACE_MESH_FAIRING_H
#define EASY3D_ALGO_SURFACE_MESH_FAIRING_H

#include <easy3d/core/surface_mesh.h>
#include <map>

namespace easy3d {

    /**
     * \brief A class for implicitly fairing a surface mesh.
     * \class SurfaceMeshFairing easy3d/algo/surface_mesh_fairing.h
     * See the following paper for more details:
     *  - Mathieu Desbrun et al. Implicit fairing of irregular meshes using diffusion and curvature flow. SIGGRAPH, 1999.
     */
    class SurfaceMeshFairing {
    public:
        //! Construct with mesh to be processed.
        SurfaceMeshFairing(SurfaceMesh *mesh);

        // destructor
        ~SurfaceMeshFairing();

        //! minimize surface area (class SurfaceFairing::fair(1))
        void minimize_area() { fair(1); }

        //! minimize surface curvature (class SurfaceFairing::fair(2))
        void minimize_curvature() { fair(2); }

        //! compute surface by solving k-harmonic equation
        void fair(unsigned int k = 2);

    private:
        void setup_matrix_row(SurfaceMesh::Vertex v, SurfaceMesh::VertexProperty<double> vweight,
                              SurfaceMesh::EdgeProperty<double> eweight,
                              unsigned int laplace_degree,
                              std::map<SurfaceMesh::Vertex, double> &row);

    private:
        SurfaceMesh *mesh_; //!< the mesh

        // property handles
        SurfaceMesh::VertexProperty <vec3> points_;
        SurfaceMesh::VertexProperty<bool> vselected_;
        SurfaceMesh::VertexProperty<bool> vlocked_;
        SurfaceMesh::VertexProperty<double> vweight_;
        SurfaceMesh::EdgeProperty<double> eweight_;
        SurfaceMesh::VertexProperty<int> idx_;
    };


} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_FAIRING_H