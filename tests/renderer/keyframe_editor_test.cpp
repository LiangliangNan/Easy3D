// SPDX-License-Identifier: GPL-3.0-or-later
// The editor's path edits are CPU work too: they move poses, times and pauses between
// keyframes, and need no context.
#include <algorithm>
#include <cmath>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/keyframe_interpolator.h>
#include <easy3d/renderer/keyframe_editor.h>
#include <easy3d/renderer/manipulated_camera_frame.h>

#include "../test.h"

using namespace easy3d;
using easy3d::test::require;

namespace {

constexpr float kTolerance = 1e-4f;

bool close_to(float a, float b, float tolerance = kTolerance) { return std::abs(a - b) <= tolerance; }

// A path along +x, one keyframe per second, so an index says a keyframe's place and
// its time at once.
void make_path(KeyframeInterpolator &kfi, int count) {
    for (int i = 0; i < count; ++i) {
        Frame frame;
        frame.set_position_and_orientation(vec3(static_cast<float>(i), 0.0f, 0.0f), quat());
        require(kfi.add_keyframe(frame, static_cast<float>(i)), "a keyframe was rejected");
    }
}

// The dwell of every keyframe, as one list to compare against.
std::vector<float> dwells_of(const KeyframeInterpolator &kfi) {
    std::vector<float> values;
    for (std::size_t i = 0; i < kfi.number_of_keyframes(); ++i)
        values.push_back(kfi.keyframe_dwell(i));
    return values;
}

bool same_dwells(const KeyframeInterpolator &kfi, const std::vector<float> &expected) {
    const std::vector<float> values = dwells_of(kfi);
    if (values.size() != expected.size())
        return false;
    return std::equal(values.begin(), values.end(), expected.begin(),
                      [](float a, float b) { return close_to(a, b); });
}

} // namespace

int main() {
    return easy3d::test::run("keyframe editor", [] {
        Camera camera;
        KeyframeInterpolator kfi(camera.frame());
        KeyframeEditor editor(&camera, &kfi);

        // --- delete keeps the pauses of the keyframes that stay -----------------
        // A delete rebuilds the path from the kept poses, and a dwell belongs to its
        // keyframe rather than to the edit that set it: dropping it silently shortens
        // the video and flattens the timeline.
        make_path(kfi, 5);
        kfi.set_keyframe_dwell(1, 2.0f);
        kfi.set_keyframe_dwell(3, 0.5f);
        editor.set_selected(0);
        require(editor.delete_selected() == 0, "the first keyframe was not deleted");
        require(kfi.number_of_keyframes() == 4, "the path kept the deleted keyframe");
        require(same_dwells(kfi, {2.0f, 0.0f, 0.5f, 0.0f}),
                "deleting a keyframe changed the dwell of the ones that stayed");
        // The pauses are part of the path's length, so the duration follows them:
        // three seconds of travel between the keyframes that stay, plus both dwells.
        require(close_to(kfi.total_duration(), 3.0f + 2.5f),
                "the rebuilt path lost the dwell time of its keyframes");

        // --- insert leaves the neighbours' pauses where they were ----------------
        // The inserted keyframe is new, so it holds nothing until it is given a dwell.
        editor.set_selected(1);
        require(editor.insert_after_selected() == 2, "no keyframe was inserted");
        require(kfi.number_of_keyframes() == 5, "the insert did not add a keyframe");
        require(same_dwells(kfi, {2.0f, 0.0f, 0.0f, 0.5f, 0.0f}),
                "inserting a keyframe moved the dwell of the ones around it");
        require(close_to(kfi.keyframe_dwell(2), 0.0f), "the inserted keyframe already dwells");

        // --- popping the last keyframe touches nothing else ----------------------
        // This one goes straight to the interpolator, so it is the pattern the two
        // rebuilds above have to match.
        kfi.set_keyframe_dwell(4, 1.0f);
        kfi.delete_last_keyframe();
        require(same_dwells(kfi, {2.0f, 0.0f, 0.0f, 0.5f}),
                "removing the last keyframe changed the dwell of the others");

        // Rebuilding for an insert rewrites the interpolator, so it must carry the
        // path's endpoint policy across delete_path() and re-add the retained poses.
        kfi.set_path_periodic(true);
        editor.set_selected(1);
        require(editor.insert_after_selected() == 2, "the periodic-path insert failed");
        require(kfi.is_path_periodic(), "the editor rebuild lost periodic interpolation");
    });
}
