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


#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/types.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/line_stream.h>

#include <fstream>



namespace easy3d {

	namespace io {

        namespace details {
            // Some OFF files may skip lines
            static void get_line(LineInputStream& in) {
                in.get_line() ;
                const char* p =  in.current_line().c_str();
                while(!in.eof() && (strlen(p) == 0 || !isprint(*p))) {
                    in.get_line() ;
                    p = in.current_line().c_str();
                }
            }
        }

		bool load_off(const std::string& file_name, SurfaceMesh* mesh)
		{
			if (!mesh) {
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

            std::ifstream in(file_name.c_str()) ;
            if(in.fail()) {
                std::cerr << "Could not open file \'" << file_name << "\'" << std::endl ;
                return false ;
            }

            mesh->clear();

            // Vertex index starts by 0 in off format.

            LineInputStream input(in) ;
            details::get_line(input) ;

            std::string magic ;
            input >> magic ;

            // NOFF is for Grimage "visual shapes".
            if(magic != "OFF" && magic != "NOFF") {
                std::cerr << "Not an OFF file. Key word is: " << magic << std::endl ;
                return false;
            }

            if(magic != "NOFF") {
                details::get_line(input) ;
            }

            int nb_vertices, nb_facets, nb_edges;
            input >> nb_vertices >> nb_facets >> nb_edges;
            if (!input.ok()) {
                std::cerr << "An error in the file header: " << input.current_line() << std::endl;
                return false;
            }

            for (int i = 0; i < nb_vertices; i++) {
                vec3 p;
                details::get_line(input);
                input >> p;
                mesh->add_vertex(p);
            }

            for (int i = 0; i < nb_facets; i++) {
                int nb_vertices;
                details::get_line(input);
                input >> nb_vertices;

                std::vector<SurfaceMesh::Vertex> vertices;
                for (int j = 0; j < nb_vertices; j++) {
                    int index;
                    input >> index;
                    vertices.push_back(SurfaceMesh::Vertex(index));
                }
                mesh->add_face(vertices);
            }

            // for mesh models, we can simply ignore the edges.
//            for (int i = 0; i < nb_edges; i++) {
//                // read the edges
//            }

            return mesh->n_faces() > 0;
		}


		//-----------------------------------------------------------------------------


		bool save_off(const std::string& file_name, const SurfaceMesh* mesh)
		{
			if (!mesh) {
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

            std::ofstream out(file_name.c_str()) ;
            if(out.fail()) {
                std::cerr << "Could not open file\'" << file_name << "\'" << std::endl ;
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
