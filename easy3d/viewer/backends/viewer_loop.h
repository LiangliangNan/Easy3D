// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_BACKENDS_VIEWER_LOOP_H
#define EASY3D_VIEWER_BACKENDS_VIEWER_LOOP_H
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
namespace easy3d { namespace detail {
// Host::draw_frame owns animation, callbacks and rendering. Event delivery never
// presents implicitly, so a positive budget counts exactly the completed frames.
template<class Host, class Continuous>
int run_viewer_frames(Host& host, unsigned max_frames, const Continuous& continuous) {
    unsigned frames = 0;
    // WaitEventsTimeout also dispatches callbacks. Always enter the next frame
    // boundary, even after close, so a retained callback error is not hidden.
    for (;;) {
        Host::poll_events();
        const bool drawn = host.draw_frame();
        if (drawn && max_frames && ++frames >= max_frames) break;
        // Bounded runs keep their visibility/camera and advance without sleeps.
        // Idle interactive hosts wake for events or a bounded timer (HUD expiry).
        if (host.should_close()) break;
        if (!drawn || (!max_frames && !continuous())) glfwWaitEventsTimeout(.05);
    }
    return 0;
}
} }
#endif
