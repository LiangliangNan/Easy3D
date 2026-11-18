// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <memory>
#include <vector>

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

bool near(float a, float b, float tolerance = 1e-3f) { return std::abs(a - b) <= tolerance; }

bool near(const vec3 &a, const vec3 &b, float tolerance = 1e-3f) {
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

// Camera looking straight down, so the walker's ups oppose the view direction and
// the camera up is the horizontal -Z (an exact orthonormal pose).
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
        // from; the default aim points back down at that foot.
        auto make_stations = [](const std::vector<vec3> &positions, const vec3 &lift_up) {
            std::vector<CameraPathBuilder::FlightStation> stations;
            stations.reserve(positions.size());
            for (const vec3 &p : positions)
                stations.push_back(CameraPathBuilder::FlightStation{p, p - lift_up});
            return stations;
        };
        // Fix the spacing the route and flight counts below are written against instead of
        // depending on the default: one character height is one eye height, 2 world units.
        builder.set_keyframe_spacing_in_character_heights(1.0f);
        require(near(builder.keyframe_spacing_in_character_heights(), 1.0f),
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
        require(near(builder.scene_box().min_point(), vec3(-5.0f)) &&
                near(builder.scene_box().max_point(), vec3(5.0f)),
                "the scene box does not match the model bounds");
        require(near(builder.ground_up_direction(), vec3(0.0f, 0.0f, -1.0f)),
                "the fallback up is not the view's up direction");
        // 0.2 scene heights along the up direction, whose scene extent is 10.
        require(near(builder.eye_height_in_world_units(), 2.0f), "the eye height in world units is wrong");

        // --- a flat route --------------------------------------------------
        std::vector<vec3> flat_stroke;
        for (int i = 0; i <= 8; ++i)
            flat_stroke.emplace_back(static_cast<float>(i) - 4.0f, -5.0f, 0.0f);
        const std::vector<vec3> flat_ups(flat_stroke.size(), vec3(0.0f, 1.0f, 0.0f));
        const std::size_t flat_count = builder.generate_ground_route(flat_stroke, flat_ups);
        require(flat_count == 5, "the flat route was not resampled at the character spacing");
        require(kfi.number_of_keyframes() == flat_count, "the route poses did not reach the interpolator");
        for (std::size_t i = 0; i < flat_count; ++i) {
            const Frame f = kfi.keyframe(i);
            require(near(f.position().y, -3.0f) && near(f.position().z, 0.0f),
                    "a flat route pose is not one eye height above its foot");
            require(near(f.orientation().rotate(local_up), vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "a flat route pose does not stand upright");
            require(near(f.orientation().rotate(local_forward), vec3(1.0f, 0.0f, 0.0f), 1e-4f),
                    "a flat route pose does not aim along the route");
        }

        // --- a tilted (terrain) route, with face winding both ways ---------
        const float angle = geom::to_radians(30.0f);
        const vec3 ramp_up(0.0f, std::cos(angle), -std::sin(angle));
        std::vector<vec3> ramp_ups;
        for (std::size_t i = 0; i < flat_stroke.size(); ++i) // flip every other normal
            ramp_ups.push_back(i % 2 == 0 ? ramp_up : -ramp_up);
        const std::size_t ramp_count = builder.generate_ground_route(flat_stroke, ramp_ups);
        require(ramp_count == 5, "the tilted route was not resampled at the character spacing");
        for (std::size_t i = ramp_count; i < flat_count + ramp_count; ++i) {
            const Frame f = kfi.keyframe(i);
            const vec3 pose_up = f.orientation().rotate(local_up);
            // Winding was mixed, so every pose must stand on the camera-facing side.
            require(near(pose_up, ramp_up, 1e-3f), "a tilted route pose does not follow the walker's up");
            const vec3 foot = f.position() - pose_up * builder.eye_height_in_world_units();
            require(near(foot.y, -5.0f), "a tilted route pose does not rise from its stroke point");
            const vec3 heading = f.orientation().rotate(local_forward);
            require(near(dot(heading, pose_up), 0.0f, 1e-4f), "the heading is not level on the slope");
            require(near(dot(heading, vec3(1.0f, 0.0f, 0.0f)), 1.0f, 1e-3f),
                    "the heading does not follow the route");
        }

        // --- degenerate inputs ---------------------------------------------
        require(builder.generate_ground_route({vec3(0.0f)}, {}) == 0, "a single-point stroke was accepted");
        require(builder.generate_ground_route(flat_stroke, {ramp_up}) == 0,
                "unpaired foot up directions were accepted");
        builder.set_eye_height_in_scene_heights(0.0f);
        require(builder.generate_ground_route(flat_stroke, flat_ups) == 0, "a zero eye height was accepted");
        builder.set_eye_height_in_scene_heights(0.2f);
        require(builder.ground_frames({vec3(0.0f)}, {vec3(0.0f, 1.0f, 0.0f)}, 1.0f).empty(),
                "ground_frames accepted a single foot");
        require(builder.ground_frames({vec3(0.0f), vec3(1.0f)}, {vec3(0.0f, 1.0f, 0.0f)}, 1.0f).empty(),
                "ground_frames accepted unpaired directions");
        require(builder.ground_frames({vec3(0.0f), vec3(1.0f)}, {vec3(0.0f), vec3(0.0f, 1.0f, 0.0f)}, 1.0f).empty(),
                "ground_frames accepted a degenerate direction");
        require(builder.flight_frames(make_stations({vec3(0.0f), vec3(1.0f)}, vec3(0.0f, 1.0f, 0.0f)),
                                      {vec3(1.0f)}, vec3(0.0f, 1.0f, 0.0f)).empty(),
                "flight_frames accepted unpaired targets");

        // --- turntable orbit -----------------------------------------------
        kfi.delete_path();
        const Line3 axis = Line3::from_point_and_direction(vec3(0.0f), vec3(0.0f, 0.0f, -1.0f));
        const std::vector<Frame> orbit = builder.orbit_frames(axis, vec3(0.0f));
        const float diagonal = builder.scene_box().diagonal_length();
        require(static_cast<int>(orbit.size()) == builder.keyframes_per_loop() * builder.num_loops() + 1,
                "the orbit does not cover the requested number of loops");
        require(near(orbit.front().position(), orbit.back().position(), 1e-3f), "the orbit does not close");
        for (const Frame &f : orbit) {
            const vec3 pos = f.position();
            require(near(pos.z, 0.0f), "a level orbit pose left the horizontal plane");
            require(near(std::sqrt(pos.x * pos.x + pos.y * pos.y), diagonal, 1e-2f),
                    "an orbit pose is not at the scene diagonal from the axis");
            require(near(f.orientation().rotate(local_up), vec3(0.0f, 0.0f, 1.0f), 1e-3f),
                    "a turntable orbit pose does not have the axis as its up");
            require(near(dot(f.orientation().rotate(local_forward), vec3(pos.x, pos.y, 0.0f)), -diagonal, 1e-2f),
                    "an orbit pose does not look at the axis");
        }

        const std::size_t added = builder.generate_orbit(axis, vec3(0.0f));
        require(added == orbit.size(), "generate_orbit did not append the orbit poses");
        require(kfi.number_of_keyframes() == added, "the orbit poses did not reach the interpolator");
        require(camera.scene_radius() >= diagonal, "the camera radius was not grown to hold the orbit");

        // --- flight poses ---------------------------------------------------
        const std::vector<vec3> positions = {vec3(0.0f, 0.0f, 10.0f), vec3(10.0f, 0.0f, 10.0f),
                                             vec3(0.0f, 10.0f, 10.0f)};
        const vec3 world_up(0.0f, 1.0f, 0.0f);
        const std::vector<CameraPathBuilder::FlightStation> flight_stations =
                make_stations(positions, world_up);
        const std::vector<vec3> targets = {vec3(0.0f), vec3(0.0f), vec3(0.0f)};
        const std::vector<Frame> flight = builder.flight_frames(flight_stations, targets, world_up);
        require(flight.size() == positions.size(), "flight_frames dropped stations");
        for (std::size_t i = 0; i < flight.size(); ++i) {
            require(near(flight[i].position(), positions[i]), "a flight pose is not at its station");
            require(near(flight[i].orientation().rotate(local_forward),
                         normalize(targets[i] - positions[i]), 1e-4f),
                    "a flight pose does not aim at its target");
        }

        // --- the flight's default aim: down at each station's foot -----------
        const std::vector<Frame> down = builder.flight_frames(flight_stations, {}, world_up);
        require(down.size() == positions.size(), "the default-aimed flight dropped stations");
        for (std::size_t i = 0; i < down.size(); ++i) {
            require(near(down[i].position(), positions[i]), "a default-aimed flight pose is not at its station");
            require(near(down[i].orientation().rotate(local_forward), -world_up, 1e-4f),
                    "a straight lift does not default to a straight-down aim");
        }

        // A tilted lift: the default aim is the reverse of the lift, i.e. the slope's
        // normal under each station, not one global down.
        const std::vector<Frame> tilted_down =
                builder.flight_frames(make_stations(positions, ramp_up), {}, world_up);
        require(tilted_down.size() == positions.size(), "the tilted default-aimed flight dropped stations");
        for (std::size_t i = 0; i < tilted_down.size(); ++i) {
            require(near(tilted_down[i].orientation().rotate(local_forward), -ramp_up, 1e-4f),
                    "a tilted lift does not default to the reverse of the lift");
        }

        // --- re-aiming default poses ----------------------------------------
        kfi.delete_path();
        const std::vector<Frame> default_orbit = builder.orbit_frames(axis, vec3(0.0f));
        const std::vector<vec3> orbit_targets(default_orbit.size(), vec3(0.0f, 0.0f, 3.0f));
        const std::vector<Frame> reaimed = builder.aimed_frames(default_orbit, orbit_targets);
        require(reaimed.size() == default_orbit.size(), "aimed_frames dropped poses");
        for (std::size_t i = 0; i < reaimed.size(); ++i) {
            require(near(reaimed[i].position(), default_orbit[i].position()), "an aimed pose moved");
            const vec3 view = normalize(orbit_targets[i] - reaimed[i].position());
            require(near(reaimed[i].orientation().rotate(local_forward), view, 1e-4f),
                    "an aimed pose does not look at its target");
            const vec3 roll_up = default_orbit[i].orientation().rotate(local_up);
            require(near(reaimed[i].orientation().rotate(vec3(1.0f, 0.0f, 0.0f)),
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
            require(near(level[i].orientation().rotate(local_forward), vec3(1.0f, 0.0f, 0.0f), 1e-4f),
                    "an aimed flight pose does not look at its target");
            require(near(level[i].orientation().rotate(local_up), vec3(0.0f, 1.0f, 0.0f), 1e-4f),
                    "an aimed flight pose does not use the supplied roll reference");
        }

        // --- an interactive flight: stations, aim, keyframes -----------------
        // The flight height is relative to the scene height: 0.5 of the 10-unit
        // extent along the view's up is 5 world units. The stroke spans 8 units, so
        // the spacing (1 x height) resamples it to 3 stations.
        require(near(builder.flight_height_in_world_units(), 5.0f), "the flight height in world units is wrong");
        const std::vector<CameraPathBuilder::FlightStation> traced = builder.flight_stations(flat_stroke, flat_ups);
        require(traced.size() == 3, "the flight track was not resampled at the flight spacing");
        for (const auto &s : traced) {
            require(near(s.position.y, 0.0f) && near(s.position.z, 0.0f),
                    "a flight station is not one flight height above its foot");
            require(near(s.foot.y, -5.0f) && near(s.foot.z, 0.0f), "a flight station lost its foot");
        }

        const std::vector<CameraPathBuilder::FlightStation> over_ramp = builder.flight_stations(flat_stroke, ramp_ups);
        require(over_ramp.size() == traced.size(), "the tilted flight track changed the station count");
        for (std::size_t i = 0; i < over_ramp.size(); ++i) {
            require(near(over_ramp[i].foot, traced[i].foot), "the tilted flight track moved its feet");
            require(near(over_ramp[i].position, over_ramp[i].foot + ramp_up * 5.0f),
                    "a tilted flight station does not rise along the surface normal under its foot");
        }

        builder.set_flight_height_in_scene_heights(0.0f);
        require(builder.flight_stations(flat_stroke, flat_ups).empty(), "a zero flight height was accepted");
        builder.set_flight_height_in_scene_heights(0.5f);
        require(builder.flight_stations({vec3(0.0f)}, {}).empty(), "a single-point flight track was accepted");
        require(builder.flight_stations(flat_stroke, {ramp_up}).empty(), "unpaired flight ups were accepted");

        const std::vector<vec3> aim_stroke = {vec3(0.0f, 0.0f, 10.0f), vec3(10.0f, 0.0f, 10.0f)};
        require(builder.aim_targets({}, 3).empty() && builder.aim_targets(aim_stroke, 0).empty(),
                "an empty aim produced targets");
        const std::vector<vec3> spaced = builder.aim_targets(aim_stroke, 5);
        require(spaced.size() == 5 && near(spaced.front(), aim_stroke.front()) &&
                near(spaced.back(), aim_stroke.back()) && near(spaced[2], vec3(5.0f, 0.0f, 10.0f)),
                "the aim targets are not evenly spaced along the aim stroke");
        const std::vector<vec3> clicked = builder.aim_targets({vec3(1.0f, 2.0f, 3.0f)}, 4);
        require(clicked.size() == 4 && near(clicked[3], vec3(1.0f, 2.0f, 3.0f)),
                "a clicked aim does not repeat its point for every station");

        kfi.delete_path();
        // A level flight: every view direction is perpendicular to the view's up
        // (0,0,-1), so the roll reference is the camera up exactly.
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
            require(near(f.position(), flying[i]), "a flight keyframe is not at its station");
            require(near(f.orientation().rotate(local_forward), normalize(aimed_at[i] - flying[i]), 1e-4f),
                    "a flight keyframe does not aim at its target");
            require(near(f.orientation().rotate(local_up), vec3(0.0f, 0.0f, -1.0f), 1e-4f),
                    "a flight keyframe does not use the view's up as its roll reference");
        }
        kfi.delete_path();
        require(builder.generate_flight(flying_stations, {}) == flying.size(),
                "the default-aimed flight did not reach the interpolator");
        for (std::size_t i = 0; i < flying.size(); ++i)
            require(near(kfi.keyframe(i).orientation().rotate(local_forward), -world_up, 1e-4f),
                    "a generated default-aimed flight pose does not look down at its foot");
        require(builder.generate_flight(flying_stations, {vec3(0.0f)}) == 0,
                "generate_flight accepted unpaired aims");
    });
}
