/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/


#include <easy3d/algo/delaunay.h>

#include <algorithm>
#include <fstream>
#include <string>


namespace easy3d {


    // \cond
    namespace details {
        template<typename FT>
        inline FT squared_distance(unsigned int dim, const FT *p1, const FT *p2) {
            FT result(0);
            for (unsigned int i = 0; i < dim; ++i) {
                FT diff = p2[i] - p1[i];
                result += diff * diff;
            }
            return result;
        }
    }
    // \endcond


    Delaunay::Delaunay(unsigned int dimension) {
        dimension_ = dimension;
        cell_size_ = dimension + 1;
        cell_v_stride_ = cell_size_;
        cell_neigh_stride_ = cell_size_;
        vertices_ = nullptr;
        nb_vertices_ = 0;
        nb_cells_ = 0;
        cell_to_v_ = nullptr;
        cell_to_cell_ = nullptr;
        is_locked_ = false;
    }


    Delaunay::~Delaunay() {
    }


    void Delaunay::set_vertices(unsigned int nb_vertices, const float *vertices) {
        nb_vertices_ = nb_vertices;
        vertices_ = vertices;
        if (nb_vertices_ < dimension() + 1) {
            LOG(WARNING) << "only " << nb_vertices << " vertices? not enough for Delaunay triangulation";
        }
    }


    void Delaunay::set_arrays(unsigned int nb_cells, const int *cell_to_v, const int *cell_to_cell) {
        nb_cells_ = nb_cells;
        cell_to_v_ = cell_to_v;
        cell_to_cell_ = cell_to_cell;
        if (cell_to_cell != nullptr) {
            update_v_to_cell();
            update_cicl();
            if (dimension() >= 6) {
                update_neighbors();
            }
        }
    }


    unsigned int Delaunay::nearest_vertex(const float *p) const {
        // Inefficient implementation (but at least it works).
        std::cerr << "TODO: replace with line walk (or maybe we could use kd-tree here)" << std::endl;
        assert(nb_vertices() > 0);
        unsigned int result = 0;
        float d = details::squared_distance(dimension(), vertex_ptr(0), p);
        for (unsigned int i = 1; i < nb_vertices(); i++) {
            float cur_d = details::squared_distance(dimension(), vertex_ptr(i), p);
            if (cur_d < d) {
                d = cur_d;
                result = i;
            }
        }
        return result;
    }

    void Delaunay::get_neighbors(unsigned int v, std::vector<unsigned int> &neighbors) const {
        assert(v < nb_vertices());
        if (neighbors_.size() != 0) {
            neighbors = neighbors_[v];
        } else {
            get_neighbors_internal(v, neighbors);
        }
    }

    void Delaunay::update_neighbors() {
        std::cerr << "update neighbors begin" << std::endl;
        neighbors_.resize(nb_vertices());

        //parallel_for(*this, &Delaunay::store_neighbors, 0, nb_vertices()) ;
        /**
         * parallel_for(f, from, to) is equivalent to
         * the following loop, computed in parallel:
         * for(unsigned int i=from; i<to; i++) {
         *    f(i)
         * }
         * f may be anything that has an operator()(unsigned int i)
         */

        for (unsigned int i = 0; i < nb_vertices(); ++i) {
            get_neighbors_internal(i, neighbors_[i]);
        }

        std::cerr << "update neighbors end" << std::endl;

        const bool show_stats = false;
        if (show_stats) {
            std::size_t min_N = neighbors_[0].size();
            std::size_t max_N = neighbors_[0].size();
            std::size_t sum = neighbors_[0].size();
            for (std::size_t i = 1; i < neighbors_.size(); i++) {
                sum += neighbors_[i].size();
                min_N = std::min(min_N, neighbors_[i].size());
                max_N = std::max(max_N, neighbors_[i].size());
            }
            std::cerr << "|Ni|: min=" << min_N << " max=" << max_N
                      << " avg=" << sum / neighbors_.size() << std::endl;

            std::vector<int> histogram(max_N + 1, 0);
            for (std::size_t i = 0; i < neighbors_.size(); i++) {
                histogram[neighbors_[i].size()]++;
            }

            static const std::string hist("histogram.dat");
            std::ofstream out(hist.c_str());
            for (std::size_t i = 0; i < histogram.size(); i++) {
                out << i << " " << histogram[i] << std::endl;
            }
        }
    }

    void Delaunay::get_neighbors_internal(
            unsigned int v, std::vector<unsigned int> &neighbors
    ) const {

        //	assert(v < nb_neighbors) ;
        //	assert(v < neighbors_.size()) ;


        // Step 1: traverse the incident cells list, and insert
        // all neighbors (may be duplicated)
        neighbors.resize(0);
        int vt = v_to_cell_[v];
        if (vt != -1) { // Happens when there are geometrically duplicated vertices.
            int t = vt;
            do {
                unsigned int lvit = index(t, v);
                // In the current cell, test all edges incident to current vertex 'it'
                for (unsigned int lv = 0; lv < cell_size(); lv++) {
                    if (lvit != lv) {
                        int neigh = cell_vertex(t, lv);
                        assert(neigh != -1);
                        neighbors.push_back(neigh);
                    }
                }
                t = next_around_vertex(t, index(t, v));
            } while (t != vt);
        }

        // Step 2: Sort the neighbors and remove all duplicates
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(
                std::unique(neighbors.begin(), neighbors.end()), neighbors.end()
        );
    }

    void Delaunay::update_v_to_cell() {
        assert(!is_locked_); // Not thread-safe
        is_locked_ = true;
        v_to_cell_.assign(nb_vertices(), -1);
        for (unsigned int c = 0; c < nb_cells(); c++) {
            for (unsigned int lv = 0; lv < cell_size(); lv++) {
                v_to_cell_[cell_vertex(c, lv)] = c;
            }
        }
        is_locked_ = false;
    }

    void Delaunay::update_cicl() {
        assert(!is_locked_); // Not thread-safe
        is_locked_ = true;
        cicl_.resize(cell_size() * nb_cells());
        for (unsigned int v = 0; v < nb_vertices(); v++) {
            int t = v_to_cell_[v];
            if (t != -1) {
                unsigned int lv = index(t, v);
                set_next_around_vertex(t, lv, t);
            }
        }
        for (unsigned int t = 0; t < nb_cells(); t++) {
            for (unsigned int lv = 0; lv < cell_size(); lv++) {
                unsigned int v = cell_vertex(t, lv);
                if (v_to_cell_[v] != t) {
                    unsigned int t1 = v_to_cell_[v];
                    unsigned int lv1 = index(t1, v);
                    unsigned int t2 = next_around_vertex(t1, lv1);
                    set_next_around_vertex(t1, lv1, t);
                    set_next_around_vertex(t, lv, t2);
                }
            }
        }
        is_locked_ = false;
    }


    bool Delaunay::check_duplicate_vertices() {
        bool result = false;
        if (neighbors_.size() != nb_vertices()) {
            LOG(WARNING) << "failed checking duplicate vertices: neighbors not stored";
            return false;
        }
        for (unsigned int v = 0; v < nb_vertices(); v++) {
            std::vector<unsigned int> N = neighbors_[v];
            for (unsigned int i = 0; i < N.size(); i++) {
                unsigned int w = N[i];
                double d = details::squared_distance(dimension(), vertex_ptr(v), vertex_ptr(w));
                if (d < 1e-30) {
                    LOG(WARNING) << "Vertices " << v << " and " << w << " are the same";

                    N[i] = v;
                    //		    neighbors_[w].clear() ;
                    result = true;
                }
            }
            while (N.size() > 0 && N[0] == v) { N.erase(N.begin()); }
        }
        return result;
    }


}