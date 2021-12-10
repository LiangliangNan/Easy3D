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


#include <easy3d/algo/gaussian_noise.h>

#include <chrono>
#include <random>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/random.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    void GaussianNoise::apply(SurfaceMesh *mesh, float sigma) {
        if (!mesh) {
            LOG(WARNING) << "empty surface mesh";
            return;
        }

        if (sigma <= 0) {
            LOG(WARNING) << "sigma must be positive";
            return;
        }

        // Define random generator with Gaussian distribution
        const double mean = 0.0;
        // construct a trivial random generator engine from a time-based seed:
        unsigned int seed = (unsigned int) std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::normal_distribution<double> distribution(mean, sigma);

        auto points = mesh->get_vertex_property<vec3>("v:point");
        for (auto v : mesh->vertices()) {
            double offset = distribution(generator);
            vec3 dir(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f),
                     random_float(-1.0f, 1.0f));
            dir = normalize(dir);
            points[v] = points[v] + dir * offset;
        }

        // update normals if exist
        if (mesh->get_vertex_property<vec3>("v:normal"))
            mesh->update_vertex_normals();
        if (mesh->get_face_property<vec3>("f:normal"))
            mesh->update_face_normals();
    }


    void GaussianNoise::apply(PointCloud *cloud, float sigma) {
        if (!cloud) {
            LOG(WARNING) << "empty point cloud";
            return;
        }

        if (sigma <= 0) {
            LOG(WARNING) << "sigma must be positive";
            return;
        }

        // Define random generator with Gaussian distribution
        const double mean = 0.0;
        // construct a trivial random generator engine from a time-based seed:
        unsigned int seed = (unsigned int) std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::normal_distribution<double> distribution(mean, sigma);

        auto points = cloud->get_vertex_property<vec3>("v:point");
        for (auto v : cloud->vertices()) {
            double offset = distribution(generator);
            vec3 dir(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f),
                     random_float(-1.0f, 1.0f));
            dir = normalize(dir);
            points[v] = points[v] + dir * offset;
        }
    }

}