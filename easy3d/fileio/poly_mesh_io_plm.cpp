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

#include <easy3d/fileio/poly_mesh_io.h>

#include <fstream>

#include <easy3d/fileio/translator.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/progress.h>


namespace easy3d {

    namespace io {

        bool load_plm(const std::string& file_name, PolyMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            std::ifstream input(file_name.c_str());
            if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            std::string dummy;
            int num_vertices(0), num_cells(0);
            input >> dummy >> num_vertices >> dummy >> num_cells;

            ProgressLogger progress(num_vertices + num_cells, true, false);

            if (Translator::instance()->status() == Translator::DISABLED) {
                vec3 p;
                for (std::size_t v = 0; v < num_vertices; ++v) {
                    input >> p;
                    mesh->add_vertex(p);
                    progress.next();
                }
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                dvec3 p, origin;
                for (std::size_t v = 0; v < num_vertices; ++v) {
                    input >> p;
                    if (v == 0) { // the first point
                        origin = p;
                        Translator::instance()->set_translation(origin);
                    }
                    mesh->add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                    progress.next();
                }

                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0,0,0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << trans[0] << "), stored as ModelProperty<dvec3>(\"translation\")";
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                dvec3 p;
                for (std::size_t v = 0; v < num_vertices; ++v) {
                    input >> p;
                    mesh->add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                    progress.next();
                }

                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0,0,0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. last known reference point (" << origin << "), stored as ModelProperty<dvec3>(\"translation\")";
            }

            int num_halffaces(0), num_valence(0), idx(0);
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
                progress.next();
            }

            return (mesh->n_vertices() > 0 && mesh->n_faces() > 0 && mesh->n_cells() > 0);
        }


        //-----------------------------------------------------------------------------


        bool save_plm(const std::string& file_name, const PolyMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
                LOG(ERROR) << "polyhedral mesh is empty";
                return false;
            }

            std::ofstream output(file_name.c_str());
            if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            output << "#vertices " << mesh->n_vertices() << std::endl
                   << "#cells    " << mesh->n_cells() << std::endl;

            auto trans = mesh->get_model_property<dvec3>("translation");
            dvec3 origin(0, 0, 0);
            if (trans)  // has translation
                origin = trans[0];

            ProgressLogger progress(mesh->n_vertices() + mesh->n_cells(), true, false);
            for (auto v : mesh->vertices()) {
                const auto& p = mesh->position(v);
                output << p.x + origin.x << " "
                       << p.y + origin.y << " "
                       << p.z + origin.z << " " << std::endl;
                progress.next();
            }

            for (const auto& c : mesh->cells()) {
                auto num_halffaces = mesh->halffaces(c).size();
                output << num_halffaces << std::endl;
                for (auto h : mesh->halffaces(c)) {
                    output << mesh->vertices(h).size() << " ";
                    for (auto v : mesh->vertices(h))
                        output << v.idx() << " ";
                    output << std::endl;
                }
                progress.next();
            }

            return true;
        }

    }

}
