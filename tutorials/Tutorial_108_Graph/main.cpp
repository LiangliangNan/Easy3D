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
 * \example{lineno} Tutorial_108_Graph/main.cpp
 *
 * \brief Building a Graph from vertices and edges — the 1D connectivity counterpart of a mesh.
 *
 * <b>Overview</b>
 * This console tutorial creates the simplest Graph: four vertices positioned in 3D and five edges
 * joining them, then prints the vertex and edge counts. It is the starting point for the graph
 * series (109 connectivity, 110 properties, 111 I/O), just as Tutorial_104 was for surface meshes.
 *
 * <b>The technique</b>
 * A Graph stores only vertices and edges — there are no faces and no halfedges. It models pure 1D
 * connectivity: networks, curves, skeletons, road maps, or the edge graph extracted from a mesh.
 * Each vertex still carries a 3D position (the built-in "v:point" property) so the graph can be
 * laid out and drawn in space, and each edge simply records the pair of vertices it joins.
 * Internally the structure keeps per-vertex adjacency so incident edges/neighbors can be queried
 * efficiently (Tutorial_109). Compare the three core structures: PointCloud has points only,
 * Graph adds 1D edges, SurfaceMesh adds 2D faces (halfedges), and PolyMesh adds 3D cells
 * (halffaces). The five edges here form a 4-cycle (v0-v1-v2-v3-v0) plus one diagonal (v1-v3).
 *
 * <b>Easy3D API</b>
 * - Graph: the core graph data structure (a Model subclass).
 * - Graph::add_vertex(pos): appends a positioned vertex, returning a Vertex handle.
 * - Graph::add_edge(a, b): connects two vertices, returning an Edge handle.
 * - Graph::n_vertices()/n_edges(): element counts.
 *
 * <b>Running it</b>
 * Console-only. It prints "vertices: 4" and "edges: 5". The graph is heap-allocated with new and
 * explicitly deleted at the end to release its memory.
 *
 * \see Graph, Tutorial_109_Graph_Connectivity, Tutorial_110_Graph_Property, Tutorial_104_SurfaceMesh
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create an empty graph. Unlike a SurfaceMesh it holds only vertices and edges (no faces).
    auto graph = new Graph;

    // Add 4 vertices
    Graph::Vertex v0 = graph->add_vertex(vec3(0, 0, 0));
    Graph::Vertex v1 = graph->add_vertex(vec3(1, 0, 0));
    Graph::Vertex v2 = graph->add_vertex(vec3(0, 1, 0));
    Graph::Vertex v3 = graph->add_vertex(vec3(0, 0, 1));

    // Add some edges. add_edge() returns an Edge handle; the trailing comments label them e0..e4
    // in creation order. Together they form a 4-cycle (v0-v1-v2-v3-v0) plus the diagonal v1-v3.
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
