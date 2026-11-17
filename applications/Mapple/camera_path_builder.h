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

#ifndef EASY3D_MAPPLE_CAMERA_PATH_BUILDER_H
#define EASY3D_MAPPLE_CAMERA_PATH_BUILDER_H

#include <memory>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/frame.h>

namespace easy3d {
    class Camera;
    class KeyFrameInterpolator;
    class Model;
}


/**
 * \brief Camera-path authoring for demonstrating a 3D scene or object in a video.
 * \details A path is a sequence of keyframe poses that the app hands to the shared
 *      KeyFrameInterpolator for preview, playback and recording. This class covers
 *      the sources that free recording cannot express:
 *
 *      - walking along a ground plane (walk_to() turns a picked surface point into
 *        a third-person pose),
 *      - orbiting a picked axis (generate_camera_path() turns a line into a closed
 *        orbit), and
 *      - pure builders for previewing or editing a path before it is inserted:
 *        orbit_frames(), ground_frames() (plane-constrained walking) and
 *        flight_frames() (a position track with a separate aim track).
 *
 *      Ported from the Qt app (Mapple-Qt/walk_through.h). Unlike the Qt version it
 *      borrows the viewer's camera and keyframe interpolator, so the keyframes it
 *      creates are the ones the Camera Path window, playback and video recording
 *      already operate on, and it does not draw the path (the viewer draws it).
 *
 *      Smoothness and timing are the interpolator's: builders return sparse poses
 *      and the interpolator fits a smooth curve through them. Per-segment timing
 *      (easing, slowing down near points of interest) is not exposed here.
 *
 * \attention The walking mode assumes the scene is upright, i.e. (0, 0, 1) is up.
 */
class CameraPathBuilder {
public:
    CameraPathBuilder(easy3d::Camera *camera, easy3d::KeyFrameInterpolator *interpolator);
    ~CameraPathBuilder() = default;

    enum class Mode {
        Off,         // no path tool is active
        Free,        // keyframes are recorded from the current camera pose
        GroundWalk,  // clicks on the scene walk the character there
        Orbit        // a picked line becomes the orbit axis
    };

    Mode mode() const { return mode_; }
    void set_mode(Mode mode) { mode_ = mode; }

    /// \name Walking mode
    //@{
    /// Bounds of the visible models, used by the eye height and the orbit.
    void set_scene(const std::vector<std::shared_ptr<easy3d::Model>> &scene);

    /// Walks the character to the \p ground_point position and records a keyframe.
    void walk_to(const easy3d::vec3 &ground_point);

    void set_follow_up(bool follow_up) { follow_up_ = follow_up; }
    bool follow_up() const { return follow_up_; }

    /// \brief The eye height above the walking plane, in scene heights (default 0.2).
    /// \details The scene height is the scene box's z extent, or the scene radius
    ///      before set_scene() has seen a visible model.
    void set_eye_height_in_scene_heights(float height_in_scene_heights);
    float eye_height_in_scene_heights() const { return eye_height_in_scene_heights_; }

    /// \brief The observer distance behind the character's eye, in character heights
    ///      (default 1.8). 0 puts the camera at the eye.
    void set_eye_distance_in_character_heights(float distance_in_character_heights);
    float eye_distance_in_character_heights() const { return eye_distance_in_character_heights_; }
    //@}

    /// \name Orbit mode
    //@{
    /// Generates the orbit around \p axis, replacing nothing: callers clear the
    /// path first when a replacement is intended.
    void generate_camera_path(const easy3d::Line3 &axis);

    /// \brief Extra orbit radius beyond the scene diagonal, in object heights
    ///      (default 0).
    void set_orbit_radius_offset_in_object_heights(float offset_in_object_heights,
                                                   bool re_generate = false);
    float orbit_radius_offset_in_object_heights() const { return orbit_radius_offset_in_object_heights_; }

    /// \brief Vertical offset of the orbit center above the picked axis point, in
    ///      object heights (default 0).
    void set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights,
                                                     bool re_generate = false);
    float orbit_vertical_offset_in_object_heights() const { return orbit_vertical_offset_in_object_heights_; }

    /// Camera pitch in degrees (default 0).
    void set_pitch_degrees(float degrees, bool re_generate = false);
    float pitch_degrees() const { return pitch_degrees_; }

    /// Keyframes per orbit loop (default 10).
    void set_keyframes_per_loop(int count, bool re_generate = false);
    int keyframes_per_loop() const { return keyframes_per_loop_; }

    /// Number of orbit loops (default 2).
    void set_num_loops(int count, bool re_generate = false);
    int num_loops() const { return num_loops_; }
    //@}

    /// \name Path builders
    /// Pure: they return the keyframe poses and never move the camera or touch the
    /// interpolator, so a caller can preview, filter or edit them first.
    //@{
    /// \brief Poses of an orbit around \p axis through \p center.
    /// \details The radius, vertical offset, pitch, keyframes per loop and loop
    ///      count come from the orbit settings; scene_box() supplies the scale.
    ///      The returned path covers `keyframes_per_loop * num_loops` segment steps
    ///      and closes the loop, i.e. its last pose equals its first.
    /// \param axis The orbit axis; the camera looks at the axis and the scene
    ///      recedes along the direction opposite to the picked line.
    /// \param center The scene center the orbit is built around.
    std::vector<easy3d::Frame> orbit_frames(const easy3d::Line3 &axis, const easy3d::vec3 &center) const;

    /// \brief Eye poses walking through \p feet on the plane with normal \p up.
    /// \details Each pose aims along the path (central-difference tangent projected
    ///      onto the plane), which keeps the heading continuous, and the
    ///      interpolator smooths the path between the poses. The feet themselves
    ///      are not projected; they are assumed to lie on the plane.
    /// \param feet Ground-contact positions in path order; at least two are needed.
    /// \param up Unit normal of the walking plane.
    /// \param eye_height Eye height above the plane, in world units, used for every
    ///      pose (terrain following needs per-foot heights and is not applied here).
    /// \return The poses, or an empty vector if fewer than two feet are given.
    std::vector<easy3d::Frame> ground_frames(const std::vector<easy3d::vec3> &feet, const easy3d::vec3 &up,
                                             float eye_height) const;

    /// \brief Flight poses along \p positions, each aiming at the matching target.
    /// \details The motion track and the aim track are independent: positions are
    ///      the camera stations, targets what the camera points at there. The
    ///      interpolator smooths both; between stations the aim is the interpolated
    ///      orientation, not an exact target lock.
    /// \param positions Camera positions in path order; at least two are needed.
    /// \param targets One aim point per position, in the same order.
    /// \param up Unit up reference that keeps the camera roll level.
    /// \return The poses, or an empty vector if the inputs do not pair up.
    std::vector<easy3d::Frame> flight_frames(const std::vector<easy3d::vec3> &positions,
                                             const std::vector<easy3d::vec3> &targets,
                                             const easy3d::vec3 &up) const;
    //@}

    /// Removes the last keyframe; in walking mode with follow-up, the camera
    /// returns to the new last keyframe.
    void delete_last_keyframe();

    /// Removes every keyframe.
    void clear();

    /// Moves the camera to the \p idx keyframe immediately and updates the pivot.
    /// \return The index of the keyframe the camera is at.
    int move_to(int idx);

    /// Index of the keyframe the camera was last moved to, or -1.
    int current_keyframe_index() const { return current_frame_idx_; }

    /// The scene bounds last passed to set_scene().
    const easy3d::Box3 &scene_box() const { return scene_box_; }

protected:
    void add_keyframe(const easy3d::Frame &frame);
    void add_keyframe(const easy3d::vec3 &cam_pos, const easy3d::vec3 &view_dir);

    // The eye height above the ground, in world units.
    float eye_height() const;
    // The distance from the character's eye to the observer, in world units.
    float eye_distance() const;
    easy3d::vec3 eye_position(const easy3d::vec3 &foot) const;
    easy3d::quat to_orientation(const easy3d::vec3 &view_dir) const;
    /// Orientation looking along \p view_dir with \p up as the roll reference; the
    /// two must be orthogonal (used by the plane-constrained builders).
    static easy3d::quat to_orientation(const easy3d::vec3 &view_dir, const easy3d::vec3 &up);
    /// Orientation looking from \p position at \p target with \p up as the roll
    /// reference; the view direction may point anywhere relative to \p up.
    static easy3d::quat look_at_orientation(const easy3d::vec3 &position, const easy3d::vec3 &target,
                                            const easy3d::vec3 &up);

protected:
    easy3d::Camera *camera_;
    easy3d::KeyFrameInterpolator *kfi_;

    Mode mode_;

    // Walking mode
    easy3d::Box3 scene_box_;
    easy3d::vec3 ground_plane_normal_;   // upright scenes only: (0, 0, 1)
    bool follow_up_;
    float eye_height_in_scene_heights_;
    float eye_distance_in_character_heights_;

    // Orbit mode
    easy3d::Line3 rotate_axis_;
    float orbit_radius_offset_in_object_heights_;
    float orbit_vertical_offset_in_object_heights_;
    float pitch_degrees_;
    int keyframes_per_loop_;
    int num_loops_;

    int current_frame_idx_;
};

#endif // EASY3D_MAPPLE_CAMERA_PATH_BUILDER_H
