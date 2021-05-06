/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialSurfaceReconstruction::TutorialSurfaceReconstruction(const std::string& title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();
}


std::string TutorialSurfaceReconstruction::usage() const {
    return ("-------------- Surface Reconstruction usage -------------- \n"
            "Press key 'r' for surface reconstruction\n"
            "---------------------------------------------------------- \n");
}


bool TutorialSurfaceReconstruction::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_R) {
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
            std::cerr << "Poisson surface reconstruction method requires normal information."
                      << " Please provide normal information. Alternatively, you can use the "
                      << " Tutorial_601_PointCloud_NormalEstimation for normal estimation" << std::endl;
            return false;
        }

        const int depth = 6;
        PoissonReconstruction algo;
        algo.set_depth(depth);
        std::cout << "reconstruction depth: " << depth << std::endl;
        Model* surface = algo.apply(cloud);
        if (surface != nullptr) {
            add_model(surface, true);
            delete_model(cloud);
            update();
        }

        return true;
    }
   else
        return Viewer::key_press_event(key, modifiers);
}
