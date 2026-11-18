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

#ifndef EASY3D_RENDERER_KEYFRAME_EDITOR_H
#define EASY3D_RENDERER_KEYFRAME_EDITOR_H

#include <functional>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/keyframe_gizmo.h>

namespace easy3d {
    class Camera;
    class Frame;
    class KeyFrameInterpolator;
    namespace graphics { struct DrawCommand; }
}

namespace easy3d {

/**
 * \class KeyFrameEditor easy3d/renderer/keyframe_editor.h
 * \brief Interactive editing of the keyframe poses stored in a KeyFrameInterpolator.
 * \details The editor owns the selection of a keyframe and the gestures that edit it:
 *
 *      - dragging the selected pose's camera glyph moves it in the screen plane
 *        through its position (or in the plane of its own up, with the upright flag),
 *        an axis arrow of the KeyFrameGizmo moves it along that axis, and a ring
 *        turns its pose around that axis; Esc restores the pose captured at the press,
 *      - aim_at() turns the selected pose so its view direction points at a target,
 *        keeping its roll,
 *      - insert_after_selected() and delete_selected() edit the path itself (the
 *        interpolator only appends, so the editor rebuilds it), and
 *      - append_overlay() draws the selected pose's station square, its aim ray and
 *        the handle gizmo as depth-tested overlays.
 *
 *      The class borrows a camera and an interpolator. It performs no picking: the
 *      press() call takes a callback that returns the keyframe whose glyph is under
 *      the cursor, so a host supplies its own picking (screen-space handle hits are
 *      tested by the editor itself, before the callback, so the handles take
 *      precedence where they overlap the glyphs).
 *
 *      The class draws nothing into the scene: the overlays it appends are returned
 *      to the caller, which submits them with its other overlay commands. Notifications
 *      are the host's: begin_edit is called once when an edit actually starts, so the
 *      host can take its undo snapshot; the host redraws after forwarding input.
 *
 *      An edit is only possible while editing() is on, a keyframe is selected and the
 *      interpolator is not playing; the host decides when editing is on (e.g. only
 *      while the path is visible).
 */
class KeyFrameEditor {
public:
    using Handle = KeyFrameGizmo::Handle;

    /// \brief What release() ended: the keyframe the drag acted on, and whether it
    ///      moved, turned, or did neither (a click that grabbed without moving).
    struct DragResult {
        int keyframe = -1; ///< The keyframe the ended drag acted on, or -1.
        bool moved = false; ///< The pose changed during the drag.
        bool turned = false; ///< The drag was a ring turn (false for arrows and glyph drags).
    };

    /// \brief Borrows \p camera and \p interpolator; neither is owned.
    /// \param camera The viewer's camera; it must remain valid for the editor's
    ///      lifetime. It views the poses and supplies the projection of the handles.
    /// \param interpolator The path that is edited; it must remain valid for the
    ///      editor's lifetime.
    KeyFrameEditor(Camera *camera, KeyFrameInterpolator *interpolator);
    ~KeyFrameEditor() = default;

    /// \brief Called once when an edit starts, before the first change is applied: the
    ///      first movement of a drag, an insert, a delete or an aim. Hook the host's
    ///      undo snapshot here.
    std::function<void()> begin_edit;

    /// \brief Sets the world size of the camera glyphs the handles are sized to.
    /// \details The host knows the glyph size (it draws them); the handles hug them.
    ///      A non-positive width draws no handles.
    void set_glyph_width(float width);
    float glyph_width() const { return glyph_width_; }

    /// \brief Turns the handles on or off. Only the host knows whether the path is
    ///      visible and whether the user asked to edit it; both must hold.
    void set_editing(bool editing);
    bool editing() const { return editing_; }

    /// \brief The keyframe that Prev/Next and the insert/delete operations act on, or
    ///      -1. Kept valid across path edits, so a selection of the last keyframe stays
    ///      on the last after a delete.
    int selected() const;

    /// \brief Selects \p index, clamped to the existing keyframes; a negative \p index
    ///      clears the selection.
    void set_selected(int index);

    /// \brief The selected keyframe whose handles are shown and edited, or -1 when
    ///      editing is off, nothing is selected or the interpolator is playing.
    int editable_keyframe() const;

    /// \brief The handle under the cursor, or none; see KeyFrameGizmo::handle_at().
    Handle handle_at(int x, int y) const;

    /// \brief Highlights the handle under the cursor; the overlay draws it prominent.
    /// \details Negative coordinates mean the cursor is not over the scene and clear
    ///      the highlight. Returns true when the highlight changed (the host redraws).
    bool update_hover(int x, int y);

    /// \brief Starts a drag on an editable keyframe: a gizmo handle first, then the
    ///      camera glyph under the cursor.
    /// \details \p keyframe_at returns the index of the keyframe whose glyph is at
    ///      (x, y), or -1; it is only called when no handle was hit. \p upright_plane
    ///      makes a glyph drag follow the plane of the pose's own up instead of the
    ///      screen plane. Returns true when the press belongs to the editor.
    bool press(int x, int y, bool upright_plane, const std::function<int(int, int)> &keyframe_at);

    /// \brief Applies an active drag to \p camera's cursor position (x, y).
    void drag(int x, int y);

    /// \brief Ends the drag and reports what it did.
    DragResult release();

    /// \brief Aborts the drag, restoring the pose captured at the press.
    void cancel();

    bool dragging() const { return dragged_keyframe_ >= 0; }

    /// \brief Turns the selected pose so its view direction points at \p target,
    ///      keeping the pose's own up (its roll). False when nothing is editable.
    bool aim_at(const vec3 &target);

    /// \brief Inserts a keyframe after the selected one; returns its index, or -1.
    /// \details The pose is interpolated halfway to the next keyframe, with the same
    ///      timing; past the last keyframe the last segment is extended so the new
    ///      glyph does not land on top of an existing one. The new keyframe becomes
    ///      the selection.
    int insert_after_selected();

    /// \brief Deletes the selected keyframe; returns its index, or -1.
    /// \details The keyframe after it becomes the selection (the new last keyframe
    ///      when the deleted one was last, or no selection when the path is now empty).
    int delete_selected();

    /// \brief Appends the selected pose's annotations and handles to \p commands.
    /// \details The station square marks the keyframe the insert/delete operations
    ///      act on and the aim ray shows its view direction; both are drawn for one
    ///      scene radius and depth tested, like the handles. Nothing is appended
    ///      without an editable keyframe.
    /// \param fb_width The framebuffer width in pixels.
    /// \param fb_height The framebuffer height in pixels.
    /// \param dpi The DPI scaling, so the strokes keep their apparent size.
    void append_overlay(std::vector<graphics::DrawCommand> &commands,
                        unsigned fb_width, unsigned fb_height, float dpi) const;

private:
    bool editable() const;
    bool rebuild(const std::vector<Frame> &frames, const std::vector<float> &times);
    void reset_drag();

private:
    Camera *camera_;
    KeyFrameInterpolator *kfi_;
    float glyph_width_ = 0.0f;
    bool editing_ = false;
    int selected_ = -1;

    // The drag in progress: the grabbed keyframe and handle, the pose captured at the
    // press, and the state that maps the cursor onto the handle. The drag plane (a
    // point and a normal) is fixed at the press so the pose follows the cursor; an
    // axis drag keeps a line parameter, a ring drag an accumulated angle.
    int dragged_keyframe_ = -1;
    Handle handle_ = Handle::none;
    Handle hovered_handle_ = Handle::none;
    vec3 drag_plane_point_;
    vec3 drag_plane_normal_;
    vec3 drag_start_position_;
    quat drag_start_orientation_;
    vec3 drag_axis_;            // the grabbed arrow's or ring's axis
    // Glyph grab: world offset from the press's plane point to the station, so the
    // station keeps its position under the cursor instead of jumping to it.
    vec3 drag_grab_offset_;
    float drag_start_scalar_ = 0.0f; // arrow grab: line parameter at the press
    float drag_angle_ = 0.0f;        // ring grab: accumulated angle
    float drag_last_angle_ = 0.0f;   // ring grab: previous cursor angle
    bool drag_moved_ = false;
};

} // namespace easy3d

#endif // EASY3D_RENDERER_KEYFRAME_EDITOR_H
