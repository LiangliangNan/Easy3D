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

#ifndef EASY_ALGO_EXT_OVERLAPPING_FACES_H
#define EASY_ALGO_EXT_OVERLAPPING_FACES_H


#include <vector>

// Use this instead to mute errors resulting from bad CGAL assertions
//#define CGAL_KERNEL_NO_ASSERTIONS

#include <CGAL/Simple_cartesian.h>
#include <CGAL/intersections.h>	// Triangle-triangle intersection
#include <CGAL/box_intersection_d.h>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /// \brief Detects/Removes duplicate and folding faces for a triangle mesh.
    /// \class OverlappingFaces  easy3d/algo_ext/overlapping_faces.h
    class OverlappingFaces
    {
    public:
        OverlappingFaces() = default;
        ~OverlappingFaces() = default;

        /**
          * \brief Detects duplicate faces and folding faces.
          * \details Two triangle faces are said duplicate if they have the same geometry (vertices within a distance
          *         threshold). Two triangle faces are said folding if they are coplanar, share one edge (i.e., have
          *         the same edge geometry), and partially overlap.
          * \param mesh The input surface mesh.
          * \param duplicate_faces Returns the duplicate face pairs found.
          * \param folding_faces Returns the folding face pairs found.
          * \param dist_threshold Two vertices are considered coincident if there distance is smaller than it.
          * \pre mesh.is_triangle_mesh().
          */
        void detect(
                SurfaceMesh *mesh,
                std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face > > &duplicate_faces,
                std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face > > &folding_faces,
                double dist_threshold = 1e-6
        );

        /**
         * \brief Removes duplicate faces and and folding faces.
          * \details Two triangle faces are said duplicate if they have the same geometry (vertices within a distance
          *         threshold). Two triangle faces are said folding if they are coplanar, share one edge (i.e., have
          *         the same edge geometry), and partially overlap.
          * \param mesh The input surface mesh.
          * \param folding_faces \c true also to remove folding faces.
          * \return The number of faces that have been deleted.
          * \pre mesh.is_triangle_mesh().
          */
        unsigned int remove(
                SurfaceMesh *mesh,
                bool folding_faces = false,
                double dist_threshold = 1e-6
        );

    private:
        typedef CGAL::Simple_cartesian<double>	Kernel;

        typedef CGAL::Point_3<Kernel>		Point_3;
        typedef CGAL::Vector_3<Kernel>		Vector_3;
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
        enum OverlapType {OT_NONE, OT_SAME, OT_FOLDING};
        OverlapType do_overlap(const Triangle& A, const Triangle& B, double sqr_eps);

        Triangles triangle_faces_;
    };

}   // namespace easy3d

#endif  // EASY_ALGO_EXT_OVERLAPPING_FACES_H
