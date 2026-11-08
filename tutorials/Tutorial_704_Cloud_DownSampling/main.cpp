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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_704_Cloud_DownSampling/main.cpp
 *
 * This example shows how to downsample (simplify) a point cloud using the
 * PointCloudSimplification class. Two strategies are demonstrated:
 *      - uniform simplification to a target number of points;
 *      - grid simplification, which keeps one representative point per cell of a
 *        regular grid whose cell size is derived from the average point spacing.
 *
 * The simplification functions return the vertices to remove. We delete them,
 * collect the garbage, and call renderer()->update() so the rendering buffers are
 * re-extracted from the reduced cloud.
 */

using namespace easy3d;


namespace {

    void remove_points(PointCloud *cloud, const std::vector<PointCloud::Vertex> &to_remove) {
        const auto old_num = cloud->n_vertices();
        for (auto v: to_remove)
            cloud->delete_vertex(v);
        cloud->collect_garbage();
        std::cout << old_num - cloud->n_vertices() << " points removed, "
                  << cloud->n_vertices() << " points remain" << std::endl;
        cloud->renderer()->update();
    }

    bool uniform_downsample(Viewer *viewer, Model *model) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (!cloud)
            return false;
        const auto target = std::max(100u, cloud->n_vertices() / 2);
        remove_points(cloud, PointCloudSimplification::uniform_simplification(cloud, target));
        viewer->update();
        return true;
    }

    bool grid_downsample(Viewer *viewer, Model *model) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (!cloud)
            return false;
        const float spacing = PointCloudSimplification::average_space(cloud);
        remove_points(cloud, PointCloudSimplification::grid_simplification(cloud, spacing * 2.0f));
        viewer->update();
        return true;
    }

} // namespace


int main(int argc, char **argv) {
    initialize();

    const std::string file = resource::directory() + "/data/fountain/pointcloud.ply";

    Viewer viewer(EXAMPLE_TITLE);
    auto cloud = dynamic_cast<PointCloud *>(viewer.add_model(file, true));
    if (!cloud) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // to view the scene from its front
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, -1));

    auto drawable = cloud->renderer()->get_points_drawable("vertices");
    drawable->set_point_size(3.0f);

    viewer.bind([](Viewer *v, Model *m) { return uniform_downsample(v, m); }, cloud, Viewer::KEY_U, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return grid_downsample(v, m); }, cloud, Viewer::KEY_G, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + u: uniform downsampling to half of the current points\n"
                     "Ctrl + g: grid downsampling (cell size = 2 x average spacing)");

    return viewer.run();
}