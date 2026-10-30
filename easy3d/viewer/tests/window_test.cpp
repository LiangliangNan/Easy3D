// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <set>
#include <cmath>
#ifdef EASY3D_NO_OPENGL
static_assert(std::is_same<easy3d::Viewer, easy3d::VulkanViewer>::value, "Viewer must select Vulkan in a native-only build");
#endif
using namespace easy3d;
void require(bool condition, const char* text) { if (!condition) throw std::runtime_error(text); }
void frame(VulkanViewer& viewer) {
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        VulkanViewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(0.01);
    }
    throw std::runtime_error("no frame was presented within bounded retries");
}
struct ThrowingViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    bool key_press_event(int, int) override { throw std::runtime_error("expected callback failure"); }
};
// A boolean must retain the same meaning as Viewer::run(), rather than
// accidentally selecting a one-frame budget in the native implementation.
struct RunViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    unsigned frames = 0;
    void post_draw() override { if (++frames == 2) request_close(); }
};
int main(int argc, char**) {
    try {
        initialize(false, false, false);
        VulkanViewerOptions options;
        options.width = 171; options.height = 129;
        options.device.validation = argc > 1;
        options.device.synchronization_validation = options.device.validation;
        VulkanViewer viewer("Vulkan window test", options);
        require(glfwGetWindowAttrib(viewer.window(), GLFW_CLIENT_API) == GLFW_NO_API, "window created a graphics context");
        require(glfwGetCurrentContext() == nullptr, "OpenGL context exists in native test");
        auto* model = viewer.add_model(resource::directory() + "/data/bunny.ply");
        require(model, "test mesh could not load");
        auto* faces = model->renderer()->get_triangles_drawable("faces");
        const bool msaa = (viewer.device().supported_sample_counts() & 4) != 0;
        if (msaa) viewer.set_samples(4);
        faces->set_uniform_coloring(vec4(1, 0, 0, 1)); faces->set_lighting(false);
        for (unsigned i = 0; i < 12; ++i) frame(viewer);
        auto captured = viewer.capture();
        unsigned hit = 0;
        for (auto id : captured.ids) hit += id.object != UINT32_MAX;
        require(hit > 200, "presented scene is empty");
        for (size_t i = 0; i < captured.ids.size(); ++i) {
            if (captured.ids[i].object == UINT32_MAX) continue;
            bool found = false;
            auto point = viewer.point_under_pixel(int(i % captured.width), int(i / captured.width), found);
            const auto projected = viewer.camera()->projectedCoordinatesOf(point);
            require(found && std::abs(projected.z - captured.depth[i]) < 1e-4f, "depth picking/unprojection mismatch");
            const auto clip=viewer.camera()->modelViewProjectionMatrix()*vec4(point,1);
            const float px=(clip.x/clip.w+1)*.5f*captured.width;
            const float py=(1-clip.y/clip.w)*.5f*captured.height;
            require(std::abs(px-float(i%captured.width)-.5f)<1e-3f &&
                    std::abs(py-float(i/captured.width)-.5f)<1e-3f,"world-point picking is offset from framebuffer pixel center");
            break;
        }
        bool found = true; viewer.point_under_pixel(-1, 0, found);
        require(!found, "out-of-range depth picking returned a hit");
        // Camera controls travel through the registered native callbacks, including
        // error containment at the C ABI boundary, rather than test-only setters.
        auto scroll = glfwSetScrollCallback(viewer.window(), nullptr);
        glfwSetScrollCallback(viewer.window(), scroll);
        const auto before = viewer.camera()->position();
        scroll(viewer.window(), 0, 2);
        require((viewer.camera()->position() - before).norm() > 0, "wheel callback did not zoom");
        auto key = glfwSetKeyCallback(viewer.window(), nullptr);
        glfwSetKeyCallback(viewer.window(), key);
        const auto projection = viewer.camera()->type();
        key(viewer.window(), GLFW_KEY_P, 0, GLFW_PRESS, 0);
        require(viewer.camera()->type() != projection, "key callback did not change projection");
        key(viewer.window(), GLFW_KEY_F, 0, GLFW_PRESS, 0);
        auto cursor = glfwSetCursorPosCallback(viewer.window(), nullptr);
        glfwSetCursorPosCallback(viewer.window(), cursor);
        auto mouse = glfwSetMouseButtonCallback(viewer.window(), nullptr);
        glfwSetMouseButtonCallback(viewer.window(), mouse);
        glfwSetCursorPos(viewer.window(), 60, 50);
        const auto direction = viewer.camera()->viewDirection();
        mouse(viewer.window(), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        cursor(viewer.window(), 90, 75);
        mouse(viewer.window(), GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
        require((viewer.camera()->viewDirection() - direction).norm() > 0, "mouse callback did not rotate");
        std::set<std::pair<int, int>> extents;
        for (unsigned i = 0; i < 8; ++i) {
            if (msaa) viewer.set_samples(i % 2 ? 1 : 4);
            viewer.resize(151 + i * 13, 113 + i * 7);
            frame(viewer);
            int w, h; viewer.framebuffer_size(w, h);
            require(viewer.width() == w && viewer.height() == h, "camera/HiDPI framebuffer extent mismatch");
            extents.insert({w, h});
            faces->set_uniform_coloring(vec4(0, float(i % 2), 1, 1));
            model->renderer()->update();
        }
        require(extents.size() >= 3, "resize test did not exercise different framebuffer extents");
        auto* points = model->renderer()->get_points_drawable("vertices");
        points->set_visible(true); points->set_point_size(9);
        points->set_impostor_type(PointsDrawable::SPHERE);
        faces->set_visible(false);
        viewer.set_samples(viewer.device().supported_sample_counts(graphics::PointStyle::sphere) & 4 ? 4 : 1);
        frame(viewer);
        auto spheres = viewer.capture();
        unsigned sphere_hits=0;
        for (size_t i=0;i<spheres.ids.size();++i) {
            if (spheres.ids[i].object==UINT32_MAX) continue;
            if (sphere_hits++ == 0) {
                bool found=false;
                const auto p=viewer.point_under_pixel(int(i%spheres.width),int(i/spheres.width),found);
                require(found && std::abs(viewer.camera()->projectedCoordinatesOf(p).z-spheres.depth[i])<1e-4f,
                        "sphere window depth picking mismatch");
            }
        }
        require(sphere_hits>100,"sphere presentation/capture is empty");
        points->set_impostor_type(PointsDrawable::SURFEL);
        viewer.set_samples(viewer.device().supported_sample_counts(graphics::PointStyle::surfel) & 4 ? 4 : 1);
        frame(viewer);
        auto discs = viewer.capture();
        unsigned disc_hits = 0;
        for (size_t i = 0; i < discs.ids.size(); ++i) {
            if (discs.ids[i].object == UINT32_MAX) continue;
            if (disc_hits++ == 0) {
                bool found = false;
                const auto p = viewer.point_under_pixel(int(i%discs.width), int(i/discs.width), found);
                require(found && std::abs(viewer.camera()->projectedCoordinatesOf(p).z-discs.depth[i]) < 1e-4f,
                        "surfel window depth picking mismatch");
            }
        }
        require(disc_hits > 100, "surfel presentation/capture is empty");
        points->set_impostor_type(PointsDrawable::PLAIN);
        frame(viewer); // same cached geometry, different point style
        points->set_visible(false);
        auto segment = std::make_shared<LinesDrawable>();
        viewer.add_drawable(segment);
        const auto center = model->bounding_box().center();
        const float extent = model->bounding_box().diagonal_length()*0.4f;
        segment->update_vertex_buffer({center-vec3(extent,0,0), center+vec3(extent,0,0)});
        segment->set_line_width(12);
        for (auto style : {LinesDrawable::CYLINDER, LinesDrawable::CONE, LinesDrawable::PLAIN}) {
            segment->set_impostor_type(style);
            const auto native_style = style == LinesDrawable::CYLINDER ? graphics::LineStyle::cylinder :
                                      style == LinesDrawable::CONE ? graphics::LineStyle::cone : graphics::LineStyle::plain;
            viewer.set_samples(viewer.device().supported_sample_counts(native_style) & 4 ? 4 : 1);
            frame(viewer);
            const auto solid = viewer.capture();
            unsigned count = 0;
            for (size_t i = 0; i < solid.ids.size(); ++i) {
                if (solid.ids[i].object != 1) continue;
                if (count++ == 0) {
                    bool found = false;
                    const auto p = viewer.point_under_pixel(int(i%solid.width), int(i/solid.width), found);
                    require(found && std::abs(viewer.camera()->projectedCoordinatesOf(p).z-solid.depth[i]) < 1e-4f,
                            "line impostor window depth picking mismatch");
                }
            }
            require(count > 20, "line impostor presentation/capture is empty");
        }
        segment->set_visible(false); faces->set_visible(true);
        viewer.hide(); VulkanViewer::poll_events(); viewer.show(); frame(viewer);
        bool rejected_samples = false;
        try { viewer.set_samples(3); } catch (const std::invalid_argument&) { rejected_samples = true; }
        require(rejected_samples, "invalid window sample count was accepted");
        {
            auto unavailable = options;
            unavailable.device.device_name = "__Easy3D_no_such_physical_device__";
            bool rejected = false;
            try { VulkanViewer bad("Expected initialization failure", unavailable); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected, "unavailable device was accepted");
        }
        frame(viewer);
        // Native GLFW lifetime and GPU caches must survive a second window closing.
        {
            VulkanViewer second("Independent Vulkan device", options);
            if (msaa) second.set_samples(4);
            require(second.add_model(resource::directory() + "/data/bunny.ply"), "second mesh load failed");
            for (unsigned i = 0; i < 4; ++i) { frame(viewer); frame(second); }
            require(second.device().validation_errors() == 0, "second device validation errors");
        }
        frame(viewer);
        {
            ThrowingViewer bad("Callback error containment", options);
            auto callback = glfwSetKeyCallback(bad.window(), nullptr);
            glfwSetKeyCallback(bad.window(), callback);
            callback(bad.window(), GLFW_KEY_A, 0, GLFW_PRESS, 0);
            bool reported = false;
            try { bad.draw_frame(); }
            catch (const std::runtime_error& e) { reported = std::string(e.what()) == "expected callback failure"; }
            require(bad.should_close() && reported, "callback failure escaped or was swallowed");
        }
        frame(viewer);
        {
            auto invalid = options; invalid.samples = 3;
            bool rejected = false;
            try { VulkanViewer bad("Expected invalid MSAA initialization", invalid); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "invalid initial sample count was accepted");
        }
        auto* scalar_mesh = dynamic_cast<SurfaceMesh*>(model);
        auto scalar = scalar_mesh->add_vertex_property<float>("v:window_scalar");
        for (auto v : scalar_mesh->vertices()) scalar[v] = scalar_mesh->position(v).z;
        faces->set_scalar_coloring(State::VERTEX,"v:window_scalar",nullptr,0,0);
        model->renderer()->update(); frame(viewer);
        const auto textured = viewer.capture();
        bool scalar_hit = false;
        for (size_t i = 0; i < textured.ids.size(); ++i) {
            if (textured.ids[i].object == UINT32_MAX) continue;
            bool found = false;
            const auto point = viewer.point_under_pixel(int(i%textured.width),int(i/textured.width),found);
            require(found && std::abs(viewer.camera()->projectedCoordinatesOf(point).z-textured.depth[i]) < 1e-4f,
                    "scalar-textured window depth picking mismatch");
            scalar_hit = true; break;
        }
        require(scalar_hit,"scalar-textured window is empty");
        std::vector<graphics::Vertex> overlay_vertices(4);
        overlay_vertices[0].position=vec3(-1,-1,0); overlay_vertices[1].position=vec3(1,-1,0);
        overlay_vertices[2].position=vec3(1,1,0); overlay_vertices[3].position=vec3(-1,1,0);
        graphics::DrawCommand overlay;
        overlay.geometry=std::make_shared<graphics::Geometry>(overlay_vertices,std::vector<uint32_t>{0,1,2,0,2,3});
        overlay.color=vec4(1,0,0,0.5f);
        viewer.set_overlays({overlay}); frame(viewer);
        const auto composed=viewer.capture();
        require(composed.rgba!=textured.rgba,"native window overlay did not composite color");
        bool overlay_pick_checked=false;
        for (size_t i=0;i<composed.ids.size();++i) {
            require(composed.ids[i].object==textured.ids[i].object && composed.ids[i].primitive==textured.ids[i].primitive &&
                    composed.depth[i]==textured.depth[i],"native overlay modified picking IDs/depth");
            if (!overlay_pick_checked && composed.ids[i].object!=UINT32_MAX) {
                bool found=false;
                const auto point=viewer.point_under_pixel(int(i%composed.width),int(i/composed.width),found);
                require(found && std::abs(viewer.camera()->projectedCoordinatesOf(point).z-composed.depth[i])<1e-4f,
                        "overlay obscured native world-point picking");
                overlay_pick_checked=true;
            }
        }
        require(overlay_pick_checked,"native overlay picking test had no scene hit");
        viewer.set_overlays({}); frame(viewer);
        require(viewer.capture().rgba==textured.rgba,"clearing overlays retained stale color");
        require(viewer.snapshot("vulkan-window.png"), "window snapshot failed");
        const auto camera_before = viewer.camera()->modelViewProjectionMatrix();
        require(viewer.snapshot("vulkan-window-scaled.png", 0.5f, 1, 2), "parameterized snapshot failed");
        int image_w=0, image_h=0, channels=0;
        std::vector<unsigned char> pixels;
        require(ImageIO::load("vulkan-window-scaled.png", pixels, image_w, image_h, channels, 4, false) &&
                image_w == int(viewer.width()*0.5) && image_h == int(viewer.height()*0.5),
                "snapshot scaling did not reach file output");
        bool transparent=false;
        for (size_t i=3; i<pixels.size(); i+=4) transparent |= pixels[i]==0;
        require(transparent, "snapshot background option was ignored");
        const auto camera_after = viewer.camera()->modelViewProjectionMatrix();
        for (unsigned i=0; i<16; ++i)
            require(std::abs(camera_after[i]-camera_before[i])<1e-6f,
                    "snapshot changed the live camera");
        {
            RunViewer run_viewer("run(bool) parity", options);
            require(run_viewer.add_model(resource::directory()+"/data/bunny.ply"), "run test model missing");
            run_viewer.camera()->setPosition(run_viewer.camera()->position()+vec3(1,2,3));
            const auto position=run_viewer.camera()->position();
            require(run_viewer.run(false)==0 && run_viewer.frames==2 &&
                    run_viewer.camera()->position()==position, "run(false) changed camera or stopped early");
            glfwSetWindowShouldClose(run_viewer.window(), GLFW_FALSE);
            run_viewer.frames=0;
            require(run_viewer.run(true)==0 && run_viewer.frames==2 &&
                    run_viewer.camera()->position()!=position, "run(true) did not fit and run until close");
            require(run_viewer.device().validation_errors()==0, "run parity validation errors");
        }
        require(viewer.device().validation_errors() == 0, "presentation validation errors");
        key(viewer.window(), GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        require(viewer.should_close(), "close callback failed");
        std::cout << "PASS: native WSI, scene, capture, callbacks, resize, hide/show, multiple devices/windows, close\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
