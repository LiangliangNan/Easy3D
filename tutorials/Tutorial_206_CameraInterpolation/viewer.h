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

#ifndef EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
#define EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/types.h>

#include <memory>
#include <vector>

namespace easy3d {
    class LinesDrawable;
}

// This tutorial shows how to interpolate camera frames to
// animate model exploration.

/**
 * \brief A Viewer that records camera keyframes, edits them interactively, and plays them back as a smooth animation.
 *
 * The base Viewer already owns a KeyFrameInterpolator bound to the camera and can draw/play a camera path;
 * this subclass remaps a few friendly keys ('k', 'space', 'd') onto that machinery and adds a direct
 * manipulation gizmo: clicking one of the drawn keyframe cameras selects it. Dragging the camera itself
 * slides it in the plane of the screen (the pose follows the cursor); three arrows (translate along one
 * world axis) and three rings (rotate around one world axis) offer constrained edits. Every drag edits
 * the keyframe with set_keyframe_position()/set_keyframe_orientation(), and the Viewer redraws the
 * interpolated path in the same frame.
 *
 * All drawing, hit testing and drag math live here; the KeyFrameInterpolator and the Viewer need no changes.
 */
class TutorialCameraInterpolation : public easy3d::Viewer {
public:
    explicit TutorialCameraInterpolation(const std::string &title);
    ~TutorialCameraInterpolation() override = default;

protected:
    // Maps 'k' (add keyframe), 'space' (play/stop), 'd' (delete path) to the interpolator, and Esc to
    // end editing.
    bool key_press_event(int key, int modifiers) override;

    // Selects a keyframe camera, grabs a gizmo handle, and drives its drag.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
    bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) override;

    // Keeps the gizmo sized to the (world-scaled) keyframe camera glyph while the view camera moves.
    void pre_draw() override;

    // Drops a drag that was interrupted (e.g., the window lost focus).
    void end_gesture() override;

private:
    /// The gizmo parts the mouse can grab. Translation and rotation share the axis colors.
    enum class Handle {
        NONE,
        MOVE_FREE,  // the keyframe camera itself: drag in the plane of the screen
        MOVE_X, MOVE_Y, MOVE_Z,
        ROTATE_X, ROTATE_Y, ROTATE_Z
    };

    /// One line segment of the gizmo, tagged with the handle it belongs to.
    struct Segment {
        easy3d::vec3 a;
        easy3d::vec3 b;
        Handle handle;
        bool ring;
    };

    /// Returns the index of the drawn keyframe camera under the cursor, or -1. The glyphs are drawn by
    /// the viewer at scene_radius() * 0.05, oriented by the keyframe, so the hit test projects the same
    /// wireframe and tests the cursor against its screen bounding box.
    int pick_keyframe(int x, int y) const;

    /// Returns the gizmo handle under the cursor (closest segment within a screen-space tolerance).
    Handle pick_handle(int x, int y) const;

    /// Projects a world point to logical screen coordinates. Returns false when the point is behind the eye.
    bool project(const easy3d::vec3 &world, easy3d::vec2 &screen) const;

    /// Radius in logical pixels of the keyframe camera glyph drawn by the viewer, measured from its apex.
    float glyph_screen_radius(int index) const;

    /// Radius in logical pixels of the rotation rings, which enclose that glyph with a small margin.
    float ring_screen_radius() const;

    /// Builds the gizmo line segments at the selected keyframe: an arrow and a ring per world axis.
    std::vector<Segment> gizmo_segments() const;

    /// World-space arrow length. The rings hug the drawn keyframe camera, so the whole gizmo has the
    /// same apparent size as that glyph (with a floor/ceiling for usability at extreme zoom).
    float gizmo_length() const;

    /// Recreates/updates the gizmo drawable; the handle under the cursor or being dragged is highlighted.
    void rebuild_gizmo();

    void select_keyframe(int index);
    void clear_selection();

    /// Moves the selected keyframe, growing the scene sphere so the path is not clipped.
    void move_keyframe(const easy3d::vec3 &position);

    static int axis_index(Handle handle);
    static bool is_translation(Handle handle);
    static easy3d::vec3 axis_direction(int axis);

    /// Captures the keyframe pose and the mouse reference used by the following drag events.
    void begin_drag(Handle handle, int x, int y);

    /// Hit point of the mouse ray on the plane through \p center orthogonal to \p normal.
    bool screen_plane_hit(const easy3d::vec3 &center, const easy3d::vec3 &normal,
                          int x, int y, easy3d::vec3 &hit) const;

    /// Parameter of the point on the line (origin + t * axis) closest to the mouse ray.
    bool axis_parameter(const easy3d::vec3 &origin, const easy3d::vec3 &axis,
                        int x, int y, float &t) const;

    /// Angle of the mouse ray's hit point on the rotation ring around \p axis.
    bool ring_angle(const easy3d::vec3 &center, int axis, int x, int y, float &angle) const;

    int selected_ = -1;              // index of the edited keyframe, -1 when nothing is selected
    Handle hovered_ = Handle::NONE;  // gizmo part under the cursor
    Handle dragged_ = Handle::NONE;  // gizmo part being dragged (NONE = not dragging)

    // Pose of the selected keyframe when the drag started. Every drag event recomputes the pose from
    // these values, so float error does not accumulate.
    easy3d::vec3 drag_start_position_;
    easy3d::quat drag_start_orientation_;
    float drag_axis_parameter_ = 0.0f;  // position along the dragged axis at the press
    float drag_last_angle_ = 0.0f;      // ring angle at the previous drag event
    float drag_total_angle_ = 0.0f;     // accumulated rotation since the press
    easy3d::vec3 drag_plane_normal_;    // screen plane of a free translation, fixed at the press
    easy3d::vec3 drag_start_hit_;       // keyframe position's companion point on that plane

    float gizmo_length_ = 0.0f;  // cached arrow length, 0 means "not built"
    // Shared with the viewer: the viewer may clear its drawables (and then call end_gesture()) on
    // teardown, so the tutorial keeps its own reference instead of a raw observer.
    std::shared_ptr<easy3d::LinesDrawable> gizmo_;
};

#endif // EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
