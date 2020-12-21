#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/core/curve.h>
#include <easy3d/util/string.h>
#include <easy3d/core/poly_mesh.h>

#include <cstring>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>


using namespace easy3d;

int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    const std::string file_name = resource::directory() + "/data/sphere.tet";

    Viewer viewer("TetMesh");
    if (!viewer.add_model(file_name)) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    // Run the viewer
    return viewer.run();
}
