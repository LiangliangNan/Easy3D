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
        require(builder.flight_frames({vec3(0.0f), vec3(1.0f)}, {vec3(1.0f)}, vec3(0.0f, 1.0f, 0.0f)).empty(),
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
        const std::vector<vec3> targets = {vec3(0.0f), vec3(0.0f), vec3(0.0f)};
        const std::vector<Frame> flight = builder.flight_frames(positions, targets, vec3(0.0f, 1.0f, 0.0f));
        require(flight.size() == positions.size(), "flight_frames dropped stations");
        for (std::size_t i = 0; i < flight.size(); ++i) {
            require(near(flight[i].position(), positions[i]), "a flight pose is not at its station");
            require(near(flight[i].orientation().rotate(local_forward),
                         normalize(targets[i] - positions[i]), 1e-4f),
                    "a flight pose does not aim at its target");
        }
    });
}
