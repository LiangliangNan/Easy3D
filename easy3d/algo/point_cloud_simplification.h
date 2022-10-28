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

#ifndef EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H
#define EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H


#include <vector>

#include <easy3d/core/point_cloud.h>


namespace easy3d {

    class KdTreeSearch;

    /// \brief PointCloudSimplification provides various point cloud simplification algorithms.
    /// \class PointCloudSimplification easy3d/algo/point_cloud_simplification.h
    class PointCloudSimplification {
    public:

        /**
         * \brief Query the average spacing of a point cloud.
         * @param cloud The point cloud.
         * @param k The number of nearest points used.
         * @param accurate True to use every point to get an accurate calculation; false to obtain aa approximate
         *                 measure, which uses only a subset (i.e., less than samples) of the points.
         * @param samples  Use less than this number of points for the calculation.
         * @param kdtree   A kdtree defined on this point cloud. If null, a new kdtree will be built and used.
         * @return The average space of the point cloud.
         */
        static float
        average_space(PointCloud *cloud, KdTreeSearch *kdtree = nullptr, int k = 6, bool accurate = false,
                      int samples = 10000
        );

        //----- simplification using a grid (non-uniform) ------------------------------------------------

        /**
         * \brief Simplification of a point cloud using a regular grid covering the bounding box of the points. Simplification
         * is done by keeping a representative point (chosen arbitrarily) for each cell of the grid. This is non-uniform
         * simplification since the representative point is chosen arbitrarily.
         * @param cloud The point cloud.
         * @param cell_size The size of the cells of the grid.
         * @return The indices of points to be deleted.
         */
        static std::vector<PointCloud::Vertex> grid_simplification(PointCloud *cloud, float cell_size);

        //----- uniform simplification (specifying distance threshold) ------------------------------------

        /**
         * @brief Uniformly downsample a point cloud based on a distance criterion. This function can also be used for
         *        removing duplicate points of a point cloud.
         * @param cloud: The point cloud.
         * @param epsilon: The minimum allowed distance between points. Two points with a distance smaller than this
         *                 value are considered identical. After simplification, the distance of any point pair is
         *                 larger than this value.
         * @param kdtree   A kdtree defined on this point cloud. If null, a new kdtree will be built and used.
         * @return The indices of points to be deleted.
         */
        static std::vector<PointCloud::Vertex>
        uniform_simplification(PointCloud *cloud, float epsilon, KdTreeSearch *kdtree = nullptr);

        //----- uniform simplification (specifying expected point number) ---------------------------------

        /**
         * @brief Uniformly downsample a point cloud given the expected point number.
         * @param cloud: The point cloud.
         * @param num:   The expected point number, which must be less than or equal to the original point number.
         * @return The indices of points to be deleted.
         */
        static std::vector<PointCloud::Vertex> uniform_simplification(PointCloud *cloud, unsigned int num);
    };


} // namespace easy3d


#endif  // EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H

