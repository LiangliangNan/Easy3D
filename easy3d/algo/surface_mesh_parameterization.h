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

#ifndef EASY3D_ALGO_SURFACE_MESH_PARAMETERIZATION_H
#define EASY3D_ALGO_SURFACE_MESH_PARAMETERIZATION_H


#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief A class for surface parameterization.
     * \class SurfaceMeshParameterization easy3d/algo/surface_mesh_parameterization.h
     * \details It implements two parameterization methods described in the following papers:
     *  - Bruno Lévy et al. Least squares conformal maps for automatic texture atlas generation. SIGGRAPH, 2002.
     *  - Mathieu Desbrun et al. Intrinsic parameterizations of surface meshes. CGF, 21(3):209–218, 2002.
     */
    class SurfaceMeshParameterization {
    public:
        /**
         * \brief Construct with mesh to be parameterized.
         * \param mesh The surface mesh to be parameterized.
         */
        explicit SurfaceMeshParameterization(SurfaceMesh *mesh);

        /**
         * \brief Compute discrete harmonic parameterization.
         * \param use_uniform_weights Flag to indicate whether to use uniform weights. Default: false.
         */
        void harmonic(bool use_uniform_weights = false);

        /**
         * \brief Compute parameterization based on least squares conformal mapping.
         */
        void lscm();

    private:
        /**
         * \brief Setup boundary constraints: map surface boundary to unit circle.
         * \return True if successful, false otherwise.
         */
        bool setup_boundary_constraints();

        /**
         * \brief Setup boundary: pin the two farthest boundary vertices.
         * \return True if successful, false otherwise.
         */
        bool setup_lscm_boundary();

    private:
        SurfaceMesh *mesh_; //!< The mesh to be parameterized.
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_PARAMETERIZATION_H