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

#ifndef EASY3D_ALGO_POLYGON_PARTITION_H
#define EASY3D_ALGO_POLYGON_PARTITION_H


#include <vector>

#include "easy3d/core/types.h"


namespace easy3d {

    /**
     * \brief Convex partition of polygons.
     * \details The algorithm assumes simply polygons without self-intersections. For complex unknown structures, you
     *      may need to use the CSG operators provided in easy3d/algo/tessellator.h to obtain simply polygons first.
     * \class PolygonPartition easy3d/algo/polygon_partition.h
     */
    class PolygonPartition {
    public:
        /// An indexed polygon representation (defined by vertex indices).
        typedef std::vector<std::size_t> Polygon;

    public:
        /// \brief Default constructor.
        PolygonPartition() = default;

        /**
         * \brief Optimal convex partition (in terms of number of resulting convex polygons) of a polygon into
         *      convex polygons by using the Keil-Snoeyink algorithm.
         * \details The optimal convex partition algorithm is described in
         *      M. Keil, J. Snoeyink, "On the time bound for convex decomposition of simple polygons", 1998.
         *      Time complexity: O(n^3), where n is the number of vertices.
         *      Space complexity: O(n^3).
         * \param poly An input polygon (without holes). Vertices have to be in the counter-clockwise order.
         * \param parts Resulting list of convex polygons, represented by the vertex indices.
         * \return true on success, false on failure.
         * \sa apply_HM, apply.
         */
        static bool apply_OPT(const std::vector<vec2> &poly, std::vector<Polygon> &parts);

        /**
         * \brief Partition a polygon into convex polygons by using the Hertel-Mehlhorn algorithm.
         * \details The algorithm gives at most four times the number of parts as the optimal algorithm.
         *      However, in practice it works much better than that and often gives optimal partition.
         *      Time complexity: O(n^2), where n is the number of vertices.
         *      Space complexity: O(n).
         *      See S. Hertel and K. Mehlhorn. Fast triangulation of simple polygons.
         *          4th Internat. Conf. Found. Comput. Theory, volume 158 of Lecture Notes Comput. Sci.,
         *          pages 207–218. Springer-Verlag, 1983.
         * \param poly An input polygon (without holes). Vertices have to be in the counter-clockwise order.
         * \param parts Resulting list of convex polygons, represented by the vertex indices.
         * \return true on success, false on failure.
         * \sa apply_OPT, apply.
         */
        static bool apply_HM(const std::vector<vec2> &poly, std::vector<Polygon> &parts);

        /**
         * \brief Convex partition of a general polygon with an arbitrary number of non-hole and hole contours.
         * \details This function partitions a list of polygons into convex parts by using the Hertel-Mehlhorn
         *      algorithm. The algorithm gives at most four times the number of parts as the optimal algorithm.
         *      However, in practice it works much better than that and often gives optimal partition.
         *      Time complexity: O(n^2), where n is the number of vertices.
         *      Space complexity: O(n).
         *      See S. Hertel and K. Mehlhorn. Fast triangulation of simple polygons.
         *          4th Internat. Conf. Found. Comput. Theory, volume 158 of Lecture Notes Comput. Sci.,
         *          pages 207–218. Springer-Verlag, 1983.
         * \param points A set of points.
         * \param polys A set of non-hole polygons (each represented by the vertex indices). The vertices of all
         *     non-hole polygons have to be in counter-clockwise order.
         * \param holes A set of holes (represented by the vertex indices). The vertices of all hole polygons
         *     have to be in clockwise order.
         * \param parts Resulting list of convex polygons (represented by the vertex indices).
         * \return true on success, false on failure.
         * \sa apply_OPT, apply_HM.
         */
        static bool apply(const std::vector<vec2> &points,
                          const std::vector<Polygon> &polys,
                          const std::vector<Polygon> &holes,
                          std::vector<Polygon> &parts);
    };

}


#endif  // EASY3D_ALGO_POLYGON_PARTITION_H
