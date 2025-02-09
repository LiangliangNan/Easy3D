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
     *      Curvature values can be smoothed. For more details, please refer to the following papers:
     *          - Discrete Differential-Geometry Operators for Triangulated 2-Manifolds. Meyer et al. 2003.
     *          - Restricted Delaunay triangulations and normal cycle. Cohen-Steiner and Morvan. 2003.
     */
    class SurfaceMeshCurvature {
    public:
        /**
         * \brief Constructor that initializes the curvature analysis with a given mesh.
         * \param mesh The surface mesh to analyze.
         */
        explicit SurfaceMeshCurvature(SurfaceMesh *mesh);
        /**
         * \brief Destructor.
         */
        ~SurfaceMeshCurvature();

        /**
         * \brief Computes principle curvature information for each vertex.
         * \param post_smoothing_steps Number of smoothing iterations to apply after computing curvatures.
         * \details Upon finish, the principle curvatures are stored as vertex properties "v:curv-min" and "v:curv-max".
         */
        void analyze(unsigned int post_smoothing_steps = 0);

        /**
         * \brief Computes principle curvature information for each vertex using tensor analysis.
         * \param post_smoothing_steps Number of smoothing iterations to apply after computing curvatures.
         * \param two_ring_neighborhood If true, uses a two-ring neighborhood for the analysis.
         * \details Upon finish, the principle curvatures are stored as vertex properties "v:curv-min" and "v:curv-max".
         */
        void analyze_tensor(unsigned int post_smoothing_steps = 0, bool two_ring_neighborhood = false);

        /**
         * Computes the mean curvature.
         * \attention This function must be called after calling to the "analyze" function.
         */
        void compute_mean_curvature();

        /**
         * Computes the Gaussian curvature.
         * \attention This function must be called after calling to the "analyze" function.
         */
        void compute_gauss_curvature();

        /**
         * \brief Computes the maximum absolute curvature.
         * \attention This function must be called after calling the "analyze" function.
         */
        void compute_max_abs_curvature();

        /**
         * \brief Returns the mean curvature of a given vertex.
         * \param v The vertex for which to return the mean curvature.
         * \return The mean curvature.
         */
        float mean_curvature(SurfaceMesh::Vertex v) const {
            return 0.5f * (min_curvature_[v] + max_curvature_[v]);
        }

        /**
         * \brief Returns the Gaussian curvature of a given vertex.
         * \param v The vertex for which to return the Gaussian curvature.
         * \return The Gaussian curvature.
         */
        float gauss_curvature(SurfaceMesh::Vertex v) const {
            return min_curvature_[v] * max_curvature_[v];
        }

        /**
         * \brief Returns the minimum (signed) curvature of a given vertex.
         * \param v The vertex for which to return the minimum curvature.
         * \return The minimum curvature.
         */
        float min_curvature(SurfaceMesh::Vertex v) const { return min_curvature_[v]; }

        /**
         * \brief Returns the maximum (signed) curvature of a given vertex.
         * \param v The vertex for which to return the maximum curvature.
         * \return The maximum curvature.
         */
        float max_curvature(SurfaceMesh::Vertex v) const { return max_curvature_[v]; }

        /**
         * \brief Returns the maximum absolute curvature of a given vertex.
         * \param v The vertex for which to return the maximum absolute curvature.
         * \return The maximum absolute curvature.
         */
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
