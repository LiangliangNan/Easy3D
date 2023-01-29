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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    SurfaceMesh *SurfaceMeshIO::load(const std::string &file_name) {
        auto mesh = new SurfaceMesh;
        mesh->set_name(file_name);

        StopWatch w;
        bool success = false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext == "ply")
            success = io::load_ply(file_name, mesh);
        else if (ext == "sm")
            success = io::load_sm(file_name, mesh);
        else if (ext == "obj")
            success = io::load_obj(file_name, mesh);
        else if (ext == "off")
            success = io::load_off(file_name, mesh);
        else if (ext == "stl")
            success = io::load_stl(file_name, mesh);
        else if (ext == "trilist")
            success = io::load_trilist(file_name, mesh);
        else if (ext == "geojson")
            success = io::load_geojson(file_name, mesh);

        else if (ext.empty()) {
            LOG(ERROR) << "unknown file format: no extension" << ext;
            success = false;
        } else {
            LOG(ERROR) << "unknown file format: " << ext;
            success = false;
        }

        if (!success || mesh->n_vertices() == 0 || mesh->n_faces() == 0) {
            LOG(INFO) << "load surface mesh failed: " << file_name;
            delete mesh;
            return nullptr;
        }

        LOG(INFO) << "surface mesh loaded ("
                  << "#face: " << mesh->n_faces() << ", "
                  << "#vertex: " << mesh->n_vertices() << ", "
                  << "#edge: " << mesh->n_edges() << "). "
                  << w.time_string();

        return mesh;
    }


    bool SurfaceMeshIO::save(const std::string &file_name, const SurfaceMesh *mesh) {
        if (!mesh || mesh->n_faces() == 0) {
            LOG(ERROR) << "surface mesh is null";
            return false;
        }

        StopWatch w;
        bool success = false;

        std::string final_name = file_name;
        const std::string &ext = file_system::extension(file_name, true);

        if (ext == "ply" || ext.empty()) {
            if (ext.empty()) {
                LOG(ERROR) << "no extension specified, default to ply" << ext;
                final_name = final_name + ".ply";
            }
            success = io::save_ply(final_name, mesh, true);
        } else if (ext == "sm")
            success = io::save_sm(final_name, mesh);
        else if (ext == "obj")
            success = io::save_obj(final_name, mesh);
        else if (ext == "off")
            success = io::save_off(final_name, mesh);
        else if (ext == "stl")
            success = io::save_stl(final_name, mesh);

        else {
            LOG(ERROR) << "unknown file format: " << ext;
            success = false;
        }

        if (success) {
            LOG(INFO) << "save model done. " << w.time_string();
            return true;
        } else {
            LOG(INFO) << "save model failed";
            return false;
        }
    }


    bool io::load_trilist(const std::string &file_name, SurfaceMesh *mesh) {
        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        vec3 a, b, c;
        while (!input.eof()) {
            input >> a >> b >> c;
            if (input.good())
                mesh->add_triangle(mesh->add_vertex(a), mesh->add_vertex(b), mesh->add_vertex(c));
        }

        return mesh->n_faces() > 0;
    }

} // namespace easy3d
