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

#ifndef EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H
#define EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H

#include <string>


namespace easy3d {

    class KdTreeSearch;
    class PointCloud;

    class PointCloudSimplification
    {
    public:

        // @k: number of nearest neighbors.
        static double	average_spacing(PointCloud* cloud, KdTreeSearch* kdtree, int k = 6, bool accurate = false, int samples = 200000);  // using the provided kd-tree
        static double	average_spacing(PointCloud* cloud, int k = 6, bool accurate = false, int samples = 200000);						// construct a new kd-tree

        //----- simplification using a grid (non-uniform) ------------------------------------------------

        // considers a regular grid covering the bounding box of the input point set, and clusters 
        // all points sharing the same cell of the grid by picking as representative one arbitrarily 
        // chosen point.
        // @epsilon (or cell size): tolerance value when merging 3D points.
        // return the indices of points to be deleted.
        static std::vector<int>	grid_simplification(PointCloud* cloud, float epsilon);

        //----- uniform simplification (specifying distance threshold) ------------------------------------

        /**
         * @brief Uniformly downsample a point cloud based on a distance criterion.
         * @param cloud: The input point cloud.
         * @param epsilon: The minimum allowed distance between points. Two points with a distance smaller than this
         *                 value are considered identical. As a result, the distance of any pair of points is >= epsilon.
         * @param kdtree An already constructed kdtree on this point cloud
         * @return The indices of points to be deleted.
         */
        static std::vector<int>	uniform_simplification(PointCloud* cloud, KdTreeSearch* kdtree, float epsilon);
        static std::vector<int>	uniform_simplification(PointCloud* cloud, float epsilon);

        //----- uniform simplification (specifying expected point number) ---------------------------------

        // return the indices of points to be deleted.
        static std::vector<int>	uniform_simplification(PointCloud* cloud, unsigned int num);
    };


} // namespace easy3d


#endif  // EASY3D_ALGO_POINT_CLOUD_SIMPLIFICATION_H

