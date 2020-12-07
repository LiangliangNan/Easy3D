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

#ifndef EASY3D_ALGO_SURFACE_MESH_TOPOLYGY_H
#define EASY3D_ALGO_SURFACE_MESH_TOPOLYGY_H


#include <cstddef>

namespace easy3d {

    class SurfaceMeshComponent;

    /// \brief Compute various topological characteristics of a surface mesh component.
    /// \class SurfaceMeshTopology easy3d/algo/surface_mesh_topology.h
    class SurfaceMeshTopology {
    public:
        SurfaceMeshTopology(const SurfaceMeshComponent *comp);

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

#endif  // EASY3D_ALGO_SURFACE_MESH_TOPOLYGY_H

