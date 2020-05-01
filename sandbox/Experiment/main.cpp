#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/fileio/image_io.h>

using namespace easy3d;

int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    Viewer v;

    const std::string file_name = "/Users/lnan/Projects/Easy3D/cmake-build-debug/bin/resources/colormaps/french.png";
    int width = 0, height = 0, comp = 0;
    std::vector<unsigned char> data;
    // flip the image vertically, so the first pixel in the output array is the bottom left
    bool success = ImageIO::load(file_name, data, width, height, comp, 0, true);
    if (!success || data.empty())
        return EXIT_FAILURE;

#if 1 // discretize a gradually varying (from left to right) color image
    const int num_colors = 16;
    discretize_image(data, width, height, comp,num_colors );
    ImageIO::save(file_name + "-" + std::to_string(num_colors) + ".png", data, width, height, comp);

#endif


    return v.run();
}

