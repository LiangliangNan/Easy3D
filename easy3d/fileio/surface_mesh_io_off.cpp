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

#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/types.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/logging.h>

#include <fstream>
#include <cstring> // for strlen()


#define RESOLVE_NONMANIFOLDNESS    1

namespace easy3d {

	namespace io {

        namespace details {
            // Some OFF files may skip lines or may have comments starting with '#'
            static void get_line(LineInputStream& in) {
                in.get_line() ;
                const char* p =  in.current_line().c_str();
                while (!in.eof() && (
                        (strlen(p) == 0 || !isprint(*p)) || // empty line
                        (strlen(p) > 0 && p[0] == '#')
                )) {
                    in.get_line() ;
                    p = in.current_line().c_str();
                }
            }
        }

		bool load_off(const std::string& file_name, SurfaceMesh* mesh)
		{
			if (!mesh) {
				LOG(ERROR) << "null mesh pointer";
				return false;
			}

            std::ifstream in(file_name.c_str()) ;
            if(in.fail()) {
				LOG(ERROR) << "Could not open file: " << file_name;
                return false ;
            }

            mesh->clear();

#if RESOLVE_NONMANIFOLDNESS
            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();
#endif

            // Vertex index starts by 0 in off format.

            LineInputStream input(in) ;
            details::get_line(input) ;

            std::string magic ;
            input >> magic ;

            // NOFF is for Grimage "visual shapes".
            if(magic != "OFF" && magic != "NOFF") {
				LOG(ERROR) << "Not an OFF file. Key word is: " << magic;
                return false;
            }

            if(magic != "NOFF") {
                details::get_line(input) ;
            }

            int nb_vertices, nb_facets, nb_edges;
            input >> nb_vertices >> nb_facets >> nb_edges;
            if (input.fail()) {
				LOG(ERROR) << "An error in the file header: " << input.current_line();
                return false;
            }

            for (int i = 0; i < nb_vertices; i++) {
                vec3 p;
                details::get_line(input);
                input >> p;
                if (!input.fail()) {
#if RESOLVE_NONMANIFOLDNESS
                    builder.add_vertex(p);
#else
                    mesh->add_vertex(p);
#endif
                }
                else {
                    LOG_FIRST_N(3, ERROR) << "failed reading the " << i << "_th vertex from file. " << COUNTER;
                }
            }

            for (int i = 0; i < nb_facets; i++) {
                int nb_vertices;
                details::get_line(input);
                input >> nb_vertices;

				if (!input.fail()) {
					std::vector<SurfaceMesh::Vertex> vertices;
					for (int j = 0; j < nb_vertices; j++) {
						int index;
						input >> index;
						if (!input.fail()) {
                            vertices.emplace_back(SurfaceMesh::Vertex(index));
                        }
                        else {
                            LOG_FIRST_N(3, ERROR) << "failed reading the " << j << "_th vertex of the " << i << "_th face from file. " << COUNTER;
                        }
					}
#if RESOLVE_NONMANIFOLDNESS
					builder.add_face(vertices);
#else
                    mesh->add_face(vertices);
#endif
				}
                else {
                    LOG_FIRST_N(3, ERROR) << "failed reading the " << i << "_th face from file. " << COUNTER;
                }
            }

            // for mesh models, we can simply ignore the edges.
//            for (int i = 0; i < nb_edges; i++) {
//                // read the edges
//            }

#if RESOLVE_NONMANIFOLDNESS
            builder.end_surface();
#endif
            return mesh->n_faces() > 0;
		}


		//-----------------------------------------------------------------------------


		bool save_off(const std::string& file_name, const SurfaceMesh* mesh)
		{
			if (!mesh) {
				LOG(ERROR) << "null mesh pointer";
				return false;
			}

            std::ofstream out(file_name.c_str()) ;
            if(out.fail()) {
				LOG(ERROR) << "could not open file: " << file_name;
                return false ;
            }
            out.precision(10) ;

            out << "OFF" << std::endl;
            out << mesh->n_vertices() << " " << mesh->n_faces() << " 0" << std::endl;

            // Output Vertices
            // Off files numbering starts with 0
            SurfaceMesh::VertexProperty<vec3> points = mesh->get_vertex_property<vec3>("v:point");
            for (auto v : mesh->vertices())
                out << points[v] << std::endl;

            // Output facets
            for (auto f : mesh->faces())
            {
                out << mesh->valence(f);

                SurfaceMesh::VertexAroundFaceCirculator fvit = mesh->vertices(f), fvend = fvit;
                do {
                    SurfaceMesh::Vertex v = (*fvit);
                    out << " " << v.idx();
                    ++fvit;
                } while (fvit != fvend);
                out << std::endl;
            }

            return true;
		}

	} // namespace io

} // namespace easy3d
