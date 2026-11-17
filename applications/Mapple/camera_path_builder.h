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
 *      - walking a traced ground route (generate_ground_route() turns a stroke of
 *        picked ground points into first-person eye-height poses aimed along the
 *        route),
 *      - orbiting a picked axis (generate_camera_path() turns a line into a closed
 *        orbit), and
 *      - pure builders for previewing or editing a path before it is inserted:
 *        orbit_frames(), ground_frames() (eye poses over ground-contact points)
 *        and flight_frames() (a position track with a separate aim track).
 *
 *      Ported from the Qt app (Mapple-Qt/walk_through.h). Unlike the Qt version it
 *      borrows the viewer's camera and keyframe interpolator, so the keyframes it
 *      creates are the ones the Camera Path window, playback and video recording
 *      already operate on, and it does not draw the path (the viewer draws it).
 *
 *      Mode only says which one-shot tool the app has armed; the app decides which
 *      gesture (a drag along the ground, a click for the orbit axis) is fed to the
 *      generators below and disarms the tool when the gesture is done.
 *
 *      Smoothness and timing are the interpolator's: builders return sparse poses
 *      and the interpolator fits a smooth curve through them. Per-segment timing
 *      (easing, slowing down near points of interest) is not exposed here.
 *
 * \attention The walking route stands the walker upright on the traced surface:
 *      each pose rises from its own ground-contact point along the surface normal
 *      under it, and the eye height is measured along that normal, so the route
 *      tilts with the terrain. Poses without a surface normal (e.g. a point-cloud
 *      pick) use the current view's up direction instead.
 */
class CameraPathBuilder {
public:
    CameraPathBuilder(easy3d::Camera *camera, easy3d::KeyFrameInterpolator *interpolator);
    ~CameraPathBuilder() = default;

    /// The one-shot tool the app has armed; Off means no tool owns the next gesture.
    enum class Mode {
        Off,         // no path tool is armed
        GroundWalk,  // the next drag on the ground becomes a walking route
        Orbit        // the next click picks the orbit axis
    };

    Mode mode() const { return mode_; }
    void set_mode(Mode mode) { mode_ = mode; }

    /// \name Walking route
    //@{
    /// Bounds of the visible models, used by the eye height and the orbit. With no
    /// visible model the box becomes invalid, so callers can refuse the gesture.
    void set_scene(const std::vector<std::shared_ptr<easy3d::Model>> &scene);

    /// \brief The up direction used where the route has no surface normal, a unit
    ///      vector.
    /// \details This is the current view's up direction. Eye poses that carry no
    ///      per-foot normal stand along it, and the eye height is measured against
    ///      the scene extent along it.
    easy3d::vec3 ground_up_direction() const;

    /// \brief The eye height above the ground, in scene heights (default 0.2).
    /// \details A scene height is the extent of the scene box along
    ///      ground_up_direction(), or the scene radius before set_scene() has seen a
    ///      visible model.
    void set_eye_height_in_scene_heights(float height_in_scene_heights);
    float eye_height_in_scene_heights() const { return eye_height_in_scene_heights_; }

    /// \brief Distance between route keyframes along the stroke, in character
    ///      heights (default 1, i.e. about one body length per keyframe).
    void set_keyframe_spacing_in_character_heights(float spacing_in_character_heights);
    float keyframe_spacing_in_character_heights() const { return keyframe_spacing_in_character_heights_; }

    /// \brief Generates a walking route through \p stroke on the ground.
    /// \details The stroke points are ground-contact points, so passing the surface
    ///      points picked under the cursor makes the route follow the terrain: each
    ///      pose sits one eye height above its own stroke point, along the walker's
    ///      up direction there, and aims level along the route. The stroke is
    ///      resampled at the keyframe spacing by arc length, and the poses are
    ///      appended to the path (an existing path is kept, so several strokes can
    ///      build one walk).
    /// \param stroke Ground-contact points in path order.
    /// \param foot_up_directions The walker's up direction at each stroke point,
    ///      typically the surface normal there, in the same order as \p stroke.
    ///      Each direction is normalized and flipped to face along
    ///      ground_up_direction(); they are smoothed along the route before posing.
    ///      An empty vector stands every pose along ground_up_direction().
    /// \return The number of keyframes added (0 if the stroke is degenerate, the
    ///      directions do not pair with the stroke, or the eye height is zero).
    std::size_t generate_ground_route(const std::vector<easy3d::vec3> &stroke,
                                      const std::vector<easy3d::vec3> &foot_up_directions);
    //@}

    /// \name Orbit
    //@{
    /// Generates the orbit around \p axis, replacing nothing: callers clear the
    /// path first when a replacement is intended.
    void generate_camera_path(const easy3d::Line3 &axis);

    /// \brief Extra orbit radius beyond the scene diagonal, in object heights
    ///      (default 0).
    void set_orbit_radius_offset_in_object_heights(float offset_in_object_heights);
    float orbit_radius_offset_in_object_heights() const { return orbit_radius_offset_in_object_heights_; }

    /// \brief Vertical offset of the orbit center above the picked axis point, in
    ///      object heights (default 0).
    void set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights);
    float orbit_vertical_offset_in_object_heights() const { return orbit_vertical_offset_in_object_heights_; }

    /// Camera pitch in degrees (default 0).
    void set_pitch_degrees(float degrees);
    float pitch_degrees() const { return pitch_degrees_; }

    /// Keyframes per orbit loop (default 10).
    void set_keyframes_per_loop(int count);
    int keyframes_per_loop() const { return keyframes_per_loop_; }

    /// Number of orbit loops (default 2).
    void set_num_loops(int count);
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

    /// \brief Eye poses walking through \p feet, each pose standing along its own
    ///      up direction.
    /// \details Each pose aims level along the path (central-difference tangent
    ///      projected onto its foot's horizon plane), which keeps the heading
    ///      continuous, and the interpolator smooths the path between the poses. The
    ///      feet may lie at different heights (terrain following); each pose uses its
    ///      own foot and up direction, so the walker tilts with the terrain.
    /// \param feet Ground-contact positions in path order; at least two are needed.
    /// \param up_directions One unit up direction per foot, in the same order; each
    ///      heading is kept perpendicular to its own direction.
    /// \param eye_height Eye height above each foot, in world units, along the up
    ///      direction of that foot.
    /// \return The poses, or an empty vector if fewer than two feet are given, the
    ///      directions do not pair with them, or a direction is degenerate.
    std::vector<easy3d::Frame> ground_frames(const std::vector<easy3d::vec3> &feet,
                                             const std::vector<easy3d::vec3> &up_directions,
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

    /// The scene bounds last passed to set_scene().
    const easy3d::Box3 &scene_box() const { return scene_box_; }

protected:
    void add_keyframe(const easy3d::Frame &frame);

    // The eye height above the ground, in world units along the up direction.
    float eye_height() const;

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

    // Walking route
    easy3d::Box3 scene_box_;
    float eye_height_in_scene_heights_;
    float keyframe_spacing_in_character_heights_;

    // Orbit
    float orbit_radius_offset_in_object_heights_;
    float orbit_vertical_offset_in_object_heights_;
    float pitch_degrees_;
    int keyframes_per_loop_;
    int num_loops_;
};

#endif // EASY3D_MAPPLE_CAMERA_PATH_BUILDER_H
