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

#ifndef EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H
#define EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    /**
     * \brief A class for Laplacian smoothing. See the following papers for more details:
     *  - Mathieu Desbrun et al. Implicit fairing of irregular meshes using diffusion and curvature flow. SIGGRAPH, 1999.
     *  - Misha Kazhdan et al. Can mean‐curvature flow be modified to be non‐singular? CGF, 2012.
     */
    class SurfaceMeshSmoothing {
    public:
        //! \brief Construct with mesh to be smoothed.
        SurfaceMeshSmoothing(SurfaceMesh *mesh);

        // destructor
        ~SurfaceMeshSmoothing();

        //! \brief Perform \p iters iterations of explicit Laplacian smoothing.
        //! Decide whether to use uniform Laplacian or cotan Laplacian (default: cotan).
        void explicit_smoothing(unsigned int iters = 10,
                                bool use_uniform_laplace = false);

        //! \brief Perform implicit Laplacian smoothing with \p timestep.
        //! Decide whether to use uniform Laplacian or cotan Laplacian (default: cotan).
        //! Decide whether to re-center and re-scale model after smoothing (default: true).
        void implicit_smoothing(float timestep = 0.001,
                                bool use_uniform_laplace = false,
                                bool rescale = true);

        //! \brief Initialize edge and vertex weights.
        void initialize(bool use_uniform_laplace = false) {
            compute_edge_weights(use_uniform_laplace);
            compute_vertex_weights(use_uniform_laplace);
        }

    private:
        //! Initialize cotan/uniform Laplace weights.
        void compute_edge_weights(bool use_uniform_laplace);

        //! Initialize cotan/uniform Laplace weights.
        void compute_vertex_weights(bool use_uniform_laplace);

    private:
        //! the mesh
        SurfaceMesh *mesh_;

        // remember for how many vertices/edges we computed weights
        // recompute if numbers change (i.e. mesh has changed)
        unsigned int how_many_edge_weights_;
        unsigned int how_many_vertex_weights_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H