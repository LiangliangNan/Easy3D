// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer_base.h>
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
#include <easy3d/viewer/viewer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
using Factory = easy3d::Viewer; // Same public factory in native-only and dual builds.
#else
using Factory = easy3d::ViewerBase;
#endif
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace easy3d;
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
ViewerOptions options(Backend backend = Backend::Automatic) {
    ViewerOptions o; o.backend = backend; o.width = 181; o.height = 151; o.visible = false; return o;
}
void invalid_options() {
    auto o = options();
    const auto rejects = [](const ViewerOptions& invalid) {
        bool rejected = false;
        try { auto viewer = Factory::create("invalid", invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "invalid viewer options reached initialization/fallback");
    };
    o.width = 0; rejects(o); o = options(); o.height = -1; rejects(o);
    for (unsigned samples : {0u, 3u, 128u}) { o = options(); o.samples = samples; rejects(o); }
    o = options(static_cast<Backend>(99)); rejects(o);
}
void exercise(ViewerBase& viewer, const std::string& prefix) {
    require(viewer.width() == 181 && viewer.height() == 151, "common dimensions are not logical window units");
    require(viewer.samples() == 1 || viewer.samples() == 4, "common MSAA count is invalid");
    require(viewer.background_color() == setting::background_color, "shared background default missing");
    require(viewer.camera()->type() == Camera::PERSPECTIVE &&
            distance(viewer.camera()->viewDirection(), vec3(-1, 0, 0)) < 1e-5f &&
            distance(viewer.camera()->upVector(), vec3(0, 0, 1)) < 1e-5f,
            "factory camera defaults differ between backends");
    auto mesh = std::make_shared<SurfaceMesh>();
    const auto a = mesh->add_vertex(vec3(-1, -1, 0)), b = mesh->add_vertex(vec3(1, -1, 0));
    const auto c = mesh->add_vertex(vec3(0, 1, 0)); mesh->add_triangle(a, b, c);
    require(viewer.add_model(mesh) == mesh.get(), "common model attachment failed");
    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_lighting(false); faces->set_uniform_coloring(vec4(.8f, .2f, .1f, 1));
    require(faces->has_cpu_buffers() == (viewer.backend() == Backend::Vulkan), "backend buffer policy differs from selection");
    viewer.camera()->setViewDirection(vec3(0, 0, -1)); viewer.camera()->setUpVector(vec3(0, 1, 0)); viewer.fit_screen();
    viewer.set_background_color(vec4(.12f, .23f, .34f, 1));
    const auto camera = viewer.camera()->modelViewProjectionMatrix();
    unsigned ticks = 0;
    viewer.set_usage("", "common backend test");
    viewer.set_animation_callback([&](ViewerBase* host) { require(host == &viewer, "animation received another host"); ++ticks; return false; });
    viewer.set_animation(true);
    require(viewer.run_frames(2) == 0 && ticks >= 2, "portable animation/loop did not run");
    viewer.set_animation(false); viewer.set_animation_callback({});
    for (unsigned i = 0; i < 16; ++i)
        require(std::abs(camera[i] - viewer.camera()->modelViewProjectionMatrix()[i]) < 1e-6f, "bounded runtime viewer changed camera");
    int fw, fh; viewer.framebuffer_size(fw, fh);
    require(fw == viewer.framebuffer_width() && fh == viewer.framebuffer_height() && fw > 0 && fh > 0, "framebuffer queries disagree");
    const auto selected = viewer.backend();
    for (int samples : {1, 4}) {
        const auto file = prefix + "-" + std::to_string(samples) + ".png";
        require(viewer.snapshot(file, 1, samples, 0), "common snapshot failed");
        int w, h, channels; std::vector<unsigned char> rgba;
        require(ImageIO::load(file, rgba, w, h, channels, 4, false) && w == fw && h == fh, "common snapshot size/readback failed");
        const auto center = (size_t(h / 2) * w + w / 2) * 4;
        require(std::abs(int(rgba[center]) - 204) <= 2 && std::abs(int(rgba[center+1]) - 51) <= 2, "common material color lost");
    }
    require(!viewer.snapshot(prefix + "-invalid.png", -1), "invalid snapshot accepted");
    require(viewer.backend() == selected, "drawing error changed backend");
    auto overlay = std::make_shared<LinesDrawable2D>("common HUD");
    require(viewer.add_drawable(overlay) == overlay.get(), "common standalone attachment failed");
    overlay->update_vertex_buffer({vec2(10, 10), vec2(90, 10)}, fw, fh); overlay->set_line_width(3);
    require(viewer.run_frames(1) == 0 && viewer.delete_drawable(overlay.get()), "common drawable lifecycle failed");
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
    bool called = false;
    viewer.bind([&](ViewerBase* host, Model* model) {
        called = host == &viewer && model == mesh.get(); return true;
    }, mesh.get(), ViewerBase::KEY_J);
    auto key = glfwSetKeyCallback(viewer.window(), nullptr); glfwSetKeyCallback(viewer.window(), key);
    require(key != nullptr, "runtime viewer has no key callback");
    key(viewer.window(), GLFW_KEY_J, 0, GLFW_PRESS, 0);
    require(called, "portable key binding did not reach selected backend");
    require(!glfwGetWindowAttrib(viewer.window(), GLFW_VISIBLE), "hidden factory viewer became visible");
#endif
    require(viewer.current_model() == mesh.get() && viewer.models().size() == 1, "common model ownership changed");
    require(viewer.delete_model(mesh.get()), "common deletion failed"); viewer.clear_scene();
    require(viewer.models().empty() && viewer.drawables().empty(), "common clearing failed");
    require(viewer.validation_errors() == 0, "runtime viewer validation errors");
    viewer.request_close(); require(viewer.should_close() && !viewer.draw_frame(), "common close control failed");
}
void unavailable(const std::string& expected) {
    bool rejected = false;
    try { auto viewer = Factory::create("unavailable", options()); }
    catch (const BackendUnavailable& e) {
        const std::string reason = e.what();
        rejected = reason.find("Vulkan:") != std::string::npos && reason.find("OpenGL:") != std::string::npos && reason.find(expected) != std::string::npos;
    }
    require(rejected, "Automatic did not report both interactive backend failures");
    for (auto backend : {Backend::Vulkan, Backend::OpenGL}) {
        rejected = false;
        try { auto viewer = Factory::create("explicit unavailable", options(backend)); }
        catch (const BackendUnavailable&) { rejected = true; }
        require(rejected, "explicit unavailable backend did not fail");
    }
}
}
int main(int argc, char** argv) {
    try {
        initialize(false, false, false); invalid_options();
        const std::string mode = argc > 1 ? argv[1] : "";
        const std::string expected = argc > 2 ? argv[2] : "";
        if (mode == "--unavailable") unavailable(expected);
        else if (mode == "--fallback") {
            {
                auto viewer = Factory::create("fallback", options());
                require(viewer->backend() == Backend::OpenGL && !viewer->fallback_reason().empty() &&
                        viewer->fallback_reason().find(expected) != std::string::npos, "Automatic did not explain OpenGL fallback");
                exercise(*viewer, "runtime-fallback");
            }
            bool rejected = false;
            try { auto viewer = Factory::create("explicit", options(Backend::Vulkan)); }
            catch (const BackendUnavailable& e) { rejected = std::string(e.what()).find(expected) != std::string::npos; }
            require(rejected, "explicit Vulkan silently fell back");
            auto requested = options(); requested.validation = true;
            rejected = false;
            try { auto viewer = Factory::create("validation", requested); }
            catch (const BackendUnavailable& e) { rejected = std::string(e.what()).find("validation mode") != std::string::npos; }
            require(rejected, "fallback silently ignored validation request");
        } else {
#if defined(EASY3D_HAS_VULKAN_VIEWER) || !defined(EASY3D_NO_OPENGL)
            {
                auto requested = options();
#ifdef EASY3D_HAS_VULKAN_VIEWER
                requested.validation = requested.synchronization_validation = mode == "--validation";
#endif
                auto viewer = Factory::create("automatic", requested);
#ifdef EASY3D_HAS_VULKAN_VIEWER
                require(viewer->backend() == Backend::Vulkan && viewer->fallback_reason().empty(), "Automatic did not prefer Vulkan");
#else
                require(viewer->backend() == Backend::OpenGL && !viewer->fallback_reason().empty(), "OpenGL-only selection failed");
#endif
                exercise(*viewer, "runtime-automatic");
            }
            for (Backend backend : {Backend::Vulkan, Backend::OpenGL}) {
#if !defined(EASY3D_HAS_VULKAN_VIEWER)
                if (backend == Backend::Vulkan) continue;
#endif
#ifdef EASY3D_NO_OPENGL
                if (backend == Backend::OpenGL) continue;
#endif
                auto requested = options(backend); requested.samples = 1;
                auto viewer = Factory::create("explicit", requested);
                require(viewer->backend() == backend && viewer->fallback_reason().empty(), "explicit backend reported fallback");
                exercise(*viewer, "runtime-explicit-" + std::to_string(int(backend)));
            }
#else
            unavailable("window support was not built");
#endif
        }
        std::cout << "PASS: runtime Viewer factory, selection/fallback, scene API, samples, snapshot, animation and key binding\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
