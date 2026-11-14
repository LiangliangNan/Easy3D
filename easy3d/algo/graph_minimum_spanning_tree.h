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

#ifndef EASY3D_ALGO_GRAPH_MINIMUM_SPANNING_TREE_H
#define EASY3D_ALGO_GRAPH_MINIMUM_SPANNING_TREE_H


#include <vector>


namespace easy3d {

    class Graph;

    /**
     * \brief Computes the minimum spanning tree of a weighted graph.
     * \details The class implements Prim's algorithm with a binary heap, which runs in
     *          O((V + E) log V) and needs no edge decrease-key operation. It is the
     *          replacement for the Boost Graph Library usage in \ref PointCloudNormals::reorient
     *          in the original Easy3D implementation.
     * \class GraphMinimumSpanningTree easy3d/algo/graph_minimum_spanning_tree.h
     */
    class GraphMinimumSpanningTree {
    public:
        /**
         * \brief Computes a minimum spanning forest of a graph.
         * \details Each tree of the forest is grown from one of the \c roots and spans that
         *          root's connected component. Since an edge never connects two components,
         *          giving one root per component covers the whole graph and every tree is a
         *          minimum spanning tree of its component.
         * \param graph The input graph.
         * \param weight The weight of every edge, indexed by \c Graph::Edge::idx(). All edges
         *          of the graph must have a weight and the weights must be finite. Equal
         *          weights are allowed.
         * \param roots Vertex indices (as in \c Graph::Vertex::idx()) the forest is grown
         *          from. They must be valid vertices of the graph.
         * \param predecessor The result, one entry per vertex (\c Graph::vertices_size()):
         *          the vertex each vertex was reached from, or -1 for a root and for the
         *          vertices not connected to any root. Indices refer to \c Graph::Vertex::idx().
         * \return True if the computation succeeded, false otherwise.
         */
        static bool compute(const Graph *graph, const std::vector<float> &weight,
                            const std::vector<int> &roots, std::vector<int> &predecessor);
    };


} // namespace easy3d


#endif  // EASY3D_ALGO_GRAPH_MINIMUM_SPANNING_TREE_H
