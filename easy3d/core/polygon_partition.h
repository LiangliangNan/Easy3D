/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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

#ifndef EASY3D_CORE_POLYGON_PARTITION_H
#define EASY3D_CORE_POLYGON_PARTITION_H


#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    /// \brief Convex partition of a general 2D polygon.
    /// \class GenericPolygon easy3d/core/polygon.h
    class PolygonPartition {
    public:
        /// \brief Methods for convex partition of a polygon
        enum Method {
            /**
             * Partitions a polygon into convex polygons by using the Hertel-Mehlhorn algorithm.
             * The algorithm gives at most four times the number of parts as the optimal algorithm.
             * However, in practice it works much better than that and often gives optimal partition.
             * It uses triangulation obtained by ear clipping as intermediate result.
             * Time complexity: O(n^2), where n is the number of vertices.
             * Space complexity: O(n).
             */
            Hertel_Mehlhorn,

            /**
             * Optimal convex partitioning (in terms of number of resulting convex polygons) using the Keil-Snoeyink
             * algorithm. M. Keil, J. Snoeyink, "On the time bound for convex decomposition of simple polygons", 1998.
             * Time complexity: O(n^3), where n is the number of vertices.
             * Space complexity: O(n^3).
             */
            Optimal
        };

        /// An indexed polygon representation (defined by vertex indices).
        typedef std::vector<std::size_t> Polygon;

    public:
        /// \brief Default constructor.
        PolygonPartition() = default;

        /// \brief Perform convex partition for a polygon without holes.
        /// \param poly An input polygon (without holes) to be partitioned. Vertices have to be in counter-clockwise order.
        /// \param parts Resulting list of convex polygons, represented by the vertex indices.
        /// \param method The method to be used.
        /// \return true on success, false on failure.
        bool apply(const std::vector<vec2> &poly,
                   std::vector<Polygon> &parts,
                   Method method = Optimal) const;

        /// \brief Perform convex partition for a general polygon. A general polygon can have an arbitrary number
        ///     of non-hole polygons and an arbitrary number of holes).
        /// \details This function partitions a list of polygons into convex parts by using the Hertel-Mehlhorn
        ///     algorithm. The algorithm gives at most four times the number of parts as the optimal algorithm.
        ///     However, in practice it works much better than that and often gives optimal partition.
        ///         Time complexity: O(n^2), where n is the number of vertices.
        ///         Space complexity: O(n).
        /// \param points A set of points.
        /// \param polys A set of non-hole polygons (each represented by the vertex indices). The vertices of all
        ///     non-hole polygons have to be in counter-clockwise order.
        /// \param holes A set of holes (represented by the vertex indices). The vertices of all hole polygons
        ///     have to be in clockwise order.
        /// \param parts Resulting list of convex polygons (represented by the vertex indices).
        /// \return true on success, false on failure.
        bool apply(const std::vector<vec2> &points,
                   const std::vector<Polygon>& polys,
                   const std::vector<Polygon> &holes,
                   std::vector<Polygon> &parts) const;
    };

}


#endif  // EASY3D_CORE_POLYGON_PARTITION_H
