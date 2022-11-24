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

#include <cstring> //for strcmp

#include <easy3d/fileio/translator.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/progress.h>


namespace easy3d {

    namespace io {

        bool load_mesh(const std::string &file_name, PolyMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            FILE *mesh_file = fopen(file_name.c_str(), "r");
            if (nullptr == mesh_file) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // get length of file
            fseek(mesh_file, 0L, SEEK_END); // seek to end of file
            long size = ftell(mesh_file);
            fseek(mesh_file, 0L, SEEK_SET); // seek back to beginning of file

            ProgressLogger progress(size, true, false);

#ifndef LINE_MAX
#  define LINE_MAX 2048
#endif

            // get the next line (ignoring comments and empty lines)
            const auto get_next_line = [](FILE *file, char* a_line) -> bool {
                bool still_comments = true;
                bool has_line = false;
                while (still_comments && !feof(file)) {
                    has_line = fgets(a_line, LINE_MAX, file) != nullptr;
                    // needs to check for both line feed (LF: '\n') or Carriage return (CR: '\r')
                    still_comments = (a_line[0] == ' ' || a_line[0] == '#' || a_line[0] == '\n' || a_line[0] == '\r');
                }
                return has_line;
            };

            char line[LINE_MAX];
            get_next_line(mesh_file, line);

            char str[LINE_MAX];
            sscanf(line, " %s", str);
            // check that first word is MeshVersionFormatted
            if (0 != strcmp(str, "MeshVersionFormatted")) {
                LOG(ERROR) << "first word should be MeshVersionFormatted instead of " << str;
                fclose(mesh_file);
                return false;
            }
            int version = -1;
            if (2 != sscanf(line, "%s %d", str, &version)) {
                if (1 != fscanf(mesh_file, " %d", &version)) {
                    LOG(ERROR) << "wrong file format or file corrupted: could not parse version";
                    fclose(mesh_file);
                    return false;
                }
            }
            if (version != 1 && version != 2) {
                LOG(ERROR) << "second word should be 1 or 2 instead of " << version;
                fclose(mesh_file);
                return false;
            }

            std::set<std::string> ignored_entries;

            std::vector<PolyMesh::Vertex> vertices;
            while (get_next_line(mesh_file, line)) {
                sscanf(line, " %s", str);
                int extra;
                // check that third word is Dimension
                if (0 == strcmp(str, "Dimension")) {
                    int three = -1;
                    if (2 != sscanf(line, "%s %d", str, &three)) {
                        // 1 appears on next line?
                        if (1 != fscanf(mesh_file, " %d", &three)) {
                            LOG(ERROR) << "wrong file format or file corrupted: Dimension must be 3";
                            fclose(mesh_file);
                            return false;
                        }
                    }
                    if (three != 3) {
                        LOG(ERROR) << "only Dimension 3 supported instead of " << three;
                        fclose(mesh_file);
                        return false;
                    }
                } else if (0 == strcmp(str, "Vertices")) {
                    int number_of_vertices(0);
                    if (2 != sscanf(line, "%s %d", str, &number_of_vertices)) {
                        // number_of_vertices appears on next line?
                        if (1 != fscanf(mesh_file, " %d", &number_of_vertices) || number_of_vertices > 1000000000) {
                            LOG(ERROR) << "expecting number of vertices should be less than 10^9...";
                            fclose(mesh_file);
                            return false;
                        }
                    }

                    LOG(INFO) << "reading " << number_of_vertices << " vertices...";

                    // allocate space for vertices
                    vertices.resize(number_of_vertices);
                    if (Translator::instance()->status() == Translator::DISABLED) {
                        double x, y, z;
                        for (int i = 0; i < number_of_vertices; i++) {
                            if (4 != fscanf(mesh_file, " %lg %lg %lg %d", &x, &y, &z, &extra)) {
                                LOG(ERROR) << "expecting vertex position...";
                                fclose(mesh_file);
                                return false;
                            }
                            vertices[i] = mesh->add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
                            progress.notify(ftell(mesh_file));
                        }
                    } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                        double x, y, z, x0, y0, z0;
                        for (int i = 0; i < number_of_vertices; i++) {
                            if (4 != fscanf(mesh_file, " %lg %lg %lg %d", &x, &y, &z, &extra)) {
                                LOG(ERROR) << "expecting vertex position...";
                                fclose(mesh_file);
                                return false;
                            }

                            if (i == 0) { // the first point
                                x0 = x;
                                y0 = y;
                                z0 = z;
                                Translator::instance()->set_translation(dvec3(x0, y0, z0));
                            }
                            vertices[i] = mesh->add_vertex(vec3(static_cast<float>(x - x0), static_cast<float>(y - y0), static_cast<float>(z - z0)));
                            progress.notify(ftell(mesh_file));
                        }

                        auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                        trans[0] = dvec3(x0, y0, z0);
                        LOG(INFO) << "model translated w.r.t. the first vertex (" << trans[0] << "), stored as ModelProperty<dvec3>(\"translation\")";
                    } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                        const dvec3 &origin = Translator::instance()->translation();
                        double x, y, z;
                        for (int i = 0; i < number_of_vertices; i++) {
                            if (4 != fscanf(mesh_file, " %lg %lg %lg %d", &x, &y, &z, &extra)) {
                                LOG(ERROR) << "expecting vertex position...";
                                fclose(mesh_file);
                                return false;
                            }
                            vertices[i] = mesh->add_vertex(vec3(static_cast<float>(x - origin.x), static_cast<float>(y - origin.y), static_cast<float>(z - origin.z)));
                            progress.notify(ftell(mesh_file));
                        }

                        auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0,0,0));
                        trans[0] = origin;
                        LOG(INFO) << "model translated w.r.t. last known reference point (" << origin << "), stored as ModelProperty<dvec3>(\"translation\")";
                    }
                } else if (0 == strcmp(str, "Tetrahedra")) {
                    int number_of_tetrahedra(0);
                    if (2 != sscanf(line, "%s %d", str, &number_of_tetrahedra)) {
                        // number_of_tetrahedra appears on next line?
                        if (1 != fscanf(mesh_file, " %d", &number_of_tetrahedra) || number_of_tetrahedra > 1000000000) {
                            LOG(ERROR) << "expecting number of tetrahedra should be less than 10^9...";
                            fclose(mesh_file);
                            return false;
                        }
                    }

                    LOG(INFO) << "reading " << number_of_tetrahedra << " tetrahedra...";

                    // tet indices
                    int indices[4];
                    for (int i = 0; i < number_of_tetrahedra; i++) {
                        if (5 != fscanf(mesh_file, " %d %d %d %d %d",
                                        &indices[0], &indices[1], &indices[2], &indices[3], &extra))
                        {
                            LOG(ERROR) << "expecting tetrahedra indices...";
                            fclose(mesh_file);
                            return false;
                        }
                        mesh->add_tetra(
                                vertices[indices[0] - 1],
                                vertices[indices[1] - 1],
                                vertices[indices[2] - 1],
                                vertices[indices[3] - 1]
                        );
                        progress.notify(ftell(mesh_file));
                    }
                } else if (0 == strcmp(str, "Hexahedra")) {
                    int number_of_hexahedra(0);
                    if (2 != sscanf(line, "%s %d", str, &number_of_hexahedra)) {
                        // number_of_hexahedra appears on next line?
                        if (1 != fscanf(mesh_file, " %d", &number_of_hexahedra) || number_of_hexahedra > 1000000000) {
                            LOG(ERROR) << "expecting number of hexahedra should be less than 10^9...";
                            fclose(mesh_file);
                            return false;
                        }
                    }

                    LOG(INFO) << "reading " << number_of_hexahedra << " hexahedra...";

                    // hex indices
                    int indices[8];
                    for (int i = 0; i < number_of_hexahedra; i++) {
                        if (9 != fscanf(mesh_file, " %d %d %d %d %d %d %d %d %d",
                                        &indices[0], &indices[1], &indices[2], &indices[3],
                                        &indices[4], &indices[5], &indices[6], &indices[7], &extra))
                        {
                            LOG(ERROR) << "expecting hexahedra indices...";
                            fclose(mesh_file);
                            return false;
                        }
                        mesh->add_hexa(
                                vertices[indices[0] - 1],
                                vertices[indices[1] - 1],
                                vertices[indices[2] - 1],
                                vertices[indices[3] - 1],
                                vertices[indices[4] - 1],
                                vertices[indices[5] - 1],
                                vertices[indices[6] - 1],
                                vertices[indices[7] - 1]
                        );
                        progress.notify(ftell(mesh_file));
                    }
                } else if (0 == strcmp(str, "End")) {
                    break;
                } else {
                    ignored_entries.insert(str);

                    // we still need to move the stream forward
                    int number_of_entries(0);
                    if (2 != sscanf(line, "%s %d", str, &number_of_entries)) {
                        // number_of_entries appears on next line?
                        if (1 != fscanf(mesh_file, " %d", &number_of_entries) || number_of_entries > 1000000000) {
                            LOG(ERROR) << "expecting number of entry should be less than 10^9...";
                            fclose(mesh_file);
                            return false;
                        }
                    }

                    for (int i = 0; i < number_of_entries; i++) {
                        if (get_next_line(mesh_file, line)) {
                            double value; // must be at least 1 value
                            if (1 != sscanf(line, "%lf", &value)) {
                                LOG(ERROR) << "incorrect file format. Line content: " << line;
                                fclose(mesh_file);
                                return false;
                            }
                        }
                        else {
                            LOG(ERROR) << "incorrect file format. Line content: " << line;
                            fclose(mesh_file);
                            return false;
                        }
                        progress.notify(ftell(mesh_file));
                    }
                }
            }

            fclose(mesh_file);

            LOG_IF(!ignored_entries.empty(), WARNING)
                << "current implementation handles 'Vertices', 'Tetrahedra', and 'Hexahedra'. "
                << "The following elements/properties have been ignored: " << ignored_entries;

            return (mesh->n_vertices() > 0 && mesh->n_faces() > 0 && mesh->n_cells() > 0);
        }


        //-----------------------------------------------------------------------------


        bool save_mesh(const std::string &file_name, const PolyMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
                LOG(ERROR) << "polyhedral mesh is empty";
                return false;
            }

            if (!mesh->is_tetraheral_mesh()) {
                LOG(ERROR) << "polyhedral mesh is not a tetrahedra (only tetrahedra can be saved in mesh format for the"
                              "the current implementation)";
                return false;
            }

            FILE *mesh_file = fopen(file_name.c_str(), "w");
            if (!mesh_file) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // print header
            fprintf(mesh_file, "MeshVersionFormatted 1\n");
            fprintf(mesh_file, "Dimension 3\n");
            // print tet vertices
            fprintf(mesh_file, "Vertices\n");
            // print number of tet vertices
            auto number_of_tet_vertices = mesh->n_vertices();
            fprintf(mesh_file, "%d\n", number_of_tet_vertices);

            ProgressLogger progress(mesh->n_vertices() + mesh->n_faces() + mesh->n_cells(), true, false);

            auto trans = mesh->get_model_property<dvec3>("translation");
            dvec3 origin(0, 0, 0);
            if (trans)  // has translation
                origin = trans[0];

            // loop over tet vertices
            for (auto v : mesh->vertices()) {
                // print position of ith tet vertex
                const auto& p = mesh->position(v);
                fprintf(mesh_file, "%.17lg %.17lg %.17lg 1\n", p.x + origin.x, p.y + origin.y, p.z + origin.z);
                progress.next();
            }

            // print faces
            fprintf(mesh_file, "Triangles\n");
            // print number of triangles
            auto number_of_triangles = mesh->n_faces();
            fprintf(mesh_file, "%d\n", number_of_triangles);
            // loop over faces
            for (auto f : mesh->faces()) {
                // loop over vertices in face
                const auto& vts = mesh->vertices(f);
                // mesh standard uses 1-based indexing
                fprintf(mesh_file, "%d %d %d 1\n", vts[0].idx() + 1, vts[1].idx() + 1, vts[2].idx() + 1);
                progress.next();
            }

            // print tetrahedra
            fprintf(mesh_file, "Tetrahedra\n");
            auto number_of_tetrahedra = mesh->n_cells();
            // print number of tetrahedra
            fprintf(mesh_file, "%d\n", number_of_tetrahedra);
            // loop over tetrahedra
            for (auto c : mesh->cells()) {
                // the order really matters.
                // we find 3 first from one of its face, then the 4th one from another face.
                auto f = mesh->halffaces(c)[0];
                auto vts = mesh->vertices(f);
                f = mesh->halffaces(c)[1];
                for (auto v : mesh->vertices(f)) {
                    if (vts[0] != v && vts[1] != v && vts[2] != v) {
                        vts.push_back(v);
                        break;
                    }
                }

                // mesh standard uses 1-based indexing
                fprintf(mesh_file, "%d %d %d %d 1\n",
                        vts[3].idx() + 1,
                        vts[0].idx() + 1,
                        vts[1].idx() + 1,
                        vts[2].idx() + 1
                );
                progress.next();
            }

            fclose(mesh_file);
            return true;
        }

    }

}
