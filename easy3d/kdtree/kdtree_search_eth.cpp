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

#include <easy3d/kdtree/kdtree_search_eth.h>
#include <easy3d/core/point_cloud.h>

#include <3rd_party/kdtree/ETH_Kd_Tree/kdTree.h>



#define get_tree(x) (reinterpret_cast<kdtree::KdTree*>(x))

namespace easy3d {

    KdTreeSearch_ETH::KdTreeSearch_ETH(const PointCloud *cloud) : KdTreeSearch(cloud) {
        // prepare data
        points_num_ = int(cloud->n_vertices());
        const std::vector<vec3>& points = cloud->points();
        points_ = const_cast<float *>(points[0].data());

        // create tree
        const int maxBucketSize = 16 ;	// number of points per bucket
        tree_ = new kdtree::KdTree(reinterpret_cast<kdtree::Vector3D*>(points_), points_num_, maxBucketSize);
    }


    KdTreeSearch_ETH::KdTreeSearch_ETH(const std::vector<vec3>& points) : KdTreeSearch(points) {
        // prepare data
        points_num_ = points.size();
        points_ = const_cast<float*>(points[0].data());

        // create tree
        const int maxBucketSize = 16;	// number of points per bucket
        tree_ = new kdtree::KdTree(reinterpret_cast<kdtree::Vector3D*>(points_), points_num_, maxBucketSize);
    }


    KdTreeSearch_ETH::~KdTreeSearch_ETH() {
        delete get_tree(tree_);
        points_ = nullptr;
    }


    int KdTreeSearch_ETH::find_closest_point(const vec3& p) const {
        kdtree::Vector3D v3d( p.x, p.y, p.z );
        get_tree(tree_)->setNOfNeighbours( 1 );
        get_tree(tree_)->queryPosition( v3d );

        int num = get_tree(tree_)->getNOfFoundNeighbours();
        if (num == 1) {
            return get_tree(tree_)->getNeighbourPositionIndex(0);
        } else
            return -1;
    }


    int KdTreeSearch_ETH::find_closest_point(const vec3& p, float& squared_distance) const {
        kdtree::Vector3D v3d( p.x, p.y, p.z );
        get_tree(tree_)->setNOfNeighbours( 1 );
        get_tree(tree_)->queryPosition( v3d );

        int num = get_tree(tree_)->getNOfFoundNeighbours();
        if (num == 1) {
            squared_distance = get_tree(tree_)->getSquaredDistance(0);
            return get_tree(tree_)->getNeighbourPositionIndex(0);
        } else {
            LOG(ERROR) << "no point found";
            return 0;
        }
    }


    void KdTreeSearch_ETH::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors
        )  const {
            kdtree::Vector3D v3d( p.x, p.y, p.z );
            get_tree(tree_)->setNOfNeighbours( k );
            get_tree(tree_)->queryPosition( v3d );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            if (num == k) {
                neighbors.resize(k);
                for (int i=0; i<k; ++i) {
                    neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
                }
            }
    // 		else
    // 			LOG(ERROR) << "less than " << k << " points found";
    }


    void KdTreeSearch_ETH::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const {
            kdtree::Vector3D v3d( p.x, p.y, p.z );
            get_tree(tree_)->setNOfNeighbours( k );
            get_tree(tree_)->queryPosition( v3d );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            if (num == k) {
                neighbors.resize(k);
                squared_distances.resize(k);
                for (int i=0; i<k; ++i) {
                    neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
                    squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
                }
            }
    // 		else
    // 			LOG(ERROR) << "less than " << k << " points found";
    }


    void KdTreeSearch_ETH::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors
        )  const {
            kdtree::Vector3D v3d( p.x, p.y, p.z );
            get_tree(tree_)->queryRange( v3d, squared_radius, true );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            neighbors.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
            }
    }


    void KdTreeSearch_ETH::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const {
            kdtree::Vector3D v3d( p.x, p.y, p.z );
            get_tree(tree_)->queryRange( v3d, squared_radius, true );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            neighbors.resize(num);
            squared_distances.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
                squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
            }
    }


    int KdTreeSearch_ETH::find_points_in_cylinder(
        const vec3& p1, const vec3& p2, float radius,
        std::vector<int>& neighbors, std::vector<float>& squared_distances,
        bool bToLine
        ) const {
            kdtree::Vector3D s( p1.x, p1.y, p1.z );
            kdtree::Vector3D t( p2.x, p2.y, p2.z );
            get_tree(tree_)->queryLineIntersection( s, t, radius, bToLine, true );

            int num = get_tree(tree_)->getNOfFoundNeighbours();

            neighbors.resize(num);
            squared_distances.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
                squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
            }

            return num;
    }


    int KdTreeSearch_ETH::find_points_in_cylinder(
        const vec3& p1, const vec3& p2, float radius,
        std::vector<int>& neighbors,
        bool bToLine
        ) const {
            kdtree::Vector3D s( p1.x, p1.y, p1.z );
            kdtree::Vector3D t( p2.x, p2.y, p2.z );
            get_tree(tree_)->queryLineIntersection( s, t, radius, bToLine, true );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            neighbors.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
            }

            return num;
    }


    int KdTreeSearch_ETH::find_points_in_cone(
        const vec3& eye, const vec3& p1, const vec3& p2, float angle_range,
        std::vector<int>& neighbors, std::vector<float>& squared_distances,
        bool bToLine
        ) const {
            kdtree::Vector3D eye3d( eye.x, eye.y, eye.z );
            kdtree::Vector3D s( p1.x, p1.y, p1.z );
            kdtree::Vector3D t( p2.x, p2.y, p2.z );
            get_tree(tree_)->queryConeIntersection( eye3d, s, t, angle_range, bToLine, true );

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            neighbors.resize(num);
            squared_distances.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
                squared_distances[i] = get_tree(tree_)->getSquaredDistance(i);
            }

            return num;
    }


    int KdTreeSearch_ETH::find_points_in_cone(
        const vec3& eye, const vec3& p1, const vec3& p2, float angle_range,
        std::vector<int>& neighbors,
        bool bToLine
        ) const {
            kdtree::Vector3D eye3d( eye.x, eye.y, eye.z );
            kdtree::Vector3D s( p1.x, p1.y, p1.z );
            kdtree::Vector3D t( p2.x, p2.y, p2.z );
            get_tree(tree_)->queryConeIntersection( eye3d, s, t, angle_range, bToLine, true);

            int num = get_tree(tree_)->getNOfFoundNeighbours();
            neighbors.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = get_tree(tree_)->getNeighbourPositionIndex(i);
            }

            return num;
    }


} // namespace easy3d
