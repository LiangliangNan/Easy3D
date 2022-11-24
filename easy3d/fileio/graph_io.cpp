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

#include <easy3d/fileio/graph_io.h>
#include <easy3d/core/graph.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    Graph* GraphIO::load(const std::string& file_name)
	{
        auto graph = new Graph;
        graph->set_name(file_name);

        StopWatch w;
        bool success = false;

        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "ply")
            success = io::load_ply(file_name, graph);
        else if (ext.empty()){
            LOG(ERROR) << "unknown file format: no extension" << ext;
            success = false;
        }
        else {
            LOG(ERROR) << "unknown file format: " << ext << ". Only PLY format is supported for Graph" << ext;
            return nullptr;
        }

        if (!success || graph->n_vertices() == 0) {
            LOG(WARNING) << "load graph failed: " << file_name;
            delete graph;
            return nullptr;
        }

        LOG(INFO) << "graph loaded ("
                  << "#vertex: " << graph->n_vertices() << ", "
                  << "#edge: " << graph->n_edges() << "). "
                  << w.time_string();

        return graph;
	}


    bool GraphIO::save(const std::string& file_name, const Graph* graph)
	{
        if (!graph || graph->n_vertices() == 0) {
            LOG(ERROR) << "graph is null";
			return false;
		}

		StopWatch w;
		bool success = false;

        std::string final_name = file_name;
        const std::string& ext = file_system::extension(file_name, true);

        if (ext == "ply" || ext.empty()) {
            if (ext.empty()) {
                LOG(ERROR) << "No extension specified. Default to ply";
                final_name = final_name + ".ply";
            }
            success = io::save_ply(final_name, graph, true);
        } else {
            LOG(ERROR) << "unknown file format: " << ext << ". Only PLY format is supported for Graph";
            success = false;
        }

        if (success) {
            LOG(INFO) << "save model done. " << w.time_string();
            return true;
        } else {
            LOG(INFO) << "save model failed";
            return false;
        }
	}


} // namespace easy3d
