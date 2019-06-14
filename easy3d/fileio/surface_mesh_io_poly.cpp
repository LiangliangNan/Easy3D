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

#include <iostream>
#include <fstream>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    namespace io {


        bool load_poly(const std::string& file_name, SurfaceMesh* mesh)
        {
            if (!mesh) {
                std::cerr << "null mesh pointer" << std::endl;
                return false;
            }

            // open file (in binary mode)
            std::ifstream input(file_name.c_str(), std::fstream::binary);
            if (input.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
                return false;
            }

            // how many elements?
            unsigned int nv, ne, nh, nf;
            input.read((char*)&nv, sizeof(unsigned int));
            input.read((char*)&ne, sizeof(unsigned int));
            input.read((char*)&nf, sizeof(unsigned int));
            nh = 2*ne;

            // resize containers
            mesh->resize(nv, ne, nf);

            // get properties
            auto vconn = mesh->vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
			auto hconn = mesh->halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
			auto fconn = mesh->face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
			auto point = mesh->vertex_property<vec3>("v:point");

            // read properties from file
            input.read((char*)vconn.data(), nv * sizeof(SurfaceMesh::VertexConnectivity)  );
            input.read((char*)hconn.data(), nh * sizeof(SurfaceMesh::HalfedgeConnectivity));
            input.read((char*)fconn.data(), nf * sizeof(SurfaceMesh::FaceConnectivity)    );
            input.read((char*)point.data(), nv * sizeof(vec3)                             );

            bool has_colors = false;
            input.read((char*)&has_colors, sizeof(bool));
            if (has_colors) {
                SurfaceMesh::VertexProperty<vec3> color = mesh->vertex_property<vec3>("v:color");
                input.read((char*)color.data(), nv * sizeof(vec3));
            }

            return mesh->n_faces() > 0;
        }


        //-----------------------------------------------------------------------------


        bool save_poly(const std::string& file_name, const SurfaceMesh* mesh)
        {
            if (!mesh) {
                std::cerr << "null mesh pointer" << std::endl;
                return false;
            }

            // open file (in binary mode)
            std::ofstream output(file_name.c_str(), std::fstream::binary);
            if (output.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
                return false;
            }

            // how many elements?
            unsigned int nv, ne, nh, nf;
            nv = mesh->n_vertices();
            ne = mesh->n_edges();
            nh = mesh->n_halfedges();
            nf = mesh->n_faces();
            nh = 2*ne;

            output.write((char*)&nv, sizeof(unsigned int));
            output.write((char*)&ne, sizeof(unsigned int));
            output.write((char*)&nf, sizeof(unsigned int));

            // get properties
			auto vconn = mesh->get_vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
            auto hconn = mesh->get_halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
            auto fconn = mesh->get_face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
            auto point = mesh->get_vertex_property<vec3>("v:point");

            // write properties to file
            output.write((char*)vconn.data(), nv * sizeof(SurfaceMesh::VertexConnectivity));
            output.write((char*)hconn.data(), nh * sizeof(SurfaceMesh::HalfedgeConnectivity));
            output.write((char*)fconn.data(), nf * sizeof(SurfaceMesh::FaceConnectivity));
            output.write((char*)point.data(), nv * sizeof(vec3));

            // check for colors
            auto color = mesh->get_vertex_property<vec3>("v:color");
            bool has_colors = color;
            output.write((char*)&has_colors, sizeof(bool));
            if (has_colors)
                output.write((char*)color.data(), nv * sizeof(vec3));

            return true;
        }

    }

}
