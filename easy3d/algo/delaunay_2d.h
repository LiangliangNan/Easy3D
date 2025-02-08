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


    /**
     * @brief 2D Delaunay triangulation, using Jonathan Richard Shewchuk's "triangle" implementation.
     * @details This class provides functionality for 2D Delaunay triangulation using the "triangle" library.
     *      It manages vertices, triangles, and their relationships, and provides methods for querying
     *      the triangulation structure.
     * @class Delaunay2 easy3d/algo/delaunay_2d.h
     * @see Delaunay, Delaunay3
     */
    class Delaunay2 : public Delaunay {
    public:
        /// @brief Default constructor.
        Delaunay2();

        /// @brief Destructor.
        ~Delaunay2() override;

        /**
         * @brief Sets the vertices from an array of floating point numbers, in which each consecutive number pair
         *       denotes a 2D point.
         * @param nb_vertices The number of vertices.
         * @param pts A pointer to the array of vertex coordinates.
         * @note The vertices array must have a size of `nb_vertices * 2`.
         */
        void set_vertices(unsigned int nb_vertices, const float *pts) override;

        /**
         * @brief Sets the vertices from an array of 2D points.
         * @param vertices A vector of 2D points.
         */
        void set_vertices(const std::vector<vec2> &vertices) {
            set_vertices(static_cast<unsigned int>(vertices.size()), vertices[0]);
        }

        /**
         * @brief Finds the index of the nearest vertex to a given point.
         * @param p A pointer to the coordinates of the point.
         * @return The index of the nearest vertex.
         */
        unsigned int nearest_vertex(const float *p) const override {
            return Delaunay::nearest_vertex(p);
        }

        /**
         * @brief Finds the index of the nearest vertex to a given 2D point.
         * @param p The 2D point.
         * @return The index of the nearest vertex.
         */
        unsigned int nearest_vertex(const vec2 &p) const {
            return nearest_vertex(p.data());
        }

        /**
         * @brief Returns the coordinates of the vertex with index `i`.
         * @param i The index of the vertex.
         * @return The coordinates of the vertex.
         */
        const vec2 &vertex(unsigned int i) const {
            return *reinterpret_cast<const vec2 *>(vertex_ptr(i));
        }

        /**
         * @brief Returns the pointer to the triangle-to-vertex mapping.
         * @return A pointer to the array mapping triangles to their vertices.
         */
        const int *tri_to_v() const { return cell_to_v(); }

        /**
         * @brief Returns the pointer to the triangle-to-triangle mapping.
         * @return A pointer to the array mapping triangles to their adjacent triangles.
         */
        const int *tri_to_tri() const { return cell_to_cell(); }

        /**
         * @brief Returns the index of a triangle containing the vertex `v`.
         * @param v The index of the vertex.
         * @return The index of a triangle containing the vertex.
         */
        int vertex_tri(int v) const { return vertex_cell(v); }

        /**
         * @brief Returns the number of triangles.
         * @return The number of triangles in the triangulation.
         */
        unsigned int nb_triangles() const { return nb_cells(); }

        /**
         * @brief Returns the index of the `lv`-th vertex in the `t`-th triangle.
         * @param t The index of the triangle.
         * @param lv The local index of the vertex within the triangle.
         * @return The global index of the vertex.
         */
        int tri_vertex(unsigned int t, unsigned int lv) const {
            return cell_vertex(t, lv);
        }

        /**
         * @brief Returns the index of the triangle adjacent to the `le`-th edge of the `t`-th triangle.
         * @param t The index of the triangle.
         * @param le The local index of the edge within the triangle.
         * @return The index of the adjacent triangle.
         */
        int tri_adjacent(unsigned int t, unsigned int le) const {
            return cell_adjacent(t, le);
        }

    protected:
        triangulateio *triangle_out_;    ///< Output data structure for the "triangle" library.
        triangulateio *triangle_in_;     ///< Input data structure for the "triangle" library.
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


