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

#include "plane_extraction.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialPlaneExtraction::TutorialPlaneExtraction(const std::string& title) : Viewer(title) {
	camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();

    std::cout << "------------ Point cloud plane extraction ----------" << std::endl
              << "Press key 'e' to extract planes" << std::endl;
}


bool TutorialPlaneExtraction::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_E) {
        PointCloud* cloud = dynamic_cast<PointCloud*>(current_model());
        if (cloud == nullptr) {
            if (!models().empty())
                std::cerr << "current model is not a point cloud" << std::endl;
            else
                std::cerr << "point cloud does not exist" << std::endl;
            return false;
        }

        auto normals = cloud->get_vertex_property<vec3>("v:normal");
        if (!normals) {
            std::cerr << "Plane extraction using RANSAC requires normal information."
                      << " Please provide normal information. Alternatively, you can use the "
                      << " Tutorial_601_PointCloud_NormalEstimation for normal estimation" << std::endl;
            return false;
        }

        PrimitivesRansac algo;
        algo.add_primitive_type(PrimitivesRansac::PLANE);

        // you can try different parameters of RANSAC (usually you don't need to tune them)
        int num = algo.detect(cloud, 1000, 0.005f, 0.02f, 0.8f, 0.001f);
        if (num > 0) {
            // assign each plane a unique color
            std::vector<vec3> color_table(num);
            for (auto& c : color_table)
                c = random_color();

            // plane extraction results has been stored as properties:
            //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
            //      - "v:primitive_index" (0, 1, 2... -1 indicating unknown)
            auto indices = cloud->get_vertex_property<int>("v:primitive_index");
            std::vector<vec3> colors;
            for (auto v : cloud->vertices()) {
                int idx = indices[v];
                colors.push_back(idx == -1 ? vec3(0, 0, 0) : color_table[idx]); // black for unkonwn type
            }
            auto drawable = cloud->points_drawable("vertices");
            // Upload the vertex normals to the GPU.
            drawable->update_color_buffer(colors);
            update();
        }

        return true;
    }
   else
        return Viewer::key_press_event(key, modifiers);
}
