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
        explicit SurfaceMeshRemeshing(SurfaceMesh *mesh);

        ~SurfaceMeshRemeshing();

        //! \brief Perform uniform remeshing.
        //! \param edge_length the target edge length.
        //! \param iterations the number of iterations
        //! \param use_projection use back-projection to the input surface
        void uniform_remeshing(float edge_length, unsigned int iterations = 10,
                               bool use_projection = true);

        //! \brief Perform adaptive remeshing.
        //! \param min_edge_length the minimum edge length.
        //! \param max_edge_length the maximum edge length.
        //! \param approx_error the maximum approximation error
        //! \param iterations the number of iterations
        //! \param use_projection use back-projection to the input surface
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
        vec3 minimize_squared_areas(SurfaceMesh::Vertex v);
        vec3 weighted_centroid(SurfaceMesh::Vertex v);
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