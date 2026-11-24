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
 *        down at each station's foot by default (the reverse of its lift, so a tilted
 *        track looks along its own surface normal), or at one target per station when
 *        a target list is given (aim_targets() turns an aim stroke into such a list; a
 *        clicked target is a repeated aim_targets() value),
 *      - pure builders for previewing or editing a path before it is inserted:
 *        orbit_frames(), ground_frames() (eye poses over ground-contact points) and
 *        flight_frames() (stations with a separate aim track), and
 *      - aimed poses for any of them: every builder produces its mode's default aim
 *        (ground: along the route, orbit: at the axis, flight: at the ground ahead
 *        along its track), and aimed_frames() re-aims existing poses at a target per pose,
 *        keeping their position and roll reference. This is how an app lets the user
 *        replace a mode's default aims by a clicked point or a traced aim stroke.
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
 * \attention The walking route and the flight keep the camera upright: every pose
 *      of a path stands along one up direction, ground_up_direction(), which is
 *      resolved once from ground_up_axis() and is the same for the whole path. A
 *      traced surface point is only where the foot is: the pose rises from it along
 *      that up (an eye height, or a flight height), so a slope, a curved roof or a
 *      face wound either way cannot bank the camera, and a point cloud needs no
 *      normals at all. Heights are measured along the up as well, so a pose on a
 *      slope keeps its vertical height above its own foot.
 */
class CameraPathBuilder {
public:
    /// \brief Borrows \p camera and \p interpolator; neither is owned.
    /// \details The camera supplies the view's up direction (the rule behind
    ///      GroundUpAxis::View), the scene center and the scene radius, which the
    ///      generating functions may grow. The
    ///      interpolator receives the poses of the generating functions; the pure
    ///      builders never touch it.
    /// \param camera The viewer's camera; it must remain valid for the builder's
    ///      lifetime.
    /// \param interpolator The interpolator that receives the poses; it must remain
    ///      valid for the builder's lifetime if a generating function is called.
    /// \note The settings start at eye height 0.2 scene heights, keyframe spacing 0.5
    ///      scene heights, both ground tracks following the terrain's slopes, flight
    ///      height 4.0 scene heights, orbit radius offset 0.2 and vertical offset 0.1
    ///      object heights, pitch 0 degrees, 10 keyframes per loop and 2 loops.
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

    /// \brief Where the path's one up direction comes from.
    /// \details The camera of a walking or flying path never banks: every pose stands
    ///      along the same up, the anti-gravity direction of the scene. The app cannot
    ///      know that direction, so the user names the rule and the builder resolves
    ///      it once, from scene_box() and the camera.
    enum class GroundUpAxis {
        Auto,   ///< The world axis of the visible scene box's shortest extent: the height
                ///< of a building- or terrain-like scene. The default.
        X,      ///< World +X, for a scan or a figure whose orientation is known.
        Y,      ///< World +Y.
        Z,      ///< World +Z.
        View    ///< The camera's up direction, for a model the user rotated and is looking
                ///< at upright (a rolled view then tilts the path).
    };

    /// \brief Sets the rule ground_up_direction() resolves from (default Auto).
    /// \details The setting applies to every pose the builders make afterwards; it
    ///      does not touch poses already in the interpolator. Auto needs scene_box(),
    ///      so call set_scene() first; while the box is invalid it falls back to the
    ///      camera's up like View.
    void set_ground_up_axis(GroundUpAxis axis);
    /// \return The rule the up direction resolves from.
    GroundUpAxis ground_up_axis() const { return ground_up_axis_; }

    /// \brief The up direction of every pose this builder makes: one anti-gravity
    ///      direction for the whole path.
    /// \details Resolved from ground_up_axis(): the world axis of the scene box's
    ///      shortest extent, a named world axis, or the camera's up. The scene height
    ///      (hence the eye and flight heights in world units) is measured along it, so
    ///      heights keep the same ratio whatever the view.
    /// \return A unit vector; +Z if the camera's up vector is degenerate or the scene
    ///      box has no extent (e.g. before set_scene()).
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

    /// \brief Distance between consecutive keyframes along a ground track, in scene
    ///      heights (default 0.5, i.e. half the scene's height per keyframe).
    /// \details One value for both ground tracks, measured against the scene rather than
    ///      against either track's own height: the walking route and the flight resample
    ///      their strokes at the same world spacing, so the eye height and the flight
    ///      height no longer move the keyframe density. A scene height is the extent of
    ///      scene_box() along ground_up_direction(), or the camera's scene radius while
    ///      the box is invalid. The spacing chooses how many intervals the stroke is
    ///      divided into (length / spacing, rounded); the intervals are then equal, with
    ///      both stroke ends included.
    /// \param spacing_in_scene_heights A positive multiple of the scene height. The
    ///      resulting world spacing is clamped to at least 1e-4, so a value at or below
    ///      zero is accepted but produces a keyframe every 1e-4 world units - an enormous
    ///      number of keyframes for a normal scene.
    void set_keyframe_spacing_in_scene_heights(float spacing_in_scene_heights);
    /// \return The keyframe spacing, in scene heights.
    float keyframe_spacing_in_scene_heights() const { return keyframe_spacing_in_scene_heights_; }

    /// \brief Distance between consecutive keyframes along a ground track, in world
    ///      units.
    /// \details scene_height_in_world_units() x keyframe_spacing_in_scene_heights(),
    ///      clamped to at least 1e-4. Both ground tracks resample at this one value.
    /// \return The keyframe spacing in world units.
    float keyframe_spacing_in_world_units() const;

    /// \brief Whether the ground tracks follow the terrain's slopes (default true).
    /// \details True, every pose keeps its own height above the point under it, so a
    ///      walk climbs a hill and a flight climbs with the ground it traces. False,
    ///      the whole track keeps the altitude of the stroke's first point: every pose
    ///      stands the same height above that first level, however far the ground
    ///      climbs or falls beneath it. The stroke still supplies the track's ground points
    ///      (its foot), which is what a flight's default aim looks at.
    /// \param follow_slopes True to follow the terrain's heights, false for a constant
    ///      altitude across the track.
    void set_follow_ground_slopes(bool follow_slopes);
    /// \return Whether the ground tracks follow the terrain's slopes.
    bool follow_ground_slopes() const { return follow_ground_slopes_; }

    /// \brief Generates a walking route through \p stroke on the ground and appends
    ///      it to the interpolator.
    /// \details The stroke points are ground-contact points, so passing the surface
    ///      points picked under the cursor makes the route follow the terrain: each
    ///      pose sits one eye height above its own stroke point along
    ///      ground_up_direction(), and aims level along the route. The stroke is
    ///      resampled by arc length at keyframe_spacing_in_scene_heights() x the scene
    ///      height: the spacing rounds to a number of intervals
    ///      and the stroke's first and last points and the equal steps between them
    ///      make up the poses, so consecutive poses are always the same distance apart
    ///      along the stroke (a stroke shorter than the spacing still yields two
    ///      poses). The poses are appended to the path (an existing path is kept, so
    ///      several strokes can build one walk).
    /// \param stroke Ground-contact points in path order; at least two are needed.
    ///      While follow_ground_slopes() is true their own heights are kept, so a route
    ///      over terrain rises and falls with it; false keeps every pose at the first
    ///      point's altitude. Either way the camera stays upright on
    ///      ground_up_direction().
    /// \return The number of poses generated, or 0 if the stroke has fewer than two
    ///      points or the eye height's world value is at most 1e-4.
    /// \note The interpolator may accept fewer keyframes; see the class description.
    std::size_t generate_ground_route(const std::vector<vec3> &stroke);
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
    ///      (default 0.2).
    /// \details Object height is the extent of scene_box() along the orbit's up
    ///      direction. The camera circles at
    ///      scene_box().diagonal_length() + offset x object height, so negative
    ///      values move it closer; keep the total radius positive.
    /// \param offset_in_object_heights The radius offset in object heights.
    void set_orbit_radius_offset_in_object_heights(float offset_in_object_heights);
    /// \return The extra orbit radius, in object heights.
    float orbit_radius_offset_in_object_heights() const { return orbit_radius_offset_in_object_heights_; }

    /// \brief Vertical offset of the orbit center above the picked point, in object
    ///      heights (default 0.1).
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
    ///      (default 4.0).
    /// \details A scene height is the extent of scene_box() along
    ///      ground_up_direction(), or the camera's scene radius while the box is
    ///      invalid. The flight resamples its track at
    ///      keyframe_spacing_in_scene_heights() x the scene height, the same spacing as
    ///      the walking route, so raising the flight does not thin out its stations.
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

    /// \brief A flight camera station: the position its ground-stroke foot was lifted to.
    /// \details The pair keeps the lift, so the default aim can point back down at the
    ///      ground the station flies over (see flight_frames()).
    struct FlightStation {
        vec3 position;  ///< The camera position, lifted from \p foot along ground_up_direction().
        vec3 foot;      ///< The ground-stroke point the station was lifted from.
    };

    /// \brief Camera stations of a flight track through \p stroke.
    /// \details While follow_ground_slopes() is true each station sits one flight
    ///      height above its own ground point along ground_up_direction(), so the track
    ///      follows the terrain's heights; false lifts every station off the first
    ///      point's altitude instead, for a level flight over a slope. Either way the
    ///      camera stays upright and the station carries its ground point as its foot. The
    ///      stroke is resampled by arc length at keyframe_spacing_in_scene_heights() x
    ///      the scene height, with the same equal-step, both-ends-included rule as the
    ///      walking route. It
    ///      modifies neither the camera nor the interpolator.
    /// \param stroke Ground-contact points in track order; at least two are needed.
    /// \return One station per resampled foot, or an empty vector if the stroke has
    ///      fewer than two points or the flight height's world value is at most 1e-4.
    std::vector<FlightStation> flight_stations(const std::vector<vec3> &stroke) const;

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

    /// \brief Appends flight poses over \p stations aimed at the matching \p targets,
    ///      with roll referenced to ground_up_direction(). Nothing is replaced:
    ///      callers clear the path first when a replacement is intended.
    /// \details Typically \p stations comes from flight_stations() and \p targets from
    ///      aim_targets(); the aim is exact at each station and the interpolator
    ///      smooths it in between. The roll reference is ground_up_direction(), so the
    ///      camera never banks. An empty \p targets keeps the flight's default aim: a
    ///      station looks at the ground ahead of it along the track (see
    ///      flight_frames()), not straight down at its own foot.
    /// \param stations Camera stations in path order; at least two are needed.
    /// \param targets One aim point per station, in the same order, or empty for the
    ///      default aim. A target equal to its station's position looks down the
    ///      reversed roll reference.
    /// \return The number of poses generated, or 0 if there are fewer than two
    ///      stations or the inputs do not pair up.
    /// \note The interpolator may accept fewer keyframes; see the class description.
    std::size_t generate_flight(const std::vector<FlightStation> &stations,
                                const std::vector<vec3> &targets);
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

    /// \brief Eye poses walking through \p feet, all standing along one up direction.
    /// \details Each pose sits \p eye_height above its foot along \p up and aims level
    ///      along the path (central-difference tangent projected onto that up's horizon
    ///      plane), which keeps the heading continuous; the interpolator smooths the
    ///      path between the poses. The feet may lie at different heights (terrain
    ///      following) and the camera still never banks, because every pose stands on
    ///      the same up. Coincident neighbours borrow the heading of the nearest
    ///      distinct foot, and a near-vertical step falls back to an arbitrary
    ///      in-plane heading.
    /// \param feet Ground-contact positions in path order; at least two are needed.
    /// \param up The path's up direction, typically ground_up_direction(): one unit
    ///      vector for every pose. A degenerate (near-zero) direction makes the result
    ///      empty.
    /// \param eye_height Eye height above each foot, in world units, along \p up. This
    ///      function does not range-check it: a zero height puts the poses at the feet
    ///      and a negative one below them, while generate_ground_route() refuses a
    ///      non-positive height.
    /// \return The poses, or an empty vector if fewer than two feet are given or
    ///      \p up is degenerate.
    std::vector<Frame> ground_frames(const std::vector<vec3> &feet,
                                     const vec3 &up,
                                     float eye_height) const;

    /// \brief Flight poses at \p stations, each aiming at the matching target.
    /// \details The motion track and the aim track are independent: stations are
    ///      the camera positions, targets what the camera points at there. The
    ///      interpolator smooths both; between stations the aim is the interpolated
    ///      orientation, not an exact target lock. Each station's orientation is an
    ///      exact look-at: a degenerate \p up falls back to +Z, and a target equal to
    ///      its station's position looks down the reversed \p up.
    ///      An empty \p targets is the flight's default aim, the ground ahead: station
    ///      i looks at the foot of the station about \p look_ahead stations further
    ///      along the track (the last station looks as far ahead as the track has),
    ///      so the camera flies where it is going instead of straight down. Each
    ///      target is lowered by the station's own height above its foot, so a flight
    ///      over a slope aims along the ground rather than into it. A view along the
    ///      reversed \p up has no preferred roll, so the basis is deterministic but not
    ///      tied to \p up.
    /// \param stations Camera stations in path order; at least two are needed.
    /// \param targets One aim point per station, in the same order, or empty for the
    ///      default aim.
    /// \param up Unit up reference that keeps the camera roll level.
    /// \param look_ahead How many stations ahead the default aim looks, at least 1
    ///      (default 2). Ignored when \p targets is given.
    /// \return The poses, or an empty vector if there are fewer than two stations or
    ///      \p targets does not pair up with them.
    std::vector<Frame> flight_frames(const std::vector<FlightStation> &stations,
                                     const std::vector<vec3> &targets,
                                     const vec3 &up,
                                     std::size_t look_ahead = 2) const;

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

    // Resamples stroke by arc length at spacing (world units) into feet. The stroke is
    // divided into round(length / spacing) equal intervals with both ends included, so
    // no two consecutive samples are closer than the rest.
    // Returns false if stroke has fewer than two points.
    bool resample_stroke(const std::vector<vec3> &stroke, float spacing,
                         std::vector<vec3> &feet) const;

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
    // The camera positions of a ground track, from its ground points: each one its own
    // height above its foot while follow_ground_slopes() is true, and the first foot's
    // altitude plus the height for the whole track otherwise. The feet themselves are
    // left alone, so a caller keeps the ground reference.
    std::vector<vec3> lifted_feet(const std::vector<vec3> &feet, float height) const;

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
    // The rule ground_up_direction() resolves from (default Auto).
    GroundUpAxis ground_up_axis_;
    // Eye height above the ground, in scene heights (default 0.2).
    float eye_height_in_scene_heights_;
    // Keyframe spacing of the walking route and the flight, in units of the scene's
    // height (default 0.5). One value for both tracks, so neither track's height moves it.
    float keyframe_spacing_in_scene_heights_;
    // Whether the ground tracks follow the terrain's heights (default true). False keeps
    // every pose at the stroke's first point's altitude.
    bool follow_ground_slopes_;

    // Flight
    // Camera stations' height above the traced ground, in scene heights (default 4.0).
    float flight_height_in_scene_heights_;

    // Orbit
    // Extra orbit radius beyond the scene diagonal, in object heights (default 0.2).
    float orbit_radius_offset_in_object_heights_;
    // Orbit center offset above the picked point, in object heights (default 0.1).
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
