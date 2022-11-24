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

#ifndef EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H
#define EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    /**
     * \brief A class for Laplacian smoothing.
     * \class SurfaceMeshSmoothing easy3d/algo/surface_mesh_smoothing.h
     * See the following papers for more details:
     *  - Mathieu Desbrun et al. Implicit fairing of irregular meshes using diffusion and curvature flow. SIGGRAPH, 1999.
     *  - Misha Kazhdan et al. Can mean‐curvature flow be modified to be non‐singular? CGF, 2012.
     */
    class SurfaceMeshSmoothing {
    public:
        //! \brief Construct with mesh to be smoothed.
        explicit SurfaceMeshSmoothing(SurfaceMesh *mesh);

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

        // remember for how many edges we computed weights
        // recompute if numbers change (i.e. mesh has changed)
        unsigned int how_many_edge_weights_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_SMOOTHING_H