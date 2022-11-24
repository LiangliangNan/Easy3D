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

#include <iostream>
#include <fstream>
#include <set>

#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/progress.h>


namespace easy3d {

    /// TODO: Translator not implemented

    namespace io {

        bool load_pm(const std::string& file_name, PolyMesh* mesh)
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
            auto& vconn = mesh->vertex_property<PolyMesh::VertexConnectivity>("v:connectivity").vector();
            auto& econn = mesh->edge_property<PolyMesh::EdgeConnectivity>("e:connectivity").vector();
            auto& hconn = mesh->halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity").vector();
            auto& cconn = mesh->cell_property<PolyMesh::CellConnectivity>("c:connectivity").vector();
            auto point = mesh->vertex_property<vec3>("v:point");

            ProgressLogger progress(nv + ne + nh + nc + nv, true, false);

            // read properties from file
            for (unsigned int i=0; i<nv; ++i) {
                vconn[i].read(input);
                progress.next();
            }

            for (unsigned int i=0; i<ne; ++i) {
                econn[i].read(input);
                progress.next();
            }

            for (unsigned int i=0; i<nh; ++i) {
                hconn[i].read(input);
                progress.next();
            }

            for (unsigned int i=0; i<nc; ++i) {
                cconn[i].read(input);
                progress.next();
            }

            input.read((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));
            progress.next();

            return (mesh->n_vertices() > 0 && mesh->n_faces() > 0 && mesh->n_cells() > 0);
        }


        //-----------------------------------------------------------------------------


        bool save_pm(const std::string& file_name, const PolyMesh* mesh)
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
            const auto& vconn = mesh->get_vertex_property<PolyMesh::VertexConnectivity>("v:connectivity").vector();
            const auto& econn = mesh->get_edge_property<PolyMesh::EdgeConnectivity>("e:connectivity").vector();
            const auto& hconn = mesh->get_halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity").vector();
            const auto& cconn = mesh->get_cell_property<PolyMesh::CellConnectivity>("c:connectivity").vector();
            auto point = mesh->get_vertex_property<vec3>("v:point");

            ProgressLogger progress(nv + ne + nh + nc + nv, true, false);

            // write properties to file
            for (unsigned int i=0; i<nv; ++i) {
                vconn[i].write(output);
                progress.next();
            }

            for (unsigned int i=0; i<ne; ++i)  {
                econn[i].write(output);
                progress.next();
            }

            for (unsigned int i=0; i<nh; ++i)  {
                hconn[i].write(output);
                progress.next();
            }

            for (unsigned int i=0; i<nc; ++i)  {
                cconn[i].write(output);
                progress.next();
            }

            output.write((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));
            progress.next();

            return true;
        }

    }

}
