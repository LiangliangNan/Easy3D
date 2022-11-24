/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

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

        ~Delaunay2() override;

        /// \brief Sets the vertices from an array of floating point numbers in which each consecutive number pair
        /// denotes a 2D point.
        void set_vertices(unsigned int nb_vertices, const float *pts) override;

        /// \brief Sets the vertices from an array of 2D points.
        void set_vertices(const std::vector<vec2> &vertices) {
            set_vertices((unsigned int) vertices.size(), &vertices[0].x);
        }

        unsigned int nearest_vertex(const float *p) const override {
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


