// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#ifdef EASY3D_NO_OPENGL
#include <easy3d/viewer/offscreen.h>
#include <type_traits>
static_assert(std::is_same<easy3d::OffScreen, easy3d::VulkanOffScreen>::value,
              "Vulkan-only OffScreen must use native Vulkan");
#endif
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
unsigned hits(const graphics::FrameImage& frame) {
    unsigned count = 0;
    for (auto id : frame.ids) count += id.object != UINT32_MAX;
    return count;
}
int main(int argc, char**) {
    try {
        initialize(false, false, false);
        graphics::DeviceOptions options;
        options.validation = argc > 1;
        options.synchronization_validation = options.validation;
        VulkanOffScreen renderer(161, 127, options);
        auto* model = renderer.add_model(resource::directory() + "/data/bunny.ply");
        require(model, "could not load bunny mesh");
        auto* faces = model->renderer()->get_triangles_drawable("faces");
        faces->set_uniform_coloring(vec4(1, 0, 0, 1));
        faces->set_lighting(false);
        auto red = renderer.capture();
        require(hits(red) > 500, "mesh did not render");
        for (size_t i = 0; i < red.ids.size(); ++i)
            if (red.ids[i].object != UINT32_MAX)
                require(red.rgba[4 * i] == 255 && red.rgba[4 * i + 1] == 0, "uniform drawable color failed");
        auto* mesh = dynamic_cast<SurfaceMesh*>(model);
        auto colors = mesh->add_vertex_property<vec3>("v:color", vec3(0, 1, 0));
        faces->set_property_coloring(State::VERTEX, "v:color");
        model->renderer()->update();
        auto green = renderer.capture();
        require(hits(green) == hits(red), "color-only update changed geometry");
        for (size_t i = 0; i < green.ids.size(); ++i)
            if (green.ids[i].object != UINT32_MAX)
                require(green.rgba[4 * i] == 0 && green.rgba[4 * i + 1] == 255, "property coloring failed");
        require(renderer.render("vulkan-bunny.png", 1.5f), "PNG capture failed");
        int w = 0, h = 0, channels = 0;
        std::vector<unsigned char> png;
        require(ImageIO::load("vulkan-bunny.png", png, w, h, channels, 4, false) && w == 241 && h == 190,
                "scaled PNG dimensions mismatch");
        faces->set_uniform_coloring(vec4(0, 0, 1, 1));
        auto blue = renderer.capture();
        for (size_t i = 0; i < blue.ids.size(); ++i)
            if (blue.ids[i].object != UINT32_MAX)
                require(blue.rgba[4 * i + 2] == 255, "uniform coloring retained stale property colors");
        faces->set_visible(false);
        require(hits(renderer.capture()) == 0, "drawable visibility failed");
        faces->set_visible(true);
        renderer.resize(97, 83);
        require(hits(renderer.capture()) > 100, "resized mesh did not render");
        require(renderer.delete_model(model), "model deletion failed");
        require(hits(renderer.capture()) == 0, "deleted model still visible");
        auto cloud = std::make_shared<PointCloud>();
        cloud->add_vertex(vec3(-1, 0, 0)); cloud->add_vertex(vec3(1, 0, 0));
        renderer.add_model(cloud);
        require(hits(renderer.capture()) > 0, "point cloud did not render");
        auto* points = cloud->renderer()->get_points_drawable("vertices");
        points->set_point_size(17); points->set_lighting(false);
        const auto large = renderer.capture();
        require(hits(large)>100,"scene adapter did not expand large points");
        points->set_impostor_type(PointsDrawable::SPHERE);
        for (auto type : {Camera::PERSPECTIVE, Camera::ORTHOGRAPHIC}) {
            renderer.camera()->setType(type);
            auto spheres = renderer.capture();
            require(hits(spheres)>50,"scene sphere adapter did not render");
            if (renderer.device().supported_sample_counts(graphics::PointStyle::sphere) & 4) {
                auto aa = renderer.capture(4);
                for (size_t i=0;i<aa.ids.size();++i)
                    require(aa.ids[i].object==spheres.ids[i].object && std::abs(aa.depth[i]-spheres.depth[i])<1e-5f,
                            "scene sphere MSAA changed depth/picking");
            }
        }
        points->set_impostor_type(PointsDrawable::SURFEL);
        bool surfel_rejected=false;
        try { renderer.capture(); } catch (const std::logic_error&) { surfel_rejected=true; }
        require(surfel_rejected,"unsupported surfel was silently rendered");
        renderer.clear_scene();
        auto custom = std::make_shared<TrianglesDrawable>();
        custom->set_opacity(1);
        renderer.add_drawable(custom); // enable CPU buffers before supplying temporary data
        custom->update_vertex_buffer({vec3(-1, -1, 0), vec3(1, -1, 0), vec3(0, 1, 0)});
        renderer.camera()->setViewDirection(vec3(0, 0, -1));
        renderer.camera()->setUpVector(vec3(0, 1, 0));
        renderer.fit_screen();
        require(hits(renderer.capture()) > 100, "custom drawable did not render");
        const auto projected = renderer.camera()->projectedCoordinatesOf(vec3(0, 0, 0));
        const auto planar = renderer.capture();
        const size_t pixel = size_t(projected.y) * planar.width + size_t(projected.x);
        require(pixel < planar.depth.size() && std::abs(planar.depth[pixel] - projected.z) < 1e-4f,
                "Vulkan depth differs from Easy3D Camera projection");
        custom->update_vertex_buffer({vec3(-0.5f, -0.5f, 0), vec3(0.5f, -0.5f, 0), vec3(0, 0.5f, 0)});
        require(hits(renderer.capture()) > 20, "custom geometry replacement did not render");
        bool rejected = false;
        try { custom->gl_draw(); } catch (const std::logic_error&) { rejected = true; }
        require(rejected, "GL-only API did not reject CPU drawable");
        if (renderer.device().supported_sample_counts() & 4) {
            custom->set_uniform_coloring(vec4(1, 0, 0, 1)); custom->set_lighting(false);
            require(renderer.render("vulkan-msaa.png", 1, 4, 2), "four-sample snapshot failed");
            require(ImageIO::load("vulkan-msaa.png", png, w, h, channels, 4, false), "MSAA PNG loading failed");
            unsigned edges = 0;
            for (size_t i = 0; i < png.size(); i += 4) {
                if (png[i + 3] == 0 || png[i + 3] == 255) continue;
                ++edges;
                require(png[i] >= 253 && png[i + 1] == 0 && png[i + 2] == 0, "transparent MSAA edges have a dark fringe");
            }
            require(edges > 0, "transparent MSAA PNG has no partial coverage");
        } else std::cout << "SKIP: device does not advertise four-sample color targets\n";
        // Test the throwing API for expected failures, keeping normal test output
        // free from the user-facing error logger of the bool render() wrapper.
        rejected = false;
        try { renderer.capture(3); } catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "invalid sample count was accepted");
        require(renderer.device().validation_errors() == 0, "Vulkan validation reported errors");
        std::cout << "PASS: Easy3D model loading, Camera, Renderer/Drawable, CPU extraction, property color, updates, visibility, resize, snapshots/MSAA, points, custom drawables\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << std::endl; return 1; }
}
