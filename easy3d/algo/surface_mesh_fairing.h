/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
        void setup_matrix_row(const SurfaceMesh::Vertex v, SurfaceMesh::VertexProperty<double> vweight,
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