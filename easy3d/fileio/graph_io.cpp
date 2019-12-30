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


#include <easy3d/fileio/graph_io.h>

#include <clocale>

#include <easy3d/core/graph.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


    Graph* GraphIO::load(const std::string& file_name)
	{
		std::setlocale(LC_NUMERIC, "C");

        Graph* graph = new Graph;
        graph->set_name(file_name);

        StopWatch w;
        bool success = false;

        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "ply")
            success = io::load_ply(file_name, graph);
        else if (ext.empty()){
            std::cerr << "unknown file format: no extention" << ext << std::endl;
            success = false;
        }
        else {
            std::cerr << "unknown file format: " << ext << ". Only PLY format is supported for Graph" << ext << std::endl;
            return nullptr;
        }

        if (!success || graph->n_vertices() == 0) {
            delete graph;
            return nullptr;
        }


#ifndef NDEBUG
        std::cout << "vertex properties on graph " << file_system::simple_name(file_name) << std::endl;
        const auto& vnames = graph->vertex_properties();
		for (const auto& n : vnames)
			std::cout << "\t" << n << std::endl;

        std::cout << "edge properties on graph " << file_system::simple_name(file_name) << std::endl;
        const auto& enames = graph->edge_properties();
        for (const auto& n : enames)
            std::cout << "\t" << n << std::endl;
#endif

        if (success)
            std::cout << "load model done. time: " << w.time_string() << std::endl;
        else
            std::cout << "load model failed" << std::endl;

        return graph;
	}


    bool GraphIO::save(const std::string& file_name, const Graph* graph)
	{
        if (!graph || graph->vertices_size() == 0) {
            std::cerr << "graph is null" << std::endl;
			return false;
		}

		StopWatch w;
		bool success = false;

        std::string final_name = file_name;
        const std::string& ext = file_system::extension(file_name, true);

        if (ext == "ply" || ext.empty()) {
            if (ext.empty()) {
                std::cerr << "No extention specified. Default to ply" << ext << std::endl;
                final_name = final_name + ".ply";
            }
            success = io::save_ply(final_name, graph, true);
        }
		else {
            std::cerr << "unknown file format: " << ext << ". Only PLY format is supported for Graph" << ext << std::endl;
			success = false;
		}

        if (success) {
            std::cout << "save model done. time: " << w.time_string() << std::endl;
            return true;
        }
        else {
            std::cout << "save model failed" << std::endl;
            return false;
        }
	}


} // namespace easy3d
