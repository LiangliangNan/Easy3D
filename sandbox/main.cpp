// just a SandBox.

#include <easy3d/viewer/viewer.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/gui/picker_model.h>

using namespace easy3d;


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
