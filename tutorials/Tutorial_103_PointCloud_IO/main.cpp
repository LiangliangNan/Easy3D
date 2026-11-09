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

#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_103_PointCloud_IO/main.cpp
 *
 * \brief Loading a point cloud from disk and writing it back out, including a hand-rolled text export.
 *
 * <b>Overview</b>
 * This console tutorial covers file input/output for PointCloud. It loads the bundled "bunny"
 * cloud, reports how many points it contains, and then exports the data to a plain-text file,
 * one point per line. It complements Tutorial_102_PointCloud_Property by showing that the same
 * named properties used to store attributes are also what get read from and written to files.
 *
 * <b>The technique</b>
 * Easy3D dispatches on the file extension: PointCloudIO::load()/save() understand several point
 * formats (bin, xyz/bxyz, ply, las/laz, vg/bvg), so the same call works regardless of the concrete
 * format. Here we load Easy3D's compact binary ".bin" file. For the export we deliberately do
 * NOT call save(); instead we stream the attributes ourselves to demonstrate that a loaded cloud
 * is just properties we can iterate. Each output line holds the xyz coordinates, followed by the
 * normal and the color when those properties exist. Because normals/colors are optional, we test
 * each property handle for validity before dereferencing it: get_vertex_property<T>() returns an
 * invalid (null) handle when the name is absent, and a VertexProperty converts to bool so that
 * "if (normals)" is a safe existence check.
 *
 * <b>Easy3D API</b>
 * - PointCloudIO::load(file): reads a cloud, returning a heap-allocated PointCloud* (nullptr on
 *   failure); the caller owns it and must delete it.
 * - resource::directory(): locates Easy3D's bundled data folder so the example file is found.
 * - PointCloud::get_vertex_property<T>(name): fetches "v:point", "v:normal", "v:color".
 * - PointCloud::vertices(): the range of point handles to iterate when writing each line.
 *
 * <b>Running it</b>
 * Console-only. It prints the loaded point count, then "saving the point cloud..." and "point
 * cloud saved to './bunny-copy.txt'". The export is written to the current working directory;
 * open it to see one "x y z [nx ny nz] [r g b]" line per point.
 *
 * \see PointCloud, PointCloudIO, Tutorial_102_PointCloud_Property, Tutorial_107_SurfaceMesh_IO
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Read the point cloud from a known file.
    PointCloud *cloud = PointCloudIO::load(resource::directory() + "/data/bunny.bin");
    if (!cloud) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "point cloud has " << cloud->n_vertices() << " points" << std::endl;

    // You can save the model into a file of one of the supported file formats using 'PointCloudIO::save()'.
    // In this example, let's save the model into a file with a special format. In each line we store the
    // x, y, z coordinates, followed by the normal (nx, ny, nz) and color (r, g, b) if they exist.
    std::ofstream output("./bunny-copy.txt");
    if (output.is_open()) { // if the file has been successfully created
        // The point coordinates. "v:point" always exists for a loaded cloud.
        PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");
        // The point normals. Optional: the handle is invalid (null) if the file stored none.
        PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
        // The point colors. Optional too, so we test it for validity before use below.
        PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
        std::cout << "saving the point cloud..." << std::endl;

        for (auto v : cloud->vertices()) {
            output << points[v];
            if (normals) // if normals exist
                output << " " << normals[v];
            if (colors) // if colors exist
                output << " " << colors[v];
            output << std::endl;
        }
        std::cout << "point cloud saved to './bunny-copy.txt'" << std::endl;
    }

    // Delete the point cloud (i.e., release memory)
    delete cloud;

    return EXIT_SUCCESS;
}
