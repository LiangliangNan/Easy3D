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
 * \example{lineno} Tutorial_109_Graph_Connectivity/main.cpp
 *
 * \brief Querying the adjacency of a Graph: neighbors, incident edges, and edge endpoints.
 *
 * <b>Overview</b>
 * Using the four-vertex / five-edge graph from Tutorial_108_Graph, this console tutorial prints
 * the three basic graph adjacency relationships:
 *      - the incident vertices (neighbors) of each vertex
 *      - the incident edges of each vertex
 *      - the two end points of each edge
 * As in Tutorial_105_SurfaceMesh_Connectivity, each traversal is written both as a range-based for
 * loop and as an explicit circulator, chosen with the USE_FOR_LOOP macro.
 *
 * <b>The technique</b>
 * A Graph has no faces or halfedges, so its connectivity is simpler than a surface mesh's: it is
 * essentially an adjacency list. Each vertex remembers the edges touching it; following an edge
 * to its other endpoint gives a neighbor. vertices(v) walks those neighbors, edges(v) walks the
 * incident edges, and vertex(e, 0/1) returns an edge's two endpoints. A circulator iterates a
 * vertex's incident edges in order and wraps back to the start (it has no natural end), which is
 * why the explicit form saves the start handle and loops until it returns there; the for-loop
 * wrapper hides that detail. These are the building blocks for graph algorithms such as
 * traversal, shortest paths, and connected components.
 *
 * <b>Easy3D API</b>
 * - Graph::vertices(v): the neighboring vertices of v.
 * - Graph::edges(v): the edges incident to v.
 * - Graph::vertex(e, i): the i-th endpoint (0 or 1) of edge e.
 * - Graph::vertices()/edges(): ranges over all vertices / all edges.
 * - VertexAroundVertexCirculator / EdgeAroundVertexCirculator: the explicit traversal forms.
 *
 * <b>Running it</b>
 * Console-only. It prints a titled section for each query, listing element handles (e.g. "v0",
 * "e1") for every vertex and edge. The graph is heap-allocated and deleted at the end.
 *
 * \see Graph, Tutorial_108_Graph, Tutorial_105_SurfaceMesh_Connectivity, Tutorial_113_PolyMesh_Connectivity
 */

using namespace easy3d;

// There are two ways to traverse the incident entities of an element.
//   - use a "for" loop (cleaner code);
//   - use a circulator.
#define USE_FOR_LOOP

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

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : graph->vertices()) {
        std::cout << "incident vertices of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident vertices
        for (auto vv : graph->vertices(v))
            std::cout << vv << " ";
#else // use circulator
        // A circulator wraps around the incident edges and has no natural end, so we save the
        // start position and stop once advancing brings us back to it.
        Graph::VertexAroundVertexCirculator cir = graph->vertices(v);
        Graph::VertexAroundVertexCirculator end = cir;
        do {
            Graph::Vertex vv = *cir;
            std::cout << vv << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident edges of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : graph->vertices()) {
        std::cout << "incident edges of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident outgoing/ingoing edges
        for (auto h : graph->edges(v))
            std::cout << h << " ";
#else // use circulator
        Graph::EdgeAroundVertexCirculator cir = graph->edges(v);
        Graph::EdgeAroundVertexCirculator end = cir;
        do {
            Graph::Edge h = *cir;
            std::cout << h << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The two end points of each edge" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all edges
    for (auto e : graph->edges()) {
        std::cout << "the two end points of edge " << e << ": ";
        Graph::Vertex vs = graph->vertex(e, 0);
        std::cout << vs << " ";
        Graph::Vertex vt = graph->vertex(e, 1);
        std::cout << vt << " " << std::endl;
    }

    // Delete the graph (i.e., release memory)
    delete graph;

    return EXIT_SUCCESS;
}
