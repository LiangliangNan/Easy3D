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

#ifndef EASY3D_ALGO_DELAUNAY_2D_H
#define EASY3D_ALGO_DELAUNAY_2D_H

#include <easy3d/algo/delaunay.h>


struct triangulateio;

namespace easy3d {


    /// \brief 2D Delaunay triangulation, using Jonathan Richard Shewchuk's "triangle" implementation.
    /// \class Delaunay2 easy3d/algo/delaunay_2d.h
    /// \see Delaunay, Delaunay3.

    class Delaunay2 : public Delaunay {
    public:
        Delaunay2();

        virtual ~Delaunay2();

        /// \brief Sets the vertices from an array of floating point numbers in which each consecutive number pair
        /// denotes a 2D point.
        virtual void set_vertices(unsigned int nb_vertices, const float *pts);

        /// \brief Sets the vertices from an array of 2D points.
        void set_vertices(const std::vector<vec2> &vertices) {
            set_vertices((unsigned int) vertices.size(), &vertices[0].x);
        }

        unsigned int nearest_vertex(const float *p) const {
            return Delaunay::nearest_vertex(p);
        }

        unsigned int nearest_vertex(const vec2 &p) const {
            return nearest_vertex(p.data());
        }

        const vec2 &vertex(unsigned int i) const {
            return *(const vec2 *) vertex_ptr(i);
        }

        const int *tri_to_v() const { return cell_to_v(); }

        const int *tri_to_tri() const { return cell_to_cell(); }

        int vertex_tri(int v) const { return vertex_cell(v); }

        /// \brief Returns the number of triangles.
        unsigned int nb_triangles() const { return nb_cells(); }

        /// \brief Returns the index of the \p lv_th vertex in the \p t_th triangle.
        int tri_vertex(unsigned int t, unsigned int lv) const {
            return cell_vertex(t, lv);
        }

        int tri_adjacent(unsigned int t, unsigned int le) const {
            return cell_adjacent(t, le);
        }

    protected:
        struct triangulateio *triangle_out_;
        struct triangulateio *triangle_in_;
    };





/*
 * The commented one is enough for basic 2D Delaunay implementation.
 * The above one is verbose for easy understanding of the interface APIs.
 */
//class MATH_API Delaunay2 : public Delaunay {
//public:
//	Delaunay2() ;
//	virtual ~Delaunay2() ;
//	virtual void set_vertices(unsigned int nb_vertices, const double* vertices) ;
//
//protected:
//	struct triangulateio* triangle_out_ ;
//	struct triangulateio* triangle_in_ ;
//} ;

}   // namespace easy3d

#endif  // EASY3D_ALGO_DELAUNAY_2D_H


