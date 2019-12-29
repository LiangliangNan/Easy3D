/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/core/graph.h>

using namespace easy3d;


// This example shows how to
//		- construct a graph from its vertices and known connectivities


int main(int /*argc*/, char** /*argv*/) {
    // Create a graph
    Graph* g = new Graph;

	// Add 4 vertices
    Graph::Vertex v0 = g->add_vertex(vec3(0, 0, 0));
    Graph::Vertex v1 = g->add_vertex(vec3(1, 0, 0));
    Graph::Vertex v2 = g->add_vertex(vec3(0, 1, 0));
    Graph::Vertex v3 = g->add_vertex(vec3(0, 0, 1));

    // Add some edges
    g->add_edge(v0, v1); // e0
    g->add_edge(v1, v2); // e1
    g->add_edge(v2, v3); // e2
    g->add_edge(v3, v0); // e3
    g->add_edge(v1, v3); // e4

    std::cout << "vertices: " << g->n_vertices() << std::endl;
    std::cout << "edges: " << g->n_edges() << std::endl;

    //---------------------------

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";


    // loop over all vertices
    for (auto v : g->vertices()) {
        std::cout << "incident vertices of vertex " << v << ": ";
        // loop over all incident vertices
        for (auto vv : g->vertices(v))
            std::cout << vv << " ";
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident edges of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : g->vertices()) {
        std::cout << "incident edges of vertex " << v << ": ";
        // loop over all incident outgoing/ingoing edges
        for (auto e : g->edges(v))
            std::cout << e << " ";
        std::cout << std::endl;
    }


    std::cout << "\n--------------------------------------\n";
    std::cout << "The two end points of each edge" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all edges
    for (auto e : g->edges()) {
        std::cout << "the two end points of edge " << e << ": ";
        Graph::Vertex vs = g->from_vertex(e);
        std::cout << vs << " ";
        Graph::Vertex vt = g->to_vertex(e);
        std::cout << vt << " " << std::endl;
    }

    // Delete the graph (i.e., release memory)
    delete g;

    return EXIT_SUCCESS;
}
