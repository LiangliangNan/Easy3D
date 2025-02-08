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

    /**
     * @brief Base class for Delaunay triangulation.
     * @details This class provides the base functionality for Delaunay triangulation in 2D and 3D.
     *      It manages vertices, cells, and their relationships, and provides methods for querying
     *      the triangulation structure.
     * @class Delaunay easy3d/algo/delaunay.h
     * @see Delaunay2D, Delaunay3D
     */
    class Delaunay {
    public:
        /**
         * @brief Constructor.
         * @param dimension The dimension of the triangulation, which must be either 2 or 3.
         */
        explicit Delaunay(unsigned int dimension);

        /// @brief Virtual destructor.
        virtual ~Delaunay();

        /**
         * @brief Returns the dimension of the triangulation.
         * @return The dimension (2 for 2D, 3 for 3D).
         */
        unsigned int dimension() const { return dimension_; }

        /**
         * @brief Returns the size of a cell.
         * @return The number of vertices per cell (3 for 2D, 4 for 3D).
         */
        unsigned int cell_size() const { return cell_size_; }

        /**
         * @brief Sets the vertices for the triangulation.
         * @param nb_vertices The number of vertices.
         * @param vertices A pointer to the array of vertex coordinates.
         * @note The vertices array must have a size of `nb_vertices * dimension()`.
         */
        virtual void set_vertices(unsigned int nb_vertices, const float *vertices);

        /**
         * @brief Returns a pointer to the vertices array.
         * @return A pointer to the array of vertex coordinates.
         */
        const float *vertices_ptr() const { return vertices_; }

        /**
         * @brief Returns a pointer to the vertex of index `i`.
         * @param i The index of the vertex.
         * @return A pointer to the coordinates of the vertex.
         */
        const float *vertex_ptr(unsigned int i) const {
            assert(i < nb_vertices());
            return vertices_ + dimension() * i;
        }

        /**
         * @brief Returns the number of vertices.
         * @return The number of vertices in the triangulation.
         */
        unsigned int nb_vertices() const { return nb_vertices_; }

        /**
         * @brief Returns the number of cells.
         * @return The number of cells in the triangulation.
         */
        unsigned int nb_cells() const { return nb_cells_; }

        /**
         * @brief Returns a pointer to the cell-to-vertex mapping.
         * @return A pointer to the array mapping cells to their vertices.
         */
        const int *cell_to_v() const { return cell_to_v_; }

        /**
         * @brief Returns a pointer to the cell-to-cell mapping.
         * @return A pointer to the array mapping cells to their adjacent cells.
         */
        const int *cell_to_cell() const { return cell_to_cell_; }

        /**
         * @brief Finds the index of the nearest vertex to a given point.
         * @param p A pointer to the coordinates of the point.
         * @return The index of the nearest vertex.
         */
        virtual unsigned int nearest_vertex(const float *p) const;

        /**
         * @brief Returns the index of the `lv`-th vertex in the `c`-th cell.
         * @param c The index of the cell.
         * @param lv The local index of the vertex within the cell.
         * @return The global index of the vertex.
         */
        int cell_vertex(unsigned int c, unsigned int lv) const {
            assert(c < nb_cells());
            assert(lv < cell_size());
            return cell_to_v_[c * cell_v_stride_ + lv];
        }

        /**
          * @brief Returns the index of the cell adjacent to the `lf`-th face of the `c`-th cell.
          * @param c The index of the cell.
          * @param lf The local index of the face within the cell.
          * @return The index of the adjacent cell.
          */
        int cell_adjacent(unsigned int c, unsigned int lf) const {
            assert(c < nb_cells());
            assert(lf < cell_size());
            return cell_to_cell_[c * cell_neigh_stride_ + lf];
        }

        /**
         * @brief Returns the index of a cell containing the vertex `v`.
         * @param v The index of the vertex.
         * @return The index of a cell containing the vertex.
         */
        int vertex_cell(unsigned int v) const {
            assert(v < nb_vertices());
            assert(v < v_to_cell_.size());
            return v_to_cell_[v];
        }

        /**
         * @brief Returns the local index of vertex `v` within cell `c`.
         * @param c The index of the cell.
         * @param v The global index of the vertex.
         * @return The local index of the vertex within the cell.
         */
        unsigned int index(unsigned int c, int v) const {
            assert(c < nb_cells());
            assert(v < (int) nb_vertices());
            for (unsigned int iv = 0; iv < cell_size(); iv++) {
                if (cell_vertex(c, iv) == v) { return iv; }
            }
            DCHECK(false) << "should not have reached here";
            return cell_size();
        }

        /**
         * @brief Returns the local index of the face shared by cells `c1` and `c2`.
         * @param c1 The index of the first cell.
         * @param c2 The index of the second cell.
         * @return The local index of the shared face.
         */
        unsigned int adjacent_index(unsigned int c1, unsigned int c2) const {
            assert(c1 < nb_cells());
            assert(c2 < nb_cells());
            for (unsigned int f = 0; f < cell_size(); f++) {
                if (cell_adjacent(c1, f) == c2) { return f; }
            }
            DCHECK(false) << "should not have reached here";
            return cell_size();
        }

        /**
         * @brief Returns the next cell around vertex `lv` in cell `c`.
         * @param c The index of the cell.
         * @param lv The local index of the vertex within the cell.
         * @return The index of the next cell.
         */
        unsigned int next_around_vertex(unsigned int c, unsigned int lv) const {
            assert(c < nb_cells());
            assert(lv < cell_size());
            return cicl_[cell_size() * c + lv];
        }

        /**
         * @brief Retrieves the one-ring neighbors of vertex `v`.
         * @param v The index of the vertex.
         * @param neighbors A vector to store the indices of the neighboring vertices.
         */
        virtual void get_neighbors(unsigned int v, std::vector<unsigned int> &neighbors) const;


        /**
         * @brief Checks for duplicate vertices in stored neighbor lists.
         * @return True if duplicate vertices are found, false otherwise.
         */
        bool check_duplicate_vertices();

    protected:

        void get_neighbors_internal(unsigned int v, std::vector<unsigned int> &neighbors) const;

        virtual void set_arrays(unsigned int nb_cells, const int *cell_to_v, const int *cell_to_cell);

        void update_v_to_cell();

        void update_cicl();

        void update_neighbors();

        void set_next_around_vertex(unsigned int c1, unsigned int lv, unsigned int c2) {
            assert(c1 < nb_cells());
            assert(c2 < nb_cells());
            assert(lv < cell_size());
            cicl_[cell_size() * c1 + lv] = static_cast<int>(c2);
        }

    protected:
        unsigned int dimension_;        ///< The dimension of the triangulation (2 or 3).
        unsigned int cell_size_;        ///< The number of vertices per cell (3 for 2D, 4 for 3
        unsigned int cell_v_stride_;    ///< The stride of the cell-to-vertex mapping.
        unsigned int cell_neigh_stride_;///< The stride of the cell-to-cell mapping.
        const float *vertices_;         ///< The array of vertex coordinates.
        unsigned int nb_vertices_;      ///< The number of vertices.
        unsigned int nb_cells_;         ///< The number of cells.
        const int *cell_to_v_;          ///< The cell-to-vertex mapping.
        const int *cell_to_cell_;       ///< The cell-to-cell mapping.
        std::vector<int> v_to_cell_;    ///< Mapping from vertices to a containing cell.
        std::vector<int> cicl_;         ///< Circular linked list for navigating around vertices.
        std::vector<std::vector<unsigned int>> neighbors_; ///< Neighbors of each vertex.
        bool is_locked_;                ///< Whether the triangulation is locked.
    };

}   // namespace easy3d


#endif  // EASY3D_ALGO_DELAUNAY_H

