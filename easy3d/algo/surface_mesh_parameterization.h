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
        //! \brief Construct with mesh to be parameterized.
        SurfaceMeshParameterization(SurfaceMesh *mesh);

        //! \brief Compute discrete harmonic parameterization.
        void harmonic(bool use_uniform_weights = false);

        //! \brief Compute parameterization based on least squares conformal mapping.
        void lscm();

    private:
        //! setup boundary constraints: map surface boundary to unit circle
        bool setup_boundary_constraints();

        //! setup boundary: pin the two farthest boundary vertices
        bool setup_lscm_boundary();

    private:
        //! the mesh
        SurfaceMesh *mesh_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_PARAMETERIZATION_H