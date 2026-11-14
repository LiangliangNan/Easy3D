// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <tests/viewer/test_window.h>   // ensure_test_glfw()
#if !defined(EASY3D_HAS_VULKAN) && defined(_WIN32)
#include <easy3d/renderer/opengl.h>     // wglGetProcAddress
#endif
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

using namespace easy3d;

namespace {
void require(bool ok, const char *text) {
    if (!ok)
        throw std::runtime_error(text);
}

// Tutorial_205_RealCamera's point workload: the fountain reconstruction (479187 points),
// 5 px points, neutral overview camera, 1600x917 logical window, 4x MSAA, hidden window.
// The tutorial's camera frusta and photo overlay are excluded; the cloud dominates the
// frame and is identical on every path. No performance threshold is asserted because
// throughput is machine-dependent; the numbers are recorded in docs/vulkan.md.
// The test is time-budget-aware: every loop runs a minimum number of frames and then keeps
// rendering until a per-path wall-clock budget is exhausted, so software rasterizers
// (lavapipe) finish well inside the ctest TIMEOUT even when a frame costs a second or more.
void measure(Viewer &v, const char *path) {
    require(v.add_model(resource::directory() + "/data/fountain/pointcloud.ply"), "fountain cloud missing");
    auto *points = v.current_model()->renderer()->get_points_drawable("vertices");
    require(points, "vertices drawable missing");
    points->set_point_size(5.0f);
    v.camera()->set_up_vector(vec3(0, 1, 0));
    v.camera()->set_view_direction(vec3(0, 0, -1));
    v.camera()->show_entire_scene();
#ifndef EASY3D_HAS_VULKAN
    // The OpenGL host enables vsync; uncapped presentation keeps the comparison render-bound
    // instead of display-bound. These calls only reach the window once this executable's own
    // copy of the static GLFW is initialized, and the window is left current, because this
    // module's record of what is current says nothing about the library's.
    ensure_test_glfw();
    glfwMakeContextCurrent(v.window());
#ifdef _WIN32
    // Not glfwSwapInterval: it looks the current context up in the requesting module's own
    // slot, which only the module that created the context fills, so here on Windows it only
    // reports GLFW_NO_CURRENT_CONTEXT. The context is current at the driver level, so the
    // extension entry point is applied to it directly.
    if (auto* swap_interval = reinterpret_cast<BOOL(WINAPI*)(int)>(wglGetProcAddress("wglSwapIntervalEXT")))
        swap_interval(0);
#else
    glfwSwapInterval(0);
#endif
#endif
    // Warm-up only needs to reach a steady state; keeping it small bounds the run on a
    // software rasterizer, where every frame is expensive.
    const int warmup_frames = 3;
    for (int i = 0; i < warmup_frames; ++i)
        require(v.draw_frame(), "warmup frame did not present");
    // Backend-independent DPI contract: logical window size, physical framebuffer, and their ratio.
    require(v.width() == 1600 && v.height() == 917, "logical window size differs from the requested size");
    require(v.framebuffer_width() >= v.width() && v.framebuffer_height() >= v.height(),
            "framebuffer is smaller than the logical window");
    require(std::abs(v.dpi_scaling() - double(v.framebuffer_width()) / v.width()) < 1e-3,
            "dpi scaling does not match the framebuffer/logical ratio");
    const auto frame = v.capture();
    std::size_t covered = 0;
    for (const auto &id : frame.ids)
        if (id.object != UINT32_MAX)
            ++covered;
    require(covered > 10000, "benchmark scene did not render");
    std::cout << path << ": logical " << v.width() << "x" << v.height() << ", framebuffer "
              << v.framebuffer_width() << "x" << v.framebuffer_height() << ", dpi " << v.dpi_scaling()
              << ", camera " << v.camera()->screen_width() << "x" << v.camera()->screen_height()
              << ", samples 4, id-pass covered " << covered << " px\n";
    // Time-budget-aware loop: run at least three frames, then keep going until ten seconds
    // elapse. Fast hardware still renders thousands of frames; a software rasterizer stops
    // early, and its expensive frames are not multiplied by a large minimum count.
    const int min_frames = 3;
    const double budget_seconds = 10.0;
    const auto start = std::chrono::steady_clock::now();
    int frames = 0;
    double elapsed = 0.0;
    do {
        require(v.draw_frame(), "timed frame did not present");
        ++frames;
        elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    } while (frames < min_frames || elapsed < budget_seconds);
    std::cout << path << ": " << frames << " frames in " << elapsed << " s, " << 1000.0 * elapsed / frames
              << " ms/frame, " << frames / elapsed << " FPS";
    if (elapsed >= budget_seconds)
        std::cout << " (budget-limited)";
    std::cout << "\n";
}
} // namespace

int main() {
    try {
        initialize(false, false, false);
#ifdef EASY3D_HAS_VULKAN
        {
            VulkanOptions options;
            options.use_native_large_points = true;
            Viewer v("Point benchmark", 1600, 917, 4, false, false, true, options);
            measure(v, "vulkan-native");
        }
        {
            VulkanOptions options;
            options.use_native_large_points = false;
            Viewer v("Point benchmark", 1600, 917, 4, false, false, true, options);
            measure(v, "vulkan-expanded");
        }
#else
        {
            Viewer v("Point benchmark", 1600, 917, 4, false, false, true);
            measure(v, "opengl");
        }
#endif
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}