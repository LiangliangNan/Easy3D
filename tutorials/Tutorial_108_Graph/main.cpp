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
//		- construct a graph from its vertices and edges


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

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

    std::cout << "vertices: " << graph->n_vertices() << std::endl;
    std::cout << "edges: " << graph->n_edges() << std::endl;

    // Delete the graph (i.e., release memory)
    delete graph;

    return EXIT_SUCCESS;
}
