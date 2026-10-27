// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
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
        points->set_impostor_type(PointsDrawable::PLAIN);
        frame(viewer); // same cached geometry, different point style
        points->set_visible(false); faces->set_visible(true);
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
        require(viewer.snapshot("vulkan-window.png"), "window snapshot failed");
        require(viewer.device().validation_errors() == 0, "presentation validation errors");
        key(viewer.window(), GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
        require(viewer.should_close(), "close callback failed");
        std::cout << "PASS: native WSI, scene, capture, callbacks, resize, hide/show, multiple devices/windows, close\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
