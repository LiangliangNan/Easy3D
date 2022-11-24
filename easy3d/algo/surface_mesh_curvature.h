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

#ifndef EASY3D_ALGO_SURFACE_MESH_CURVATURE_H
#define EASY3D_ALGO_SURFACE_MESH_CURVATURE_H

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    /**
     * \brief Compute per-vertex curvatures, i.e., principle (min, max), mean, Gaussian.
     *
     * \class SurfaceMeshCurvature easy3d/algo/surface_mesh_curvature.h
     *
     * \details Curvature values for boundary vertices are interpolated from their interior neighbors.
     * Curvature values can be smoothed. For more details, please refer to the following papers:
     *    - Discrete Differential-Geometry Operators for Triangulated 2-Manifolds. Meyer et al. 2003.
     *    - Restricted Delaunay triangulations and normal cycle. Cohen-Steiner and Morvan. 2003.
     */
    class SurfaceMeshCurvature {
    public:
        explicit SurfaceMeshCurvature(SurfaceMesh *mesh);

        ~SurfaceMeshCurvature();

        /**
         * Computes principle curvature information for each vertex, optionally followed by some smoothing iterations
         * of the curvature values. Upon finish, the principle curvatures are stored as vertex properties "v:curv-min"
         * and "v:curv-max", respectively.
         */
        void analyze(unsigned int post_smoothing_steps = 0);

        /**
         * Computes principle curvature information for each vertex, optionally followed by some smoothing iterations
         * of the curvature values. Upon finish, the principle curvatures are stored as vertex properties "v:curv-min"
         * and "v:curv-max", respectively.
         */
        void analyze_tensor(unsigned int post_smoothing_steps = 0, bool two_ring_neighborhood = false);

        /**
         * Computes the mean curvature.
         * @attention This function must be called after calling to the "analyze" function.
         */
        void compute_mean_curvature();

        /**
         * Computes the Gaussian curvature.
         * @attention This function must be called after calling to the "analyze" function.
         */
        void compute_gauss_curvature();

        /**
         * Computes the max. abs. curvature.
         * @attention This function must be called after calling to the "analyze" function.
         */
        void compute_max_abs_curvature();

        //! return mean curvature
        float mean_curvature(SurfaceMesh::Vertex v) const {
            return 0.5f * (min_curvature_[v] + max_curvature_[v]);
        }

        //! return Gaussian curvature
        float gauss_curvature(SurfaceMesh::Vertex v) const {
            return min_curvature_[v] * max_curvature_[v];
        }

        //! return minimum (signed) curvature
        float min_curvature(SurfaceMesh::Vertex v) const { return min_curvature_[v]; }

        //! return maximum (signed) curvature
        float max_curvature(SurfaceMesh::Vertex v) const { return max_curvature_[v]; }

        //! return maximum absolute curvature
        float max_abs_curvature(SurfaceMesh::Vertex v) const {
            return std::max(fabs(min_curvature_[v]), fabs(max_curvature_[v]));
        }

    private:
        //! smooth curvature values
        void smooth_curvatures(unsigned int iterations);

    private:
        SurfaceMesh *mesh_;
        SurfaceMesh::VertexProperty<float> min_curvature_;
        SurfaceMesh::VertexProperty<float> max_curvature_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_CURVATURE_H
