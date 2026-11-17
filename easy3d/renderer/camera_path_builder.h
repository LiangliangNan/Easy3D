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

#ifndef EASY3D_RENDERER_CAMERA_PATH_BUILDER_H
#define EASY3D_RENDERER_CAMERA_PATH_BUILDER_H

#include <cstddef>
#include <memory>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/frame.h>

namespace easy3d {
    class Camera;
    class KeyFrameInterpolator;
    class Model;
}

namespace easy3d {

/**
 * \brief Camera-path authoring for demonstrating a 3D scene or object in a video.
 * \details A path is a sequence of keyframe poses that an app hands to a
 *      KeyFrameInterpolator for preview, playback and recording. This class covers
 *      the sources that free recording cannot express:
 *
 *      - walking a traced ground route: generate_ground_route() turns a stroke of
 *        picked ground points into first-person eye-height poses aimed along the
 *        route,
 *      - orbiting a picked axis: generate_orbit() turns a line into a closed orbit,
 *      - recording a flight: flight_stations() lifts a traced ground stroke to camera
 *        stations at the flight height, aim_targets() turns an aim stroke into one
 *        target per station (a clicked target is a repeated aim_targets() value), and
 *        generate_flight() appends the aimed poses, and
 *      - pure builders for previewing or editing a path before it is inserted:
 *        orbit_frames(), ground_frames() (eye poses over ground-contact points) and
 *        flight_frames() (a position track with a separate aim track).
 *
 *      The class borrows a camera and a keyframe interpolator. The builders return
 *      plain frames; the generating functions append them to the interpolator the
 *      caller owns, so the poses feed exactly the path the caller's path UI, playback
 *      and video recording already operate on. The convenience of the generating
 *      functions is that they also grow the camera's scene radius to keep the new
 *      poses inside the view frustum. The class draws nothing and never moves or
 *      orients the camera: drawing an authored path is the caller's choice.
 *
 *      Smoothness and timing are the interpolator's: builders return sparse poses
 *      and the interpolator fits a smooth curve through them. Per-segment timing
 *      (easing, slowing down near points of interest) is not exposed here.
 *
 * \attention The walking route stands the walker upright on the traced surface:
 *      each pose rises from its own ground-contact point along the surface normal
 *      under it, and the eye height is measured along that normal, so the route
 *      tilts with the terrain. The normals are oriented to the side facing the
 *      camera (the opposite of its view direction), so a face wound either way
 *      cannot put the eye centers behind the surface. Poses without a surface
 *      normal (e.g. a point-cloud pick) use the current view's up direction.
 */
class CameraPathBuilder {
public:
    /// Borrows \p camera (for the view's up direction and the scene radius) and
    /// \p interpolator (the path that receives the poses).
    CameraPathBuilder(Camera *camera, KeyFrameInterpolator *interpolator);
    ~CameraPathBuilder() = default;

    /// \name Walking route
    //@{
    /// Bounds of the visible models, used by the eye height and the orbit. With no
    /// visible model the box becomes invalid, so callers can refuse the gesture.
    void set_scene(const std::vector<std::shared_ptr<Model>> &scene);

    /// The scene bounds last passed to set_scene().
    const Box3 &scene_box() const { return scene_box_; }

    /// \brief The up direction used where the route has no surface normal, a unit
    ///      vector.
    /// \details This is the current view's up direction. Eye poses that carry no
    ///      per-foot normal stand along it, and the eye height is measured against
    ///      the scene extent along it.
    vec3 ground_up_direction() const;

    /// \brief The eye height above the ground, in scene heights (default 0.2).
    /// \details A scene height is the extent of the scene box along
    ///      ground_up_direction(), or the scene radius before set_scene() has seen a
    ///      visible model.
    void set_eye_height_in_scene_heights(float height_in_scene_heights);
    float eye_height_in_scene_heights() const { return eye_height_in_scene_heights_; }

    /// \brief The eye height above the ground in world units, i.e.
    ///      eye_height_in_scene_heights() resolved against the scene box along
    ///      ground_up_direction() (or the scene radius before set_scene() has seen a
    ///      visible model).
    float eye_height_in_world_units() const;

    /// \brief Distance between route keyframes along the stroke, in character
    ///      heights (default 1, i.e. about one body length per keyframe).
    void set_keyframe_spacing_in_character_heights(float spacing_in_character_heights);
    float keyframe_spacing_in_character_heights() const { return keyframe_spacing_in_character_heights_; }

    /// \brief Generates a walking route through \p stroke on the ground and appends
    ///      it to the interpolator.
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
    ///      Each direction is normalized and flipped to oppose the camera's view
    ///      direction, i.e. to point at the rendered side of the surface; they are
    ///      smoothed along the route before posing. An empty vector stands every
    ///      pose along ground_up_direction().
    /// \return The number of keyframes added (0 if the stroke is degenerate, the
    ///      directions do not pair with the stroke, or the eye height is zero).
    std::size_t generate_ground_route(const std::vector<vec3> &stroke,
                                      const std::vector<vec3> &foot_up_directions);
    //@}

    /// \name Orbit
    //@{
    /// \brief Generates the orbit around \p axis through \p center and appends it to
    ///      the interpolator. Nothing is replaced: callers clear the path first when
    ///      a replacement is intended.
    /// \return The number of keyframes added (0 if the scene box is invalid or the
    ///      orbit settings produce no poses).
    std::size_t generate_orbit(const Line3 &axis, const vec3 &center);

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

    /// \name Flight
    //@{
    /// \brief The camera stations' height above the traced ground, in scene heights
    ///      (default 0.5).
    /// \details A scene height is the extent of the scene box along
    ///      ground_up_direction(), or the scene radius before set_scene() has seen a
    ///      visible model.
    void set_flight_height_in_scene_heights(float height_in_scene_heights);
    float flight_height_in_scene_heights() const { return flight_height_in_scene_heights_; }

    /// The flight height in world units, i.e. flight_height_in_scene_heights()
    /// resolved against the scene box along ground_up_direction() (or the scene
    /// radius before set_scene() has seen a visible model).
    float flight_height_in_world_units() const;

    /// \brief Camera stations of a flight track through \p stroke.
    /// \details Each station sits one flight height above its own ground point along
    ///      the surface up there, so the track follows the terrain. The stroke is
    ///      resampled by arc length at the keyframe spacing scaled by the flight
    ///      height, and the surface ups are oriented toward the camera and smoothed
    ///      like the walking route's.
    /// \param stroke Ground-contact points in track order.
    /// \param surface_up_directions The up direction at each stroke point, in the
    ///      same order; an empty vector stands every station along
    ///      ground_up_direction().
    /// \return The stations, or an empty vector if the stroke is degenerate, the
    ///      directions do not pair with it, or the flight height is zero.
    std::vector<vec3> flight_stations(const std::vector<vec3> &stroke,
                                      const std::vector<vec3> &surface_up_directions) const;

    /// \brief One aim point per station, sampled along \p stroke.
    /// \details The stroke is sampled at \p count equal arc-length fractions, so the
    ///      aim starts and ends where the stroke does and progresses with the flight.
    ///      A single stroke point aims every station at it.
    /// \return \p count targets, or an empty vector if \p stroke is empty or
    ///      \p count is zero.
    std::vector<vec3> aim_targets(const std::vector<vec3> &stroke, std::size_t count) const;

    /// \brief Appends flight poses over \p positions aimed at the matching \p targets,
    ///      with roll referenced to ground_up_direction(). Nothing is replaced:
    ///      callers clear the path first when a replacement is intended.
    /// \return The number of keyframes added (0 if the inputs do not pair up).
    std::size_t generate_flight(const std::vector<vec3> &positions, const std::vector<vec3> &targets);
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
    std::vector<Frame> orbit_frames(const Line3 &axis, const vec3 &center) const;

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
    std::vector<Frame> ground_frames(const std::vector<vec3> &feet,
                                     const std::vector<vec3> &up_directions,
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
    std::vector<Frame> flight_frames(const std::vector<vec3> &positions,
                                     const std::vector<vec3> &targets,
                                     const vec3 &up) const;
    //@}

protected:
    // The scene extent along ground_up_direction() in world units (the scene radius
    // before set_scene() has seen a visible model).
    float scene_height_in_world_units() const;

    // Resamples \p stroke and its camera-facing, smoothed surface ups at \p spacing;
    // false on degenerate input.
    bool resample_stroke(const std::vector<vec3> &stroke,
                         const std::vector<vec3> &surface_up_directions,
                         float spacing,
                         std::vector<vec3> &feet,
                         std::vector<vec3> &ups) const;

    /// Appends \p frame to the interpolator and grows the camera's scene radius to
    /// keep it inside the view frustum.
    void add_keyframe(const Frame &frame);

    /// Orientation looking along \p view_dir with \p up as the roll reference; the
    /// two must be orthogonal (used by the plane-constrained builders).
    static quat to_orientation(const vec3 &view_dir, const vec3 &up);
    /// Orientation looking from \p position at \p target with \p up as the roll
    /// reference; the view direction may point anywhere relative to \p up.
    static quat look_at_orientation(const vec3 &position, const vec3 &target, const vec3 &up);

protected:
    Camera *camera_;
    KeyFrameInterpolator *kfi_;

    // Walking route
    Box3 scene_box_;
    float eye_height_in_scene_heights_;
    float keyframe_spacing_in_character_heights_;

    // Flight
    float flight_height_in_scene_heights_;

    // Orbit
    float orbit_radius_offset_in_object_heights_;
    float orbit_vertical_offset_in_object_heights_;
    float pitch_degrees_;
    int keyframes_per_loop_;
    int num_loops_;
};

} // namespace easy3d

#endif // EASY3D_RENDERER_CAMERA_PATH_BUILDER_H
