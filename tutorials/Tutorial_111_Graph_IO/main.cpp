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
#include <easy3d/fileio/graph_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_111_Graph_IO/main.cpp
 *
 * \brief Loading a Graph from a PLY file and saving it back out with GraphIO.
 *
 * <b>Overview</b>
 * The graph counterpart of Tutorial_103 (point clouds) and Tutorial_107 (surface meshes). This
 * console program loads the bundled "graph.ply", prints its vertex and edge counts, leaves a
 * placeholder for processing, and writes a copy "graph-copy.ply". It follows the same
 * load -> use -> save -> free lifecycle as the other I/O tutorials.
 *
 * <b>The technique</b>
 * GraphIO currently supports only the PLY format. PLY (the Polygon File Format, from Stanford)
 * is a flexible, self-describing format: a header declares the elements and their properties,
 * followed by the data, in ASCII or binary. For a graph it stores the vertex positions and the
 * edge list (pairs of vertex indices), which is exactly the 1D connectivity a Graph needs. As
 * with the other loaders, load() returns a heap-allocated Graph* (nullptr on failure) that the
 * caller owns and must delete, and save() returns a bool indicating success. Because only PLY is
 * supported, both the input and output files use the ".ply" extension.
 *
 * <b>Easy3D API</b>
 * - GraphIO::load(file) / GraphIO::save(file, graph): PLY-only graph I/O.
 * - resource::directory(): locates the bundled data folder containing graph.ply.
 * - Graph::n_vertices()/n_edges(): element counts printed after loading.
 *
 * <b>Running it</b>
 * Console-only. It prints "graph loaded." with the vertex and edge counts, then "graph saved to
 * './graph-copy.ply'". The copy is written to the current working directory.
 *
 * \see Graph, GraphIO, Tutorial_107_SurfaceMesh_IO, Tutorial_103_PointCloud_IO, Tutorial_115_PolyMesh_IO
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Read a graph specified by its file name. GraphIO supports only PLY, so the file is ".ply";
    // load() returns a heap-allocated graph (nullptr on failure) that we own and must delete.
    const std::string file_name = resource::directory() + "/data/graph.ply";
    Graph *graph = GraphIO::load(file_name);
    if (!graph) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "graph loaded. " << std::endl;
    std::cout << "\tvertices: " << graph->n_vertices() << std::endl;
    std::cout << "\tedges: " << graph->n_edges() << std::endl;

    // ...
    // Do fancy stuff with the graph
    // ...

    // Write the graph to a new PLY file. save() returns true on success, letting us confirm the
    // copy was written (the ".ply" extension is required since PLY is the only supported format).
    const std::string save_file_name = "./graph-copy.ply";
    if (GraphIO::save(save_file_name, graph))
        std::cout << "graph saved to \'" << save_file_name << "\'" << std::endl;
    else
        std::cerr << "failed create the new file" << std::endl;

    // delete the graph (i.e., release memory)
    delete graph;

    return EXIT_SUCCESS;
}
