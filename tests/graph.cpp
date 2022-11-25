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
#include <easy3d/fileio/graph_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>


using namespace easy3d;

int test_graph() {
    // Create a graph
    Graph graph;

    // construct a graph with 4 vertices and 5 edges
    {
        // Add 4 vertices
        Graph::Vertex v0 = graph.add_vertex(vec3(0, 0, 0));
        Graph::Vertex v1 = graph.add_vertex(vec3(1, 0, 0));
        Graph::Vertex v2 = graph.add_vertex(vec3(0, 1, 0));
        Graph::Vertex v3 = graph.add_vertex(vec3(0, 0, 1));

        // Add some edges
        graph.add_edge(v0, v1); // e0
        graph.add_edge(v1, v2); // e1
        graph.add_edge(v2, v3); // e2
        graph.add_edge(v3, v0); // e3
        graph.add_edge(v1, v3); // e4
    }
    
    // test the access of graph values and adjacency information
    {
        // loop over all vertices
        for (auto v : graph.vertices()) {
            std::cout << "incident vertices of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident vertices
        for (auto vv : graph.vertices(v))
            std::cout << vv << " ";
#else   // use circulator
            Graph::VertexAroundVertexCirculator cir = graph.vertices(v);
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
        for (auto v : graph.vertices()) {
            std::cout << "incident edges of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident outgoing/ingoing edges
        for (auto h : graph.edges(v))
            std::cout << h << " ";
#else   // use circulator
            Graph::EdgeAroundVertexCirculator cir = graph.edges(v);
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
        for (auto e : graph.edges()) {
            std::cout << "the two end points of edge " << e << ": ";
            Graph::Vertex vs = graph.vertex(e, 0);
            std::cout << vs << " ";
            Graph::Vertex vt = graph.vertex(e, 1);
            std::cout << vt << " " << std::endl;
        }

        // -----------------------------

        auto points = graph.vertex_property<vec3>("v:point");

        // We add a per-vertex property "v:color" to assign a color to each vertex
        auto colors = graph.add_vertex_property<vec3>("v:color");
        for (auto v : graph.vertices()) {
            // We assign each vertex a color that is equal to his position (you should
            // assign a more meaningful color in practice).
            colors[v] = points[v];
            std::cout << "vertex: " << v << ", position: " << points[v] << ", color: " << colors[v]
                      << std::endl;
        }

        // We add a per-edge property "e:length" to store the edge lengths
        auto lengths = graph.add_edge_property<float>("e:length");
        for (auto e : graph.edges()) {
            lengths[e] = graph.edge_length(e);
            std::cout << "edge: " << e << ", length: " << lengths[e] << std::endl;
        }
    }

    {
        // Read a graph specified by its file name
        const std::string file_name = resource::directory() + "/data/graph.ply";
        Graph* graph = GraphIO::load(file_name);
        if (!graph) {
            LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
            return EXIT_FAILURE;
        }
        std::cout << "graph loaded. " << std::endl;
        std::cout << "\tvertices: " << graph->n_vertices() << std::endl;
        std::cout << "\tedges: " << graph->n_edges() << std::endl;

        // ...
        // Do fancy stuff with the graph
        // ...

        // Write the graph to a new file.
        const std::string save_file_name = "./graph-copy.ply";
        if (GraphIO::save(save_file_name, graph))
            std::cout << "graph saved to \'" << save_file_name << "\'"  << std::endl;
        else
            std::cerr << "failed create the new file" << std::endl;

        if (file_system::delete_file(save_file_name))
            std::cout << "the saved file has been deleted"  << std::endl;
        else
            std::cerr << "failed to delete the saved file" << std::endl;

        // delete the graph (i.e., release memory)
        delete graph;
    }

    return EXIT_SUCCESS;
}
