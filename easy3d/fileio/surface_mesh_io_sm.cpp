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

#include <iostream>
#include <fstream>

#include <easy3d/core/surface_mesh.h>


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications and
 * significant enhancement.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/


namespace easy3d {

    namespace io {

        /// TODO: Translator not implemented

        bool load_sm(const std::string& file_name, SurfaceMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            // open file (in binary mode)
            std::ifstream input(file_name.c_str(), std::fstream::binary);
            if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
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
            input.read((char*)vconn.data(), static_cast<long>(nv * sizeof(SurfaceMesh::VertexConnectivity)  ));
            input.read((char*)hconn.data(), static_cast<long>(nh * sizeof(SurfaceMesh::HalfedgeConnectivity)));
            input.read((char*)fconn.data(), static_cast<long>(nf * sizeof(SurfaceMesh::FaceConnectivity)    ));
            input.read((char*)point.data(), static_cast<long>(nv * sizeof(vec3)                             ));

            bool has_colors = false;
            input.read((char*)&has_colors, sizeof(bool));
            if (has_colors) {
                SurfaceMesh::VertexProperty<vec3> color = mesh->vertex_property<vec3>("v:color");
                input.read((char*)color.data(), static_cast<long>(nv * sizeof(vec3)));
            }

            return mesh->n_faces() > 0;
        }


        //-----------------------------------------------------------------------------


        bool save_sm(const std::string& file_name, const SurfaceMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            // open file (in binary mode)
            std::ofstream output(file_name.c_str(), std::fstream::binary);
            if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // how many elements?
            unsigned int nv, ne, nh, nf;
            nv = mesh->n_vertices();
            ne = mesh->n_edges();
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
            output.write((char*)vconn.data(), static_cast<long>(nv * sizeof(SurfaceMesh::VertexConnectivity)));
            output.write((char*)hconn.data(), static_cast<long>(nh * sizeof(SurfaceMesh::HalfedgeConnectivity)));
            output.write((char*)fconn.data(), static_cast<long>(nf * sizeof(SurfaceMesh::FaceConnectivity)));
            output.write((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));

            // check for colors
            auto color = mesh->get_vertex_property<vec3>("v:color");
            bool has_colors = color;
            output.write((char*)&has_colors, sizeof(bool));
            if (has_colors)
                output.write((char*)color.data(), static_cast<long>(nv * sizeof(vec3)));

            return true;
        }

    }

}
