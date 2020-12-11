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

#ifndef EASY3D_ALGO_DELAUNAY_H
#define EASY3D_ALGO_DELAUNAY_H

#include <cassert>

#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    /// \brief Base class for Delaunay triangulation.
    /// \class Delaunay easy3d/algo/delaunay.h
    /// \see Delaunay2D, Delaunay3D.

    class Delaunay {

    public:
        /// \brief Constructor.
        /// \param dimension The dimension, must be either 2 or 3.
        Delaunay(unsigned int dimension);

        virtual ~Delaunay();

        /// \brief Returns the dimension.
        unsigned int dimension() const { return dimension_; }

        /// \brief Returns the size of the cell.
        unsigned int cell_size() const { return cell_size_; }

        /// \brief Sets the vertices.
        virtual void set_vertices(unsigned int nb_vertices, const float *vertices);

        /// \brief Returns the pointer to the vertices.
        const float *vertices_ptr() const { return vertices_; }

        /// \brief Returns the pointer to the vertex of index \p i.
        const float *vertex_ptr(unsigned int i) const {
            assert(i < nb_vertices());
            return vertices_ + dimension() * i;
        }

        /// \brief Returns the number of vertices.
        unsigned int nb_vertices() const { return nb_vertices_; }

        /// \brief Returns the number of cells.
        unsigned int nb_cells() const { return nb_cells_; }

        const int *cell_to_v() const { return cell_to_v_; }

        const int *cell_to_cell() const { return cell_to_cell_; }

        virtual unsigned int nearest_vertex(const float *p) const;

        /// \brief Returns the index of the \p lv_th vertex in the \p c_th cell.
        int cell_vertex(unsigned int c, unsigned int lv) const {
            assert(c < nb_cells());
            assert(lv < cell_size());
            return cell_to_v_[c * cell_v_stride_ + lv];
        }

        int cell_adjacent(unsigned int c, unsigned int lf) const {
            assert(c < nb_cells());
            assert(lf < cell_size());
            return cell_to_cell_[c * cell_neigh_stride_ + lf];
        }

        int vertex_cell(unsigned int v) const {
            assert(v < nb_vertices());
            assert(v < v_to_cell_.size());
            return v_to_cell_[v];
        }

        unsigned int index(unsigned int c, int v) const {
            assert(c < nb_cells());
            assert(v < (int) nb_vertices());
            for (unsigned int iv = 0; iv < cell_size(); iv++) {
                if (cell_vertex(c, iv) == v) { return iv; }
            }
            DLOG_ASSERT(false) << "should not have reached here";
            return cell_size();
        }

        unsigned int adjacent_index(unsigned int c1, unsigned int c2) const {
            assert(c1 < nb_cells());
            assert(c2 < nb_cells());
            for (unsigned int f = 0; f < cell_size(); f++) {
                if (cell_adjacent(c1, f) == c2) { return f; }
            }
            DLOG_ASSERT(false) << "should not have reached here";
            return cell_size();
        }

        unsigned int next_around_vertex(unsigned int c, unsigned int lv) const {
            assert(c < nb_cells());
            assert(lv < cell_size());
            return cicl_[cell_size() * c + lv];
        }

        /**
         * \brief Retrieves the one-ring neighbors of vertex v.
         */
        virtual void get_neighbors(unsigned int v, std::vector<unsigned int> &neighbors) const;


        /**
         * \brief Checks for duplicate vertices in stored neighbor lists.
         * Returns true if there where some duplicate vertices.
         */
        bool check_duplicate_vertices();

    protected:

        void get_neighbors_internal(
                unsigned int v, std::vector<unsigned int> &neighbors
        ) const;

        virtual void set_arrays(
                unsigned int nb_cells, const int *cell_to_v, const int *cell_to_cell
        );

        void update_v_to_cell();

        void update_cicl();

        void update_neighbors();

        void set_next_around_vertex(
                unsigned int c1, unsigned int lv, unsigned int c2
        ) {
            assert(c1 < nb_cells());
            assert(c2 < nb_cells());
            assert(lv < cell_size());
            cicl_[cell_size() * c1 + lv] = c2;
        }

    protected:
        unsigned int dimension_;
        unsigned int cell_size_;
        unsigned int cell_v_stride_;
        unsigned int cell_neigh_stride_;
        const float *vertices_;
        unsigned int nb_vertices_;
        unsigned int nb_cells_;
        const int *cell_to_v_;
        const int *cell_to_cell_;
        std::vector<int> v_to_cell_;
        std::vector<int> cicl_;
        std::vector <std::vector<unsigned int>> neighbors_;
        bool is_locked_;
    };

}   // namespace easy3d


#endif  // EASY3D_ALGO_DELAUNAY_H

