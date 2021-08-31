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

#include <vector>
#include "hrbf.h"

using namespace easy3d;


#if 0

bool test_hrbf() {
    typedef HRBF<float, 3, Rbf_pow3<float> > HRBF;
    HRBF fit;

    {
        // Define samples (position, normal);
        HRBF::Vector pts[] = { HRBF::Vector( 0.f, 0.f, 0.f), HRBF::Vector( 1.f, 0.f, 0.f), HRBF::Vector( 0.f, 0.f, 2.f) };
        HRBF::Vector nrs[] = { HRBF::Vector(-1.f, 0.f, 0.f), HRBF::Vector( 1.f, 0.f, 0.f), HRBF::Vector( 0.f, 0.f, 1.f) };

        const int size = sizeof(pts) / sizeof(HRBF::Vector);
        std::vector<HRBF::Vector> p(pts, pts + size );
        std::vector<HRBF::Vector> n(nrs, nrs + size );
        // Solve linear system;
        fit.fit(p, n);
    }

    HRBF::Vector x( 0.f, 0.f, 0.f);
    float potential = fit.eval( x );
    HRBF::Vector gf = fit.grad( x );

    std::cout << "alphas: " << fit.alphas() << std::endl;
    std::cout << "betas: " << fit.betas() << std::endl;

    return true;
}
#else

#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/kdtree/kdtree_search_nanoflann.h>

bool test_hrbf() {
    std::string file = "/Users/lnan/Projects/Data/PolyFit_data/sphere-100K.vg";
    PointCloud* cloud = PointCloudIO::load(file);
    if (!cloud)
        return false;
    auto vnormal = cloud->get_vertex_property<vec3>("v:normal");
    if (!vnormal)
        return false;

    KdTreeSearch_NanoFLANN kdtree;
    kdtree.begin();
    kdtree.add_point_cloud(cloud);
    kdtree.end();

    typedef HRBF<float, 3, Rbf_pow3<float> > HRBF;
    typedef HRBF::Vector                     Point;
    typedef HRBF::Vector                     Normal;

    HRBF fit;

    const int k = 128;
    for (auto v : cloud->vertices()) {
        std::vector<int> neighbors;
        kdtree.find_closest_k_points(cloud->position(v), k, neighbors);

        std::vector<Point> points(neighbors.size());
        std::vector<Normal> normals(neighbors.size());
        for (std::size_t i = 0; i < neighbors.size(); ++i) {
            auto vtx = PointCloud::Vertex(neighbors[i]);
            points[i] = Point(cloud->position(vtx).data());
            normals[i] = Normal(vnormal[vtx].data());
        }

        // Solve linear system;
        fit.fit(points, normals);

#if 0
        std::cout << "alphas: " << fit.alphas() << std::endl;
        std::cout << "betas: " << fit.betas() << std::endl;
#endif

        std::vector<float> alphas(fit.alphas().begin(), fit.alphas().end());
        std::sort(alphas.begin(), alphas.end());
        for (auto a : alphas)
            std::cout << std::setprecision(5) << std::setw(8) << std::setfill(' ') << a << " ";
        std::cout << std::endl << std::endl;
    }

    return true;
}

#endif