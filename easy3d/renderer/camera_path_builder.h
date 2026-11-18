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
 *        stations at the flight height and generate_flight() appends the poses aiming
 *        straight down by default, or at one target per station when a target list is
 *        given (aim_targets() turns an aim stroke into such a list; a clicked target
 *        is a repeated aim_targets() value),
 *      - pure builders for previewing or editing a path before it is inserted:
 *        orbit_frames(), ground_frames() (eye poses over ground-contact points) and
 *        flight_frames() (a position track with a separate aim track), and
 *      - aimed poses for any of them: every builder produces its mode's default aim
 *        (ground: along the route, orbit: at the axis, flight: straight down), and
 *        aimed_frames() re-aims existing poses at a target per pose, keeping their
 *        position and roll reference. This is how an app lets the user replace a
 *        mode's default aims by a clicked point or a traced aim stroke.
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
 *      The setters store their arguments unchecked; the generating functions refuse
 *      the settings that would produce a degenerate track (a zero eye or flight
 *      height, non-positive orbit counts) and return nothing. Each function documents
 *      the refusal and any clamping.
 *
 * \note The generating functions append with KeyFrameInterpolator::add_keyframe
 *      (const Frame&), which times each new pose one second after the previous one and
 *      drops a pose whose position coincides with the previous one. The number of
 *      keyframes therefore sets the path duration, and a closed stroke (whose last
 *      point equals its first) loses one of the coincident first/last poses. Add poses
 *      through the interpolator yourself if you need explicit times.
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
    /// \brief Borrows \p camera and \p interpolator; neither is owned.
    /// \details The camera supplies the view's up and view directions (the fallback
    ///      stand direction and the camera-facing normal rule), the scene center and
    ///      the scene radius, which the generating functions may grow. The
    ///      interpolator receives the poses of the generating functions; the pure
    ///      builders never touch it.
    /// \param camera The viewer's camera; it must remain valid for the builder's
    ///      lifetime.
    /// \param interpolator The interpolator that receives the poses; it must remain
    ///      valid for the builder's lifetime if a generating function is called.
    /// \note The settings start at eye height 0.2 scene heights, keyframe spacing 0.5
    ///      character heights, flight height 0.5 scene heights, orbit radius and
    ///      vertical offsets 0 object heights, pitch 0 degrees, 10 keyframes per loop
    ///      and 2 loops.
    CameraPathBuilder(Camera *camera, KeyFrameInterpolator *interpolator);
    /// Destroys the builder. The camera and the interpolator are borrowed, so
    /// neither is destroyed.
    ~CameraPathBuilder() = default;

    /// \name Walking route
    //@{
    /// \brief Sets the scene bounds from the visible models in \p scene.
    /// \details The box is the union of the models' bounding boxes; a model without a
    ///      renderer or hidden with Renderer::set_visible(false) is ignored. The box is
    ///      recomputed from scratch on every call, so a model that becomes visible again
    ///      needs another set_scene().
    ///      The box supplies the scale of every height and offset setting: the scene
    ///      height of the walking and flight modes and the object height of the orbit.
    ///      While it is invalid, those heights fall back to the camera's scene radius
    ///      and the orbit refuses to build.
    /// \param scene The models to bound; typically the viewer's models.
    /// \attention Call this again whenever the set or the visibility of the scene
    ///      changes; the builders only see the last box.
    void set_scene(const std::vector<std::shared_ptr<Model>> &scene);

    /// \return The union of the visible models last passed to set_scene(); invalid
    ///      before the first call or when no model was visible.
    const Box3 &scene_box() const { return scene_box_; }

    /// \brief The up direction used where a route has no surface normal.
    /// \details This is the current view's up direction. Eye poses that carry no
    ///      per-foot normal stand along it, and the scene height (hence the eye and
    ///      flight heights in world units) is measured along it, so heights keep the
    ///      same ratio in any view.
    /// \return A unit vector; +Z if the camera's up vector is degenerate.
    vec3 ground_up_direction() const;

    /// \brief Sets the eye height above the ground, in scene heights (default 0.2).
    /// \details A scene height is the extent of scene_box() along
    ///      ground_up_direction(), or the camera's scene radius while the box is
    ///      invalid. generate_ground_route() uses the height twice: as the eye offset
    ///      above each foot and as the unit of the route's keyframe spacing (spacing x
    ///      eye height), so a taller eye also walks in larger steps.
    /// \param height_in_scene_heights A positive fraction of the scene height;
    ///      typically 0.1-0.3. A height whose world value is at most 1e-4 makes
    ///      generate_ground_route() add nothing.
    void set_eye_height_in_scene_heights(float height_in_scene_heights);
    /// \return The eye height above the ground, in scene heights.
    float eye_height_in_scene_heights() const { return eye_height_in_scene_heights_; }

    /// \brief The eye height above the ground in world units, i.e.
    ///      eye_height_in_scene_heights() resolved against scene_box() along
    ///      ground_up_direction() (or the camera's scene radius while the box is
    ///      invalid).
    /// \return The eye height in world units.
    float eye_height_in_world_units() const;

    /// \brief Distance between consecutive route keyframes along the stroke, in
    ///      character heights (default 0.5, i.e. half a character height per keyframe).
    /// \details The walking route resamples its stroke at
    ///      keyframe_spacing_in_character_heights() x eye_height_in_world_units(), and
    ///      flight_stations() at keyframe_spacing_in_character_heights() x
    ///      flight_height_in_world_units(): the same value gives different absolute
    ///      spacing for the two tracks.
    /// \param spacing_in_character_heights A positive multiple of the track's height.
    ///      The resulting world spacing is clamped to at least 1e-4, so a value at or
    ///      below zero is accepted but produces a keyframe every 1e-4 world units - an
    ///      enormous number of keyframes for a normal scene.
    void set_keyframe_spacing_in_character_heights(float spacing_in_character_heights);
    /// \return The keyframe spacing, in character heights.
    float keyframe_spacing_in_character_heights() const { return keyframe_spacing_in_character_heights_; }

    /// \brief Generates a walking route through \p stroke on the ground and appends
    ///      it to the interpolator.
    /// \details The stroke points are ground-contact points, so passing the surface
    ///      points picked under the cursor makes the route follow the terrain: each
    ///      pose sits one eye height above its own stroke point, along the walker's
    ///      up direction there, and aims level along the route. The stroke is
    ///      resampled at keyframe_spacing_in_character_heights() x
    ///      eye_height_in_world_units() by arc length, which keeps the stroke's first
    ///      and last points (a stroke shorter than the spacing still yields two poses)
    ///      and drops a sample that would crowd the end. The poses are appended to the
    ///      path (an existing path is kept, so several strokes can build one walk).
    /// \param stroke Ground-contact points in path order; at least two are needed.
    /// \param foot_up_directions The walker's up direction at each stroke point,
    ///      typically the surface normal there, in the same order as \p stroke.
    ///      Each direction is normalized and flipped to oppose the camera's view
    ///      direction, i.e. to point at the rendered side of the surface; they are
    ///      smoothed along the route before posing. An empty vector stands every
    ///      pose along ground_up_direction(); a degenerate direction at a stroke point
    ///      uses ground_up_direction() there.
    /// \return The number of poses generated, or 0 if the stroke has fewer than two
    ///      points, the directions do not pair with the stroke, or the eye height's
    ///      world value is at most 1e-4.
    /// \note The interpolator may accept fewer keyframes; see the class description.
    std::size_t generate_ground_route(const std::vector<vec3> &stroke,
                                      const std::vector<vec3> &foot_up_directions);
    //@}

    /// \name Orbit
    //@{
    /// \brief Generates the orbit around \p axis, centered where \p center projects
    ///      onto it, and appends it to the interpolator.
    /// \details The radius, vertical offset, pitch, keyframes per loop and loop
    ///      count come from the orbit settings; scene_box() supplies the scale. The
    ///      path closes the requested loops: its last pose repeats its first. Nothing
    ///      is replaced: callers clear the path first when a replacement is intended.
    /// \param axis The picked axis. The camera's up is its reversed direction, so a
    ///      line picked pointing into the screen produces an upright turntable.
    /// \param center The orbit center; the vertical offset is added to it along the
    ///      orbit's up and the result is projected onto \p axis.
    /// \return The number of poses generated, or 0 while scene_box() is invalid or
    ///      either count is not positive.
    /// \note The interpolator may accept fewer keyframes; see the class description.
    std::size_t generate_orbit(const Line3 &axis, const vec3 &center);

    /// \brief Extra orbit radius beyond the scene diagonal, in object heights
    ///      (default 0).
    /// \details Object height is the extent of scene_box() along the orbit's up
    ///      direction. The camera circles at
    ///      scene_box().diagonal_length() + offset x object height, so negative
    ///      values move it closer; keep the total radius positive.
    /// \param offset_in_object_heights The radius offset in object heights.
    void set_orbit_radius_offset_in_object_heights(float offset_in_object_heights);
    /// \return The extra orbit radius, in object heights.
    float orbit_radius_offset_in_object_heights() const { return orbit_radius_offset_in_object_heights_; }

    /// \brief Vertical offset of the orbit center above the picked point, in object
    ///      heights (default 0).
    /// \details The offset is applied along the orbit's up (the reversed axis) before
    ///      the center is projected onto the axis, so it moves the center along the
    ///      axis: it raises the orbit for a level axis and shifts it diagonally for an
    ///      oblique one.
    /// \param offset_in_object_heights The offset in object heights; a positive
    ///      value raises the center.
    void set_orbit_vertical_offset_in_object_heights(float offset_in_object_heights);
    /// \return The orbit center offset, in object heights.
    float orbit_vertical_offset_in_object_heights() const { return orbit_vertical_offset_in_object_heights_; }

    /// \brief Camera pitch in degrees (default 0).
    /// \details The poses look at the axis raised or lowered by radius x tan(pitch)
    ///      and the camera's up is tilted to match, so a positive pitch views the
    ///      orbit from below and a negative one from above.
    /// \param degrees The pitch in degrees. Keep |degrees| < 90, where the look-at
    ///      offset diverges.
    void set_pitch_degrees(float degrees);
    /// \return The camera pitch, in degrees.
    float pitch_degrees() const { return pitch_degrees_; }

    /// \brief Keyframes per orbit loop (default 10).
    /// \details Poses are placed every 360 / count degrees around the axis, so the
    ///      total is keyframes_per_loop() x num_loops() + 1 (the closing pose).
    /// \param count A positive number; a value at or below zero makes the orbit empty.
    void set_keyframes_per_loop(int count);
    /// \return The number of keyframes per loop.
    int keyframes_per_loop() const { return keyframes_per_loop_; }

    /// \brief Number of orbit loops (default 2).
    /// \param count A positive number; a value at or below zero makes the orbit empty.
    void set_num_loops(int count);
    /// \return The number of orbit loops.
    int num_loops() const { return num_loops_; }
    //@}

    /// \name Flight
    //@{
    /// \brief The camera stations' height above the traced ground, in scene heights
    ///      (default 0.5).
    /// \details A scene height is the extent of scene_box() along
    ///      ground_up_direction(), or the camera's scene radius while the box is
    ///      invalid. The flight resamples its track at
    ///      keyframe_spacing_in_character_heights() x its world height, so a higher
    ///      flight also has sparser stations.
    /// \param height_in_scene_heights A positive fraction of the scene height; a
    ///      height whose world value is at most 1e-4 makes flight_stations() return
    ///      nothing.
    void set_flight_height_in_scene_heights(float height_in_scene_heights);
    /// \return The flight height above the ground, in scene heights.
    float flight_height_in_scene_heights() const { return flight_height_in_scene_heights_; }

    /// \brief The flight height in world units, i.e. flight_height_in_scene_heights()
    ///      resolved against scene_box() along ground_up_direction() (or the camera's
    ///      scene radius while the box is invalid).
    /// \return The flight height in world units.
    float flight_height_in_world_units() const;

    /// \brief Camera stations of a flight track through \p stroke.
    /// \details Each station sits one flight height above its own ground point along
    ///      the surface up there, so the track follows the terrain. The stroke is
    ///      resampled by arc length at keyframe_spacing_in_character_heights() x
    ///      flight_height_in_world_units(), keeping the stroke's first and last points,
    ///      and the surface ups are oriented toward the camera and smoothed like the
    ///      walking route's. It modifies neither the camera nor the interpolator.
    /// \param stroke Ground-contact points in track order; at least two are needed.
    /// \param surface_up_directions The up direction at each stroke point, in the
    ///      same order; an empty vector stands every station along
    ///      ground_up_direction(), and a degenerate direction at a stroke point uses
    ///      ground_up_direction() there.
    /// \return One station per resampled foot, or an empty vector if the stroke has
    ///      fewer than two points, the directions do not pair with it, or the flight
    ///      height's world value is at most 1e-4.
    std::vector<vec3> flight_stations(const std::vector<vec3> &stroke,
                                      const std::vector<vec3> &surface_up_directions) const;

    /// \brief One aim point per station, sampled along \p stroke.
    /// \details The stroke is sampled at \p count equal arc-length fractions, so the
    ///      first station aims at the stroke's start and the last at its end, and the
    ///      aim progresses with the flight. A single stroke point (or a zero-length
    ///      stroke) aims every station at that point, and \p count 1 aims at the
    ///      stroke's midpoint.
    /// \param stroke The aim path; an empty vector yields no targets.
    /// \param count The number of stations to aim; zero yields no targets.
    /// \return \p count aim points in path order.
    std::vector<vec3> aim_targets(const std::vector<vec3> &stroke, std::size_t count) const;

    /// \brief Appends flight poses over \p positions aimed at the matching \p targets,
    ///      with roll referenced to ground_up_direction(). Nothing is replaced:
    ///      callers clear the path first when a replacement is intended.
    /// \details Typically \p positions comes from flight_stations() (or recorded
    ///      poses) and \p targets from aim_targets(); the aim is exact at each station
    ///      and the interpolator smooths it in between. The roll reference is the
    ///      view's up at the time of the call, so the camera never banks. An empty
    ///      \p targets keeps the flight's default aim, straight down.
    /// \param positions Camera positions in path order; at least two are needed.
    /// \param targets One aim point per position, in the same order, or empty for the
    ///      default aim. A target equal to its position looks down the reversed roll
    ///      reference.
    /// \return The number of poses generated, or 0 if there are fewer than two
    ///      positions or the inputs do not pair up.
    /// \note The interpolator may accept fewer keyframes; see the class description.
    std::size_t generate_flight(const std::vector<vec3> &positions, const std::vector<vec3> &targets);
    //@}

    /// \name Path builders
    /// Pure: they return the keyframe poses and never move the camera or touch the
    /// interpolator, so a caller can preview, filter or edit them first.
    //@{
    /// \brief Poses of an orbit around \p axis.
    /// \details The radius, vertical offset, pitch, keyframes per loop and loop
    ///      count come from the orbit settings; scene_box() supplies the scale.
    ///      The returned path covers keyframes_per_loop() x num_loops() segment steps
    ///      and closes the loop, i.e. its last pose equals its first.
    /// \param axis The picked axis. The camera's up is its reversed direction (a line
    ///      picked pointing into the screen gives an upright turntable), and the poses
    ///      look at the axis.
    /// \param center The orbit center; the vertical offset is added to it along the
    ///      orbit's up and the result is projected onto \p axis.
    /// \return orbit poses in path order (keyframes_per_loop() x num_loops() + 1 of
    ///      them), or an empty vector while scene_box() is invalid or either count is
    ///      not positive.
    std::vector<Frame> orbit_frames(const Line3 &axis, const vec3 &center) const;

    /// \brief Eye poses walking through \p feet, each pose standing along its own
    ///      up direction.
    /// \details Each pose sits \p eye_height above its foot along that foot's up and
    ///      aims level along the path (central-difference tangent projected onto its
    ///      foot's horizon plane), which keeps the heading continuous; the
    ///      interpolator smooths the path between the poses. The feet may lie at
    ///      different heights (terrain following); each pose uses its own foot and up
    ///      direction, so the walker tilts with the terrain. Coincident neighbours
    ///      borrow the heading of the nearest distinct foot, and a near-vertical step
    ///      falls back to an arbitrary in-plane heading.
    /// \param feet Ground-contact positions in path order; at least two are needed.
    /// \param up_directions One unit up direction per foot, in the same order; each
    ///      heading is kept perpendicular to its own direction. A degenerate
    ///      (near-zero) direction makes the result empty.
    /// \param eye_height Eye height above each foot, in world units, along the up
    ///      direction of that foot. This function does not range-check it: a zero
    ///      height puts the poses at the feet and a negative one below them, while
    ///      generate_ground_route() refuses a non-positive height.
    /// \return The poses, or an empty vector if fewer than two feet are given, the
    ///      directions do not pair with them, or a direction is degenerate.
    std::vector<Frame> ground_frames(const std::vector<vec3> &feet,
                                     const std::vector<vec3> &up_directions,
                                     float eye_height) const;

    /// \brief Flight poses along \p positions, each aiming at the matching target.
    /// \details The motion track and the aim track are independent: positions are
    ///      the camera stations, targets what the camera points at there. The
    ///      interpolator smooths both; between stations the aim is the interpolated
    ///      orientation, not an exact target lock. Each station's orientation is an
    ///      exact look-at: a degenerate \p up falls back to +Z, and a target equal to
    ///      its position looks down the reversed \p up. An empty \p targets is the
    ///      flight's default aim: every station looks straight down, along the
    ///      reversed \p up (a straight-down view has no preferred roll, so the basis
    ///      is deterministic but not tied to \p up).
    /// \param positions Camera positions in path order; at least two are needed.
    /// \param targets One aim point per position, in the same order, or empty for the
    ///      default aim.
    /// \param up Unit up reference that keeps the camera roll level.
    /// \return The poses, or an empty vector if there are fewer than two positions or
    ///      \p targets does not pair up with them.
    std::vector<Frame> flight_frames(const std::vector<vec3> &positions,
                                     const std::vector<vec3> &targets,
                                     const vec3 &up) const;

    /// \brief \p frames re-aimed so each pose looks at the matching target.
    /// \details The poses keep their positions and, by default, their own up as the
    ///      roll reference: a walking pose keeps standing upright on its terrain and
    ///      an orbit pose keeps the orbit's up. Some modes need another reference
    ///      (a flight keeps the horizon level), which \p roll_ups supplies. The aim is
    ///      an exact look-at per pose, with look_at_orientation()'s degenerate rules
    ///      (a target equal to its position looks down the reversed roll reference).
    ///      This is how a default-aimed path is aimed at a clicked point or a traced
    ///      aim stroke.
    /// \param frames The poses to re-aim; at least one is needed.
    /// \param targets One aim point per pose, in the same order.
    /// \param roll_ups One up reference per pose, in the same order, or empty to keep
    ///      each pose's own up.
    /// \return The re-aimed poses, or an empty vector if \p frames is empty or
    ///      \p targets or a non-empty \p roll_ups does not pair up with it.
    std::vector<Frame> aimed_frames(const std::vector<Frame> &frames,
                                    const std::vector<vec3> &targets,
                                    const std::vector<vec3> &roll_ups = {}) const;
    //@}

protected:
    // The scene extent along ground_up_direction(): the extent of scene_box_
    // projected on that direction, or the camera's scene radius while the box is
    // invalid. In world units.
    float scene_height_in_world_units() const;

    // Resamples stroke and its surface ups by arc length at spacing (world units):
    // feet receives the resampled ground points and ups the normalized,
    // camera-oriented, smoothed up directions. A sample closer than a quarter
    // spacing to the stroke end is dropped so the last keyframes do not crowd.
    // Returns false if stroke has fewer than two points or surface_up_directions
    // does not pair with it; an empty direction vector means "every foot uses
    // ground_up_direction()", and a degenerate direction uses it for that foot.
    bool resample_stroke(const std::vector<vec3> &stroke,
                         const std::vector<vec3> &surface_up_directions,
                         float spacing,
                         std::vector<vec3> &feet,
                         std::vector<vec3> &ups) const;

    // Appends frame to the interpolator, which times it one second after the previous
    // keyframe and may drop a pose coinciding with the previous one. When the pose was
    // accepted, grows the camera's scene radius to cover its distance from the scene
    // center if needed, keeping the authored path inside the view frustum.
    void add_keyframe(const Frame &frame);

    // Orientation looking along view_dir with up as the roll reference; the two must
    // be orthogonal (used by the plane-constrained builders), otherwise the basis is
    // not orthonormal.
    static quat to_orientation(const vec3 &view_dir, const vec3 &up);
    // Orientation looking from position at target with up as the roll reference; the
    // view direction may point anywhere relative to up. A zero up falls back to +Z
    // and a zero aim direction looks down the reversed up.
    static quat look_at_orientation(const vec3 &position, const vec3 &target, const vec3 &up);

protected:
    // Borrowed viewer camera: the view's up and view directions, the scene center
    // and the scene radius.
    Camera *camera_;
    // Borrowed path receiver for the generate_*() functions; the pure builders do
    // not use it.
    KeyFrameInterpolator *kfi_;

    // Walking route
    // Union of the visible models last passed to set_scene(); invalid when none was
    // visible.
    Box3 scene_box_;
    // Eye height above the ground, in scene heights (default 0.2).
    float eye_height_in_scene_heights_;
    // Keyframe spacing of the walking route and the flight, in units of the track's
    // height (default 0.5).
    float keyframe_spacing_in_character_heights_;

    // Flight
    // Camera stations' height above the traced ground, in scene heights (default 0.5).
    float flight_height_in_scene_heights_;

    // Orbit
    // Extra orbit radius beyond the scene diagonal, in object heights (default 0).
    float orbit_radius_offset_in_object_heights_;
    // Orbit center offset above the picked point, in object heights (default 0).
    float orbit_vertical_offset_in_object_heights_;
    // Orbit camera pitch, in degrees (default 0).
    float pitch_degrees_;
    // Orbit poses per loop (default 10).
    int keyframes_per_loop_;
    // Number of closed orbit loops (default 2).
    int num_loops_;
};

} // namespace easy3d

#endif // EASY3D_RENDERER_CAMERA_PATH_BUILDER_H
