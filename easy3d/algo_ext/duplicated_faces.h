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

#ifndef ALGO_DUPLICATED_FACES_H
#define ALGO_DUPLICATED_FACES_H


#include <vector>

// Use this instead to mute errors resulting from bad CGAL assertions
//#define CGAL_KERNEL_NO_ASSERTIONS

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>	// Triangle triangle intersection
#include <CGAL/box_intersection_d.h>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {


    class DuplicatedFaces
    {
    public:
        DuplicatedFaces() {}
        ~DuplicatedFaces() {}

        // detect and return the duplicated faces
        // exact == true: do exact predict; otherwise use the distance threshold.
        // upon return, the second component of each entry contains the set of faces
        // duplicating the one stored as the first component.
        // TODO: implement using threshold (now only 'exact' is implemented).
        std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
        detect(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);

        // detect and remove duplicated faces. Returns the number of deleted faces.
        // exact == true: do exact predict; otherwise use the distance threshold.
        unsigned int remove(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);

    private:
        typedef CGAL::Exact_predicates_exact_constructions_kernel	Kernel;

        typedef CGAL::Point_3<Kernel>		Point_3;
        typedef CGAL::Triangle_3<Kernel>	Triangle_3;

    private:
        struct Triangle {
        public:
            Triangle(const Point_3& a, const Point_3& b, const Point_3& c, SurfaceMesh::Face f) : triangle(a, b, c), face(f) {}
            Triangle_3 triangle;
            SurfaceMesh::Face face;
            std::vector<SurfaceMesh::Vertex> vertices;
        };

        // Axis-align boxes for all-pairs self-intersection detection
        typedef std::vector<Triangle>				Triangles;
        typedef typename Triangles::iterator		TrianglesIterator;
        typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TrianglesIterator>	Box;

    private:
        Triangles mesh_to_cgal_triangle_list(SurfaceMesh* esh);

        // test if two triangles duplicate
        bool do_duplicate(const Triangle& A, const Triangle& B, bool exact, double sqr_eps);

        Triangles triangle_faces_;
    };

}

#endif
