#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/curve.h>
#include <easy3d/util/string.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/core/curve.h>
#include <easy3d/util/file_system.h>

#include <cstring>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>

using namespace easy3d;


int main()
{
    std::string file = "/Users/lnan/Downloads/LargeMeshAnimation/kf/path_short.kf";
//    std::string file = "/Users/lnan/Dropbox/Students/2020-PhD-Zexin/data/Lille-3D/test.kf";

    std::ifstream is(file.c_str());
    KeyFrameInterpolator kfi;
    kfi.read_keyframes(is);

    std::vector<vec3> points(kfi.number_of_keyframes());
    for (std::size_t i=0; i<kfi.number_of_keyframes(); ++i)
        points[i] = kfi.keyframe(i).position();


    const int resolution = 1000; // Number of line subdivisions to display the spline
    const int order = 120; // Smoothness of the spline (min 2)
//    curve::spline::Spline<vec3> spline_curve(order, curve::spline::Spline<vec3>::eUNIFORM);
    curve::spline::Spline<vec3> spline_curve(order, curve::spline::Spline<vec3>::eOPEN_UNIFORM);
    spline_curve.set_ctrl_points(points);


    std::string output_file = file_system::name_less_extension(file) +  "-Spline-order-" + std::to_string(order) + "-eOPEN_UNIFORM.xyz";
    std::ofstream output(output_file.c_str());
    for(int i = 0; i < resolution; ++i) {
        const vec3 p = spline_curve.eval_f( float(i) / float(resolution-1) );
        output << p << std::endl;
    }

    return EXIT_SUCCESS;
}