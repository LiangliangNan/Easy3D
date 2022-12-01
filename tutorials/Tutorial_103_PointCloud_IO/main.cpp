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

#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;


// This example shows how to
//		- load a point cloud from a file;
//		- save a point cloud to a file.


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

	// Read the point cloud from a known file. 
    PointCloud* cloud = PointCloudIO::load(resource::directory() + "/data/bunny.bin");
    if (!cloud) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    std::cout << "point cloud has " << cloud->n_vertices() << " points" << std::endl;

    // You can save the model into a file of one of the supported file formats using 'PointCloudIO::save()'.
    // In this example, let's save the model into a file with a special format. In each line we store the
    // x, y, z coordinates, followed by the normal (nx, ny, nz) and color (r, g, b) if they exist.
    std::ofstream output("./bunny-copy.txt");
    if (output.is_open()) { // if the file has been successfully created
        // The point coordinates.
        PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");
        // The point normals.
        PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
        // The point colors.
        PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
        std::cout << "saving the point cloud..." << std::endl;

        for (auto v : cloud->vertices()) {
            output << points[v];
            if (normals)	// if normals exist
                output << " " << normals[v];
            if (colors)		// if colors exist
                output << " " << colors[v];
            output << std::endl;
        }
        std::cout << "point cloud saved to './bunny-copy.txt'" << std::endl;
    }

    // Delete the point cloud (i.e., release memory)
    delete cloud;

    return EXIT_SUCCESS;
}

