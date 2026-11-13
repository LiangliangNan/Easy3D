/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */


#ifndef EASY3D_VIEWER_BACKENDS_VIEWER_LOOP_H
#define EASY3D_VIEWER_BACKENDS_VIEWER_LOOP_H
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
namespace easy3d {
    namespace detail {
        // Host::draw_frame owns animation, callbacks and rendering. Event delivery never
        // presents implicitly, so a positive budget counts exactly the completed frames.
        template <class Host, class Continuous>
        int run_viewer_frames(Host &host, unsigned max_frames, const Continuous &continuous) {
            unsigned frames = 0;
            // WaitEventsTimeout also dispatches callbacks. Always enter the next frame
            // boundary, even after close, so a retained callback error is not hidden.
            for (;;) {
                Host::poll_events();
                const bool drawn = host.draw_frame();
                if (drawn && max_frames && ++frames >= max_frames)
                    break;
                // Bounded runs keep their visibility/camera and advance without sleeps.
                // Idle interactive hosts wake for events or a bounded timer (HUD expiry).
                if (host.should_close())
                    break;
                if (!drawn || (!max_frames && !continuous()))
                    glfwWaitEventsTimeout(.05);
            }
            return 0;
        }
    } // namespace detail
} // namespace easy3d
#endif
