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
#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    PolyMesh *PolyMeshIO::load(const std::string &file_name) {
        auto mesh = new PolyMesh;
        mesh->set_name(file_name);

        StopWatch w;
        bool success = false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext == "plm")
            success = io::load_plm(file_name, mesh);
        else if (ext == "pm")
            success = io::load_pm(file_name, mesh);
        else if (ext == "mesh")
            success = io::load_mesh(file_name, mesh);
        else if (ext.empty()) {
            LOG(ERROR) << "unknown file format: no extension" << ext;
            success = false;
        } else {
            LOG(ERROR) << "unknown file format: " << ext;
            success = false;
        }

        if (!success || mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
            LOG(WARNING) << "load surface mesh failed: " << file_name;
            delete mesh;
            return nullptr;
        }

        LOG(INFO) << "polyhedral mesh loaded ("
                  << "#vertex: " << mesh->n_vertices() << ", "
                  << "#edge: " << mesh->n_edges() << ", "
                  << "#face: " << mesh->n_faces() << ", "
                  << "#cell: " << mesh->n_cells() << "). "
                  << w.time_string();

        return mesh;
    }


    bool PolyMeshIO::save(const std::string &file_name, const PolyMesh *mesh) {
        if (!mesh || mesh->n_vertices() == 0 || mesh->n_faces() == 0 || mesh->n_cells() == 0) {
            LOG(ERROR) << "polyhedral mesh is null";
            return false;
        }

        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        StopWatch w;
        bool success = false;

        std::string final_name = file_name;
        const std::string &ext = file_system::extension(file_name, true);

        if (ext == "plm" || ext.empty()) {
            if (ext.empty()) {
                LOG(WARNING) << "no extension specified, default to plm" << ext;
                final_name = final_name + ".plm";
            }
            success = io::save_plm(final_name, mesh);
        } else if (ext == "pm")
            success = io::save_pm(final_name, mesh);
        else if (ext == "mesh")
            success = io::save_mesh(file_name, mesh);
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

} // namespace easy3d
