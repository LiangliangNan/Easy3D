// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TESTS_TEST_WINDOW_H
#define EASY3D_TESTS_TEST_WINDOW_H

#include <easy3d/viewer/viewer.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <stdexcept>

namespace easy3d {
// The vendored GLFW is a private dependency of easy3d_viewer, so a test that calls GLFW
// itself links its own copy of the static library. On Windows that copy was never
// initialized and every public GLFW function returns without touching the window
// (_GLFW_REQUIRE_INIT_OR_RETURN), so glfwSetKeyCallback(window, nullptr) hands back a null
// host callback, glfwMakeContextCurrent() keeps the library's context current, and the
// synthetic input driven from the callbacks faults. Initializing the local copy makes these
// calls work on the window the library created; the two modules register separate window
// classes, so this is safe and is the same workaround Tutorial_202_Viewer_imgui applies.
// Linux and macOS resolve GLFW to one instance.
inline void ensure_test_glfw() {
#if defined(_WIN32) && defined(_MSC_VER)
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed for the test executable's GLFW copy");
#endif
}
}

#ifdef EASY3D_HAS_VULKAN   // The helpers below pass Vulkan device options to the viewer
namespace easy3d {

// Window and device settings of a test. The viewer constructors take them as separate parameters.
struct TestWindow {
    TestWindow() { ensure_test_glfw(); }
    int width = 800, height = 600, samples = 1;
    bool visible = true, resizable = true;
    VulkanOptions device;
};

// Single-view tests need both the framebuffer callback and camera resize to settle
// before a capture. Logical window-size acknowledgement alone is asynchronous. The
// camera carries logical window dimensions on both backends (Batch 46); the framebuffer
// is the physical target, so the two are checked against their respective sizes.
inline void set_size(Viewer& v, const TestWindow& w) {
    int old_width, old_height, old_fw, old_fh;
    v.viewer_size(old_width, old_height); v.framebuffer_size(old_fw, old_fh);
    if (old_width<=0 || old_height<=0 || old_fw<=0 || old_fh<=0)
        throw std::runtime_error("initial test window has no framebuffer");
    const int expected_fw=int(std::lround(double(w.width)*old_fw/old_width));
    const int expected_fh=int(std::lround(double(w.height)*old_fh/old_height));
    v.resize(w.width, w.height);
    for (int i = 0; i < 300; ++i) {
        Viewer::poll_events();
        int width, height, fw, fh;
        v.viewer_size(width, height); v.framebuffer_size(fw, fh);
        if (width==w.width && height==w.height && std::abs(fw-expected_fw)<=1 && std::abs(fh-expected_fh)<=1 &&
            v.camera()->screen_width()==width && v.camera()->screen_height()==height) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("test window/framebuffer/camera resize did not settle");
}

// Pixel comparisons control their camera explicitly. Keep the original visible
// tutorial and its synthetic hover/keys, but ignore OS mouse-button and wheel
// events at the window level so unrelated desktop navigation cannot move the
// camera. Hover is unaffected, and tests still inject events through the saved
// host callbacks.
inline void block_navigation_input(Viewer& v) {
    if (GLFWwindow* window = v.window()) {
        glfwSetMouseButtonCallback(window, [](GLFWwindow*, int, int, int) {});
        glfwSetScrollCallback(window, [](GLFWwindow*, double, double) {});
    }
}
}
#endif   // EASY3D_HAS_VULKAN

#endif
