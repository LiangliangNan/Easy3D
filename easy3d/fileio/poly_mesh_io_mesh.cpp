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

#include <iostream>
#include <cstring> //for strcmp

#include <easy3d/core/poly_mesh.h>


namespace easy3d {

    namespace io {

        bool load_mesh(const std::string &file_name, PolyMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            FILE *mesh_file = fopen(file_name.c_str(), "r");
            if (NULL == mesh_file) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

#ifndef LINE_MAX
#  define LINE_MAX 2048
#endif

            // eat comments at beginning of file
            const auto eat_comments = [](FILE *file, char* a_line) -> bool {
                bool still_comments = true;
                bool has_line = false;
                while (still_comments && !feof(file)) {
                    has_line = fgets(a_line, LINE_MAX, file) != NULL;
                    still_comments = (a_line[0] == '#' || a_line[0] == '\n');
                }
                return has_line;
            };

            char line[LINE_MAX];
            eat_comments(mesh_file, line);

            char str[LINE_MAX];
            sscanf(line, " %s", str);
            // check that first word is MeshVersionFormatted
            if (0 != strcmp(str, "MeshVersionFormatted")) {
                LOG(ERROR) << "first word should be MeshVersionFormatted instead of " << str;
                fclose(mesh_file);
                return false;
            }
            int version = -1;
            if (2 != sscanf(line, "%s %d", str, &version))
                fscanf(mesh_file, " %d", &version);
            if (version != 1 && version != 2) {
                LOG(ERROR) << "second word should be 1 or 2 instead of " << version;
                fclose(mesh_file);
                return false;
            }

            std::vector<PolyMesh::Vertex> vertices;
//            std::vector<PolyMesh::HalfFace> faces; // PolyMesh adds the faces when adding the tetrahedra
            while (eat_comments(mesh_file, line)) {
                sscanf(line, " %s", str);
                int extra;
                // check that third word is Dimension
                if (0 == strcmp(str, "Dimension")) {
                    int three = -1;
                    if (2 != sscanf(line, "%s %d", str, &three)) {
                        // 1 appears on next line?
                        fscanf(mesh_file, " %d", &three);
                    }
                    if (three != 3) {
                        LOG(ERROR) << "only Dimension 3 supported instead of " << three;
                        fclose(mesh_file);
                        return false;
                    }
                } else if (0 == strcmp(str, "Vertices")) {
                    int number_of_vertices(0);
                    if (1 != fscanf(mesh_file, " %d", &number_of_vertices) || number_of_vertices > 1000000000) {
                        LOG(ERROR) << "expecting number of vertices should be less than 10^9...";
                        fclose(mesh_file);
                        return false;
                    }
                    // allocate space for vertices
                    vertices.resize(number_of_vertices);
                    for (int i = 0; i < number_of_vertices; i++) {
                        double x, y, z;
                        if (4 != fscanf(mesh_file, " %lg %lg %lg %d", &x, &y, &z, &extra)) {
                            LOG(ERROR) << "expecting vertex position...";
                            fclose(mesh_file);
                            return false;
                        }
                        vertices[i] = mesh->add_vertex(vec3(x, y, z));
                    }
                } else if (0 == strcmp(str, "Triangles")) {
                    int number_of_triangles(0);
                    if (1 != fscanf(mesh_file, " %d", &number_of_triangles)) {
                        LOG(ERROR) << "expecting number of triangles...";
                        fclose(mesh_file);
                        return false;
                    }

//                    faces.resize(number_of_triangles);
                    int tri[3]; // triangle indices
                    for (int i = 0; i < number_of_triangles; i++) {
                        if (4 != fscanf(mesh_file, " %d %d %d %d", &tri[0], &tri[1], &tri[2], &extra)) {
                            LOG(ERROR) << "expecting triangle indices...";
                            return false;
                        }
//                        faces[i] = mesh->add_triangle(
//                                PolyMesh::Vertex(tri[0] - 1),
//                                PolyMesh::Vertex(tri[1] - 1),
//                                PolyMesh::Vertex(tri[2] - 1)
//                        );
                    }
                } else if (0 == strcmp(str, "Tetrahedra")) {
                    int number_of_tetrahedra(0);
                    if (1 != fscanf(mesh_file, " %d", &number_of_tetrahedra)) {
                        LOG(ERROR) << "expecting number of tetrahedra...";
                        fclose(mesh_file);
                        return false;
                    }

                    // tet indices
                    int a, b, c, d;
                    for (int i = 0; i < number_of_tetrahedra; i++) {
                        if (5 != fscanf(mesh_file, " %d %d %d %d %d", &a, &b, &c, &d, &extra)) {
                            LOG(ERROR) << "expecting tetrahedra indices...";
                            fclose(mesh_file);
                            return false;
                        }
                        mesh->add_tetra(vertices[a - 1], vertices[b - 1], vertices[c - 1], vertices[d - 1]);
                    }
                } else if (0 == strcmp(str, "Edges")) {
                    int number_of_edges;
                    if (1 != fscanf(mesh_file, " %d", &number_of_edges)) {
                        LOG(ERROR) << "expecting number of edges...";
                        fclose(mesh_file);
                        return false;
                    }
                    // allocate space for edges
                    std::vector<std::pair<int, int> > edges(number_of_edges);
                    // tet indices
                    int a, b;
                    for (int i = 0; i < number_of_edges; i++) {
                        if (3 != fscanf(mesh_file, " %d %d %d", &a, &b, &extra)) {
                            LOG(ERROR) << "expecting tetrahedra indices...";
                            fclose(mesh_file);
                            return false;
                        }
                        edges[i] = {a - 1, b - 1};
                    }
                } else if (0 == strcmp(str, "End")) {
                    break;
                } else {
                    LOG(ERROR) << "expecting Dimension|Triangles|Vertices|Tetrahedra|Edges instead of " << str;
                    fclose(mesh_file);
                    return false;
                }
            }

            fclose(mesh_file);

            return (mesh->n_vertices() > 0 && mesh->n_faces() > 0 && mesh->n_cells() > 0);
        }


        //-----------------------------------------------------------------------------


        bool save_mesh(const std::string &file_name, const PolyMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (!mesh || mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
                LOG(ERROR) << "polyhedral mesh is empty";
                return false;
            }

            FILE *mesh_file = fopen(file_name.c_str(), "w");
            if (NULL == mesh_file) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // print header
            fprintf(mesh_file, "MeshVersionFormatted 1\n");
            fprintf(mesh_file, "Dimension 3\n");
            // print tet vertices
            fprintf(mesh_file, "Vertices\n");
            // print number of tet vertices
            int number_of_tet_vertices = mesh->n_vertices();
            fprintf(mesh_file, "%d\n", number_of_tet_vertices);
            // loop over tet vertices
            for (auto v : mesh->vertices()) {
                // print position of ith tet vertex
                const auto& p = mesh->position(v);
                fprintf(mesh_file, "%.17lg %.17lg %.17lg 1\n", p.x, p.y, p.z);
            }

            // print faces
            fprintf(mesh_file, "Triangles\n");
            // print number of triangles
            int number_of_triangles = mesh->n_faces();
            fprintf(mesh_file, "%d\n", number_of_triangles);
            // loop over faces
            for (auto f : mesh->faces()) {
                // loop over vertices in face
                const auto& vts = mesh->vertices(f);
                // mesh standard uses 1-based indexing
                fprintf(mesh_file, "%d %d %d 1\n", vts[0].idx() + 1, vts[1].idx() + 1, vts[2].idx() + 1);
            }

            // print tetrahedra
            fprintf(mesh_file, "Tetrahedra\n");
            int number_of_tetrahedra = mesh->n_cells();
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
            }
            fclose(mesh_file);
            return true;
        }

    }

}
