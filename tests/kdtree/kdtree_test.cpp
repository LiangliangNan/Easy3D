// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/kdtree/kdtree_search_ann.h>
#include <easy3d/kdtree/kdtree_search_eth.h>
#include <easy3d/kdtree/kdtree_search_flann.h>
#include <easy3d/kdtree/kdtree_search_nanoflann.h>
#include <algorithm>
#include <cmath>
using namespace easy3d;
template<class Tree> void check(const PointCloud& cloud) {
    Tree tree(&cloud);float squared=-1;
    test::require(tree.find_closest_point(vec3(.1f,0,0),squared)==0 && std::abs(squared-.01f)<1e-5f,"nearest point/distance differs");
    std::vector<int> ids;tree.find_closest_k_points(vec3(.1f,0,0),2,ids);std::sort(ids.begin(),ids.end());
    test::require(ids==std::vector<int>({0,1}),"k-nearest set differs");
    tree.find_points_in_range(vec3(0),1.1f,ids);std::sort(ids.begin(),ids.end());
    test::require(ids==std::vector<int>({0,1}),"radius query set differs");
}
int main() { return test::run("kd-tree backends against known neighbors",[] {
    PointCloud cloud;cloud.add_vertex(vec3(0));cloud.add_vertex(vec3(1,0,0));cloud.add_vertex(vec3(3,0,0));cloud.add_vertex(vec3(0,4,0));
    check<KdTreeSearch_ANN>(cloud);check<KdTreeSearch_ETH>(cloud);check<KdTreeSearch_FLANN>(cloud);check<KdTreeSearch_NanoFLANN>(cloud);
}); }
