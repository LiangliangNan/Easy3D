// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#ifdef EASY3D_TEST_OPENGL_VIEWER
#include <easy3d/renderer/opengl.h>
using TestHost = easy3d::Viewer;
#else
using TestHost = easy3d::VulkanViewer;
#endif
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
#ifndef EASY3D_TEST_OPENGL_VIEWER
VulkanViewerOptions options(bool validation) {
    VulkanViewerOptions result; result.width = 151; result.height = 117; result.visible = false;
    result.device.validation = result.device.synchronization_validation = validation; return result;
}
#endif
struct LoopViewer : TestHost {
#ifdef EASY3D_TEST_OPENGL_VIEWER
    explicit LoopViewer(bool) : TestHost("Frame loop", 4, 3, 2, false, true, 24, 8, 151, 117) { manual_.clear(); }
#else
    explicit LoopViewer(bool validation) : TestHost("Frame loop", options(validation)) { manual_.clear(); }
#endif
    unsigned initializations = 0;
    bool throw_init = false;
    void init() override {
        if (throw_init) throw std::runtime_error("expected initialization failure");
        ++initializations; TestHost::init();
    }
    unsigned before = 0, after = 0, ticks = 0, observed_ticks = 0, close_at = 0;
    bool throw_frame = false, throw_key = false, inject_key_after_frame = false;
    void pre_draw() override {
        TestHost::pre_draw();
        if (throw_frame) throw std::runtime_error("expected frame failure");
        ++before; observed_ticks = ticks;
    }
    void post_draw() override {
        TestHost::post_draw();
        ++after;
        if (inject_key_after_frame) {
            inject_key_after_frame = false;
            auto key = glfwSetKeyCallback(window(), nullptr); glfwSetKeyCallback(window(), key);
            key(window(), GLFW_KEY_F9, 0, GLFW_PRESS, 0);
        }
        if (close_at && after >= close_at) request_close();
    }
    bool key_press_event(int key, int modifiers) override {
        if (throw_key) throw std::runtime_error("expected input failure");
        return TestHost::key_press_event(key, modifiers);
    }
};
void one_frame(LoopViewer& viewer) {
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        TestHost::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("no frame completed within bounded retries");
}
void scene(LoopViewer& viewer) {
    auto mesh = std::make_shared<SurfaceMesh>();
    const auto a = mesh->add_vertex(vec3(-1, -1, 0));
    const auto b = mesh->add_vertex(vec3(1, -1, 0));
    const auto c = mesh->add_vertex(vec3(0, 1, 0)); mesh->add_triangle(a, b, c);
    viewer.add_model(mesh);
    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_uniform_coloring(vec4(.8f, .2f, .1f, 1)); faces->set_lighting(false);
    viewer.camera()->setViewDirection(vec3(0, 0, -1)); viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.fit_screen();
}
void same_camera(const mat4& a, const mat4& b) {
    for (unsigned i = 0; i < 16; ++i) require(std::abs(a[i] - b[i]) < 1e-6f, "bounded execution changed the camera");
}
void check_file(LoopViewer& viewer) {
    const auto ticks = viewer.ticks, frames = viewer.after;
    require(viewer.snapshot("viewer-loop.png", 1, 4, 0), "common snapshot failed");
    int w, h, channels; std::vector<unsigned char> rgba;
    require(ImageIO::load("viewer-loop.png", rgba, w, h, channels, 4, false), "snapshot unreadable");
    require(w == viewer.framebuffer_width() && h == viewer.framebuffer_height(), "framebuffer sizing API diverged");
    const auto center = (size_t(h / 2) * w + w / 2) * 4;
    require(std::abs(int(rgba[center]) - 204) <= 2 && std::abs(int(rgba[center+1]) - 51) <= 2,
            "loop did not render the model's material");
    require(ticks == viewer.ticks && frames == viewer.after, "snapshot advanced interactive animation/hooks");
}
void exercise(bool validation) {
    LoopViewer viewer(validation); scene(viewer);
    viewer.throw_init = true;
    bool init_failed = false;
    try { viewer.snapshot("viewer-init-failed.png", 1, 4, 0); }
    catch (const std::runtime_error&) { init_failed = true; }
    require(init_failed, "initialization failure was swallowed");
    require(viewer.initializations == 0 && viewer.after == 0, "failed initialization completed a frame");
    viewer.throw_init = false;
    check_file(viewer);
    require(viewer.initializations == 1 && viewer.before == 0 && viewer.after == 0,
            "capture did not initialize exactly once without advancing frame hooks");
    const auto camera = viewer.camera()->modelViewProjectionMatrix();
    const int visibility = glfwGetWindowAttrib(viewer.window(), GLFW_VISIBLE);
    require(viewer.run_frames(2) == 0 && viewer.before >= 2 && viewer.after == 2, "bounded frame budget was not exact");
    require(glfwGetWindowAttrib(viewer.window(), GLFW_VISIBLE) == visibility, "bounded run showed/hidden the window");
    same_camera(camera, viewer.camera()->modelViewProjectionMatrix());
    const auto prepared = viewer.before;
    auto refresh = glfwSetWindowRefreshCallback(viewer.window(), nullptr);
    glfwSetWindowRefreshCallback(viewer.window(), refresh);
    require(refresh != nullptr, "refresh callback missing");
    refresh(viewer.window()); refresh(viewer.window()); TestHost::poll_events();
    require(viewer.before == prepared && viewer.after == 2, "event delivery rendered an unbudgeted frame");
    one_frame(viewer);
    require(viewer.after == 3, "single-frame execution failed");
    viewer.set_animation(true);
    require(!viewer.is_animating(), "missing animation callback reported as active");
    viewer.animation_func_ = [&](TestHost*) { ++viewer.ticks; return false; };
    const auto start = viewer.after, attempts = viewer.before;
    require(viewer.run_frames(3) == 0 && viewer.after == start + 3 && viewer.ticks == viewer.before - attempts &&
            viewer.observed_ticks == viewer.ticks,
            "animation order/count or advisory return behavior differs");
    check_file(viewer);
    const auto animated_ticks = viewer.ticks;
    viewer.animation_func_ = [&](TestHost* host) { ++viewer.ticks; host->set_animation(false); return true; };
    require(viewer.run_frames(2) == 0 && viewer.ticks == animated_ticks + 1 && !viewer.is_animating(), "animation could not pause itself");
    viewer.set_animation(true);
    viewer.animation_func_ = [&](TestHost* host) { ++viewer.ticks; host->request_close(); return true; };
    const auto closed_after = viewer.after;
    require(!viewer.draw_frame() && viewer.should_close() && viewer.after == closed_after && viewer.ticks == animated_ticks + 2,
            "close during animation still drew a frame");
    require(viewer.run_frames(1) == 0 && viewer.after == closed_after, "closed window consumed a frame");
    glfwSetWindowShouldClose(viewer.window(), GLFW_FALSE); viewer.set_animation(false);
    viewer.close_at = viewer.after + 2;
    const auto preserved = viewer.camera()->modelViewProjectionMatrix();
    require(viewer.run(false) == 0 && viewer.after == viewer.close_at, "run(false) did not run until close");
    same_camera(preserved, viewer.camera()->modelViewProjectionMatrix());
    glfwSetWindowShouldClose(viewer.window(), GLFW_FALSE);
    viewer.camera()->setPosition(viewer.camera()->position() + vec3(2, 3, 4));
    const auto displaced = viewer.camera()->position();
    viewer.close_at = viewer.after + 2;
    require(viewer.run(true) == 0 && viewer.after == viewer.close_at && viewer.camera()->position() != displaced,
            "run(true) did not fit the scene and run until close");
    glfwSetWindowShouldClose(viewer.window(), GLFW_FALSE); viewer.close_at = 0;
    viewer.hide(); viewer.resize(163, 123); viewer.set_position(30, 40);
    require(viewer.run_frames(1) == 0, "bounded run failed after resize/hide");
    int w = 0, h = 0, logical_w = 0, logical_h = 0;
    viewer.framebuffer_size(w, h); viewer.viewer_size(logical_w, logical_h);
    int actual_w = 0, actual_h = 0; glfwGetWindowSize(viewer.window(), &actual_w, &actual_h);
    require(w == viewer.framebuffer_width() && h == viewer.framebuffer_height() && w > 0 && h > 0 &&
            logical_w == actual_w && logical_h == actual_h, "window/framebuffer getters disagree");
    const auto window = viewer.window(); const auto presented = viewer.after;
    viewer.throw_frame = true;
    bool failed = false;
    try { viewer.draw_frame(); } catch (const std::runtime_error&) { failed = true; }
    require(failed && viewer.after == presented, "direct rendering failure was swallowed or posted a frame");
    require(viewer.run_frames(1) != 0 && viewer.window() == window, "loop failure destroyed/replaced its window");
    viewer.throw_frame = false;
    require(viewer.run_frames(1) == 0 && viewer.after == presented + 1, "rendering failure left the host unusable");
    require(viewer.initializations == 1, "bounded/repeated runs reinitialized the host");
#ifdef EASY3D_TEST_OPENGL_VIEWER
    require(glGetError() == GL_NO_ERROR, "OpenGL frame execution error");
#else
    require(viewer.device().validation_errors() == 0, "Vulkan frame execution validation error");
#endif
    viewer.exit(); require(viewer.should_close() && !viewer.draw_frame(), "exit did not close the host");
}
void callback_failure(bool validation) {
    // Cover pending input errors, input delivered by animation, and input from
    // post_draw on the final budgeted frame. Close must not conceal any of them.
    for (unsigned route = 0; route < 3; ++route) {
        LoopViewer viewer(validation); scene(viewer); viewer.throw_key = true;
        auto key = glfwSetKeyCallback(viewer.window(), nullptr); glfwSetKeyCallback(viewer.window(), key);
        require(key != nullptr, "keyboard callback missing");
        if (route == 0) key(viewer.window(), GLFW_KEY_F9, 0, GLFW_PRESS, 0);
        else if (route == 1) {
            viewer.set_animation(true);
            viewer.animation_func_ = [&](TestHost*) {
                key(viewer.window(), GLFW_KEY_F9, 0, GLFW_PRESS, 0); return true;
            };
        } else viewer.inject_key_after_frame = true;
        const auto window = viewer.window();
        require(viewer.run_frames(1) != 0 && viewer.window() == window,
                "callback failure was hidden by close/frame budget or switched hosts");
        bool failed = false;
        try { viewer.draw_frame(); } catch (const std::runtime_error& e) {
            failed = std::string(e.what()) == "expected input failure";
        }
        require(failed && viewer.should_close(), "callback failure was not retained at the frame boundary");
    }
}
}
int main(int argc, char**) {
    try {
        initialize(false, false, false);
        exercise(argc > 1); callback_failure(argc > 1);
        std::cout << "PASS: common frame loop, exact budgets, refresh, animation, run(bool), camera, resize, failure containment\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
