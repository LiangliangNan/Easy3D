// SPDX-License-Identifier: GPL-3.0-or-later
/********************************************************************
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
 ********************************************************************/
#ifndef EASY3D_VIEWER_VIEWER_CALLBACKS_H
#define EASY3D_VIEWER_VIEWER_CALLBACKS_H
#include <functional>
#include <string>
#include <vector>
namespace easy3d {
    class ViewerBase;

    /**
 * \brief Optional frame and input callbacks shared by all viewer backends.
 *
 * \details Callbacks execute on the UI/main thread. Mouse positions,
 * pointer-motion deltas, and resize dimensions use logical window units;
 * mouse positions have a top-left origin, x increases rightward, and y
 * increases downward. Scroll offsets use the window system's wheel units. Key,
 * modifier, and button values use ViewerInput's identifiers. Returning true
 * from an input callback consumes the event before Viewer-specific legacy
 * handlers and built-in controls run.
 *
 * A consumed mouse press captures that button's gesture through its matching
 * release: drag and release are treated as handled even if their callbacks
 * return false. Losing focus clears the captured gesture. Replacing the
 * callback set while one is executing does not destroy the active callable.
 *
 * \see ViewerInput, ViewerBase, Viewer
 */
    struct ViewerCallbacks {
        /// Called immediately before the viewer's legacy pre-draw hook and scene draw.
        std::function<void(ViewerBase *)> pre_draw;

        /// Called after the scene draw and the viewer's legacy post-draw hook.
        std::function<void(ViewerBase *)> post_draw;

        /// Called when logical or framebuffer dimensions change; dimensions are logical.
        std::function<void(ViewerBase *, int, int)> post_resize;

        /// Mouse press: viewer, logical x/y, ViewerInput::Button, and modifier mask.
        std::function<bool(ViewerBase *, int, int, int, int)> mouse_press_event;

        /// Mouse release: viewer, logical x/y, ViewerInput::Button, and modifier mask.
        std::function<bool(ViewerBase *, int, int, int, int)> mouse_release_event;

        /// Drag: viewer, logical x/y, logical dx/dy, button, and modifier mask.
        std::function<bool(ViewerBase *, int, int, int, int, int, int)> mouse_drag_event;

        /// Move without a pressed button: viewer, logical x/y, logical dx/dy, and modifier mask.
        std::function<bool(ViewerBase *, int, int, int, int, int)> mouse_free_move_event;

        /// Scroll: viewer, logical x/y, and horizontal/vertical scroll offsets.
        std::function<bool(ViewerBase *, int, int, int, int)> mouse_scroll_event;

        /// Key press: viewer, ViewerInput::Key identifier, and modifier mask.
        std::function<bool(ViewerBase *, int, int)> key_press_event;

        /// Key release: viewer, ViewerInput::Key identifier, and modifier mask.
        std::function<bool(ViewerBase *, int, int)> key_release_event;

        /// Text input: viewer and Unicode code point (native-endian UTF-32 value).
        std::function<bool(ViewerBase *, unsigned int)> char_input_event;

        /// File drop: viewer and the dropped file paths.
        std::function<bool(ViewerBase *, const std::vector<std::string> &)> drop_event;

        /// Focus change: viewer and true when focus is gained, false when lost.
        std::function<bool(ViewerBase *, bool)> focus_event;
    };
} // namespace easy3d
#endif
