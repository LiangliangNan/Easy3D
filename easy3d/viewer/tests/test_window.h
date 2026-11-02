// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TESTS_TEST_WINDOW_H
#define EASY3D_TESTS_TEST_WINDOW_H

#include <easy3d/viewer/viewer.h>

#ifdef EASY3D_HAS_VULKAN   // Vulkan tests only; the shared tests build this header in both configurations
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

namespace easy3d {
// Window and device settings of a test. The viewer constructors take them as separate parameters.
struct TestWindow {
    unsigned int width = 800, height = 600, samples = 1;
    bool visible = true, resizable = true;
    VulkanOptions device;
};

// Resizes a viewer and waits until the window system has applied it (the request is asynchronous).
inline void set_size(VulkanViewer& v, const TestWindow& w) {
    v.resize(static_cast<int>(w.width), int(w.height));
    for (int i = 0; i < 300; ++i) {
        VulkanViewer::poll_events();
        int width = 0, height = 0;
        v.viewer_size(width, height);
        if (width == int(w.width) && height == int(w.height)) return;
        glfwWaitEventsTimeout(.01);
    }
}
}
#endif   // EASY3D_HAS_VULKAN

#endif
