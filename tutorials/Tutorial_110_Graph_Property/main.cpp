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

#include <easy3d/core/graph.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_110_Graph_Property/main.cpp
 *
 * \brief Adding per-vertex and per-edge properties (colors, lengths) to a Graph and reading them back.
 *
 * <b>Overview</b>
 * This console tutorial extends the property lessons of Tutorial_102 and Tutorial_106 to graphs.
 * On the graph from Tutorial_108 it creates a per-vertex color property "v:color" and a per-edge
 * length property "e:length", fills them, and prints each vertex's position/color and each edge's
 * length. It demonstrates that graphs support both vertex and edge properties.
 *
 * <b>The technique</b>
 * Properties are parallel arrays indexed by element handle, named with a "v:" (vertex) or "e:"
 * (edge) prefix. The vertex positions are the built-in "v:point" property; here we fetch it with
 * vertex_property<T>(), a get-or-create convenience that returns the existing property (or adds
 * it if missing). For demonstration the color of each vertex is set equal to its position — in
 * real code you would assign a meaningful color. The length of an edge is the Euclidean distance
 * between its two endpoints; Graph::edge_length() computes it, and we cache the result in
 * "e:length" so later algorithms (e.g. shortest paths) can reuse it without recomputing.
 *
 * <b>Easy3D API</b>
 * - Graph::vertex_property<T>(name): get-or-add a vertex property (used for "v:point").
 * - Graph::add_vertex_property<T>(name) / add_edge_property<T>(name): create new properties,
 *   returning VertexProperty<T> / EdgeProperty<T> accessors indexed by handle.
 * - Graph::vertices()/edges(): ranges to iterate; Graph::edge_length(e): endpoint distance.
 *
 * <b>Running it</b>
 * Console-only. It prints one line per vertex ("vertex: ..., position: ..., color: ...") then one
 * line per edge ("edge: ..., length: ..."). The graph is heap-allocated and deleted at the end.
 *
 * \see Graph, Tutorial_102_PointCloud_Property, Tutorial_106_SurfaceMesh_Property, Tutorial_108_Graph
 */

using namespace easy3d;

// the graph created in the previous tutorial (so you can skip it)
Graph *old_graph_from_previous_example() {
    // Create a graph
    auto graph = new Graph;

    // Add 4 vertices
    Graph::Vertex v0 = graph->add_vertex(vec3(0, 0, 0));
    Graph::Vertex v1 = graph->add_vertex(vec3(1, 0, 0));
    Graph::Vertex v2 = graph->add_vertex(vec3(0, 1, 0));
    Graph::Vertex v3 = graph->add_vertex(vec3(0, 0, 1));

    // Add some edges
    graph->add_edge(v0, v1); // e0
    graph->add_edge(v1, v2); // e1
    graph->add_edge(v2, v3); // e2
    graph->add_edge(v3, v0); // e3
    graph->add_edge(v1, v3); // e4

    return graph;
}

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    Graph *graph = old_graph_from_previous_example();

    // Fetch the built-in position property. vertex_property() is "get-or-add": it returns the
    // existing "v:point" array (creating it only if it were somehow absent).
    auto points = graph->vertex_property<vec3>("v:point");

    // We add a per-vertex property "v:color" to assign a color to each vertex
    auto colors = graph->add_vertex_property<vec3>("v:color");
    for (auto v : graph->vertices()) {
        // We assign each vertex a color that is equal to his position (you should
        // assign a more meaningful color in practice).
        colors[v] = points[v];
        std::cout << "vertex: " << v << ", position: " << points[v] << ", color: " << colors[v] << std::endl;
    }

    // We add a per-edge property "e:length" to store the edge lengths
    auto lengths = graph->add_edge_property<float>("e:length");
    for (auto e : graph->edges()) {
        // edge_length() is the Euclidean distance between the edge's two endpoints; we cache it.
        lengths[e] = graph->edge_length(e);
        std::cout << "edge: " << e << ", length: " << lengths[e] << std::endl;
    }

    delete graph;

    return EXIT_SUCCESS;
}
