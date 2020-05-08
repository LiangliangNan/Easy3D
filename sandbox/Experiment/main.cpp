#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/core/hash.h>

using namespace easy3d;

int main(int argc, char **argv) {
    std::vector<float> a = {16, 0};
    std::vector<float> b = {4, 12};
    std::cout << "a: \n" << hash_range(a.begin(), a.end()) << std::endl;
    std::cout << "b: \n" << hash_range(b.begin(), b.end()) << std::endl;
    return 0;
}

