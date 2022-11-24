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

#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/translator.h>
#include <easy3d/core/types.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>

#include <cstring> // for strlen()


namespace easy3d {

	namespace io {

        namespace internal {
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

            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

            // Vertex index starts by 0 in off format.

            LineInputStream input(in) ;
            internal::get_line(input) ;

            std::string magic ;
            input >> magic ;

            // NOFF is for Grimage "visual shapes".
            if(magic != "OFF" && magic != "NOFF") {
				LOG(ERROR) << "Not an OFF file. Key word is: " << magic;
                return false;
            }

            if(magic != "NOFF") {
                internal::get_line(input) ;
            }

            int nb_vertices, nb_facets, nb_edges;
            input >> nb_vertices >> nb_facets >> nb_edges;
            if (input.fail()) {
				LOG(ERROR) << "An error in the file header: " << input.current_line();
                return false;
            }

            ProgressLogger progress(nb_vertices + nb_facets, true, false);

            if (Translator::instance()->status() == Translator::DISABLED) {
                vec3 p;
                for (int i = 0; i < nb_vertices; i++) {
                    internal::get_line(input);
                    input >> p;
                    if (!input.fail())
                        builder.add_vertex(p);
                    else
                        LOG_N_TIMES(3, ERROR) << "failed reading the " << i << "_th vertex from file. " << COUNTER;
                    progress.next();
                }
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                dvec3 p, origin;
                for (int i = 0; i < nb_vertices; i++) {
                    internal::get_line(input);
                    input >> p;
                    if (!input.fail()) {
                        if (i == 0) { // the first point
                            origin = p;
                            Translator::instance()->set_translation(origin);
                        }
                        builder.add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                    }
                    else
                        LOG_N_TIMES(3, ERROR) << "failed reading the " << i << "_th vertex from file. " << COUNTER;
                    progress.next();
                }
                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0,0,0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << trans[0] << "), stored as ModelProperty<dvec3>(\"translation\")";
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                dvec3 p;
                for (int i = 0; i < nb_vertices; i++) {
                    internal::get_line(input);
                    input >> p;
                    if (!input.fail())
                        builder.add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                    else
                        LOG_N_TIMES(3, ERROR) << "failed reading the " << i << "_th vertex from file. " << COUNTER;
                    progress.next();
                }
                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0,0,0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. last known reference point (" << origin << "), stored as ModelProperty<dvec3>(\"translation\")";
            }

            for (int i = 0; i < nb_facets; i++) {
                int nv;
                internal::get_line(input);
                input >> nv;

                if (!input.fail()) {
                    std::vector<SurfaceMesh::Vertex> vertices;
                    for (int j = 0; j < nv; j++) {
                        int index;
                        input >> index;
                        if (!input.fail()) {
                            vertices.emplace_back(SurfaceMesh::Vertex(index));
                        } else {
                            LOG_N_TIMES(3, ERROR) << "failed reading the " << j << "_th vertex of the " << i
                                                  << "_th face from file. " << COUNTER;
                        }
                    }
                    builder.add_face(vertices);
                } else
                    LOG_N_TIMES(3, ERROR) << "failed reading the " << i << "_th face from file. " << COUNTER;

                progress.next();
            }

            // for mesh models, we can simply ignore the edges.
//            for (int i = 0; i < nb_edges; i++) {
//                // read the edges
//            }

            builder.end_surface();

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

            ProgressLogger progress(mesh->n_vertices() + mesh->n_faces(), true, false);

            // Output Vertices

            auto trans = mesh->get_model_property<dvec3>("translation");
            dvec3 origin(0, 0, 0);
            if (trans)  // has translation
                origin = trans[0];

            // Off files numbering starts with 0
            auto points = mesh->get_vertex_property<vec3>("v:point");
            for (auto v : mesh->vertices()) {
                const auto& p = points[v];
                out << p.x + origin.x << " "
                    << p.y + origin.y << " "
                    << p.z + origin.z << " " << std::endl;
                progress.next();
            }

            // Output facets
            for (auto f : mesh->faces()) {
                out << mesh->valence(f);

                SurfaceMesh::VertexAroundFaceCirculator fvit = mesh->vertices(f), fvend = fvit;
                do {
                    SurfaceMesh::Vertex v = (*fvit);
                    out << " " << v.idx();
                    ++fvit;
                } while (fvit != fvend);
                out << std::endl;
                progress.next();
            }

            return true;
		}

	} // namespace io

} // namespace easy3d
