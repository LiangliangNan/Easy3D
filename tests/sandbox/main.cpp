#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

int main (int argc, char *argv[])
{
    easy3d::logging::initialize(argv[0]);

    const std::string path("/Users/Liangliang/PolyFit.app");

    CHECK_EQ(file_system::stripped_name(path), "PolyFit");
    CHECK_EQ(file_system::parent_directory(path), "/Users/Liangliang");

    LOG(INFO) << "Tests succeeded!!!";
    return EXIT_SUCCESS;
}
