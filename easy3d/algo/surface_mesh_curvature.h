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
        SurfaceMeshCurvature(SurfaceMesh *mesh);

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
