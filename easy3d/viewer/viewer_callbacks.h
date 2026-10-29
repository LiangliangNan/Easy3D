// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VIEWER_CALLBACKS_H
#define EASY3D_VIEWER_VIEWER_CALLBACKS_H
#include <functional>
#include <string>
#include <vector>
namespace easy3d {
class ViewerBase;
// UI-thread callbacks for either built-in backend. Mouse coordinates/deltas and
// resize dimensions use logical window units, with a top-left mouse origin.
// Returning true consumes an input event before legacy hooks/built-in controls.
// A consumed mouse press captures that gesture through its matching release.
struct ViewerCallbacks {
    std::function<void(ViewerBase*)> pre_draw, post_draw;
    std::function<void(ViewerBase*, int, int)> post_resize;
    std::function<bool(ViewerBase*, int, int, int, int)> mouse_press_event, mouse_release_event;
    std::function<bool(ViewerBase*, int, int, int, int, int, int)> mouse_drag_event;
    std::function<bool(ViewerBase*, int, int, int, int, int)> mouse_free_move_event;
    std::function<bool(ViewerBase*, int, int, int, int)> mouse_scroll_event;
    std::function<bool(ViewerBase*, int, int)> key_press_event, key_release_event;
    std::function<bool(ViewerBase*, unsigned int)> char_input_event;
    std::function<bool(ViewerBase*, const std::vector<std::string>&)> drop_event;
    std::function<bool(ViewerBase*, bool)> focus_event;
};
}
#endif
