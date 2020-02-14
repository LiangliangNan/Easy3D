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

#ifndef EASY3D_KD_TREE_SEARCH_FLANN_H
#define EASY3D_KD_TREE_SEARCH_FLANN_H

#include <easy3d/kdtree/kdtree_search.h>


namespace easy3d {

    class PointCloud;

    class KdTreeSearch_FLANN : public KdTreeSearch  {
    public:
        KdTreeSearch_FLANN();
        virtual ~KdTreeSearch_FLANN();

        // 'checks' specifies the maximum leafs to visit when searching for neighbors.
        // A higher value for this parameter would give better search precision, but also
        // take more time. For all leafs to be checked use the value FLANN_CHECKS_UNLIMITED.
        // If automatic configuration was used when the index was created, the number
        // of checks required to achieve the specified precision was also computed, to use
        // that value specify FLANN_CHECKS_AUTOTUNED.
        // The default value is FLANN_CHECKS_AUTOTUNED. (Liangliang: 32 is also a good value).
        void set_checks(int chk);

        //______________ tree construction __________________________

        virtual void begin() ;
        virtual void add_point_cloud(PointCloud* cloud) ;
        virtual void end() ;

        //________________ closest point ____________________________

        // NOTE: *squared* distance is returned
        virtual int find_closest_point(const vec3& p, float& squared_distance) const ;
        virtual int find_closest_point(const vec3& p) const ;

        //_________________ K-nearest neighbors ____________________

        // NOTE: *squared* distances are returned
        virtual void find_closest_k_points(
            const vec3& p, int k,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
            ) const ;

        virtual void find_closest_k_points(
            const vec3& p, int k,
            std::vector<int>& neighbors
            ) const ;

        //___________________ radius search ___________________________

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius and *squared* distances are returned
        virtual void find_points_in_range(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
        ) const;

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius
        virtual void find_points_in_range(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors
        ) const;


    protected:
        int		points_num_;
        float*	points_; // reference of the original point cloud data

        void*	tree_;
        int		checks_;
    } ;

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_FLANN_H


