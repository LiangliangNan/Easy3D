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
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/gui/picker_model.h>

using namespace easy3d;


// This example shows how to
//		- override the file loading function of the default easy3d viewer to visualize textured meshes;


int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    try {
        const std::vector<std::string> files = {
//                resource::directory() + "/data/repair/non_manifold/complex_edges_1.off",
                resource::directory() + "/data/repair/non_manifold/complex_vertices.off",
                resource::directory() + "/data/repair/non_manifold/3_umbrellas.off",
        };

        // Create the viewer.
        Viewer viewer;
        for (const auto& name : files) {
            if (!viewer.add_model(name, true))
                LOG(FATAL) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        }

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
