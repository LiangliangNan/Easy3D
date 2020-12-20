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

#include <easy3d/fileio/tetra_mesh_io.h>

#include <clocale>

#include <easy3d/core/tetra_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>


namespace easy3d {


	TetraMesh* TetraMeshIO::load(const std::string& file_name)
	{
		std::setlocale(LC_NUMERIC, "C");

        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return nullptr;
        }

        const std::string& ext = file_system::extension(file_name, true);
        if (ext != "tet") {
            LOG(ERROR) << "unsupported file format: " << ext;
            return nullptr;
        }

        TetraMesh* mesh = new TetraMesh;
        mesh->set_name(file_name);

		StopWatch w;

        std::string dummy;
        int num_vertices, num_tets;
        input >> dummy >> num_vertices >> num_tets;

        vec3 p;
        for (std::size_t i = 0; i < num_vertices; ++i) {
            input >> p;
            mesh->add_vertex(TetraMesh::Vertex(p.x, p.y, p.z));
        }

        ivec4 v;
        for (std::size_t i = 0; i < num_tets; ++i) {
            input >> v;
            mesh->add_tetra(TetraMesh::Tetra(v[0], v[1], v[2], v[3]));
        }

		if (num_vertices == 0 || num_tets == 0) {
            LOG(WARNING) << "no valid data in file: " << file_name;
			delete mesh;
			return nullptr;
		}
		else {
		    mesh->build_adjacency();

            LOG(INFO) << "tetrhedral mesh loaded ("
                      << "#vertex: " << mesh->n_vertices() << ", "
                      << "#tetrahedra: " << mesh->n_tetrahedra() << "). "
                      << w.time_string();
            return mesh;
        }
	}


	bool TetraMeshIO::save(const std::string& file_name, const TetraMesh* mesh)
	{
        if (!mesh || mesh->n_vertices() == 0 || mesh->n_tetrahedra() == 0) {
			LOG(ERROR) << "surface mesh is null";
			return false;
		}

        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        const std::string& ext = file_system::extension(file_name, true);
        if (ext != "tet") {
            LOG(ERROR) << "unsupported file format: " << ext;
            return false;
        }

        const std::vector<vec3>& verts = mesh->points();
        const std::vector<TetraMesh::Tetra>& tets = mesh->tets();

		StopWatch w;

        output << "tet " << verts.size() << " " << tets.size() << std::endl;

        for (std::size_t i = 0; i < verts.size(); ++i)
            output << verts[i] << std::endl;

        for (std::size_t i = 0; i < tets.size(); ++i)
            output << tets[i] << std::endl;

        LOG(INFO) << "save model done. " << w.time_string();
        return true;
	}

} // namespace easy3d
