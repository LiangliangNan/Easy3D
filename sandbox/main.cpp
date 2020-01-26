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
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>

using namespace easy3d;


// This example shows how to
//		- override the file loading function of the default easy3d viewer to visualize textured meshes;


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize(argv[0]);

    try {
        const std::vector<std::string> files = {
				//setting::resource_directory() + "/data/repair/non_manifold/complex_edges_0.off",
				//setting::resource_directory() + "/data/repair/non_manifold/complex_edges_1.off",
				//setting::resource_directory() + "/data/repair/non_manifold/complex_edges_2.off",
				//setting::resource_directory() + "/data/repair/non_manifold/complex_vertices.off",
				//setting::resource_directory() + "/data/repair/non_manifold/complex_and_isolated_vertices.off",
				setting::resource_directory() + "/data/repair/non_manifold/2_umbrellas_open.off",
				setting::resource_directory() + "/data/repair/non_manifold/3_umbrellas_open.off",
				setting::resource_directory() + "/data/repair/non_manifold/5_umbrellas_open.off",
				setting::resource_directory() + "/data/repair/non_manifold/6_umbrellas_open.off",
				//setting::resource_directory() + "/data/repair/non_manifold/3_umbrellas.off",
			    //setting::resource_directory() + "/data/repair/non_manifold/5_umbrellas.off",
			    //setting::resource_directory() + "/data/repair/non_manifold/6_umbrellas.off",
				//setting::resource_directory() + "/data/domik/domik.obj",
				//setting::resource_directory() + "/data/house/house.obj",
				//setting::resource_directory() + "/data/other/sponza/sponza.obj",
				//setting::resource_directory() + "/data/tree.obj"
		};



#if 0
        for (int i = 0; i < files.size(); ++i) {
            SurfaceMeshIO::load(files[i]);
        }
        return EXIT_SUCCESS;
#else
        // Create the viewer.
        Viewer viewer;

        for (int i = 0; i < files.size(); ++i) {
            if (!viewer.add_model(files[i], true))
                LOG(FATAL) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        }

        // Run the viewer
        viewer.run();
#endif

    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "Caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
