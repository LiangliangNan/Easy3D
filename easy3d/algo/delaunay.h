/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_ALGO_DELAUNAY_H
#define EASY3D_ALGO_DELAUNAY_H

#include <cassert>

#include <easy3d/core/types.h>


namespace easy3d {

    /// \brief Base class for Delaunay triangulation.
    /// \class Delaunay easy3d/algo/delaunay.h
    /// \see Delaunay2D, Delaunay3D.

    class Delaunay {

    public:
        /// \brief Constructor.
        /// \param dimension The dimension, must be either 2 or 3.
        explicit Delaunay(unsigned int dimension);

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
            DCHECK(false) << "should not have reached here";
            return cell_size();
        }

        unsigned int adjacent_index(unsigned int c1, unsigned int c2) const {
            assert(c1 < nb_cells());
            assert(c2 < nb_cells());
            for (unsigned int f = 0; f < cell_size(); f++) {
                if (cell_adjacent(c1, f) == c2) { return f; }
            }
            DCHECK(false) << "should not have reached here";
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
            cicl_[cell_size() * c1 + lv] = static_cast<int>(c2);
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

