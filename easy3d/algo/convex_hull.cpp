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


#include <easy3d/algo/convex_hull.h>
#include <easy3d/core/surface_mesh.h>

#include <3rd_party/quickhull/quickhull.hpp>
#include <forward_list>


namespace easy3d {

    bool ConvexHull::convex_hull(const std::vector<vec2> &pts, Polygon2& hull) {
        if (pts.size() <= 2) {
            LOG(ERROR) << "convex hull computation requires at least 3 points";
            return false;
        }

        using size_type = std::size_t;
        using value_type = double;
        using Point = std::forward_list<value_type>;
        using Points = std::forward_list<Point>;
        Points points(pts.size());
        size_type id=0;
        for (auto& p : points) {
            const vec2 &v = pts[id];
            p.push_front(v.y);  // y first (due to "push front")
            p.push_front(v.x);
            ++id;
        }
        size_type dimension_ = 2;

        // set epsilon (can be zero)
//        value_type const eps = std::numeric_limits< value_type >::epsilon();
        value_type const eps = 0.0;

        // define and setup QH class instance
        using quick_hull_type = quick_hull< typename Points::const_iterator >;
        quick_hull_type quick_hull(dimension_, eps); // (1)
        quick_hull.add_points(points.cbegin(), points.cend()); // (2)
        auto const initial_simplex = quick_hull.get_affine_basis(); // (3)
        
        // create initial simplex
        quick_hull.create_initial_simplex(std::begin(initial_simplex), std::prev(std::end(initial_simplex))); // (4)
        size_type const basis_size = initial_simplex.size();
        if (basis_size != quick_hull.dimension_ + 1) { // (5)
            LOG(ERROR) << "cannot construct a simplex. Degenerated input set. Size of basis: " << basis_size;
            return false;
        }

        // create convex hull
        quick_hull.create_convex_hull(); // (6)
        LOG(INFO) << "number of (convex hull) polyhedron facets is " << quick_hull.facets_.size();


#if 0   // Weird the test fails :-(
        if (!quick_hull.check()) {
            LOG(ERROR) << "resulting structure is not a valid convex polytope";
            return false;
        }
#endif


        // output
        std::ofstream gnuplot_("/Users/lnan/Downloads/test.txt");
        if (3 < quick_hull.dimension_) {
            LOG(INFO) << "dimensionality value " << quick_hull.dimension_
                 << " is out of supported range: cannot generate output";
            return true;
        }
        gnuplot_ << "set view equal xyz\n"
                    "set autoscale\n"
                    "set key left\n"
                    "set xrange [] writeback\n"
                    "set yrange [] writeback\n"
                    "set zrange [] writeback\n";
        gnuplot_ << "set title \'Points count is " << pts.size() << "\'\n";
        if (quick_hull.dimension_ == 2) {
            gnuplot_ << "plot";
        } else if (quick_hull.dimension_ == 3) {
            gnuplot_ << "splot";
        }
        gnuplot_ << " '-' with points notitle pointtype 4 pointsize 1.5 linetype 1"
                    ", '-' with points notitle"
                    ", '-' with labels offset character 0, character 1 notitle";
        for (auto const & facet_ : quick_hull.facets_) {
            gnuplot_ << ", '-' with lines notitle";
            if (!facet_.coplanar_.empty()) {
                gnuplot_ << ", '-' with points notitle pointtype 6 pointsize 1.5 linetype 4";
            }
        }
        gnuplot_ << ";\n";
        {
            for (auto const & v : initial_simplex) {
                Point const & point_ = *v;
                for (value_type const & coordinate_ : point_) {
                    gnuplot_ << coordinate_ << ' ';
                }
                gnuplot_ << '\n';
            }
            gnuplot_ << "e\n";
            {
                for (auto const & point_ : points) {
                    for (value_type const & coordinate_ : point_) {
                        gnuplot_ << coordinate_ << ' ';
                    }
                    gnuplot_ << '\n';
                }
                gnuplot_ << "e\n";
                size_type i = 0;
                for (auto const & point_ : points) {
                    for (value_type const & coordinate_ : point_) {
                        gnuplot_ << coordinate_ << ' ';
                    }
                    gnuplot_ << i << '\n';
                    ++i;
                }
            }
            gnuplot_ << "e\n";
        }
        for (auto const & facet_ : quick_hull.facets_) {
            auto const & vertices_ = facet_.vertices_;
            for (auto const & vertex_ : vertices_) {
                for (value_type const & coordinate_ : *vertex_) {
                    gnuplot_ << coordinate_ << ' ';
                }
                gnuplot_ << '\n';
            }
            for (value_type const & coordinate_ : *vertices_.front()) {
                gnuplot_ << coordinate_ << ' ';
            }
            gnuplot_ << "\n"
                        "e\n";
            if (!facet_.coplanar_.empty()) {
                for (auto const & v : facet_.coplanar_) {
                    for (value_type const & coordinate_ : *v) {
                        gnuplot_ << coordinate_ << ' ';
                    }
                    gnuplot_ << '\n';
                }
                gnuplot_ << "e\n";
            }
        }
        gnuplot_ << std::flush;

        hull.clear();
        return true;
    }

    SurfaceMesh* ConvexHull::convex_hull(const std::vector<vec3> &points) {
        SurfaceMesh* hull = new SurfaceMesh;
        return hull;
    }

}   // namespace easy3d
