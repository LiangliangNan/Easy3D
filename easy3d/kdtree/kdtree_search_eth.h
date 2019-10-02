/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_KD_TREE_SEARCH_ETH_H
#define EASY3D_KD_TREE_SEARCH_ETH_H

#include <easy3d/kdtree/kdtree_search.h>


namespace easy3d {

    class PointCloud;

    class KdTreeSearch_ETH : public KdTreeSearch  {
    public:
        KdTreeSearch_ETH();
        virtual ~KdTreeSearch_ETH();

        //______________ tree construction __________________________

        virtual void begin() ;
        virtual void add_point_cloud(PointCloud* cloud) ;
        virtual void end() ;

        //________________ closest point ____________________________

        // find the closest neighbor.
        // NOTE: *squared* distance is returned
        virtual int find_closest_point(const vec3& p, float& squared_distance) const ;

        // find the closest neighbor.
        virtual int find_closest_point(const vec3& p) const ;

        //_________________ K-nearest neighbors ____________________

        // find the K closest neighbors.
        // NOTE: *squared* distances are returned
        virtual void find_closest_K_points(
            const vec3& p, int k,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
            ) const ;

        // find the K closest neighbors.
        virtual void find_closest_K_points(
            const vec3& p, int k,
            std::vector<int>& neighbors
            ) const ;

        //___________________ radius search __________________________

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius and *squared* distances are returned
        virtual void find_points_in_radius(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
        ) const;

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius
        virtual void find_points_in_radius(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors
        ) const;

        //____________________ cylinder range search _________________

        // Search for the nearest points whose distances to line segment $v1$-$v2$ are smaller
        // than $radius$. If $bToLine$ is true, the points found are ordered by their distances
        // to the line segment. Otherwise, they are ordered by their distances to $v1$.
        // NOTE: the range must be *radius* (not squared) and *squared* distances are returned
        int find_points_in_cylinder(
            const vec3& p1, const vec3& p2, float radius,
            std::vector<int>& neighbors, std::vector<float>& squared_distances,
            bool bToLine = true
            ) const ;

        // Search for the nearest points whose distances to line segment $v1$-$v2$ are smaller
        // than $radius$. If $bToLine$ is true, the points found are ordered by their distances
        // to the line segment. Otherwise, they are ordered by their distances to $v1$.
        // NOTE: the range must be *radius* (not squared)
        int find_points_in_cylinder(
            const vec3& p1, const vec3& p2, float radius,
            std::vector<int>& neighbors,
            bool bToLine = true
            ) const ;

        //_______________________ cone range search __________________

        // Search for the nearest points $p_i$ with an cone from $v1$ to $v2$ defined by v1 and v2.
        // As a result, the angle between $v1-p_i$ and $v1-v2$ is smaller than $angle_range$.
        // NOTE: angle is in radian and *squared* distances are returned
        int find_points_in_cone(
            const vec3& eye, const vec3& p1, const vec3& p2, float angle_range,
            std::vector<int>& neighbors, std::vector<float>& squared_distances,
            bool bToLine = true
            ) const ;

        // Search for the nearest points $p_i$ with an cone from $v1$ to $v2$ defined by v1 and v2.
        // As a result, the angle between $v1-p_i$ and $v1-v2$ is smaller than $angle_range$.
        // NOTE: angle is in radian
        int find_points_in_cone(
            const vec3& eye, const vec3& p1, const vec3& p2, float angle_range,
            std::vector<int>& neighbors,
            bool bToLine = true
            ) const ;

    protected:
        int		points_num_;
        float*	points_; // reference of the original point cloud data

        void*	tree_;
    } ;

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_ETH_H


