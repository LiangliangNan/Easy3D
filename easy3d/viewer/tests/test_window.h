// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TESTS_TEST_WINDOW_H
#define EASY3D_TESTS_TEST_WINDOW_H

#include <easy3d/viewer/viewer.h>

#ifdef EASY3D_HAS_VULKAN   // Vulkan tests only; the shared tests build this header in both configurations
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <stdexcept>

namespace easy3d {
// Window and device settings of a test. The viewer constructors take them as separate parameters.
struct TestWindow {
    int width = 800, height = 600, samples = 1;
    bool visible = true, resizable = true;
    VulkanOptions device;
};

// Single-view tests need both the framebuffer callback and camera resize to settle
// before a capture. Logical window-size acknowledgement alone is asynchronous.
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
            v.camera()->screenWidth()==fw && v.camera()->screenHeight()==fh) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("test window/framebuffer/camera resize did not settle");
}

// Pixel comparisons control their camera explicitly. Keep the original visible
// tutorial and its synthetic hover/keys, but consume unrelated desktop navigation.
inline void block_navigation_input(Viewer& v) {
    ViewerCallbacks inputs;
    inputs.mouse_press_event=[](ViewerBase*,int,int,int,int) { return true; };
    inputs.mouse_drag_event=[](ViewerBase*,int,int,int,int,int,int) { return true; };
    inputs.mouse_scroll_event=[](ViewerBase*,int,int,int,int) { return true; };
    v.set_callbacks(std::move(inputs));
}
}
#endif   // EASY3D_HAS_VULKAN

#endif
