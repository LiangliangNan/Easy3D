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

#include <easy3d/fileio/poly_mesh_io.h>

#include <clocale>

#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    PolyMesh *PolyMeshIO::load(const std::string &file_name) {
        std::setlocale(LC_NUMERIC, "C");

        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return nullptr;
        }

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "plm") {
            LOG(ERROR) << "unsupported file format: " << ext;
            return nullptr;
        }

        PolyMesh *mesh = new PolyMesh;
        mesh->set_name(file_name);

        StopWatch w;

        std::string dummy;
        int num_vertices, num_cells;
        input >> dummy >> num_vertices >> dummy >> num_cells;

        vec3 p;
        for (std::size_t v = 0; v < num_vertices; ++v) {
            input >> p;
            mesh->add_vertex(p);
        }

        int num_halffaces, num_valence, idx;
        for (std::size_t c = 0; c < num_cells; ++c) {
            input >> num_halffaces;
            std::vector<PolyMesh::HalfFace> halffaces(num_halffaces);
            for (std::size_t hf = 0; hf < num_halffaces; ++hf) {
                input >> num_valence;
                std::vector<PolyMesh::Vertex> vts(num_valence);
                for (std::size_t v = 0; v < num_valence; ++v) {
                    input >> idx;
                    vts[v] = PolyMesh::Vertex(idx);
                }
                halffaces[hf] = mesh->add_face(vts);
            }
            mesh->add_cell(halffaces);
        }

        if (num_vertices == 0 || num_cells == 0) {
            LOG(WARNING) << "no valid data in file: " << file_name;
            delete mesh;
            return nullptr;
        } else {
            LOG(INFO) << "polyhedral mesh loaded ("
                      << "#vertex: " << mesh->n_vertices() << ", "
                      << "#edge: " << mesh->n_edges() << ", "
                      << "#face: " << mesh->n_faces() << ", "
                      << "#cell: " << mesh->n_cells() << "). "
                      << w.time_string();

            return mesh;
        }
    }


    bool PolyMeshIO::save(const std::string &file_name, const PolyMesh *mesh) {
        if (!mesh || mesh->n_vertices() == 0 || mesh->n_cells() == 0) {
            LOG(ERROR) << "surface mesh is null";
            return false;
        }

        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "plm") {
            LOG(ERROR) << "unsupported file format: " << ext;
            return false;
        }

        StopWatch w;

        output << "#vertices " << mesh->n_vertices() << std::endl
               << "#cells    " << mesh->n_cells() << std::endl;

        for (auto v : mesh->vertices())
            output << mesh->position(v) << std::endl;

        for (auto c : mesh->cells()) {
            int num_halffaces = mesh->halffaces(c).size();
            output << num_halffaces << std::endl;
            for (auto h : mesh->halffaces(c)) {
                output << mesh->vertices(h).size() << " ";
                for (auto v : mesh->vertices(h))
                    output << v.idx() << " ";
                output << std::endl;
            }
        }

        LOG(INFO) << "save model done. " << w.time_string();
        return true;
    }

} // namespace easy3d
