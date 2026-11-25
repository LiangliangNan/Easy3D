// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/camera_path_builder.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>

#include "../test.h"

using namespace easy3d;
using easy3d::test::require;

namespace {

bool close_to(float a, float b, float tolerance = 1e-3f) { return std::abs(a - b) <= tolerance; }

bool close_to(const vec3 &a, const vec3 &b, float tolerance = 1e-3f) {
    return distance(a, b) <= tolerance;
}

// A box of vertices is enough: the builders only need the bounds of the visible
// models, not their geometry.
std::shared_ptr<SurfaceMesh> make_box_model(float half_extent) {
    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->add_vertex(vec3(-half_extent));
    mesh->add_vertex(vec3(half_extent));
    mesh->set_renderer(std::make_shared<Renderer>(mesh.get(), false));
    return mesh;
}

// Camera looking straight down, so the camera up is the horizontal -Z (an exact
// orthonormal pose): the path's up is world +Y while the view's up is -Z, which is what
// separates the two rules.
void aim_down(Camera &camera) {
    camera.set_position(vec3(0.0f, 30.0f, 0.0f));
    camera.set_view_direction(vec3(0.0f, -1.0f, 0.0f));
    camera.set_up_vector(vec3(0.0f, 0.0f, -1.0f));
}

const vec3 local_up(0.0f, 1.0f, 0.0f);
const vec3 local_forward(0.0f, 0.0f, -1.0f);

} // namespace

int main() {
    return easy3d::test::run("camera path builder", [] {
        auto box = make_box_model(5.0f);
        Camera camera;
        aim_down(camera);
        camera.set_scene_center(vec3(0.0f));
        camera.set_scene_radius(1.0f);
        KeyFrameInterpolator kfi(camera.frame());
        CameraPathBuilder builder(&camera, &kfi);
        // A flight station is the lifted camera position and the ground foot it rose
        // from; the default aim looks at the ground ahead along the track.
        auto make_stations = [](const std::vector<vec3> &positions, const vec3 &lift_up) {
            std::vector<CameraPathBuilder::FlightStation> stations;
            stations.reserve(positions.size());
            for (const vec3 &p : positions)
                stations.push_back(CameraPathBuilder::FlightStation{p, p - lift_up});
            return stations;
        };
        // Fix the spacing the route and flight counts below are written against instead of
        // depending on the default: 0.2 of the 10-unit scene is 2 world units, the same
        // for the walk and the flight.
        builder.set_keyframe_spacing_in_scene_heights(0.2f);
        require(close_to(builder.keyframe_spacing_in_scene_heights(), 0.2f),
                "the keyframe spacing was not stored");

        // --- scene bounds and eye height ---------------------------------
        builder.set_scene({});
        require(!builder.scene_box().is_valid(), "an empty scene produced valid bounds");

        auto hidden = make_box_model(5.0f);
        hidden->renderer()->set_visible(false);
        builder.set_scene({hidden});
        require(!builder.scene_box().is_valid(), "hidden models contributed to the bounds");

        builder.set_scene({box});
        require(builder.scene_box().is_valid(), "the visible model produced no bounds");
        require(close_to(builder.scene_box().min_point(), vec3(-5.0f)) &&
                close_to(builder.scene_box().max_point(), vec3(5.0f)),
                "the scene box does not match the model bounds");
        // Auto: the box is 10 x 10 x 10, so every extent ties and X wins; the named axes and
        // the view rule resolve exactly, and the scene height is measured along the up.
        require(close_to(builder.ground_up_direction(), vec3(1.0f, 0.0f, 0.0f)),
                "Auto did not resolve the scene box's shortest extent");
        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::View);
        require(close_to(builder.ground_up_direction(), vec3(0.0f, 0.0f, -1.0f)),
                "the view rule is not the camera's up direction");
        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Y);
        require(close_to(builder.ground_up_direction(), vec3(0.0f, 1.0f, 0.0f)),
                "the +Y rule did not resolve to +Y");
        // A flat scene (wide and deep, shallow in Y) stands on +Y: Y is its shortest extent.
        auto flat = std::make_shared<SurfaceMesh>();
        for (int i = 0; i < 8; ++i) // the corners of a 40 x 10 x 40 box
            flat->add_vertex(vec3((i & 1) ? 20.0f : -20.0f, (i & 2) ? 5.0f : -5.0f, (i & 4) ? 20.0f : -20.0f));
        flat->set_renderer(std::make_shared<Renderer>(flat.get(), false));
        builder.set_scene({flat});
        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Auto);
        require(close_to(builder.ground_up_direction(), vec3(0.0f, 1.0f, 0.0f)),
                "Auto did not stand a flat scene on its shortest extent");
        // A tall scene (10 x 40 x 10) is equally shallow in X and Z, so the shortest extent
        // ties and X wins, the same way the box's own accessor resolves ties.
        auto tall = std::make_shared<SurfaceMesh>();
        for (int i = 0; i < 8; ++i) // the corners of a 10 x 40 x 10 box
            tall->add_vertex(vec3((i & 1) ? 5.0f : -5.0f, (i & 2) ? 20.0f : -20.0f, (i & 4) ? 5.0f : -5.0f));
        tall->set_renderer(std::make_shared<Renderer>(tall.get(), false));
        builder.set_scene({tall});
        require(close_to(builder.ground_up_direction(), vec3(1.0f, 0.0f, 0.0f)),
                "Auto did not resolve a tied shortest extent the way the box does");
        builder.set_scene({box});
        // 0.2 scene heights along the up direction, whose scene extent is 10.
        require(close_to(builder.eye_height_in_world_units(), 2.0f), "the eye height in world units is wrong");

        // --- a flat route --------------------------------------------------
        // Back to the 10 x 10 x 10 cube, whose three extents tie and so gives Auto no answer:
        // the rule is pinned to +Y below, and the eye height above is 2 world units along it.
        builder.set_scene({box});
        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Y);
        std::vector<vec3> flat_stroke;
        for (int i = 0; i <= 8; ++i)
            flat_stroke.emplace_back(static_cast<float>(i) - 4.0f, -5.0f, 0.0f);
        const std::size_t flat_count = builder.generate_ground_route(flat_stroke);
        require(flat_count == 5, "the flat route was not resampled at the scene spacing");
        require(kfi.number_of_keyframes() == flat_count, "the route poses did not reach the interpolator");
        for (std::size_t i = 0; i < flat_count; ++i) {
            const Frame f = kfi.keyframe(i);
            require(close_to(f.position().y, -3.0f) && close_to(f.position().z, 0.0f),
                    "a flat route pose is not one eye height above its foot");
            require(close_to(f.orientation().rotate(local_up), vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "a flat route pose does not stand upright");
            require(close_to(f.orientation().rotate(local_forward), vec3(1.0f, 0.0f, 0.0f), 1e-4f),
                    "a flat route pose does not aim along the route");
        }

        // --- a diagonal route: the heading follows it, nothing tilts -------
        // The stroke climbs in Z, one of the two axes across the path's +Y up, so the ground
        // under it never rises. The stroke further down climbs along +Y itself, which is
        // where the slope rule is exercised.
        std::vector<vec3> diagonal_stroke;
        for (int i = 0; i <= 8; ++i)
            diagonal_stroke.emplace_back(static_cast<float>(i) - 4.0f, -5.0f, static_cast<float>(i));
        const std::size_t diagonal_count = builder.generate_ground_route(diagonal_stroke);
        // 8 units across and 8 deeper is 11.31 units of stroke, so a 2-unit spacing gives
        // round(11.31 / 2) = 6 intervals, i.e. 7 poses.
        require(diagonal_count == 7, "the diagonal route was not resampled at the scene spacing");
        for (std::size_t i = flat_count; i < flat_count + diagonal_count; ++i) {
            const Frame f = kfi.keyframe(i);
            const vec3 pose_up = f.orientation().rotate(local_up);
            require(close_to(pose_up, vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "a diagonal pose does not stand on the path's one up direction");
            const vec3 foot = f.position() - pose_up * builder.eye_height_in_world_units();
            require(close_to(foot.y, -5.0f), "a diagonal pose does not rise from its stroke point");
            require(close_to(foot.z, foot.x + 4.0f, 1e-3f), "a diagonal pose left the stroke");
            const vec3 heading = f.orientation().rotate(local_forward);
            require(close_to(dot(heading, pose_up), 0.0f, 1e-4f), "the heading is not level on the diagonal");
            require(close_to(heading, normalize(vec3(1.0f, 0.0f, 1.0f)), 1e-3f),
                    "the heading does not follow the route");
        }

        // --- a track that climbs along the up: Follow slopes on -------------
        // The diagonal stroke above climbs in Z, which is across the path's +Y up, so the
        // slope rule cannot change a pose on it. This stroke climbs along +Y itself, and that
        // is where following the ground shows: every pose stands over its own foot.
        std::vector<vec3> climb_stroke;
        for (int i = 0; i <= 8; ++i)
            climb_stroke.emplace_back(static_cast<float>(i) - 4.0f, -5.0f + 0.5f * static_cast<float>(i), 0.0f);
        require(builder.follow_ground_slopes(), "a new builder did not follow the slopes");
        // 8 across and 4 up is 8.94 units of stroke, so a 2-unit spacing gives
        // round(8.94 / 2) = 4 intervals, i.e. 5 poses standing at x = -4, -2, 0, 2, 4.
        const std::size_t climbed_count = builder.generate_ground_route(climb_stroke);
        require(climbed_count == 5, "the climbing route was not resampled at the scene spacing");
        const std::size_t climbed_first = flat_count + diagonal_count;
        for (std::size_t i = 0; i < climbed_count; ++i) {
            const Frame f = kfi.keyframe(climbed_first + i);
            // The feet climb one unit for every two they travel, and each eye stands 2 above
            // its own foot: from (-4, -5) up to (4, -1).
            require(close_to(f.position().x, -4.0f + 2.0f * static_cast<float>(i)) &&
                            close_to(f.position().y, -3.0f + static_cast<float>(i)),
                    "a climbing pose did not rise with the ground under it");
            require(close_to(f.orientation().rotate(local_forward), vec3(1.0f, 0.0f, 0.0f), 1e-4f),
                    "a climbing pose does not look along the level part of the route");
        }

        // --- the same track with the rule off --------------------------------
        // One altitude for the whole track: the first foot's plus one eye height, however far
        // the ground has climbed beneath it.
        builder.set_follow_ground_slopes(false);
        require(!builder.follow_ground_slopes(), "the slope rule was not stored");
        const std::size_t level_count = builder.generate_ground_route(climb_stroke);
        require(level_count == climbed_count, "the level route changed the keyframe count");
        const std::size_t level_first = climbed_first + climbed_count;
        for (std::size_t i = 0; i < level_count; ++i) {
            const Frame f = kfi.keyframe(level_first + i);
            require(close_to(f.position().y, -3.0f), "a level route pose did not keep the first altitude");
            require(close_to(f.orientation().rotate(local_up), vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "a level route pose is not upright");
        }
        // Flight stations lift off the same altitude, and keep their own ground feet: here the
        // feet climb while the stations hold one height.
        const std::vector<CameraPathBuilder::FlightStation> level_stations =
                builder.flight_stations(climb_stroke);
        require(!level_stations.empty(), "the level flight produced no stations");
        const float level_altitude = level_stations.front().position.y;
        for (std::size_t i = 0; i < level_stations.size(); ++i) {
            require(close_to(level_stations[i].position.y, level_altitude, 1e-4f),
                    "a level flight station changed its altitude");
            require(close_to(level_stations[i].foot.x, -4.0f + 2.0f * static_cast<float>(i)) &&
                            close_to(level_stations[i].foot.y, -5.0f + static_cast<float>(i)),
                    "a level flight station lost its ground foot");
        }
        builder.set_follow_ground_slopes(true);

        // --- a point cloud: no faces, no normals, the same path -----------
        // A point-cloud pick has no face to read a normal from, which is exactly why the up
        // cannot come from one; the route through the same points must be identical.
        auto cloud = std::make_shared<PointCloud>();
        for (const vec3 &p : flat_stroke)
            cloud->add_vertex(p);
        cloud->set_renderer(std::make_shared<Renderer>(cloud.get(), false));
        builder.set_scene({cloud});
        require(close_to(builder.ground_up_direction(), vec3(1.0f, 0.0f, 0.0f)) ||
                close_to(builder.ground_up_direction(), vec3(0.0f, 1.0f, 0.0f)),
                "Auto failed on a point cloud");
        builder.set_scene({box});
        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Y);
        const std::size_t cloud_count = builder.generate_ground_route(flat_stroke);
        require(cloud_count == flat_count, "the point-cloud route changed the keyframe count");
        for (std::size_t i = 0; i < flat_count; ++i) {
            const Frame a = kfi.keyframe(i);
            const Frame b = kfi.keyframe(level_first + level_count + i);
            require(close_to(a.position(), b.position()) &&
                    close_to(a.orientation().rotate(local_up), b.orientation().rotate(local_up)),
                    "the route through a point cloud differs from the mesh route");
        }

        // --- equal steps along a stroke of odd length -----------------------
        // The spacing rounds the stroke into whole intervals: 7 units at a 1.5-unit
        // spacing (0.15 scene heights of a 10-unit scene) is five intervals of 1.4, and the poses
        // stay on the stroke's ends, so no consecutive pair is cramped.
        std::vector<vec3> odd_stroke;
        for (int i = 0; i <= 7; ++i)
            odd_stroke.emplace_back(static_cast<float>(i), -5.0f, 0.0f);
        builder.set_keyframe_spacing_in_scene_heights(0.15f);
        const std::size_t odd_count = builder.generate_ground_route(odd_stroke);
        builder.set_keyframe_spacing_in_scene_heights(0.2f);
        require(odd_count == 6, "the odd-length route was not divided into equal intervals");
        for (std::size_t i = 0; i < odd_count; ++i) {
            const Frame f = kfi.keyframe(level_first + level_count + cloud_count + i);
            require(close_to(f.position().x, 1.4f * static_cast<float>(i)) && close_to(f.position().y, -3.0f) &&
                    close_to(f.position().z, 0.0f),
                    "an odd-length route pose is not at an equal arc-length step");
        }

        // --- degenerate inputs ---------------------------------------------
        require(builder.generate_ground_route({vec3(0.0f)}) == 0, "a single-point stroke was accepted");
        builder.set_eye_height_in_scene_heights(0.0f);
        require(builder.generate_ground_route(flat_stroke) == 0, "a zero eye height was accepted");
        builder.set_eye_height_in_scene_heights(0.2f);
        require(builder.ground_frames({vec3(0.0f)}, vec3(0.0f, 1.0f, 0.0f), 1.0f).empty(),
                "ground_frames accepted a single foot");
        require(builder.ground_frames({vec3(0.0f), vec3(1.0f)}, vec3(0.0f), 1.0f).empty(),
                "ground_frames accepted a degenerate up");
        require(builder.flight_frames(make_stations({vec3(0.0f), vec3(1.0f)}, vec3(0.0f, 1.0f, 0.0f)),
                                      {vec3(1.0f)}, vec3(0.0f, 1.0f, 0.0f)).empty(),
                "flight_frames accepted unpaired targets");

        // --- turntable orbit -----------------------------------------------
        // The orbit poses below are written against a turntable that is level and sits exactly at
        // the scene diagonal, so pin the two orbit offsets instead of depending on the defaults.
        kfi.delete_path();
        builder.set_orbit_radius_offset_in_object_heights(0.0f);
        builder.set_orbit_vertical_offset_in_object_heights(0.0f);
        const Line3 axis = Line3::from_point_and_direction(vec3(0.0f), vec3(0.0f, 0.0f, -1.0f));
        const std::vector<Frame> orbit = builder.orbit_frames(axis, vec3(0.0f));
        const float diagonal = builder.scene_box().diagonal_length();
        require(static_cast<int>(orbit.size()) == builder.keyframes_per_loop() * builder.num_loops() + 1,
                "the orbit does not cover the requested number of loops");
        require(close_to(orbit.front().position(), orbit.back().position(), 1e-3f), "the orbit does not close");
        for (const Frame &f : orbit) {
            const vec3 pos = f.position();
            require(close_to(pos.z, 0.0f), "a level orbit pose left the horizontal plane");
            require(close_to(std::sqrt(pos.x * pos.x + pos.y * pos.y), diagonal, 1e-2f),
                    "an orbit pose is not at the scene diagonal from the axis");
            require(close_to(f.orientation().rotate(local_up), vec3(0.0f, 0.0f, 1.0f), 1e-3f),
                    "a turntable orbit pose does not have the axis as its up");
            require(close_to(dot(f.orientation().rotate(local_forward), vec3(pos.x, pos.y, 0.0f)), -diagonal, 1e-2f),
                    "an orbit pose does not look at the axis");
        }

        // The two offsets are what lift the turntable off the level plane and pull it back, which
        // is what the shipped defaults use. One object height here is the 10-unit scene extent
        // along the axis.
        builder.set_orbit_vertical_offset_in_object_heights(0.1f);
        builder.set_orbit_radius_offset_in_object_heights(0.5f);
        const Frame raised = builder.orbit_frames(axis, vec3(0.0f)).front();
        require(close_to(raised.position().z, 1.0f), "the vertical offset did not lift the orbit");
        require(close_to(std::sqrt(raised.position().x * raised.position().x +
                               raised.position().y * raised.position().y), diagonal + 5.0f, 1e-2f),
                "the radius offset did not pull the orbit back");
        builder.set_orbit_vertical_offset_in_object_heights(0.0f);
        builder.set_orbit_radius_offset_in_object_heights(0.0f);

        const std::size_t added = builder.generate_orbit(axis, vec3(0.0f));
        require(added == orbit.size(), "generate_orbit did not append the orbit poses");
        require(kfi.number_of_keyframes() == added, "the orbit poses did not reach the interpolator");
        require(camera.scene_radius() >= diagonal, "the camera radius was not grown to hold the orbit");

        // --- flight poses ---------------------------------------------------
        // A track that never turns back: three stations one step apart in X, the third
        // climbing. A track that doubles back (the second station behind the first) makes a
        // default aim parallel to the roll reference, which has no horizon to keep.
        const std::vector<vec3> positions = {vec3(0.0f, 0.0f, 10.0f), vec3(10.0f, 0.0f, 10.0f),
                                             vec3(20.0f, 10.0f, 10.0f)};
        const vec3 world_up(0.0f, 1.0f, 0.0f);
        const std::vector<CameraPathBuilder::FlightStation> flight_stations =
                make_stations(positions, world_up);
        const std::vector<vec3> targets = {vec3(0.0f), vec3(0.0f), vec3(0.0f)};
        const std::vector<Frame> flight = builder.flight_frames(flight_stations, targets, world_up);
        require(flight.size() == positions.size(), "flight_frames dropped stations");
        for (std::size_t i = 0; i < flight.size(); ++i) {
            require(close_to(flight[i].position(), positions[i]), "a flight pose is not at its station");
            require(close_to(flight[i].orientation().rotate(local_forward),
                         normalize(targets[i] - positions[i]), 1e-4f),
                    "a flight pose does not aim at its target");
        }

        // --- the flight's default aim: the ground ahead ---------------------
        // make_stations() lifts by a unit vector, so every station is one unit above its
        // foot. Station i aims at station i+2's foot lowered by that unit; the last two aim
        // as far ahead as the track has, at the last foot.
        const std::vector<Frame> down = builder.flight_frames(flight_stations, {}, world_up);
        require(down.size() == positions.size(), "the default-aimed flight dropped stations");
        for (std::size_t i = 0; i < down.size(); ++i) {
            require(close_to(down[i].position(), positions[i]), "a default-aimed flight pose is not at its station");
            // Station i aims at station i+2's foot lowered by station i's own height above
            // its foot; the last stations aim at the last foot.
            const vec3 ahead_foot = flight_stations[std::min(i + 2, flight_stations.size() - 1)].foot;
            const float height = dot(positions[i] - flight_stations[i].foot, world_up);
            const vec3 target = ahead_foot - world_up * height;
            require(close_to(down[i].orientation().rotate(local_forward), normalize(target - positions[i]), 1e-4f),
                    "a default-aimed flight pose does not look at the ground ahead");
            // The default aim looks down at the ground, so the world up is the roll
            // reference and the camera up is that reference squared up to the view: the
            // horizon stays level without the pose ever being rolled.
            const vec3 view = normalize(target - positions[i]);
            if (cross(view, world_up).length2() < 1e-6f) // looking straight down: no horizon
                continue;
            const vec3 right = normalize(cross(view, world_up));
            const vec3 cam_up = normalize(cross(right, view));
            require(close_to(down[i].orientation().rotate(local_up), cam_up, 1e-4f),
                    "a default-aimed flight pose does not keep the horizon level");
        }
        // The look-ahead is the caller's: one station ahead looks nearer.
        const std::vector<Frame> one_ahead = builder.flight_frames(flight_stations, {}, world_up, 1);
        const vec3 one_ahead_target = flight_stations[1].foot - world_up;
        require(close_to(one_ahead[0].orientation().rotate(local_forward),
                         normalize(one_ahead_target - positions[0]), 1e-4f),
                "the flight look-ahead was ignored");

        // A tilted lift changes only the stations' height above their feet: the aim stays the
        // ground ahead and the roll stays on the supplied up, so the lift cannot bank the
        // flight. The height is measured along the up, not along the lift.
        const vec3 tilted_lift = normalize(vec3(0.0f, 3.0f, 1.0f));
        const std::vector<CameraPathBuilder::FlightStation> tilted_stations =
                make_stations(positions, tilted_lift);
        const std::vector<Frame> tilted_down = builder.flight_frames(tilted_stations, {}, world_up);
        require(tilted_down.size() == positions.size(), "the tilted default-aimed flight dropped stations");
        for (std::size_t i = 0; i < tilted_down.size(); ++i) {
            const vec3 ahead_foot = tilted_stations[std::min(i + 2, tilted_stations.size() - 1)].foot;
            const float height = dot(positions[i] - tilted_stations[i].foot, world_up);
            const vec3 view = normalize(ahead_foot - world_up * height - positions[i]);
            if (cross(view, world_up).length2() < 1e-6f) // looking straight down: no horizon
                continue;
            const vec3 cam_up = normalize(cross(normalize(cross(view, world_up)), view));
            require(close_to(tilted_down[i].orientation().rotate(local_up), cam_up, 1e-4f),
                    "a tilted lift banked the flight");
        }

        // --- re-aiming default poses ----------------------------------------
        kfi.delete_path();
        const std::vector<Frame> default_orbit = builder.orbit_frames(axis, vec3(0.0f));
        const std::vector<vec3> orbit_targets(default_orbit.size(), vec3(0.0f, 0.0f, 3.0f));
        const std::vector<Frame> reaimed = builder.aimed_frames(default_orbit, orbit_targets);
        require(reaimed.size() == default_orbit.size(), "aimed_frames dropped poses");
        for (std::size_t i = 0; i < reaimed.size(); ++i) {
            require(close_to(reaimed[i].position(), default_orbit[i].position()), "an aimed pose moved");
            const vec3 view = normalize(orbit_targets[i] - reaimed[i].position());
            require(close_to(reaimed[i].orientation().rotate(local_forward), view, 1e-4f),
                    "an aimed pose does not look at its target");
            const vec3 roll_up = default_orbit[i].orientation().rotate(local_up);
            require(close_to(reaimed[i].orientation().rotate(vec3(1.0f, 0.0f, 0.0f)),
                         normalize(cross(view, roll_up)), 1e-3f),
                    "an aimed pose lost its own up as the roll reference");
        }
        require(builder.aimed_frames(default_orbit, {vec3(0.0f)}).empty(),
                "aimed_frames accepted unpaired targets");
        require(builder.aimed_frames(default_orbit, orbit_targets, {vec3(0.0f, 1.0f, 0.0f)}).empty(),
                "aimed_frames accepted unpaired roll references");

        // A flight re-aimed level: the supplied roll reference keeps the horizon
        // level even though its default (straight-down) poses have no usable roll.
        std::vector<vec3> level_targets;
        for (const vec3 &p : positions)
            level_targets.emplace_back(100.0f, p.y, p.z);
        const std::vector<vec3> level_ups(positions.size(), vec3(0.0f, 1.0f, 0.0f));
        const std::vector<Frame> level = builder.aimed_frames(down, level_targets, level_ups);
        require(level.size() == positions.size(), "the aimed flight dropped stations");
        for (std::size_t i = 0; i < level.size(); ++i) {
            require(close_to(level[i].orientation().rotate(local_forward), vec3(1.0f, 0.0f, 0.0f), 1e-4f),
                    "an aimed flight pose does not look at its target");
            require(close_to(level[i].orientation().rotate(local_up), vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "an aimed flight pose does not use the supplied roll reference");
        }

        // --- an interactive flight: stations, aim, keyframes -----------------
        // The flight height is relative to the scene height: 4.0 times the 10-unit
        // extent along the path's up (+Y) is 40 world units. The spacing is the scene's, not
        // the flight's, so the 8-unit stroke at 2 world units is four intervals, i.e. 5
        // stations - the same count the walking route gets over it.
        require(close_to(builder.flight_height_in_world_units(), 40.0f), "the flight height in world units is wrong");
        const std::vector<CameraPathBuilder::FlightStation> traced = builder.flight_stations(flat_stroke);
        require(traced.size() == 5, "the flight track was not resampled at the scene spacing");
        for (const auto &s : traced) {
            require(close_to(s.position.y, 35.0f) && close_to(s.position.z, 0.0f),
                    "a flight station is not one flight height above its foot");
            require(close_to(s.foot.y, -5.0f) && close_to(s.foot.z, 0.0f), "a flight station lost its foot");
        }
        // The same equal-step rule as the walking route: both ends are stations and the
        // steps between them are equal.
        require(close_to(traced.front().foot, vec3(-4.0f, -5.0f, 0.0f)) &&
                close_to(traced.back().foot, vec3(4.0f, -5.0f, 0.0f)),
                "the flight track does not start and end on the stroke");
        require(close_to(traced[1].foot, vec3(-2.0f, -5.0f, 0.0f)) &&
                close_to(traced[3].foot, vec3(2.0f, -5.0f, 0.0f)),
                "the flight track is not sampled at equal distances along the stroke");

        // The stations rise along the path's up, whatever the stroke's own heights do.
        // The slope stroke is 11.31 units long, so at 2 world units it is 6 intervals.
        const std::vector<CameraPathBuilder::FlightStation> over_diagonal = builder.flight_stations(diagonal_stroke);
        require(over_diagonal.size() == 7, "the diagonal flight track was not resampled at the scene spacing");
        for (std::size_t i = 0; i < over_diagonal.size(); ++i) {
            require(close_to(over_diagonal[i].position, over_diagonal[i].foot + vec3(0.0f, 1.0f, 0.0f) * 40.0f),
                    "a diagonal flight station does not rise along the path's up");
            require(close_to(over_diagonal[i].foot.y, -5.0f), "a diagonal flight station left its stroke");
        }

        builder.set_flight_height_in_scene_heights(0.0f);
        require(builder.flight_stations(flat_stroke).empty(), "a zero flight height was accepted");
        builder.set_flight_height_in_scene_heights(0.5f);
        require(builder.flight_stations({vec3(0.0f)}).empty(), "a single-point flight track was accepted");

        const std::vector<vec3> aim_stroke = {vec3(0.0f, 0.0f, 10.0f), vec3(10.0f, 0.0f, 10.0f)};
        require(builder.aim_targets({}, 3).empty() && builder.aim_targets(aim_stroke, 0).empty(),
                "an empty aim produced targets");
        const std::vector<vec3> spaced = builder.aim_targets(aim_stroke, 5);
        require(spaced.size() == 5 && close_to(spaced.front(), aim_stroke.front()) &&
                close_to(spaced.back(), aim_stroke.back()) && close_to(spaced[2], vec3(5.0f, 0.0f, 10.0f)),
                "the aim targets are not evenly spaced along the aim stroke");
        const std::vector<vec3> clicked = builder.aim_targets({vec3(1.0f, 2.0f, 3.0f)}, 4);
        require(clicked.size() == 4 && close_to(clicked[3], vec3(1.0f, 2.0f, 3.0f)),
                "a clicked aim does not repeat its point for every station");

        kfi.delete_path();
        // A level flight: the outer stations look across the path's up (+Y), so the roll
        // reference is the camera up exactly. The middle one looks along it and has no
        // horizon to roll against, which is what the guard in the loop below skips.
        const std::vector<vec3> flying = {vec3(-10.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
                                          vec3(10.0f, 10.0f, 0.0f)};
        const std::vector<vec3> aimed_at = {vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 10.0f, 0.0f),
                                            vec3(20.0f, 10.0f, 0.0f)};
        const std::vector<CameraPathBuilder::FlightStation> flying_stations = make_stations(flying, world_up);
        const std::size_t flight_keys = builder.generate_flight(flying_stations, aimed_at);
        require(flight_keys == flying.size(), "generate_flight dropped stations");
        require(kfi.number_of_keyframes() == flight_keys, "the flight poses did not reach the interpolator");
        for (std::size_t i = 0; i < flight_keys; ++i) {
            const Frame f = kfi.keyframe(i);
            require(close_to(f.position(), flying[i]), "a flight keyframe is not at its station");
            require(close_to(f.orientation().rotate(local_forward), normalize(aimed_at[i] - flying[i]), 1e-4f),
                    "a flight keyframe does not aim at its target");
            // The roll reference is the path's up; the camera up is that reference squared
            // up to the view, so a pose that looks down is not rolled.
            const vec3 flight_view = normalize(aimed_at[i] - flying[i]);
            const vec3 flight_up = builder.ground_up_direction();
            if (cross(flight_view, flight_up).length2() < 1e-6f)
                continue;
            const vec3 flight_cam_up = normalize(cross(normalize(cross(flight_view, flight_up)), flight_view));
            require(close_to(f.orientation().rotate(local_up), flight_cam_up, 1e-4f),
                    "a flight keyframe does not use the path's up as its roll reference");
        }
        kfi.delete_path();
        require(builder.generate_flight(flying_stations, {}) == flying.size(),
                "the default-aimed flight did not reach the interpolator");
        for (std::size_t i = 0; i < flying.size(); ++i) {
            const Frame f = kfi.keyframe(i);
            const vec3 view = f.orientation().rotate(local_forward);
            const vec3 up_ref = builder.ground_up_direction();
            if (cross(view, up_ref).length2() < 1e-6f) // looking straight down: no horizon
                continue;
            const vec3 cam_up = normalize(cross(normalize(cross(view, up_ref)), view));
            require(close_to(f.orientation().rotate(local_up), cam_up, 1e-4f),
                    "a generated default-aimed flight pose does not keep the path's up as its roll");
        }
        require(builder.generate_flight(flying_stations, {vec3(0.0f)}) == 0,
                "generate_flight accepted unpaired aims");

        // --- reproducible sources and their settings influence -----------------
        auto same_frames = [](const std::vector<Frame> &a, const std::vector<Frame> &b) {
            if (a.size() != b.size())
                return false;
            for (std::size_t i = 0; i < a.size(); ++i) {
                if (!close_to(a[i].position(), b[i].position(), 1e-4f) ||
                    !close_to(a[i].orientation().rotate(local_up), b[i].orientation().rotate(local_up), 1e-4f) ||
                    !close_to(a[i].orientation().rotate(local_forward),
                              b[i].orientation().rotate(local_forward), 1e-4f))
                    return false;
            }
            return true;
        };

        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Y);
        builder.set_keyframe_spacing_in_scene_heights(0.2f);
        builder.set_follow_ground_slopes(true);
        builder.set_flight_height_in_scene_heights(0.5f);

        // A Walk keeps its strokes separate and concatenates one generated route per stroke.
        kfi.delete_path();
        builder.generate_ground_route(flat_stroke);
        builder.generate_ground_route(diagonal_stroke);
        std::vector<Frame> expected_walk;
        for (std::size_t i = 0; i < kfi.number_of_keyframes(); ++i)
            expected_walk.push_back(kfi.keyframe(i));
        const CameraPathBuilder::Source walk = CameraPathBuilder::Source::walk({flat_stroke, diagonal_stroke});
        const std::vector<Frame> rebuilt_walk = builder.frames_of(walk);
        require(same_frames(rebuilt_walk, expected_walk),
                "frames_of did not reproduce the walking routes");

        // A Flight joins its strokes before resampling, so the gap between two drags is flown
        // rather than treated as a break in the route.
        const std::vector<vec3> first_half(flat_stroke.begin(), flat_stroke.begin() + 5);
        const std::vector<vec3> second_half(flat_stroke.begin() + 5, flat_stroke.end());
        CameraPathBuilder::Source flight_source;
        flight_source.kind = CameraPathBuilder::Source::Kind::Flight;
        flight_source.strokes = {first_half, second_half};
        const std::vector<CameraPathBuilder::FlightStation> source_stations =
                builder.flight_stations(flat_stroke);
        const std::vector<Frame> expected_flight =
                builder.flight_frames(source_stations, {}, builder.ground_up_direction());
        require(same_frames(builder.frames_of(flight_source), expected_flight),
                "frames_of did not reproduce the joined flight track");

        // An Orbit stores the picked anchor and re-resolves its axis from the current Up rule.
        const vec3 orbit_anchor(1.0f, -5.0f, 2.0f);
        const vec3 orbit_center(0.0f, 1.0f, 0.0f);
        const CameraPathBuilder::Source orbit_source =
                CameraPathBuilder::Source::orbit(orbit_anchor, orbit_center);
        const Line3 source_axis = Line3::from_point_and_direction(
                orbit_anchor, -builder.ground_up_direction());
        std::vector<Frame> expected_orbit = builder.orbit_frames(source_axis, orbit_center);
        expected_orbit.pop_back(); // the interpolator cannot store the closing duplicate
        require(same_frames(builder.frames_of(orbit_source), expected_orbit),
                "frames_of did not reproduce the orbit from its anchor");

        // Raw aims are part of the source, not a one-off edit of the generated orientations.
        CameraPathBuilder::Source aimed_walk = walk;
        aimed_walk.aims = {vec3(0.0f, 0.0f, 3.0f)};
        const std::vector<vec3> walk_targets =
                builder.aim_targets(aimed_walk.aims, rebuilt_walk.size());
        require(same_frames(builder.frames_of(aimed_walk),
                            builder.aimed_frames(rebuilt_walk, walk_targets)),
                "frames_of did not keep the walk's raw aim");
        CameraPathBuilder::Source aimed_flight = flight_source;
        aimed_flight.aims = {vec3(0.0f, 0.0f, 10.0f), vec3(10.0f, 0.0f, 10.0f)};
        const std::vector<vec3> flight_targets =
                builder.aim_targets(aimed_flight.aims, expected_flight.size());
        const std::vector<vec3> flight_roll_ups(expected_flight.size(), builder.ground_up_direction());
        require(same_frames(builder.frames_of(aimed_flight),
                            builder.aimed_frames(expected_flight, flight_targets, flight_roll_ups)),
                "frames_of did not keep the flight's raw aim and roll reference");
        CameraPathBuilder::Source aimed_orbit = orbit_source;
        aimed_orbit.aims = {vec3(-2.0f, 0.0f, 3.0f), vec3(2.0f, 0.0f, 3.0f)};
        const std::vector<vec3> orbit_aims =
                builder.aim_targets(aimed_orbit.aims, expected_orbit.size());
        require(same_frames(builder.frames_of(aimed_orbit),
                            builder.aimed_frames(expected_orbit, orbit_aims)),
                "frames_of did not keep the orbit's raw aim over its stored poses");

        const CameraPathBuilder::Settings built = builder.settings();
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "unchanged settings reported a stale source");
        require(!builder.settings_changed_for(CameraPathBuilder::Source{}, built),
                "a source with no mode reported a settings change");

        builder.set_eye_height_in_scene_heights(built.eye_height_in_scene_heights + 0.1f);
        require(builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "eye height influenced a mode other than Walk");
        builder.set_eye_height_in_scene_heights(built.eye_height_in_scene_heights);

        builder.set_flight_height_in_scene_heights(built.flight_height_in_scene_heights + 0.1f);
        require(!builder.settings_changed_for(walk, built) &&
                builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "flight height influenced a mode other than Flight");
        builder.set_flight_height_in_scene_heights(built.flight_height_in_scene_heights);

        builder.set_keyframe_spacing_in_scene_heights(built.keyframe_spacing_in_scene_heights + 0.1f);
        require(builder.settings_changed_for(walk, built) &&
                builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "keyframe spacing did not influence exactly the two ground tracks");
        builder.set_keyframe_spacing_in_scene_heights(built.keyframe_spacing_in_scene_heights);

        builder.set_follow_ground_slopes(!built.follow_ground_slopes);
        require(builder.settings_changed_for(walk, built) &&
                builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "the slope rule influenced a mode other than Walk or Flight");
        builder.set_follow_ground_slopes(built.follow_ground_slopes);

        builder.set_ground_up_axis(CameraPathBuilder::GroundUpAxis::Z);
        require(builder.settings_changed_for(walk, built) &&
                builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "the Up rule did not influence every generated mode");
        builder.set_ground_up_axis(built.ground_up_axis);

        builder.set_orbit_radius_offset_in_object_heights(
                built.orbit_radius_offset_in_object_heights + 0.1f);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "orbit radius influenced a mode other than Orbit");
        builder.set_orbit_radius_offset_in_object_heights(built.orbit_radius_offset_in_object_heights);

        builder.set_orbit_vertical_offset_in_object_heights(
                built.orbit_vertical_offset_in_object_heights + 0.1f);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "orbit vertical offset influenced a mode other than Orbit");
        builder.set_orbit_vertical_offset_in_object_heights(built.orbit_vertical_offset_in_object_heights);

        builder.set_pitch_degrees(built.pitch_degrees + 1.0f);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "orbit pitch influenced a mode other than Orbit");
        builder.set_pitch_degrees(built.pitch_degrees);

        builder.set_keyframes_per_loop(built.keyframes_per_loop + 1);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "orbit loop density influenced a mode other than Orbit");
        builder.set_keyframes_per_loop(built.keyframes_per_loop);

        builder.set_num_loops(built.num_loops + 1);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                builder.settings_changed_for(orbit_source, built),
                "orbit loop count influenced a mode other than Orbit");
        builder.set_num_loops(built.num_loops);
        require(!builder.settings_changed_for(walk, built) &&
                !builder.settings_changed_for(flight_source, built) &&
                !builder.settings_changed_for(orbit_source, built),
                "returning a setting to its built value did not clear the change");

        const std::vector<Frame> walk_built = builder.frames_of(walk);
        const std::vector<Frame> flight_built = builder.frames_of(flight_source);
        const std::vector<Frame> orbit_built = builder.frames_of(orbit_source);

        builder.set_eye_height_in_scene_heights(built.eye_height_in_scene_heights + 0.1f);
        require(!same_frames(walk_built, builder.frames_of(walk)) &&
                same_frames(flight_built, builder.frames_of(flight_source)) &&
                same_frames(orbit_built, builder.frames_of(orbit_source)),
                "changing eye height did not regenerate exactly the Walk poses");
        builder.set_eye_height_in_scene_heights(built.eye_height_in_scene_heights);

        builder.set_keyframe_spacing_in_scene_heights(built.keyframe_spacing_in_scene_heights + 0.1f);
        const std::vector<Frame> respaced_walk = builder.frames_of(walk);
        require(!same_frames(walk_built, respaced_walk) &&
                !same_frames(flight_built, builder.frames_of(flight_source)) &&
                same_frames(orbit_built, builder.frames_of(orbit_source)),
                "changing spacing did not regenerate exactly the ground-track poses");
        const std::vector<vec3> respaced_aims =
                builder.aim_targets(aimed_walk.aims, respaced_walk.size());
        require(same_frames(builder.frames_of(aimed_walk),
                            builder.aimed_frames(respaced_walk, respaced_aims)),
                "raw aims were not resampled after the generated pose count changed");
        builder.set_keyframe_spacing_in_scene_heights(built.keyframe_spacing_in_scene_heights);

        builder.set_num_loops(built.num_loops + 1);
        require(same_frames(walk_built, builder.frames_of(walk)) &&
                same_frames(flight_built, builder.frames_of(flight_source)) &&
                !same_frames(orbit_built, builder.frames_of(orbit_source)),
                "changing loop count did not regenerate exactly the Orbit poses");
        builder.set_num_loops(built.num_loops);
    });
}
