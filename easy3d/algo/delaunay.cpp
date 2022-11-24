/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/


#include <easy3d/algo/delaunay.h>
#include <algorithm>


namespace easy3d {


    // \cond
    namespace internal {
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


    Delaunay::~Delaunay() = default;


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
        float d = internal::squared_distance(dimension(), vertex_ptr(0), p);
        for (unsigned int i = 1; i < nb_vertices(); i++) {
            float cur_d = internal::squared_distance(dimension(), vertex_ptr(i), p);
            if (cur_d < d) {
                d = cur_d;
                result = i;
            }
        }
        return result;
    }

    void Delaunay::get_neighbors(unsigned int v, std::vector<unsigned int> &neighbors) const {
        assert(v < nb_vertices());
        if (neighbors_.empty()) {
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

#if 0
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
#endif
    }

    void Delaunay::get_neighbors_internal(
            unsigned int v, std::vector<unsigned int> &neighbors
    ) const {

        //	assert(v < nb_neighbors) ;
        //	assert(v < neighbors_.size()) ;


        // Step 1: traverse the incident cells list, and insert
        // all neighbors (maybe duplicated)
        neighbors.resize(0);
        int vt = v_to_cell_[v];
        if (vt != -1) { // Happens when there are geometrically duplicated vertices.
            unsigned int t = vt;
            do {
                unsigned int lvit = index(t, static_cast<int>(v));
                // In the current cell, test all edges incident to current vertex 'it'
                for (unsigned int lv = 0; lv < cell_size(); lv++) {
                    if (lvit != lv) {
                        int neigh = cell_vertex(t, lv);
                        assert(neigh != -1);
                        neighbors.push_back(static_cast<unsigned int>(neigh));
                    }
                }
                t = next_around_vertex(t, index(t, static_cast<int>(v)));
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
                v_to_cell_[cell_vertex(c, lv)] = static_cast<int>(c);
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
                unsigned int lv = index(static_cast<unsigned int>(t), static_cast<int>(v));
                set_next_around_vertex(t, lv, t);
            }
        }
        for (unsigned int t = 0; t < nb_cells(); t++) {
            for (unsigned int lv = 0; lv < cell_size(); lv++) {
                unsigned int v = cell_vertex(t, lv);
                if (v_to_cell_[v] != t) {
                    unsigned int t1 = v_to_cell_[v];
                    unsigned int lv1 = index(t1, static_cast<int>(v));
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
            for (auto& w : N) {
                double d = internal::squared_distance(dimension(), vertex_ptr(v), vertex_ptr(w));
                if (d < 1e-30) {
                    LOG(WARNING) << "Vertices " << v << " and " << w << " are the same";

                    w = v;
                    //		    neighbors_[w].clear() ;
                    result = true;
                }
            }
            while (!N.empty() && N[0] == v) { N.erase(N.begin()); }
        }
        return result;
    }


}