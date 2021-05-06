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

#ifndef EASY3D_ALGO_SURFACE_MESH_REMESHING_H
#define EASY3D_ALGO_SURFACE_MESH_REMESHING_H

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    class TriangleMeshKdTree;

    /**
     * \brief A class for uniform and adaptive surface remeshing.
     *
     * \class SurfaceMeshRemeshing easy3d/algo/surface_mesh_remeshing.h
     *
     * \details The algorithm implemented here performs incremental remeshing based on edge collapse, split, flip,
     * and tangential relaxation. See the following papers for more details:
     *  - Mario Botsch and Leif Kobbelt. A remeshing approach to multiresolution modeling. SGP, 2004.
     *  - Marion Dunyach et al. Adaptive remeshing for real-time mesh deformation. EG (Short Papers) 2013.
     */
    class SurfaceMeshRemeshing {
    public:
        SurfaceMeshRemeshing(SurfaceMesh *mesh);

        ~SurfaceMeshRemeshing();

        //! \brief uniform remeshing with target edge length
        void uniform_remeshing(float edge_length, unsigned int iterations = 10,
                               bool use_projection = true);

        //! \brief adaptive remeshing with min/max edge length and approximation error
        void adaptive_remeshing(float min_edge_length, float max_edge_length,
                                float approx_error, unsigned int iterations = 10,
                                bool use_projection = true);

    private:
        void preprocessing();
        void postprocessing();

        void split_long_edges();
        void collapse_short_edges();
        void flip_edges();
        void tangential_smoothing(unsigned int iterations);

        void remove_caps();

        void project_to_reference(SurfaceMesh::Vertex v);

        bool is_too_long(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) const {
            return distance(points_[v0], points_[v1]) >
                   4.0 / 3.0 * std::min(vsizing_[v0], vsizing_[v1]);
        }

        bool is_too_short(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) const {
            return distance(points_[v0], points_[v1]) <
                   4.0 / 5.0 * std::min(vsizing_[v0], vsizing_[v1]);
        }

    private:
        SurfaceMesh *mesh_;
        SurfaceMesh *refmesh_;

        bool use_projection_;
        TriangleMeshKdTree *kd_tree_;

        bool uniform_;
        float target_edge_length_;
        float min_edge_length_;
        float max_edge_length_;
        float approx_error_;

        SurfaceMesh::VertexProperty <vec3> points_;
        SurfaceMesh::VertexProperty <vec3> vnormal_;
        SurfaceMesh::VertexProperty<bool> vfeature_;
        SurfaceMesh::EdgeProperty<bool> efeature_;
        SurfaceMesh::VertexProperty<bool> vlocked_;
        SurfaceMesh::EdgeProperty<bool> elocked_;
        SurfaceMesh::VertexProperty<float> vsizing_;

        SurfaceMesh::VertexProperty <vec3> refpoints_;
        SurfaceMesh::VertexProperty <vec3> refnormals_;
        SurfaceMesh::VertexProperty<float> refsizing_;
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_REMESHING_H