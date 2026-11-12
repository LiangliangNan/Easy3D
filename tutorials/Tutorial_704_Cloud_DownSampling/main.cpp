/** \cond LICENSE
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
 \endcond */

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
 * \brief Reducing a point cloud's size (downsampling) with two complementary strategies.
 *
 * <b>Overview</b>
 * This interactive tutorial loads the fountain point cloud and offers two keyboard-triggered
 * simplification steps: uniform downsampling to a target point count (Ctrl+U) and grid-based
 * downsampling (Ctrl+G). Both shrink the cloud in place and refresh the rendering so the effect is
 * immediately visible. Downsampling is a common preprocessing step to tame the massive clouds
 * produced by LiDAR and photogrammetry.
 *
 * <b>The technique</b>
 * The goal is to keep far fewer points while preserving the overall shape. Uniform simplification
 * targets an explicit point count and removes points so the survivors stay roughly evenly spaced
 * (it can also serve to drop near-duplicate points). Grid simplification instead overlays a regular
 * 3D grid on the cloud's bounding box and keeps a single representative point per occupied cell —
 * very fast, but the representative is chosen arbitrarily, so the result is non-uniform. A good cell
 * size is tied to the data: here it is twice the average nearest-neighbor spacing, reported by
 * average_space(). Both algorithms return the set of vertices to remove; the caller then deletes
 * them, calls collect_garbage() to compact the storage, and updates the renderer so the GPU buffers
 * are re-extracted from the reduced cloud.
 *
 * <b>Easy3D API</b>
 * - PointCloudSimplification::uniform_simplification(cloud, num): returns vertices to delete to
 *   reach the target count num. ::grid_simplification(cloud, cell_size): one survivor per grid cell.
 *   ::average_space(cloud): estimates the mean point spacing used to size the grid.
 * - PointCloud::delete_vertex(v) / collect_garbage(): remove points and compact the data structure.
 * - Renderer::update(): re-extract rendering buffers after the cloud changes.
 * - Viewer::bind_shortcut(...): registers the two callbacks on Ctrl+U and Ctrl+G.
 *
 * <b>Running it</b>
 * Press Ctrl+U to uniformly downsample to half the current points (never fewer than 100), or
 * Ctrl+G for grid downsampling with cell size = 2 x average spacing. The console reports how many
 * points were removed and how many remain after each step.
 *
 * \see PointCloudSimplification, PointCloud, Renderer
 */

using namespace easy3d;

namespace {

    // Shared helper: delete the vertices the algorithm flagged, compact the cloud, and refresh
    // the rendering buffers so the reduced cloud is what gets drawn.
    void remove_points(PointCloud *cloud, const std::vector<PointCloud::Vertex> &to_remove) {
        const auto old_num = cloud->n_vertices();
        for (auto v : to_remove)
            cloud->delete_vertex(v); // marks a vertex deleted (does not compact yet)
        cloud->collect_garbage();    // actually removes deleted vertices and renumbers handles
        std::cout << old_num - cloud->n_vertices() << " points removed, " << cloud->n_vertices() << " points remain"
                  << std::endl;
        cloud->renderer()->update(); // re-extract GPU buffers from the now-smaller cloud
    }

    // Callback for Ctrl+U: uniformly reduce the cloud to half its current size (min 100 points).
    bool uniform_downsample(Viewer *viewer, Model *model) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (!cloud)
            return false;
        // Target count: half the points, but never below 100 so the cloud stays visible.
        const auto target = std::max(100u, cloud->n_vertices() / 2);
        remove_points(cloud, PointCloudSimplification::uniform_simplification(cloud, target));
        viewer->update();
        return true;
    }

    // Callback for Ctrl+G: keep one representative point per cell of a regular grid.
    bool grid_downsample(Viewer *viewer, Model *model) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (!cloud)
            return false;
        // Size the grid from the data: cell = 2 x the average nearest-neighbor spacing.
        const float spacing = PointCloudSimplification::average_space(cloud);
        remove_points(cloud, PointCloudSimplification::grid_simplification(cloud, spacing * 2.0f));
        viewer->update();
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    const std::string file = resource::directory() + "/data/fountain/pointcloud.ply";

    // Create the viewer and load the fountain cloud; downcast the generic Model to PointCloud.
    Viewer viewer(EXAMPLE_TITLE);
    auto cloud = dynamic_cast<PointCloud *>(viewer.add_model(file, true));
    if (!cloud) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Orient the camera to view the scene from its front.
    viewer.camera()->set_up_vector(vec3(0, 1, 0));
    viewer.camera()->set_view_direction(vec3(0, 0, -1));

    auto drawable = cloud->renderer()->get_points_drawable("vertices");
    drawable->set_point_size(3.0f);

    // Register the two strategies on Ctrl+U and Ctrl+G. The lambdas adapt the (Viewer*, Model*)
    // callback signature expected by bind() to our helper functions.
    viewer.bind_shortcut([](Viewer *v, Model *m) { return uniform_downsample(v, m); }, cloud, Viewer::KEY_U, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return grid_downsample(v, m); }, cloud, Viewer::KEY_G, Viewer::MODIF_CTRL);
    // Show the shortcuts in the on-screen help overlay.
    viewer.set_usage(
        "default",
        "Ctrl + u: uniform downsampling to half of the current points\n" "Ctrl + g: grid downsampling (cell size = 2 x average spacing)");

    // Enter the event loop; returns when the window is closed.
    return viewer.run();
}