/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_TOPOLOGY_H
#define EASY3D_ALGO_SURFACE_MESH_TOPOLOGY_H


#include <cstddef>

namespace easy3d {

    class SurfaceMeshComponent;

    /// \brief Compute various topological characteristics of a surface mesh component.
    /// \class SurfaceMeshTopology easy3d/algo/surface_mesh_topology.h
    class SurfaceMeshTopology {
    public:
        explicit SurfaceMeshTopology(const SurfaceMeshComponent *comp);

        /**
         * \brief returns the Euler-Poincare characteristic,
         *   1: a disc
         *   2: a sphere
         */
        int euler_poincare() const;

        /** \brief returns 0 for a closed surface. */
        std::size_t number_of_borders() const { return number_of_borders_; }

        /** \brief returns the number of edges in the largest border. */
        std::size_t largest_border_size() const { return largest_border_size_; }

        /** \brief returns if the surface is closed. */
        bool is_closed() const { return number_of_borders_ == 0; }

        /** \brief returns if the surface is topologically equivalent to a sphere. */
        bool is_sphere() const { return (number_of_borders() == 0) && (euler_poincare() == 2); }

        /** \brief returns if the surface is topologically equivalent to a disk. */
        bool is_disc() const { return (number_of_borders() == 1) && (euler_poincare() == 1); }

        /** \brief returns if the surface is topologically equivalent to a cylinder. */
        bool is_cylinder() const { return (number_of_borders() == 2) && (euler_poincare() == 0); }

        /** \brief returns if the surface is topologically equivalent to a torus. */
        bool is_torus() const { return (number_of_borders() == 0) && (euler_poincare() == 0); }

    private:
        const SurfaceMeshComponent *component_;

        std::size_t number_of_borders_;
        std::size_t largest_border_size_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_TOPOLOGY_H

