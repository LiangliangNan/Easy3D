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

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

bool load_off(const std::string &file_name, SurfaceMesh *mesh);

int main (int argc, char *argv[])
{
    easy3d::logging::initialize(argv[0]);

    Viewer viewer("Sandbox");

    const std::string& file_name = setting::resource_directory() + "/data/repair/non_manifold/complex_edges_0.off";
    //const std::string& file_name = setting::resource_directory() + "/data/repair/non_manifold/complex_edges_1.off";
    //const std::string& file_name = setting::resource_directory() + "/data/repair/non_manifold/complex_edges_2.off";
    SurfaceMesh* mesh = new SurfaceMesh;
    if (load_off(file_name, mesh)) {
        viewer.add_model(mesh, true);
        viewer.run();
    }
    else {
        delete mesh;
        LOG(ERROR) << "loading file failed";
    }
    return EXIT_SUCCESS;
}
