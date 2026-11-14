/** \cond LICENSE
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
 \endcond */

#include <easy3d/algo/graph_minimum_spanning_tree.h>

#include <easy3d/core/graph.h>
#include <easy3d/util/logging.h>

#include <cmath>
#include <limits>
#include <queue>
#include <vector>


namespace easy3d {

    namespace {

        // A vertex waiting in the heap, keyed by the weight of the cheapest edge reaching it.
        struct Candidate {
            float key;
            int vertex;
        };

        // Ties are broken by the vertex index so that the tree does not depend on the order
        // in which the equal weight edges were relaxed.
        struct CandidateGreater {
            bool operator()(const Candidate &lhs, const Candidate &rhs) const {
                return lhs.key > rhs.key || (lhs.key == rhs.key && lhs.vertex > rhs.vertex);
            }
        };

    }

    bool GraphMinimumSpanningTree::compute(const Graph *graph, const std::vector<float> &weight,
                                           const std::vector<int> &roots, std::vector<int> &predecessor) {
        if (!graph) {
            LOG(ERROR) << "empty input graph";
            return false;
        }

        if (graph->edges_size() > weight.size()) {
            LOG(ERROR) << "graph has " << graph->edges_size() << " edges but only " << weight.size()
                       << " weights are given";
            return false;
        }

        // a NaN weight would silently discard the edges carrying it
        for (auto w: weight) {
            if (!std::isfinite(w)) {
                LOG(ERROR) << "edge weights must be finite";
                return false;
            }
        }

        const int num = static_cast<int>(graph->vertices_size());
        predecessor.assign(static_cast<std::size_t>(num), -1);

        std::vector<char> in_tree(static_cast<std::size_t>(num), 0);
        std::vector<float> dist(static_cast<std::size_t>(num), std::numeric_limits<float>::infinity());

        std::priority_queue<Candidate, std::vector<Candidate>, CandidateGreater> heap;
        // A root grows its own tree and is never reached from another one, so the result is a
        // forest of exactly one tree per root even when a negative weight beats the zero of a
        // root that has not been popped yet.
        std::vector<char> root(static_cast<std::size_t>(num), 0);
        for (auto r : roots) {
            if (r < 0 || r >= num || graph->is_deleted(Graph::Vertex(r))) {
                LOG(WARNING) << "cannot grow a tree from invalid vertex " << r;
                return false;
            }
            root[static_cast<std::size_t>(r)] = 1;
            dist[static_cast<std::size_t>(r)] = 0;
            heap.push(Candidate{0, r});
        }

        while (!heap.empty()) {
            const int idx = heap.top().vertex;
            heap.pop();

            // the vertex was reached more cheaply by an earlier entry of the heap
            if (in_tree[static_cast<std::size_t>(idx)])
                continue;
            in_tree[static_cast<std::size_t>(idx)] = 1;

            const Graph::Vertex v(idx);
            for (auto e : graph->edges(v)) {
                Graph::Vertex neighbor = graph->vertex(e, 0);
                if (neighbor == v)
                    neighbor = graph->vertex(e, 1);
                const int nidx = neighbor.idx();
                if (in_tree[static_cast<std::size_t>(nidx)] || root[static_cast<std::size_t>(nidx)])
                    continue;

                // the adjacency lists of the graph never contain a deleted edge
                const float key = weight[static_cast<std::size_t>(e.idx())];
                if (key < dist[static_cast<std::size_t>(nidx)]) {
                    dist[static_cast<std::size_t>(nidx)] = key;
                    predecessor[static_cast<std::size_t>(nidx)] = idx;
                    heap.push(Candidate{key, nidx});
                }
            }
        }

        return true;
    }

} // namespace easy3d
