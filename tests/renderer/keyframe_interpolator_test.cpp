// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>

#include "../test.h"

using namespace easy3d;
using easy3d::test::require;

namespace {

constexpr float kTolerance = 1e-4f;

bool close_to(float a, float b, float tolerance = kTolerance) { return std::abs(a - b) <= tolerance; }

bool close_to(const vec3 &a, const vec3 &b, float tolerance = kTolerance) {
    return distance(a, b) <= tolerance;
}

// A path along +x with one keyframe per second, all poses keeping the identity
// orientation, so a frame's position alone says where in time the camera is.
void make_path(KeyFrameInterpolator &kfi, int count, float step) {
    for (int i = 0; i < count; ++i) {
        Frame frame;
        frame.set_position_and_orientation(vec3(static_cast<float>(i), 0.0f, 0.0f), quat());
        require(kfi.add_keyframe(frame, static_cast<float>(i) * step), "a keyframe was rejected");
    }
}

// The interpolated frames as positions, with the per-frame time step the samples use.
std::vector<vec3> sample_positions(KeyFrameInterpolator &kfi, float &interval) {
    interval = kfi.interpolation_speed() * static_cast<float>(kfi.interpolation_period()) / 1000.0f;
    const std::vector<Frame> &frames = kfi.interpolate();
    std::vector<vec3> positions;
    positions.reserve(frames.size());
    for (const Frame &f : frames)
        positions.push_back(f.position());
    return positions;
}

// The sample at a given path time (the interpolator samples every `interval`).
vec3 position_at(const std::vector<vec3> &positions, float interval, float time) {
    const auto index = static_cast<std::size_t>(std::lround(time / interval));
    if (index >= positions.size())
        throw std::runtime_error("the path is shorter than the requested time");
    return positions[index];
}

} // namespace

int main() {
    return easy3d::test::run("keyframe interpolator", [] {
        Camera camera;
        KeyFrameInterpolator kfi(camera.frame());
        // 100 fps: the samples land on the tenths of a second these checks use.
        kfi.set_frame_rate(100);

        // --- dwell time ---------------------------------------------------
        make_path(kfi, 3, 1.0f);
        float interval = 0.0f;
        std::vector<vec3> positions = sample_positions(kfi, interval);
        // 3 keyframes 1 s apart, no dwell: 2 s of travel.
        require(positions.size() == 201, "the path without dwell has the wrong sample count");
        // The camera accelerates from rest at the path's start, so it is short of the halfway
        // point at the halfway time; it must be moving and past the first keyframe.
        require(position_at(positions, interval, 0.5f).x > 0.05f &&
                        position_at(positions, interval, 0.5f).x < 0.5f,
                "the path does not move through its first segment");

        // A 1 s dwell at the middle keyframe holds its pose and lengthens the path.
        kfi.set_keyframe_dwell(1, 1.0f);
        require(close_to(kfi.keyframe_dwell(1), 1.0f), "the dwell time was not stored");
        positions = sample_positions(kfi, interval);
        require(positions.size() == 301, "the dwell time did not lengthen the path");
        for (float t = 1.0f; t <= 1.95f; t += 0.25f) {
            const float x = position_at(positions, interval, t).x;
            if (!close_to(x, 1.0f))
                throw std::runtime_error("the camera moved during a dwell (t = " + std::to_string(t) +
                                         ", x = " + std::to_string(x) + ", sample = " +
                                         std::to_string(std::lround(t / interval)) + ", n = " +
                                         std::to_string(positions.size()) + ", interval = " +
                                         std::to_string(interval) + ", t[0] = " +
                                         std::to_string(positions[0].x) + ", t[1] = " +
                                         std::to_string(positions[1].x) + ", t[100] = " +
                                         std::to_string(positions[100].x) + ")");
        }
        // The travel after the dwell starts where the keyframe is and ends on the last one.
        require(position_at(positions, interval, 2.5f).x > 1.05f,
                "the travel after a dwell does not start at the dwell's end");
        require(close_to(positions.back(), vec3(2.0f, 0.0f, 0.0f)), "the path does not end on the last keyframe");

        // Two keyframes: the end dwells hold their poses too.
        kfi.delete_path();
        make_path(kfi, 2, 1.0f);
        kfi.set_keyframe_dwell(0, 1.0f);
        kfi.set_keyframe_dwell(1, 1.0f);
        positions = sample_positions(kfi, interval);
        require(positions.size() == 301, "the two-keyframe path ignores its dwell times");

        require(close_to(position_at(positions, interval, 0.5f), vec3(0.0f, 0.0f, 0.0f)),
                "the first keyframe does not hold its pose during its dwell");
        require(close_to(position_at(positions, interval, 2.5f), vec3(1.0f, 0.0f, 0.0f)),
                "the last keyframe does not hold its pose during its dwell");

        // --- speed profile ------------------------------------------------
        // The camera runs at one constant speed, with a ramp at each end of every run of travel:
        //   - the first keyframe accelerates from rest up to that speed;
        //   - the last keyframe, and every keyframe that holds a dwell, decelerates to rest;
        //   - the keyframe after a dwell accelerates from rest again.
        // A keyframe that holds no dwell is passed at the constant speed (never a stop), and
        // every dwell is a full stop of exactly its own length.
        const auto per_frame_steps = [&](const std::vector<vec3> &samples) {
            std::vector<float> steps;
            steps.reserve(samples.size());
            for (std::size_t i = 1; i < samples.size(); ++i)
                steps.push_back(distance(samples[i], samples[i - 1]));
            return steps;
        };
        // The step covering the frame that ends at path time t (the sample index nearest t, and
        // at least 1, so t = 0 reads the very first frame's step).
        const auto step_at = [&](const std::vector<float> &steps, float t) {
            const auto index = std::max<std::size_t>(1, static_cast<std::size_t>(std::lround(t / interval)));
            if (index == 0 || index > steps.size())
                throw std::runtime_error("no frame step at path time " + std::to_string(t));
            return steps[index - 1];
        };

        // A path with a dwell at its middle keyframe: 4 keyframes 1 s apart, x = 0, 1, 2, 3,
        // and a 1 s dwell at keyframe 1 (x = 1).
        // Timeline: travel 0..1 s (x 0 -> 1), dwell 1..2 s at x = 1, travel 2..3 s (x 1 -> 2),
        // travel 3..4 s (x 2 -> 3). The keyframe at x = 2 holds no dwell, so the camera only
        // passes it. The runs of travel are 0..1 s (x 0 -> 1) and 2..4 s (x 1 -> 3).
        kfi.delete_path();
        require(kfi.number_of_keyframes() == 0, "delete_path left keyframes behind");
        make_path(kfi, 4, 1.0f);
        kfi.set_keyframe_dwell(1, 1.0f);
        positions = sample_positions(kfi, interval);
        require(positions.size() == 401, "the path with one dwell has the wrong sample count");
        {
            const std::vector<float> steps = per_frame_steps(positions);
            // The cruise speed: the pace of the first run, in its middle.
            const float cruise = step_at(steps, 0.5f);
            // The first keyframe accelerates from rest: the first frame is a small fraction of
            // the pace and every frame up to the cruise is longer than the one before it.
            if (!(step_at(steps, 0.01f) < 0.15f * cruise))
                throw std::runtime_error("the path does not start from rest (first frame " +
                                         std::to_string(step_at(steps, 0.01f)) + " against " +
                                         std::to_string(cruise) + ")");
            for (int i = 1; i <= 15; ++i) {
                const float before = step_at(steps, static_cast<float>(i - 1) / 100.0f);
                const float after = step_at(steps, static_cast<float>(i) / 100.0f);
                if (after < before - 1e-5f)
                    throw std::runtime_error("the path does not accelerate away from its first keyframe (t " +
                                             std::to_string(static_cast<float>(i) / 100.0f) + ": " +
                                             std::to_string(before) + " -> " + std::to_string(after) + ")");
            }
            // It reaches the cruise speed well before the dwell and holds it.
            for (float t : {0.3f, 0.5f, 0.7f})
                if (!close_to(step_at(steps, t), cruise, 0.05f * cruise))
                    throw std::runtime_error("the path does not cruise before the dwell (t " +
                                             std::to_string(t) + ", step " +
                                             std::to_string(step_at(steps, t)) + " against " +
                                             std::to_string(cruise) + ")");
            // Into the dwell: the frames shorten, frame by frame, to (almost) nothing.
            for (int i = 85; i < 100; ++i) {
                const float before = step_at(steps, static_cast<float>(i - 1) / 100.0f);
                const float after = step_at(steps, static_cast<float>(i) / 100.0f);
                if (after > before + 1e-4f)
                    throw std::runtime_error("the path does not decelerate into the dwell (t " +
                                             std::to_string(static_cast<float>(i) / 100.0f) + ": " +
                                             std::to_string(before) + " -> " + std::to_string(after) + ")");
            }
            if (!(step_at(steps, 0.99f) < 0.4f * cruise))
                throw std::runtime_error("the path does not come to rest at the dwell (last frame " +
                                         std::to_string(step_at(steps, 0.99f)) + " against " +
                                         std::to_string(cruise) + ")");
            // The dwell is a full stop of exactly its own length.
            for (float t = 1.0f; t <= 2.0f; t += 0.25f) {
                if (distance(position_at(positions, interval, t), vec3(1.0f, 0.0f, 0.0f)) > 1e-4f)
                    throw std::runtime_error("the camera moved during the dwell (t " + std::to_string(t) + ")");
            }
            // Out of the dwell the camera accelerates from rest again, back to the same speed.
            if (!(step_at(steps, 2.01f) < 0.15f * cruise))
                throw std::runtime_error("the path does not leave the dwell from rest (first frame " +
                                         std::to_string(step_at(steps, 2.01f)) + " against " +
                                         std::to_string(cruise) + ")");
            for (int i = 201; i <= 215; ++i) {
                const float before = step_at(steps, static_cast<float>(i - 1) / 100.0f);
                const float after = step_at(steps, static_cast<float>(i) / 100.0f);
                if (after < before - 1e-5f)
                    throw std::runtime_error("the path does not accelerate out of the dwell (t " +
                                             std::to_string(static_cast<float>(i) / 100.0f) + ": " +
                                             std::to_string(before) + " -> " + std::to_string(after) + ")");
            }
            // The keyframe at x = 2 holds no dwell: it is passed at the cruise speed, so the
            // frames either side of it are at the pace and the one that crosses it is not short.
            for (float t : {2.5f, 3.1f})
                if (!close_to(step_at(steps, t), cruise, 0.05f * cruise))
                    throw std::runtime_error("the path does not pass a keyframe without a dwell at its cruise "
                                             "speed (t " + std::to_string(t) + ", step " +
                                             std::to_string(step_at(steps, t)) + " against " +
                                             std::to_string(cruise) + ")");
            if (!(step_at(steps, 3.0f) > 0.9f * cruise))
                throw std::runtime_error("the path stops at a keyframe that holds no dwell (step " +
                                         std::to_string(step_at(steps, 3.0f)) + " against " +
                                         std::to_string(cruise) + ")");
            // The last keyframe decelerates to rest.
            if (!(step_at(steps, 3.99f) < 0.4f * cruise))
                throw std::runtime_error("the path does not end at rest (last frame " +
                                         std::to_string(step_at(steps, 3.99f)) + " against " +
                                         std::to_string(cruise) + ")");
            require(close_to(positions.back().x, 3.0f), "the path does not end on the last keyframe");
        }

        // A path with no dwell at all: one run of travel, so the camera accelerates once at the
        // start, cruises the whole way and decelerates once into the last keyframe.
        kfi.delete_path();
        make_path(kfi, 4, 1.0f);    // x = 0, 1, 2, 3 over 3 s, no dwell
        positions = sample_positions(kfi, interval);
        {
            const std::vector<float> steps = per_frame_steps(positions);
            const float cruise = step_at(steps, 1.5f);
            if (!(step_at(steps, 0.01f) < 0.15f * cruise && step_at(steps, 2.99f) < 0.4f * cruise))
                throw std::runtime_error("the path without dwells does not start and end at rest (first " +
                                         std::to_string(step_at(steps, 0.01f)) + ", last " +
                                         std::to_string(step_at(steps, 2.99f)) + " against " +
                                         std::to_string(cruise) + ")");
            // The keyframes in between hold no dwell and are passed at the cruise speed.
            for (float t : {1.25f, 1.5f, 1.75f})
                if (!close_to(step_at(steps, t), cruise, 0.05f * cruise))
                    throw std::runtime_error("the path without dwells does not cruise past its keyframes (t " +
                                             std::to_string(t) + ", step " +
                                             std::to_string(step_at(steps, t)) + " against " +
                                             std::to_string(cruise) + ")");
            require(close_to(positions.back().x, 3.0f), "the path without dwells does not end on the last keyframe");
        }

        // Every keyframe holding a dwell: each stretch is its own run, so the camera comes to
        // rest at every keyframe and accelerates out of it again.
        kfi.delete_path();
        make_path(kfi, 4, 1.0f);
        for (std::size_t i = 0; i < kfi.number_of_keyframes(); ++i)
            kfi.set_keyframe_dwell(i, 0.5f);
        positions = sample_positions(kfi, interval);
        {
            const std::vector<float> steps = per_frame_steps(positions);
            // Timeline: dwell 0..0.5, travel 0.5..1.5, dwell 1.5..2.0, travel 2.0..3.0, ...
            // The cruise of the first stretch is at its middle.
            const float cruise = step_at(steps, 1.0f);
            for (float t : {0.51f, 1.49f, 2.01f, 2.99f})
                if (!(step_at(steps, t) < 0.4f * cruise))
                    throw std::runtime_error("a stretch between two dwells does not start and end at rest (t " +
                                             std::to_string(t) + ", step " +
                                             std::to_string(step_at(steps, t)) + " against " +
                                             std::to_string(cruise) + ")");
            if (!close_to(step_at(steps, 1.0f), cruise, 0.05f * cruise))
                throw std::runtime_error("a stretch between two dwells does not cruise in its middle (step " +
                                         std::to_string(step_at(steps, 1.0f)) + " against " +
                                         std::to_string(cruise) + ")");
        }

        // --- continuity along a many-keyframe path -------------------------
        // A path like a real recording (19 keyframes, ~730 units apart, turning 3-11 degrees
        // at each keyframe) must be sampled smoothly: no sample may be far from its
        // neighbour, and the camera must not jump back towards the previous keyframe at a
        // segment boundary. This is the "video jumps to an earlier view at every keyframe"
        // report, which a per-segment sampling parameter produces.
        kfi.delete_path();
        const float step = 730.0f;
        const float radius = 5000.0f;
        const int count = 19;
        for (int i = 0; i < count; ++i) {
            const float angle = static_cast<float>(i) / static_cast<float>(count - 1) * 3.0f;
            Frame frame;
            frame.set_position_and_orientation(
                    vec3(radius * std::sin(angle), 1440.0f, radius * std::cos(angle)), quat());
            require(kfi.add_keyframe(frame, static_cast<float>(i) * step / 730.0f),
                    "a keyframe of the long path was rejected");
        }
        {
            positions = sample_positions(kfi, interval);
            // The path is ~13 100 units long over 18 seconds; at 100 fps a sample covers ~7
            // units at constant speed; EASE_IN_OUT peaks at twice the pace it averages (the
            // camera is at rest at both ends, so it may cover ~14 units) and CINEMATIC's
            // stronger ease peaks at 2.69x (~19 units).
            // The threshold is relative to the pace the path's own samples have, so it does
            // not depend on how many samples the recording happens to use.
            // A sample that lands on the path's end holds there (the last frame and the curve's
            // own rounding), so it is not a jump: the steps are measured up to the arrival.
            float worst = 0.0f, length = 0.0f;
            std::size_t moving = 0;
            for (std::size_t i = 1; i < positions.size(); ++i) {
                const float d = distance(positions[i], positions[i - 1]);
                if (d < 1e-3f)
                    continue;
                worst = std::max(worst, d);
                length += d;
                ++moving;
            }
            const float mean = length / static_cast<float>(moving);
            if (worst >= 8.0f * mean) {
                std::string msg("the sampled path jumps between frames (worst step " + std::to_string(worst) +
                                ", " + std::to_string(positions.size()) + " samples, " +
                                std::to_string(kfi.total_duration()) + " s)");
                throw std::runtime_error(msg);
            }
            // Monotone progress along the path: the camera never goes back. A step is compared
            // over a span long enough to show its direction, since the eased walk barely moves
            // in the two frames either side of the path's end - and a sample that lands on the
            // end (the last frame, or the curve's own rounding) is left out: it holds, it does
            // not go back.
            for (std::size_t i = 2; i + 4 < positions.size(); ++i) {
                if (distance(positions[i + 2], positions[i]) < 1e-3f || distance(positions[i], positions[i - 2]) < 1e-3f)
                    continue;
                const vec3 back = positions[i] - positions[i - 2];
                const vec3 ahead = positions[i + 2] - positions[i];
                if (!(dot(back, ahead) > 0.0f))
                    throw std::runtime_error("backwards at sample " + std::to_string(i) + " of " +
                                             std::to_string(positions.size()) + "), back " +
                                             std::to_string(distance(back, vec3(0.0f))) + " ahead " +
                                             std::to_string(distance(ahead, vec3(0.0f))) + " at keyframe " +
                                             std::to_string(kfi.number_of_keyframes()));
            }
        }

        // --- entering and leaving a dwell ------------------------------------
        // The sample right before a dwell holds, and the one after the dwell must leave the
        // keyframe at the path's pace: it may not jump (a "freeze then jump" reads as a cut).
        // Measured on a path shaped like domik.kf (19 keyframes 730 units apart, 24 fps, a 1 s
        // dwell at keyframe 5): 0 units for the whole dwell, then 22.95 at 6.04 s against a
        // pace of 33.55. The jump is bounded by the path's length over the video's length.
        kfi.delete_path();
        {
            const int count = 19;
            for (int i = 0; i < count; ++i) {
                const float angle = static_cast<float>(i) * 0.17f;
                Frame frame;
                frame.set_position_and_orientation(
                        vec3(5000.0f * std::sin(angle), 1440.0f, 5000.0f * std::cos(angle)), quat());
                require(kfi.add_keyframe(frame, static_cast<float>(i)), "a keyframe was rejected");
            }
            kfi.set_frame_rate(24);
            kfi.set_keyframe_dwell(5, 1.0f);
            positions = sample_positions(kfi, interval);
            const float total = kfi.total_duration();
            const int frames = static_cast<int>(positions.size());
            // The ideal pace: the path's length over the video's length.
            float length = 0.0f;
            for (std::size_t i = 1; i < positions.size(); ++i)
                length += distance(positions[i], positions[i - 1]);
            const float ideal = length / total / 24.0f;   // per frame
            // The dwell holds from t = 5 s to t = 6 s. The first frame after it may not cover
            // more than twice the path's own pace, and the next ones must be close to it.
            const int dwell_end_frame = static_cast<int>(6.0f * 24.0f);
            for (int k = dwell_end_frame; k < dwell_end_frame + 6; ++k) {
                if (k + 1 >= frames) break;
                const float step = distance(positions[k + 1], positions[k]);
                const float allowed = (k == dwell_end_frame) ? 2.0f * ideal : 1.5f * ideal;
                if (step > allowed)
                    throw std::runtime_error("the camera jumps out of a dwell (step " +
                                             std::to_string(step) + " against an ideal " +
                                             std::to_string(ideal) + ")");
            }
        }

    });
}
