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

#ifndef EASY3D_KD_TREE_SEARCH_H
#define EASY3D_KD_TREE_SEARCH_H


#include <vector>
#include <easy3d/core/types.h>


/***********************************************************************
 I have collected several different implementations: ANN, ETH, and FLANN,
 and tried to get the best performance of each implementation. Runtime 
 tests (on Windows, using a Visual C++ 2008, release build) indicated the 
 ETH implementation has the best performance. Bellow is the statistic
 summary of the test on two point clouds.

 Build:  time for constructing the kd-tree.
 Single: time for querying closest vertex (for each point in the point cloud).
 KNN:    time for querying K(=16) closest vertex.
 Radius: time for querying closest vertex within a radius.
 
 Test 1: 362, 271 points (the Stanford bunny). radius = 0.001
 --------------------------------------------------------------------------------------
       Build		 |		Single		   |		KNN			 |		Radius
 --------------------|---------------------|---------------------|---------------------
 ANN    ETH   FLANN  |  ANN   ETH   FLANN  |  ANN	 ETH   FLANN |  ANN	  ETH   FLANN  
 --------------------|---------------------|-------------------- |---------------------
 0.14   0.05   0.12  |  0.17  0.11   0.71  |  1.33   1.0   1.48  |  1.32  1.01  1.51
 --------------------------------------------------------------------------------------
 
 Test 2: 4, 116, 466 points (an noisy urban scan). radius = 0.03
 --------------------------------------------------------------------------------------
       Build		 |		Single		   |		KNN			 |		Radius
 --------------------|---------------------|---------------------|---------------------
 ANN    ETH   FLANN  |  ANN   ETH   FLANN  |  ANN	 ETH   FLANN |  ANN	  ETH   FLANN  
 --------------------|---------------------|-------------------- |---------------------
 2.2   0.76   1.88   |  2.61  1.84  11.8   |  20.8   13.5  22.0  |  8.75  4.79  15.1
 --------------------------------------------------------------------------------------

 TODO: Thread-safe?
 FLANN and nanoflann are thread-safe. Others have not been tested yet.
************************************************************************/


namespace easy3d {

    class PointCloud;

    class KdTreeSearch
    {
    public:
        KdTreeSearch();
        virtual ~KdTreeSearch();

        //______________ tree construction __________________________

        virtual void begin() = 0;
        virtual void add_point_cloud(PointCloud* cloud) = 0;
        virtual void end() = 0;

        //________________ closest point ____________________________

        // NOTE: *squared* distance is returned
        virtual int find_closest_point(const vec3& p, float& squared_distance) const = 0;
        virtual int find_closest_point(const vec3& p) const = 0;

        //_________________ K-nearest neighbors ____________________

        // NOTE: *squared* distances are returned
        virtual void find_closest_k_points(const vec3& p, int k, std::vector<int>& neighbors, std::vector<float>& squared_distances) const = 0;
        virtual void find_closest_k_points(const vec3& p, int k, std::vector<int>& neighbors) const = 0;

        //___________________ radius search __________________________

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius and *squared* distances are returned
        virtual void find_points_in_range(const vec3& p, float squared_radius, std::vector<int>& neighbors, std::vector<float>& squared_distances) const = 0;
        virtual void find_points_in_range(const vec3& p, float squared_radius, std::vector<int>& neighbors) const = 0;
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_H


