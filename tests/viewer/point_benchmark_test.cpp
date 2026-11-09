// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
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
void measure(Viewer &v, const char *path) {
    require(v.add_model(resource::directory() + "/data/fountain/pointcloud.ply"), "fountain cloud missing");
    auto *points = v.current_model()->renderer()->get_points_drawable("vertices");
    require(points, "vertices drawable missing");
    points->set_point_size(5.0f);
    v.camera()->setUpVector(vec3(0, 1, 0));
    v.camera()->setViewDirection(vec3(0, 0, -1));
    v.camera()->showEntireScene();
#ifndef EASY3D_HAS_VULKAN
    // The OpenGL host enables vsync; uncapped presentation keeps the comparison render-bound.
    const auto previous = glfwGetCurrentContext();
    glfwMakeContextCurrent(v.window());
    glfwSwapInterval(0);
    glfwMakeContextCurrent(previous);
#endif
    for (int i = 0; i < 30; ++i)
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
              << ", camera " << v.camera()->screenWidth() << "x" << v.camera()->screenHeight()
              << ", samples 4, id-pass covered " << covered << " px\n";
    const int frames = 300;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < frames; ++i)
        require(v.draw_frame(), "timed frame did not present");
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << path << ": " << frames << " frames in " << seconds << " s, " << 1000.0 * seconds / frames
              << " ms/frame, " << frames / seconds << " FPS\n";
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