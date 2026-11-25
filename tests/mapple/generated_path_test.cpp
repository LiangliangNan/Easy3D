// SPDX-License-Identifier: GPL-3.0-or-later
#include <vector>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/keyframe_interpolator.h>

#include "../../applications/Mapple/generated_path.h"
#include "../test.h"

using namespace easy3d;
using mapple::CameraPath;
using easy3d::test::require;

int main() {
    return easy3d::test::run("generated path provenance", [] {
        {
            mapple::GeneratedPath path;
            path.source = CameraPath::Source::walk({{{0.0f, 0.0f, 0.0f},
                                                           {4.0f, 0.0f, 0.0f}}});
            path.first = 0;
            path.last = 4;
            path.pending = false;

            path.keyframe_inserted(5);
            require(path.first == 0 && path.last == 4 && path.size() == 5 &&
                    path.manual_before.empty(),
                    "inserting after a generated path changed its ownership");

            path.keyframe_inserted(3);
            path.keyframe_inserted(2);
            require(path.first == 0 && path.last == 6 && path.size() == 5 &&
                    path.manual_before == std::vector<int>({2, 3}),
                    "insertions inside a generated path were not tracked as manual gaps");
            require(path.manual_position(0) == 2 && path.manual_position(1) == 4 &&
                    path.generated_before(4) == 3,
                    "manual gaps do not retain their generated anchors");

            path.keyframe_removed(1);
            require(path.first == 0 && path.last == 5 && path.size() == 4 &&
                    path.manual_before == std::vector<int>({1, 2}) &&
                    path.manual_position(0) == 1 && path.manual_position(1) == 3,
                    "removing a generated pose did not shift later manual gaps");

            path.keyframe_removed(1);
            require(path.first == 0 && path.last == 4 && path.size() == 4 &&
                    path.manual_before == std::vector<int>({2}) &&
                    path.manual_position(0) == 2,
                    "removing a manual pose did not remove only its own gap");

            path.keyframe_removed(4);
            require(path.first == 0 && path.last == 3 && path.size() == 3,
                    "Remove Last did not shrink the generated path");
            path.keyframe_removed(0);
            require(path.first == 0 && path.last == 2 && path.size() == 2 &&
                    path.manual_before == std::vector<int>({1}),
                    "removing a generated pose before an anchor did not shift the range");

            path.keyframe_inserted(0);
            require(path.first == 1 && path.last == 3 && path.size() == 2 &&
                    path.manual_position(0) == 2,
                    "inserting before a generated path did not shift its range");

            mapple::GeneratedPath later;
            later.source = path.source;
            later.first = 5;
            later.last = 6;
            later.keyframe_inserted(2);
            require(later.first == 6 && later.last == 7,
                    "an insertion before a generated path did not shift its range");
            later.keyframe_removed(2);
            require(later.first == 5 && later.last == 6,
                    "a removal before a generated path did not shift its range");
        }

        {
            mapple::GeneratedPath path;
            path.source = CameraPath::Source::orbit(vec3(0.0f), vec3(1.0f));
            path.first = 5;
            path.last = 4;
            path.keyframe_inserted(5);
            require(path.first == 5 && path.last == 5 && path.size() == 0 &&
                    path.manual_before == std::vector<int>({0}),
                    "an insertion at an empty generated range was not retained");
            path.keyframe_removed(5);
            require(path.first == 5 && path.last == 4 && path.size() == 0 &&
                    path.manual_before.empty(),
                    "removing the only pose did not restore empty provenance");
        }

        Camera camera;
        Frame frame;
        KeyframeInterpolator interpolator;
        interpolator.set_frame(&frame);
        CameraPath builder(&camera, &interpolator);
        builder.set_scene_box(Box3(vec3(-2.0f, -1.0f, -2.0f), vec3(2.0f, 1.0f, 2.0f)));
        CameraPath::Settings settings = builder.settings();
        settings.ground_up_axis = CameraPath::GroundUpAxis::Y;
        settings.eye_height_in_scene_heights = 0.0f;
        settings.flight_height_in_scene_heights = 0.0f;
        builder.set_settings(settings);
        const auto walk = CameraPath::Source::walk({{{0.0f, 0.0f, 0.0f},
                                                           {4.0f, 0.0f, 0.0f}}});
        require(builder.frames_of(walk).empty(), "the mode-switch fixture accepted a zero Walk height");
        mapple::prepare_new_path_settings(builder, CameraPath::Source::Kind::Walk);
        require(builder.eye_height_in_scene_heights() == CameraPath::Settings().eye_height_in_scene_heights &&
                        !builder.frames_of(walk).empty() && builder.flight_height_in_scene_heights() == 0.0f,
                "arming Walk did not default only its degenerate height");

        const auto flight = CameraPath::Source::flight({vec3(0.0f), vec3(4.0f, 0.0f, 0.0f)});
        require(builder.frames_of(flight).empty(), "the mode-switch fixture accepted a zero Flight height");
        mapple::prepare_new_path_settings(builder, CameraPath::Source::Kind::Flight);
        require(builder.flight_height_in_scene_heights() == CameraPath::Settings().flight_height_in_scene_heights &&
                        builder.frames_of(flight).size() >= 2,
                "arming Flight did not default its degenerate height");

        const auto source = CameraPath::Source::walk({{{0.0f, 0.0f, 0.0f},
                                                             {4.0f, 0.0f, 0.0f}}});
        mapple::GeneratedPath first;
        first.source = source;
        first.built_settings = builder.settings();
        first.first = 0;
        first.last = 2;
        first.pending = false;

        std::vector<mapple::GeneratedPath> generated_paths{first};
        const std::vector<mapple::GeneratedPath> snapshot = generated_paths;
        mapple::GeneratedPath second = first;
        second.first = 3;
        second.last = 5;
        generated_paths.push_back(second);
        generated_paths = snapshot;

        require(generated_paths.size() == 1 && generated_paths.front().first == 0 &&
                generated_paths.front().last == 2 && generated_paths.front().source.kind ==
                CameraPath::Source::Kind::Walk,
                "restoring an undo snapshot lost the earlier generated path");
        builder.set_eye_height_in_scene_heights(builder.eye_height_in_scene_heights() + 0.1f);
        require(generated_paths.front().stale(builder),
                "the restored generated path no longer responds to its parameters");
    });
}
