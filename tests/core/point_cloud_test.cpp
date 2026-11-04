// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <memory>
using namespace easy3d;
int main() { return test::run("point cloud properties, deletion and I/O",[] {
    PointCloud cloud;auto color=cloud.add_vertex_property<vec3>("v:color");
    for(int i=0;i<10;++i) {auto v=cloud.add_vertex(vec3(float(i),0,0));color[v]=vec3(1,0,0);}
    test::require(cloud.n_vertices()==10,"vertex insertion failed");
    PointCloud copy(cloud);cloud.delete_vertex(PointCloud::Vertex(3));cloud.collect_garbage();
    test::require(cloud.n_vertices()==9 && cloud.points().size()==9,"deleted vertices survived garbage collection");
    test::require(copy.n_vertices()==10,"copy shared mutable topology");
    for(auto v:cloud.vertices()) test::require(color[v]==vec3(1,0,0),"property lost during compaction");
    test::require(PointCloudIO::save("cloud.ply",&cloud),"point cloud save failed");
    std::unique_ptr<PointCloud> loaded(PointCloudIO::load("cloud.ply"));
    test::require(loaded && loaded->n_vertices()==9,"point cloud round trip failed");
    for(size_t i=0;i<cloud.points().size();++i) test::require(loaded->points()[i]==cloud.points()[i],"I/O changed coordinates");
}); }
