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
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


// This example shows how to
//		- load a graph from a file;
//		- save a graph into a file.


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    // Read a graph specified by its file name
    const std::string file_name = resource::directory() + "/data/graph.ply";
    Graph* graph = GraphIO::load(file_name);
    if (!graph) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
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

    // delete the graph (i.e., release memory)
    delete graph;

    return EXIT_SUCCESS;
}

