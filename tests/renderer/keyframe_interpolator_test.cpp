// SPDX-License-Identifier: GPL-3.0-or-later
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
        require(close_to(position_at(positions, interval, 0.5f), vec3(0.5f, 0.0f, 0.0f)),
                "the uniform path does not pass the first segment's middle");

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

        // --- speed profiles -----------------------------------------------
        // A profile shapes the distance covered over the whole path, not over each segment:
        // the camera eases in once at the start and out once at the end, and it keeps its
        // speed as it passes a keyframe. A 3-keyframe path 1 s apart therefore passes the
        // middle keyframe at half the path time.
        kfi.delete_path();
        require(kfi.number_of_keyframes() == 0, "delete_path left keyframes behind");
        make_path(kfi, 3, 1.0f);
        require(kfi.speed_profile() == KeyFrameInterpolator::SpeedProfile::CONSTANT,
                "the default speed profile is not constant");

        // The profile is shaped per path, so a profile change must invalidate the samples: a
        // cached path from before the change is what the first version of this check measured
        // (it read the constant-profile samples and reported them as an un-eased profile).
        // A keyframe the camera only passes is passed at an even pace: no step is eased unless it
        // touches a dwell, otherwise the camera would stop dead at every keyframe
        // ("stop -> move -> stop -> move", which is what easing every step produced).
        kfi.set_speed_profile(KeyFrameInterpolator::SpeedProfile::EASE_IN_OUT);
        positions = sample_positions(kfi, interval);
        // A tenth of a second of the path (2 units in 2 s) covers 0.1 units, mid-step and at
        // the plain keyframe alike.
        for (float q : {0.25f, 0.5f, 0.75f}) {
            const float step = position_at(positions, interval, q + 0.05f).x - position_at(positions, interval, q - 0.05f).x;
            require(close_to(step, 0.1f, 2e-2f), "the camera stops at a keyframe that holds no dwell");
        }
        // Same pace on both sides of the middle keyframe (x = 1, t = 1 s).
        const auto step_between = [&](float t0, float t1) {
            return (position_at(positions, interval, t1).x - position_at(positions, interval, t0).x) / (t1 - t0);
        };
        require(close_to(step_between(0.9f, 1.0f), step_between(1.0f, 1.1f), 0.15f),
                "the camera changes pace at a keyframe that holds no dwell");
        require(close_to(positions.back().x, 2.0f), "the eased path does not end on the last keyframe");

        // A keyframe that holds a dwell is where the camera stops, so the step before it must
        // ease to rest there instead of carrying the camera across the last stretch in one
        // frame. (Measured on domik.kf: 1257 units in one frame at the dwell's entry with a
        // path-wide ease, against a 32-unit pace.) The whole step is eased, so the path has no
        // gap in front of the dwell and the frame steps shrink towards it.
        kfi.delete_path();
        make_path(kfi, 4, 1.0f);            // 3 units of travel in 3 s
        kfi.set_keyframe_dwell(1, 1.0f);    // the only stop: x = 1
        kfi.set_speed_profile(KeyFrameInterpolator::SpeedProfile::EASE_IN_OUT);
        positions = sample_positions(kfi, interval);
        // Timeline: travel 0..1 s (x 0->1), dwell 1..2 s, travel 2..3 s (x 1->2), dwell 2..3.
        // The whole path's travel is eased, so the camera moves slowly for a while before it
        // reaches the dwelled keyframe and for a while after it: no single frame covers a long
        // stretch, and the frames closest to the dwell barely move.
        // (A window that straddles the dwell's end averages the held frames into the travel
        // that follows, so each side of a boundary is measured on its own.)
        const auto frame_step = [&](float t0, float t1) {
            return position_at(positions, interval, t1).x - position_at(positions, interval, t0).x;
        };
        // Just inside the dwell (0.999 s) the camera is a hair short of the keyframe, and the
        // frame before it barely moves: the whole path's travel is eased, so it has crawled to
        // rest over the last stretch of the approach.
        require(position_at(positions, interval, 0.999f).x > 0.77f &&
                frame_step(0.99f, 0.999f) < 0.02f &&
                frame_step(0.5f, 0.6f) > 0.02f,
                "the camera does not come to rest at a keyframe that holds a dwell");
        // Leaving the dwell (from 2.0 s, where the held pose ends) it starts slowly and is back
        // near its pace by 2.45 s.
        require(position_at(positions, interval, 2.01f).x - 1.0f < 0.03f &&
                position_at(positions, interval, 2.05f).x - 1.0f < 0.1f * (position_at(positions, interval, 2.5f).x - 1.0f) &&
                position_at(positions, interval, 2.4f).x - 1.0f > 0.2f,
                "the camera does not leave a dwell slowly");
        require(close_to(position_at(positions, interval, 1.5f), vec3(1.0f, 0.0f, 0.0f)),
                "the camera moved during a dwell");
        // Leaving the dwell it starts slowly and is back at pace in the middle of the step.
        require(close_to(positions.back().x, 3.0f), "the dwelled path does not end on the last keyframe");

        // Setting a profile invalidates the path, so the next sample uses it. A constant profile
        // does not ease within a step, so the camera passes a keyframe at its full pace.
        kfi.set_speed_profile(KeyFrameInterpolator::SpeedProfile::CONSTANT);
        positions = sample_positions(kfi, interval);
        require(close_to(position_at(positions, interval, 0.5f).x, 0.5f, 1e-2f),
                "the constant profile does not pass the first step evenly");

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
        for (auto profile : {KeyFrameInterpolator::SpeedProfile::CONSTANT,
                             KeyFrameInterpolator::SpeedProfile::EASE_IN_OUT,
                             KeyFrameInterpolator::SpeedProfile::CINEMATIC}) {
            kfi.set_speed_profile(profile);
            positions = sample_positions(kfi, interval);
            // The path is ~13 100 units long over 18 seconds; at 100 fps a sample covers ~7
            // units at constant speed, and the ease-in/out is at most ~4x slower at the ends.
            // The threshold is relative to the pace the path's own samples have, so it does
            // not depend on how many samples the recording happens to use.
            float worst = 0.0f, length = 0.0f;
            for (std::size_t i = 1; i < positions.size(); ++i) {
                const float d = distance(positions[i], positions[i - 1]);
                worst = std::max(worst, d);
                length += d;
            }
            const float mean = length / static_cast<float>(positions.size() - 1);
            if (worst >= 4.0f * mean) {
                std::string msg("the sampled path jumps between frames (worst step " + std::to_string(worst) +
                                ", profile " + std::to_string(static_cast<int>(profile)) + ", " +
                                std::to_string(positions.size()) + " samples, " +
                                std::to_string(kfi.total_duration()) + " s)");
                throw std::runtime_error(msg);
            }
            // Monotone progress along the path: the camera never goes back.
            for (std::size_t i = 1; i + 1 < positions.size(); ++i) {
                const vec3 back = positions[i] - positions[i - 1];
                const vec3 ahead = positions[i + 1] - positions[i];
                require(dot(back, ahead) > 0.0f, "the sampled camera path goes backwards at a keyframe");
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
            kfi.set_speed_profile(KeyFrameInterpolator::SpeedProfile::CONSTANT);
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
