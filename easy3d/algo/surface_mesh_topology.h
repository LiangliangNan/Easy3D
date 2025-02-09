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
        /**
         * \brief Construct with the surface mesh component to analyze.
         * \param comp The surface mesh component to analyze.
         */
        explicit SurfaceMeshTopology(const SurfaceMeshComponent *comp);

        /**
         * \brief Returns the Euler-Poincare characteristic.
         * \return 1 for a disc, 2 for a sphere.
         */
        int euler_poincare() const;

        /**
         * \brief Returns the number of borders.
         * \return 0 for a closed surface.
         */
        std::size_t number_of_borders() const { return number_of_borders_; }

        /**
         * \brief Returns the number of edges in the largest border.
         * \return The number of edges in the largest border.
         */
        std::size_t largest_border_size() const { return largest_border_size_; }

        /**
         * \brief Returns if the surface is closed.
         * \return True if the surface is closed, false otherwise.
         */
        bool is_closed() const { return number_of_borders_ == 0; }

        /**
         * \brief Returns if the surface is topologically equivalent to a sphere.
         * \return True if the surface is topologically equivalent to a sphere, false otherwise.
         */
        bool is_sphere() const { return (number_of_borders() == 0) && (euler_poincare() == 2); }

        /**
         * \brief Returns if the surface is topologically equivalent to a disc.
         * \return True if the surface is topologically equivalent to a disc, false otherwise.
         */
        bool is_disc() const { return (number_of_borders() == 1) && (euler_poincare() == 1); }

        /**
         * \brief Returns if the surface is topologically equivalent to a cylinder.
         * \return True if the surface is topologically equivalent to a cylinder, false otherwise.
         */
        bool is_cylinder() const { return (number_of_borders() == 2) && (euler_poincare() == 0); }

        /**
         * \brief Returns if the surface is topologically equivalent to a torus.
         * \return True if the surface is topologically equivalent to a torus, false otherwise.
         */
        bool is_torus() const { return (number_of_borders() == 0) && (euler_poincare() == 0); }

    private:
        const SurfaceMeshComponent *component_;

        std::size_t number_of_borders_;
        std::size_t largest_border_size_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_TOPOLOGY_H

