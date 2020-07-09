/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
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
 */

#include <easy3d/core/graph.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


// This example shows how to
//		- construct a graph from its vertices and edges


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

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

    // Delete the graph (i.e., release memory)
    delete g;

    return EXIT_SUCCESS;
}
