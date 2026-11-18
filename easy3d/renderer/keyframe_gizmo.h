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

#ifndef EASY3D_RENDERER_KEYFRAME_GIZMO_H
#define EASY3D_RENDERER_KEYFRAME_GIZMO_H

#include <vector>

#include <easy3d/core/types.h>

namespace easy3d {
    class Camera;
    class Frame;
    namespace graphics { struct DrawCommand; }
}

namespace easy3d {

/**
 * \class KeyFrameGizmo easy3d/renderer/keyframe_gizmo.h
 * \brief The manipulation handles of one keyframe pose: three axis arrows and three
 *      turn rings.
 * \details The handles are real world-space geometry centred on the pose, sized
 *      relative to the camera glyph that KeyFrameInterpolator draws for the pose, so
 *      the gizmo hugs the glyph it edits instead of dwarfing it. Their rendering is
 *      a depth-tested overlay: the parts behind the scene are hidden, as for a real
 *      object, and they keep a constant apparent size on HiDPI displays.
 *
 *      The class is stateless. Geometry(), handle_at(), axis_parameter_at() and
 *      ring_angle_at() map a pose and the cursor to handles and cursor parameters;
 *      append_overlay() draws the gizmo for a caller-owned keyframe. An interactive
 *      editor built on these is KeyFrameEditor.
 *
 *      The arrows move the pose along its own axes; the rings turn it around them.
 *      An element seen end-on carries no usable cursor parameter (an arrow along the
 *      view is a dot, a ring's plane is a line), so it is neither drawn nor grabbable.
 */
class KeyFrameGizmo {
public:
    /// \brief The part of the gizmo under the cursor.
    enum class Handle {
        none,       ///< Nothing.
        free,       ///< The camera glyph itself, not a drawn handle; never returned by handle_at().
        move_x, move_y, move_z, ///< The three axis arrows.
        turn_x, turn_y, turn_z  ///< The three turn rings.
    };

    /// \brief The pose's world axes and the sizes of the handles drawn along them.
    struct Geometry {
        vec3 axes[3];              ///< The pose's own x, y and z axes, normalized.
        float arrow_length = 0.0f; ///< Length of an arrow, from the pose to its tip.
        float ring_radius = 0.0f;  ///< Radius of a ring around the pose.
    };

    /// \brief The pose's own axes and the handle sizes for a glyph of \p glyph_width.
    /// \details The arrows reach a little past the glyph and the rings wrap it closely.
    /// \param frame The pose whose orientation orients the axes.
    /// \param glyph_width The world size of the camera glyph the handles are sized to.
    /// \return The pose's axes with the arrow and ring sizes for \p glyph_width.
    static Geometry geometry(const Frame &frame, float glyph_width);

    /// \brief Whether \p handle is one of the turn rings.
    /// \param handle The handle to classify.
    /// \return \c true for the three turn rings, \c false for the arrows and for
    ///      none/free.
    static bool is_turn(Handle handle);

    /// \brief The axis of \p handle (0: x, 1: y, 2: z), or -1 for none and free.
    /// \param handle The handle to query.
    /// \return The axis index (0 for x, 1 for y, 2 for z), or -1 for none and free.
    static int axis_of(Handle handle);

    /// \brief The handle nearest to the cursor (x, y) within a few logical pixels, or
    ///      none when nothing of the gizmo can be grabbed there.
    /// \details The arrows win over the rings where they overlap (the shaft passes
    ///      close to the ring at the ring's radius), and end-on elements are skipped.
    ///      The result is none when the pose has no stable screen position, e.g. when
    ///      the camera stands on it.
    /// \param camera The camera that views the pose.
    /// \param frame The pose whose handles are tested.
    /// \param glyph_width The world size of the camera glyph the handles are sized to.
    /// \param x The cursor's x coordinate, in logical window pixels (top-left origin).
    /// \param y The cursor's y coordinate, in logical window pixels (top-left origin).
    /// \return The grabbed handle, or Handle::none.
    static Handle handle_at(const Camera *camera, const Frame &frame, float glyph_width, float x, float y);

    /// \brief The parameter on the line (origin + s * axis) nearest to the cursor ray.
    /// \details Used to drag along an arrow: the pose follows the cursor on the axis.
    ///      False when the axis is seen end-on.
    /// \param camera The camera that views the scene.
    /// \param x The cursor's x coordinate, in logical window pixels (top-left origin).
    /// \param y The cursor's y coordinate, in logical window pixels (top-left origin).
    /// \param origin A point on the line.
    /// \param axis The line's unit direction.
    /// \param parameter Receives the line parameter \p s of the point nearest to the
    ///      cursor ray; left untouched when the call returns \c false.
    /// \return \c true when the axis is not seen end-on and \p parameter is set.
    static bool axis_parameter_at(const Camera *camera, float x, float y, const vec3 &origin, const vec3 &axis,
                                  float &parameter);

    /// \brief The cursor's angle around \p axis in the plane through \p center.
    /// \details Used to turn around a ring: the accumulated angle of consecutive
    ///      calls is the turn applied to the pose. False when the plane is seen
    ///      edge-on or behind the eye.
    /// \param camera The camera that views the scene.
    /// \param x The cursor's x coordinate, in logical window pixels (top-left origin).
    /// \param y The cursor's y coordinate, in logical window pixels (top-left origin).
    /// \param center The plane's point the angle is measured around.
    /// \param axis The plane's normal, i.e. the turn axis.
    /// \param angle Receives the angle in radians, in the plane's basis; left untouched
    ///      when the call returns \c false.
    /// \return \c true when the plane is not seen edge-on and is in front of the eye.
    static bool ring_angle_at(const Camera *camera, float x, float y, const vec3 &center, const vec3 &axis,
                              float &angle);

    /// \brief Projects a world point to logical window coordinates (top-left origin).
    /// \details False when the point is behind the eye or outside the depth range. A
    ///      perspective projection divides by the view depth, which is zero exactly at
    ///      the eye (e.g. a camera standing on a keyframe station), so that depth is
    ///      rejected before the division; nothing nearer than the near plane is
    ///      visible anyway.
    /// \param camera The camera that views the point.
    /// \param point The world point to project.
    /// \param screen Receives the point's logical window coordinates; left untouched
    ///      when the call returns \c false.
    /// \return \c true when the point has a stable projection in front of the eye.
    static bool project_to_screen(const Camera *camera, const vec3 &point, vec2 &screen);

    /// \brief World length of one logical pixel at \p point's depth, measured through
    ///      \p camera; zero when the point has no stable projection.
    /// \details Lets an annotation keep its on-screen size while being real world-space
    ///      geometry, so the scene depth can occlude it.
    /// \param camera The camera that views the point.
    /// \param point The world point whose depth is measured.
    /// \return The world-space length of one logical pixel, or 0 when the point has no
    ///      stable projection.
    static float pixel_size_at(const Camera *camera, const vec3 &point);

    /// \brief Appends the gizmo of \p frame to \p commands as depth-tested overlays.
    /// \param commands The command list the gizmo is appended to.
    /// \param camera The camera that views the pose.
    /// \param frame The pose whose axes orient the handles.
    /// \param glyph_width World size of the camera glyph the gizmo is sized to.
    /// \param active The grabbed or hovered handle, drawn full and thick while the
    ///      others dim so the handle a gesture will act on stays unmistakable; none
    ///      draws every handle the same.
    /// \param fb_width The framebuffer width in pixels.
    /// \param fb_height The framebuffer height in pixels.
    /// \param dpi The DPI scaling, so the handle widths keep their apparent size.
    static void append_overlay(std::vector<graphics::DrawCommand> &commands, const Camera *camera,
                               const Frame &frame, float glyph_width, Handle active,
                               unsigned fb_width, unsigned fb_height, float dpi);
};

} // namespace easy3d

#endif // EASY3D_RENDERER_KEYFRAME_GIZMO_H
