/********************************************************************
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
 ********************************************************************/

#include <easy3d/core/graph.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;

// This example shows how to
//		- access the incident vertices of each vertex
//		- access the incident edges of each vertex
//		- access the two end points of each edge

// There are two ways to traverse the incident entities of an element.
//   - use a "for" loop (cleaner code);
//   - use a circulator.
#define USE_FOR_LOOP

// the graph created in the previous tutorial (so you can skip it)
Graph* old_graph_from_previous_example() {
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


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

    Graph* graph = old_graph_from_previous_example();

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
#else   // use circulator
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
#else   // use circulator
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
