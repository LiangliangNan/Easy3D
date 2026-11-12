#include "test_window.h"
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
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <set>
#include <cmath>
#ifdef EASY3D_HAS_VULKAN
static_assert(std::is_same<easy3d::Viewer, easy3d::Viewer>::value, "Viewer must select Vulkan in a native-only build");
#endif
using namespace easy3d;
void require(bool condition, const char* text) { if (!condition) throw std::runtime_error(text); }
void frame(Viewer& viewer) {
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        Viewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(0.01);
    }
    throw std::runtime_error("no frame was presented within bounded retries");
}
// pick() consumes logical window coordinates; the GPU samples the framebuffer pixel
// floor(logical * framebuffer / logical). Map a logical pixel to that capture texel so
// a pick depth can be compared against the matching capture sample.
size_t sampled_texel(const Viewer& v, int lx, int ly, int fw, int fh) {
    const int sx = std::min(fw - 1, int(std::floor(double(lx) * fw / v.width())));
    const int sy = std::min(fh - 1, int(std::floor(double(ly) * fh / v.height())));
    return size_t(sy) * fw + sx;
}
// Find the first logical pixel that picks the scene and verify its depth matches the
// capture texel the query samples (pick is logical; capture is framebuffer-sized).
void require_pick_depth(const Viewer& v, const graphics::FrameImage& cap, const char* msg) {
    for (int ly = 0; ly < v.height(); ++ly)
        for (int lx = 0; lx < v.width(); ++lx) {
            const size_t si = sampled_texel(v, lx, ly, cap.width, cap.height);
            if (cap.ids[si].object == UINT32_MAX) continue;
            const auto hit = v.pick(lx, ly);
            if (!hit) continue;
            require(std::abs(v.camera()->projected_coordinates_of(hit.position).z - cap.depth[si]) < 1e-4f, msg);
            return;
        }
    require(false, msg);
}
struct ThrowingViewer : Viewer {
    using Viewer::Viewer;
    bool key_press_event(int, int) override { throw std::runtime_error("expected callback failure"); }
};
// A boolean must retain the same meaning as Viewer::run(), rather than
// accidentally selecting a one-frame budget in the native implementation.
struct RunViewer : Viewer {
    using Viewer::Viewer;
    unsigned frames = 0;
    void post_draw() override { if (++frames == 2) request_close(); }
};
int main(int argc, char**) {
    try {
        initialize(false, false, false);
        TestWindow options;
        options.width = 171; options.height = 129;
        options.device.validation = argc > 1;
        options.device.sync_validation = options.device.validation;
        {   // Constructor with explicit parameters: a requested count of 4 never fails (it steps down on
            // devices without 4x MSAA), 0 means no MSAA, and negative or non-power-of-two counts are rejected.
            Viewer four("Explicit parameters", 171, 129, 4, false, false, true, options.device);
            require(four.samples() >= 1 && four.samples() <= 4 && four.width() == 171 && four.height() == 129,
                    "explicit constructor ignored its parameters");
            Viewer none("No MSAA", 171, 129, 0, false, false, true, options.device);
            require(none.samples() == 1, "0 samples did not mean no MSAA");
            bool rejected = false;
            try { Viewer bad("Negative samples", 171, 129, -1, false, false, true, options.device); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "negative sample count was accepted");
        }
        Viewer viewer("Vulkan window test", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
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
        bool picked = false;
        for (int ly = 0; ly < viewer.height() && !picked; ++ly)
            for (int lx = 0; lx < viewer.width() && !picked; ++lx) {
                const auto result = viewer.pick(lx, ly);
                if (!result) continue;
                const size_t si = sampled_texel(viewer, lx, ly, captured.width, captured.height);
                const auto point = result.position;
                const auto projected = viewer.camera()->projected_coordinates_of(point);
                require(std::abs(projected.z - captured.depth[si]) < 1e-4f, "depth picking/unprojection mismatch");
                const auto clip = viewer.camera()->model_view_projection_matrix() * vec4(point, 1);
                const float px = (clip.x / clip.w + 1) * .5f * captured.width;
                const float py = (1 - clip.y / clip.w) * .5f * captured.height;
                require(std::abs(px - float(si % captured.width) - .5f) < 1e-3f &&
                        std::abs(py - float(si / captured.width) - .5f) < 1e-3f,
                        "world-point picking is offset from framebuffer pixel center");
                picked = true;
            }
        require(picked, "no logical window pixel produced a scene pick");
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
        key(viewer.window(), Viewer::KEY_P, 0, GLFW_PRESS, 0);
        require(viewer.camera()->type() != projection, "key callback did not change projection");
        key(viewer.window(), Viewer::KEY_F, 0, GLFW_PRESS, 0);
        auto cursor = glfwSetCursorPosCallback(viewer.window(), nullptr);
        glfwSetCursorPosCallback(viewer.window(), cursor);
        auto mouse = glfwSetMouseButtonCallback(viewer.window(), nullptr);
        glfwSetMouseButtonCallback(viewer.window(), mouse);
        glfwSetCursorPos(viewer.window(), 60, 50);
        const auto direction = viewer.camera()->view_direction();
        mouse(viewer.window(), Viewer::BUTTON_LEFT, GLFW_PRESS, 0);
        cursor(viewer.window(), 90, 75);
        mouse(viewer.window(), Viewer::BUTTON_LEFT, GLFW_RELEASE, 0);
        require((viewer.camera()->view_direction() - direction).norm() > 0, "mouse callback did not rotate");
        std::set<std::pair<int, int>> extents;
        for (unsigned i = 0; i < 8; ++i) {
            if (msaa) viewer.set_samples(i % 2 ? 1 : 4);
            viewer.resize(151 + i * 13, 113 + i * 7);
            frame(viewer);
            int w, h; viewer.framebuffer_size(w, h);
            require(viewer.framebuffer_width() == w && viewer.framebuffer_height() == h, "camera/HiDPI framebuffer extent mismatch");
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
        for (size_t i=0;i<spheres.ids.size();++i) if (spheres.ids[i].object!=UINT32_MAX) ++sphere_hits;
        require(sphere_hits>100,"sphere presentation/capture is empty");
        require_pick_depth(viewer, spheres, "sphere window depth picking mismatch");
        points->set_impostor_type(PointsDrawable::SURFEL);
        viewer.set_samples(viewer.device().supported_sample_counts(graphics::PointStyle::surfel) & 4 ? 4 : 1);
        frame(viewer);
        auto discs = viewer.capture();
        unsigned disc_hits = 0;
        for (size_t i = 0; i < discs.ids.size(); ++i) if (discs.ids[i].object != UINT32_MAX) ++disc_hits;
        require(disc_hits > 100, "surfel presentation/capture is empty");
        require_pick_depth(viewer, discs, "surfel window depth picking mismatch");
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
            for (size_t i = 0; i < solid.ids.size(); ++i) if (solid.ids[i].object == 1) ++count;
            require(count > 20, "line impostor presentation/capture is empty");
            require_pick_depth(viewer, solid, "line impostor window depth picking mismatch");
        }
        segment->set_visible(false); faces->set_visible(true);
        viewer.hide(); Viewer::poll_events(); viewer.show(); frame(viewer);
        bool rejected_samples = false;
        try { viewer.set_samples(3); } catch (const std::invalid_argument&) { rejected_samples = true; }
        require(rejected_samples, "invalid window sample count was accepted");
        {
            auto unavailable = options;
            unavailable.device.device_name = "__Easy3D_no_such_physical_device__";
            bool rejected = false;
            try { Viewer bad("Expected initialization failure", unavailable.width, unavailable.height, unavailable.samples, false, unavailable.visible, unavailable.resizable, unavailable.device); }
            catch (const std::runtime_error&) { rejected = true; }
            require(rejected, "unavailable device was accepted");
        }
        frame(viewer);
        // Native GLFW lifetime and GPU caches must survive a second window closing.
        {
            Viewer second("Independent Vulkan device", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
            if (msaa) second.set_samples(4);
            require(second.add_model(resource::directory() + "/data/bunny.ply"), "second mesh load failed");
            for (unsigned i = 0; i < 4; ++i) { frame(viewer); frame(second); }
            require(second.device().validation_errors() == 0, "second device validation errors");
        }
        frame(viewer);
        {
            ThrowingViewer bad("Callback error containment", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
            auto callback = glfwSetKeyCallback(bad.window(), nullptr);
            glfwSetKeyCallback(bad.window(), callback);
            callback(bad.window(), Viewer::KEY_A, 0, GLFW_PRESS, 0);
            bool reported = false;
            try { bad.draw_frame(); }
            catch (const std::runtime_error& e) { reported = std::string(e.what()) == "expected callback failure"; }
            require(bad.should_close() && reported, "callback failure escaped or was swallowed");
        }
        frame(viewer);
        {
            auto invalid = options; invalid.samples = 3;
            bool rejected = false;
            try { Viewer bad("Expected invalid MSAA initialization", invalid.width, invalid.height, invalid.samples, false, invalid.visible, invalid.resizable, invalid.device); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "invalid initial sample count was accepted");
        }
        auto* scalar_mesh = dynamic_cast<SurfaceMesh*>(model);
        auto scalar = scalar_mesh->add_vertex_property<float>("v:window_scalar");
        for (auto v : scalar_mesh->vertices()) scalar[v] = scalar_mesh->position(v).z;
        faces->set_scalar_coloring(State::VERTEX,"v:window_scalar",nullptr,0,0);
        model->renderer()->update(); frame(viewer);
        const auto textured = viewer.capture();
        unsigned scalar_hits = 0;
        for (size_t i = 0; i < textured.ids.size(); ++i) if (textured.ids[i].object != UINT32_MAX) ++scalar_hits;
        require(scalar_hits > 0,"scalar-textured window is empty");
        require_pick_depth(viewer, textured, "scalar-textured window depth picking mismatch");
        std::vector<graphics::Vertex> overlay_vertices(4);
        overlay_vertices[0].position=vec3(-1,-1,0); overlay_vertices[1].position=vec3(1,-1,0);
        overlay_vertices[2].position=vec3(1,1,0); overlay_vertices[3].position=vec3(-1,1,0);
        graphics::DrawCommand overlay;
        overlay.geometry=std::make_shared<graphics::Geometry>(overlay_vertices,std::vector<uint32_t>{0,1,2,0,2,3});
        overlay.color=vec4(1,0,0,0.5f);
        viewer.set_overlays({overlay}); frame(viewer);
        const auto composed=viewer.capture();
        require(composed.rgba!=textured.rgba,"native window overlay did not composite color");
        for (size_t i=0;i<composed.ids.size();++i)
            require(composed.ids[i].object==textured.ids[i].object && composed.ids[i].primitive==textured.ids[i].primitive &&
                    composed.depth[i]==textured.depth[i],"native overlay modified picking IDs/depth");
        require_pick_depth(viewer, composed, "overlay obscured native world-point picking");
        viewer.set_overlays({}); frame(viewer);
        require(viewer.capture().rgba==textured.rgba,"clearing overlays retained stale color");
        require(viewer.snapshot("vulkan-window.png"), "window snapshot failed");
        const auto camera_before = viewer.camera()->model_view_projection_matrix();
        require(viewer.snapshot("vulkan-window-scaled.png", 0.5f, 1, 2), "parameterized snapshot failed");
        int image_w=0, image_h=0, channels=0;
        std::vector<unsigned char> pixels;
        require(ImageIO::load("vulkan-window-scaled.png", pixels, image_w, image_h, channels, 4, false) &&
                image_w == int(viewer.framebuffer_width()*0.5) && image_h == int(viewer.framebuffer_height()*0.5),
                "snapshot scaling did not reach file output");
        bool transparent=false;
        for (size_t i=3; i<pixels.size(); i+=4) transparent |= pixels[i]==0;
        require(transparent, "snapshot background option was ignored");
        const auto camera_after = viewer.camera()->model_view_projection_matrix();
        for (unsigned i=0; i<16; ++i)
            require(std::abs(camera_after[i]-camera_before[i])<1e-6f,
                    "snapshot changed the live camera");
        {
            RunViewer run_viewer("run(bool) parity", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
            require(run_viewer.add_model(resource::directory()+"/data/bunny.ply"), "run test model missing");
            run_viewer.camera()->set_position(run_viewer.camera()->position()+vec3(1,2,3));
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
        key(viewer.window(), Viewer::KEY_ESCAPE, 0, GLFW_PRESS, 0);
        require(viewer.should_close(), "close callback failed");
        std::cout << "PASS: native WSI, scene, capture, callbacks, resize, hide/show, multiple devices/windows, close\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
