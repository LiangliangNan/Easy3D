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
#include <fstream>

#include <easy3d/core/poly_mesh.h>


namespace easy3d {

    namespace io {

        bool load_pmesh(const std::string& file_name, PolyMesh* mesh)
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
            unsigned int nv, ne, nh, nf, nc;
            input.read((char*)&nv, sizeof(unsigned int));
            input.read((char*)&ne, sizeof(unsigned int));
            input.read((char*)&nf, sizeof(unsigned int));
            input.read((char*)&nc, sizeof(unsigned int));
            nh = nf * 2;

            // resize containers
            mesh->resize(nv, ne, nf, nc);

            // get properties
            auto vconn = mesh->vertex_property<PolyMesh::VertexConnectivity>("v:connectivity");
            auto econn = mesh->edge_property<PolyMesh::EdgeConnectivity>("e:connectivity");
            auto hconn = mesh->halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity");
            auto cconn = mesh->cell_property<PolyMesh::CellConnectivity>("c:connectivity");
            auto point = mesh->vertex_property<vec3>("v:point");

            // read properties from file
            input.read((char*)vconn.data(), nv * sizeof(PolyMesh::VertexConnectivity)  );
            input.read((char*)econn.data(), ne * sizeof(PolyMesh::EdgeConnectivity));
            input.read((char*)hconn.data(), nh * sizeof(PolyMesh::HalfFaceConnectivity));
            input.read((char*)cconn.data(), nc * sizeof(PolyMesh::CellConnectivity)    );
            input.read((char*)point.data(), nv * sizeof(vec3));

            return (mesh->n_vertices() > 0 && mesh->n_faces() > 0 && mesh->n_cells() > 0);
        }


        //-----------------------------------------------------------------------------


        bool save_pmesh(const std::string& file_name, const PolyMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
                LOG(ERROR) << "empty polyhedral mesh";
                return false;
            }

            // open file (in binary mode)
            std::ofstream output(file_name.c_str(), std::fstream::binary);
            if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // how many elements?
            unsigned int nv, ne, nh, nf, nc;
            nv = mesh->n_vertices();
            ne = mesh->n_edges();
            nf = mesh->n_faces();
            nh = nf * 2;
            nc = mesh->n_cells();

            output.write((char*)&nv, sizeof(unsigned int));
            output.write((char*)&ne, sizeof(unsigned int));
            output.write((char*)&nf, sizeof(unsigned int));
            output.write((char*)&nc, sizeof(unsigned int));

            // get properties
            auto vconn = mesh->get_vertex_property<PolyMesh::VertexConnectivity>("v:connectivity");
            auto econn = mesh->get_edge_property<PolyMesh::EdgeConnectivity>("e:connectivity");
            auto hconn = mesh->get_halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity");
            auto cconn = mesh->get_cell_property<PolyMesh::CellConnectivity>("c:connectivity");
            auto point = mesh->get_vertex_property<vec3>("v:point");

            // write properties to file
            output.write((char*)vconn.data(), nv * sizeof(PolyMesh::VertexConnectivity)  );
            output.write((char*)econn.data(), ne * sizeof(PolyMesh::EdgeConnectivity));
            output.write((char*)hconn.data(), nh * sizeof(PolyMesh::HalfFaceConnectivity));
            output.write((char*)cconn.data(), nc * sizeof(PolyMesh::CellConnectivity)    );
            output.write((char*)point.data(), nv * sizeof(vec3));

            return true;
        }

    }

}
