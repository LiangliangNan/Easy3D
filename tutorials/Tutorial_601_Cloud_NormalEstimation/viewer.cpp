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
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialNormalEstimation::TutorialNormalEstimation(const std::string& title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();
}


std::string TutorialNormalEstimation::usage() const {
    return ("----------- Normal Estimation usage ------------ \n"
            "Press key 'e' to estimation point cloud normals\n"
            "press key 'r' to re-orient the normals\n"
            "------------------------------------------------ \n");
}


bool TutorialNormalEstimation::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_E) {
        PointCloud* cloud = dynamic_cast<PointCloud*>(current_model());
        if (cloud == nullptr) {
            if (!models().empty())
                std::cerr << "current model is not a point cloud" << std::endl;
            else
                std::cerr << "point cloud does not exist" << std::endl;
            return false;
        }

        PointCloudNormals algo;
        if (algo.estimate(cloud)) {
            update_rendering();
            return true;
        }
        else
            return false;
    }
    else if (key == GLFW_KEY_R) {
        PointCloud* cloud = dynamic_cast<PointCloud*>(current_model());
        if (cloud == nullptr) {
            if (!models().empty())
                std::cerr << "current model is not a point cloud" << std::endl;
            else
                std::cerr << "point cloud does not exist" << std::endl;
            return false;
        }

        PointCloudNormals algo;
        if (algo.reorient(cloud)) {
            update_rendering();
            return true;
        }
        else
            return false;
    }
   else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialNormalEstimation::update_rendering() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(current_model());
    if (cloud == nullptr)
        return;

    // The "normal" property
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (normals) {
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Upload the vertex normals to the GPU.
        drawable->update_normal_buffer(normals.vector());
        update();
    }
}
