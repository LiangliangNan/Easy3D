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

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/buffers.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialPlaneExtraction::TutorialPlaneExtraction(const std::string& title)
    : Viewer(title)
{
}


std::string TutorialPlaneExtraction::usage() const {
    return ("---------- Plane Extraction usage ---------- \n"
            "Press key 'e' to extract planes\n"
            "-------------------------------------------- \n");
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
            std::cerr << "Plane extraction using RANSAC requires normal information but it is not available" << std::endl;
            return false;
        }

        PrimitivesRansac algo;
        algo.add_primitive_type(PrimitivesRansac::PLANE);

        // you can try different parameters of RANSAC (usually you don't need to tune them)
        const int num = algo.detect(cloud, 200, 0.005f, 0.02f, 0.8f, 0.001f);
        if (num > 0) {
            std::cout << num << " primitives extracted" << std::endl;

            // assign each plane a unique color
            auto segments = cloud->vertex_property<int>("v:primitive_index");
            const std::string color_name = "v:color-segments";
            auto coloring = cloud->vertex_property<vec3>(color_name, vec3(0, 0, 0));
            Renderer::color_from_segmentation(cloud, segments, coloring);

            auto drawable = cloud->renderer()->get_points_drawable("vertices");
            drawable->set_property_coloring(State::VERTEX, color_name);

            drawable->update();
            update();
        }

        return true;
    }
   else
        return Viewer::key_press_event(key, modifiers);
}
