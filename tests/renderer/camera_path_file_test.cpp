#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/camera_path_builder.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>

#include <3rd_party/nlohmann/json.hpp>

#include "../../applications/Mapple/camera_path_file.h"
#include "../test.h"

using namespace easy3d;
using easy3d::test::require;

namespace {

    struct TestFile {
        explicit TestFile(const char *name) : name(name) {}
        ~TestFile() { std::remove(name.c_str()); }
        std::string name;
    };

    void capture_context(mapple::GeneratedPath &path, const CameraPathBuilder &builder, const Camera &camera) {
        path.scene_box = builder.scene_box();
        path.scene_radius = camera.scene_radius();
        path.view_up = camera.up_vector();
        path.has_build_context = true;
    }

    void add_frames(KeyFrameInterpolator &kfi, const std::vector<Frame> &frames) {
        for (std::size_t i = 0; i < frames.size(); ++i) {
            require(kfi.add_keyframe(frames[i], 1.0f + static_cast<float>(i) * 1.75f),
                    "a generated keyframe was rejected");
            if (i == 2)
                kfi.set_keyframe_dwell(i, 0.5f);
        }
    }

    void require_same_path(KeyFrameInterpolator &a, KeyFrameInterpolator &b) {
        require(a.number_of_keyframes() == b.number_of_keyframes(), "the keyframe count changed on load");
        for (std::size_t i = 0; i < a.number_of_keyframes(); ++i) {
            const vec3 &a_position = a.keyframe_position(i);
            const vec3 &b_position = b.keyframe_position(i);
            require(a_position.x == b_position.x && a_position.y == b_position.y && a_position.z == b_position.z,
                    "a keyframe position changed on load");
            const quat &a_orientation = a.keyframe_orientation(i);
            const quat &b_orientation = b.keyframe_orientation(i);
            require(a_orientation[0] == b_orientation[0] && a_orientation[1] == b_orientation[1] &&
                    a_orientation[2] == b_orientation[2] && a_orientation[3] == b_orientation[3],
                    "a keyframe orientation changed on load");
            require(a.keyframe_time(i) == b.keyframe_time(i), "a keyframe time changed on load");
            require(a.keyframe_dwell(i) == b.keyframe_dwell(i), "a keyframe dwell changed on load");
        }
        const std::vector<Frame> &a_path = a.interpolate();
        const std::vector<Frame> &b_path = b.interpolate();
        require(a_path.size() == b_path.size(), "the interpolated path length changed on load");
        for (std::size_t i = 0; i < a_path.size(); ++i) {
            require(distance(a_path[i].position(), b_path[i].position()) < 1e-5f,
                    "an interpolated position changed on load");
            require(std::abs(std::abs(quat::dot(a_path[i].orientation(), b_path[i].orientation())) - 1.0f) < 1e-5f,
                    "an interpolated orientation changed on load");
        }
    }

    void test_stream_io() {
        Camera camera;
        KeyFrameInterpolator source(camera.frame());
        for (int i = 0; i < 4; ++i)
            require(source.add_keyframe(Frame(vec3(static_cast<float>(i)), quat()), 0.25f + static_cast<float>(i)),
                    "stream fixture keyframe was rejected");
        source.set_keyframe_dwell(1, 0.75f);
        source.set_path_periodic(true);

        std::ostringstream output;
        require(source.save_keyframes(output), "saving keyframes to a stream failed");
        KeyFrameInterpolator loaded(camera.frame());
        std::istringstream input(output.str());
        require(loaded.load_keyframes(input), "loading keyframes from a stream failed");
        require_same_path(source, loaded);
        require(loaded.is_path_periodic(), "the periodic flag was lost in the stream");
    }

    void test_positive_scene_box_without_playback() {
        TestFile file("mapple-positive-box-provenance.kf");
        Camera camera;
        camera.set_scene_radius(100.0f);
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        const vec3 lo(1000.0f, 20.0f, 2000.0f);
        const vec3 hi(3000.0f, 60.0f, 4000.0f);
        const Box3 box(lo, hi);
        require(box.min_point() == lo && box.max_point() == hi,
                "constructing a box with positive corners expanded it to the origin");
        builder.set_scene_box(box);
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Y;
        builder.set_settings(settings);
        const CameraPathBuilder::Source input = CameraPathBuilder::Source::walk({
                {vec3(1200.0f, 30.0f, 2200.0f), vec3(2000.0f, 30.0f, 3000.0f),
                 vec3(2800.0f, 30.0f, 3800.0f)}
        });
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() >= 2, "the positive-box fixture produced too few poses");
        add_frames(source, generated);

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(mapple::save_camera_path(file.name, source, {path}), "saving the positive-box fixture failed");

        std::ifstream input_file(file.name.c_str());
        std::ostringstream contents;
        contents << input_file.rdbuf();
        const std::string text = contents.str();
        const std::size_t marker = text.find("provenance:");
        require(marker != std::string::npos, "the positive-box fixture has no provenance");
        nlohmann::json provenance = nlohmann::json::parse(text.substr(marker + std::string("provenance:").size()));
        provenance.erase("playback");
        {
            std::ofstream output(file.name.c_str());
            output << text.substr(0, marker) << "provenance:\n" << provenance.dump(2) << '\n';
        }

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading the positive-box fixture failed");
        require(result.generated_paths.size() == 1 &&
                        result.generated_paths.front().scene_box.min_point() == lo &&
                        result.generated_paths.front().scene_box.max_point() == hi,
                "positive scene bounds or early provenance without playback were not restored");
        require_same_path(source, loaded);
    }

    void test_orientation_edits_preserve_provenance() {
        TestFile file("mapple-orientation-edit-provenance.kf");
        Camera camera;
        camera.set_scene_radius(20.0f);
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f), vec3(5.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Y;
        builder.set_settings(settings);
        const CameraPathBuilder::Source input = CameraPathBuilder::Source::walk({
                {vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f)}
        });
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() >= 2, "the orientation-edit fixture produced too few poses");
        add_frames(source, generated);
        source.set_keyframe_orientation(1, quat(vec3(0.0f, 0.0f, 1.0f), 0.1f) * source.keyframe_orientation(1));

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(mapple::save_camera_path(file.name, source, {path}), "saving edited orientation provenance failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading edited orientation provenance failed");
        require(result.generated_paths.size() == 1, "an orientation edit discarded reproducible path provenance");
        require_same_path(source, loaded);
    }

    void test_edited_keyframe_round_trip() {
        Camera camera;
        KeyFrameInterpolator source(camera.frame());
        require(source.add_keyframe(Frame(vec3(0.0f), quat()), 0.0f), "the first edited fixture pose was rejected");
        require(source.add_keyframe(Frame(vec3(1.0f), quat(vec3(0.0f, 0.0f, 1.0f), 0.5f)), 1.0f),
                "the second edited fixture pose was rejected");
        require(source.add_keyframe(Frame(vec3(2.0f), quat(vec3(0.0f, 1.0f, 0.0f), 0.25f)), 2.0f),
                "the third edited fixture pose was rejected");
        source.set_keyframe_position(1, source.keyframe_position(0));
        source.set_keyframe_time(0, 3.0f);
        source.set_keyframe_dwell(1, 0.5f);

        std::ostringstream output;
        require(source.save_keyframes(output), "saving edited keyframes failed");
        KeyFrameInterpolator loaded(camera.frame());
        std::istringstream input(output.str());
        require(loaded.load_keyframes(input), "loading edited keyframes failed");
        require(loaded.number_of_keyframes() == 3, "loading edited keyframes changed their count");
        for (std::size_t i = 0; i < source.number_of_keyframes(); ++i) {
            require(loaded.keyframe_position(i) == source.keyframe_position(i),
                    "loading edited keyframes changed a position");
            const quat &expected_orientation = source.keyframe_orientation(i);
            const quat &loaded_orientation = loaded.keyframe_orientation(i);
            require(loaded_orientation[0] == expected_orientation[0] && loaded_orientation[1] == expected_orientation[1] &&
                    loaded_orientation[2] == expected_orientation[2] && loaded_orientation[3] == expected_orientation[3],
                    "loading edited keyframes changed an orientation");
            require(loaded.keyframe_time(i) == source.keyframe_time(i), "loading edited keyframes changed a time");
            require(loaded.keyframe_dwell(i) == source.keyframe_dwell(i), "loading edited keyframes changed a dwell");
        }
    }

    void test_walk_and_manual_gap() {
        TestFile file("mapple-walk-provenance.kf");
        Camera camera;
        camera.set_scene_radius(12.0f);
        camera.set_up_vector(normalize(vec3(0.2f, 1.0f, 0.1f)));
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f), vec3(5.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::View;
        settings.eye_height_in_scene_heights = 0.3f;
        settings.keyframe_spacing_in_scene_heights = 0.15f;
        builder.set_settings(settings);
        CameraPathBuilder::Source input = CameraPathBuilder::Source::walk({
                {vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f)}
        });
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() >= 4, "the walk fixture produced too few poses");

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        Camera replay_camera;
        replay_camera.set_scene_radius(path.scene_radius);
        replay_camera.set_up_vector(path.view_up);
        CameraPathBuilder replay(&replay_camera, nullptr);
        replay.set_scene_box(path.scene_box);
        replay.set_settings(path.built_settings);
        const std::vector<Frame> replayed = replay.frames_of(path.source);
        require(replayed.size() == generated.size(), "the captured build context changed the pose count");
        for (std::size_t i = 0; i < generated.size(); ++i) {
            require(distance(replayed[i].position(), generated[i].position()) < 1e-5f,
                    "the captured build context changed a position");
            require(std::abs(std::abs(quat::dot(replayed[i].orientation(), generated[i].orientation())) - 1.0f) < 1e-6f,
                    "the captured build context changed an orientation");
        }
        path.keyframe_inserted(2);
        for (int i = 0; i < static_cast<int>(generated.size()); ++i) {
            if (i == 2) {
                const Frame manual((generated[1].position() + generated[2].position()) * 0.5f,
                                   generated[1].orientation());
                require(source.add_keyframe(manual, 1.0f + static_cast<float>(i) * 1.75f),
                        "the manual keyframe was rejected");
            }
            require(source.add_keyframe(generated[static_cast<std::size_t>(i)],
                                        1.0f + static_cast<float>(i + (i >= 2)) * 1.75f),
                    "the walk keyframe was rejected");
        }
        source.set_keyframe_dwell(2, 0.5f);
        require(mapple::save_camera_path(file.name, source, {path}), "saving walk provenance failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading walk provenance failed");
        require(result.generated_paths.size() == 1, "the walk provenance was not restored");
        require(result.generated_paths.front().source.kind == CameraPathBuilder::Source::Kind::Walk,
                "the walk mode was not restored");
        require(result.generated_paths.front().manual_before == std::vector<int>{2},
                "the manual gap was not restored");
        require_same_path(source, loaded);
        require(!loaded.is_path_periodic(), "a loaded walk became periodic");

        builder.set_settings(result.settings);
        builder.set_scene_box(result.generated_paths.front().scene_box);
        CameraPathBuilder::Settings changed = result.settings;
        changed.eye_height_in_scene_heights *= 2.0f;
        builder.set_settings(changed);
        const std::vector<Frame> rebuilt = builder.frames_of(result.generated_paths.front().source);
        require(!rebuilt.empty() && distance(rebuilt.front().position(), generated.front().position()) > 0.1f,
                "the restored provenance did not drive regeneration");
    }

    void test_view_up_without_scene_box() {
        TestFile file("mapple-view-up-provenance.kf");
        Camera camera;
        camera.set_scene_radius(2.0f);
        camera.set_up_vector(normalize(vec3(0.2f, 1.0f, 0.1f)));
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::View;
        settings.keyframe_spacing_in_scene_heights = 0.15f;
        builder.set_settings(settings);
        const CameraPathBuilder::Source input = CameraPathBuilder::Source::walk({
                {vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)}
        });
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() >= 4, "the invalid-box fixture produced too few poses");
        add_frames(source, generated);

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(!path.scene_box.is_valid(), "the invalid-box fixture unexpectedly has scene bounds");
        require(mapple::save_camera_path(file.name, source, {path}), "saving invalid-box provenance failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading invalid-box provenance failed");
        require(result.generated_paths.size() == 1 && !result.generated_paths.front().scene_box.is_valid(),
                "the invalid build-time scene box was not restored");
        require_same_path(source, loaded);
    }

    void test_flight() {
        TestFile file("mapple-flight-provenance.kf");
        Camera camera;
        camera.set_scene_radius(10.0f);
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f), vec3(5.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Y;
        settings.flight_height_in_scene_heights = 1.5f;
        settings.keyframe_spacing_in_scene_heights = 0.5f;
        builder.set_settings(settings);
        CameraPathBuilder::Source input = CameraPathBuilder::Source::flight({
                vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f)
        });
        input.aims = {vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 2.0f)};
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() >= 4, "the flight fixture produced too few poses");
        add_frames(source, generated);

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(mapple::save_camera_path(file.name, source, {path}), "saving flight provenance failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading flight provenance failed");
        require(result.generated_paths.size() == 1 &&
                result.generated_paths.front().source.kind == CameraPathBuilder::Source::Kind::Flight,
                "the flight provenance was not restored");
        require(result.generated_paths.front().source.aims.size() == 2, "the flight aims were not restored");
        require_same_path(source, loaded);
        require(!loaded.is_path_periodic(), "a loaded flight became periodic");
    }

    void test_multiple_blocks() {
        TestFile file("mapple-multiple-provenance.kf");
        Camera camera;
        camera.set_scene_radius(12.0f);
        camera.set_up_vector(vec3(0.0f, 0.0f, 1.0f));
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-8.0f, -1.0f, -5.0f), vec3(8.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Y;
        settings.eye_height_in_scene_heights = 0.3f;
        settings.keyframe_spacing_in_scene_heights = 0.4f;
        settings.flight_height_in_scene_heights = 1.0f;
        builder.set_settings(settings);

        int count = 0;
        auto append = [&](const Frame &frame) {
            require(source.add_keyframe(frame, 0.25f + static_cast<float>(count) * 0.5f),
                    "a multiple-block keyframe was rejected");
            if (count == 2)
                source.set_keyframe_dwell(static_cast<std::size_t>(count), 0.75f);
            ++count;
        };
        append(Frame(vec3(-3.0f, 0.0f, 0.0f), quat()));

        CameraPathBuilder::Source walk = CameraPathBuilder::Source::walk({
                {vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f)}
        });
        const std::vector<Frame> walk_frames = builder.frames_of(walk);
        mapple::GeneratedPath walk_path;
        walk_path.pending = false;
        walk_path.source = walk;
        walk_path.built_settings = settings;
        walk_path.first = count;
        capture_context(walk_path, builder, camera);
        for (const Frame &frame : walk_frames)
            append(frame);
        walk_path.last = count - 1;

        CameraPathBuilder::Source flight = CameraPathBuilder::Source::flight({
                vec3(3.0f, 0.0f, 0.0f), vec3(4.5f, 0.0f, 0.0f), vec3(6.0f, 0.0f, 0.0f)
        });
        const std::vector<Frame> flight_frames = builder.frames_of(flight);
        mapple::GeneratedPath flight_path;
        flight_path.pending = false;
        flight_path.source = flight;
        flight_path.built_settings = settings;
        flight_path.first = count;
        capture_context(flight_path, builder, camera);
        for (const Frame &frame : flight_frames)
            append(frame);
        flight_path.last = count - 1;
        append(Frame(vec3(7.0f, 0.0f, 0.0f), quat()));
        source.set_interpolation_method(KeyFrameInterpolator::FITTING);
        source.set_interpolation_speed(1.25f);
        source.set_frame_rate(50);

        require(mapple::save_camera_path(file.name, source, {walk_path, flight_path}),
                "saving multiple provenance blocks failed");
        std::ifstream input(file.name.c_str());
        std::ostringstream contents;
        contents << input.rdbuf();
        const std::string text = contents.str();
        require(text.find("provenance:") != std::string::npos &&
                        text.find("provenance:") > text.find("num_key_frames:"),
                "provenance was not appended after the ordinary keyframes");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading multiple provenance blocks failed");
        require(result.generated_paths.size() == 2, "not all provenance blocks were restored");
        require(result.generated_paths[0].source.kind == CameraPathBuilder::Source::Kind::Walk &&
                        result.generated_paths[1].source.kind == CameraPathBuilder::Source::Kind::Flight,
                "the provenance block modes changed");
        require(result.generated_paths.front().first == 1 &&
                        result.generated_paths.back().last == count - 2,
                "unowned leading or trailing keyframes were not preserved");
        require(loaded.interpolation_method() == KeyFrameInterpolator::FITTING &&
                        std::abs(loaded.interpolation_speed() - 1.25f) < 1e-6f && loaded.frame_rate() == 50,
                "playback parameters were not restored");
        require_same_path(source, loaded);
    }

    void test_orbit() {
        TestFile file("mapple-orbit-provenance.kf");
        Camera camera;
        camera.set_scene_radius(10.0f);
        KeyFrameInterpolator source(camera.frame());
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f), vec3(5.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Z;
        settings.keyframes_per_loop = 12;
        settings.pitch_degrees = 8.0f;
        builder.set_settings(settings);
        CameraPathBuilder::Source input = CameraPathBuilder::Source::orbit(vec3(0.0f), vec3(0.0f));
        const std::vector<Frame> generated = builder.frames_of(input);
        require(generated.size() == 13, "the orbit fixture has the wrong pose count");
        add_frames(source, generated);
        source.set_path_periodic(true);

        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(mapple::save_camera_path(file.name, source, {path}), "saving orbit provenance failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "loading orbit provenance failed");
        require(result.generated_paths.size() == 1 &&
                result.generated_paths.front().source.kind == CameraPathBuilder::Source::Kind::Orbit,
                "the orbit provenance was not restored");
        require_same_path(source, loaded);
        require(loaded.is_path_periodic(), "the loaded orbit did not restore periodic interpolation");

        path.keyframe_inserted(6);
        KeyFrameInterpolator manual_source(camera.frame());
        for (int i = 0; i < static_cast<int>(generated.size()); ++i) {
            if (i == 6) {
                const Frame manual((generated[5].position() + generated[6].position()) * 0.5f,
                                   generated[5].orientation());
                require(manual_source.add_keyframe(manual, 1.0f + static_cast<float>(i) * 1.75f),
                        "the manual orbit keyframe was rejected");
            }
            require(manual_source.add_keyframe(generated[static_cast<std::size_t>(i)],
                                              1.0f + static_cast<float>(i + (i >= 6)) * 1.75f),
                    "the manual-gap orbit keyframe was rejected");
        }
        manual_source.set_path_periodic(true);
        require(mapple::save_camera_path(file.name, manual_source, {path}), "saving an orbit manual gap failed");
        KeyFrameInterpolator manual_loaded(camera.frame());
        require(mapple::read_camera_path(file.name, &manual_loaded, result), "loading an orbit manual gap failed");
        require(result.generated_paths.size() == 1 &&
                        result.generated_paths.front().manual_before == std::vector<int>{6},
                "the orbit manual gap was not restored");
        require_same_path(manual_source, manual_loaded);
        require(manual_loaded.is_path_periodic(), "an orbit manual gap disabled periodic interpolation on load");
    }

    void test_fallbacks() {
        TestFile legacy("mapple-legacy.kf");
        Camera camera;
        KeyFrameInterpolator source(camera.frame());
        add_frames(source, {Frame(vec3(0.0f), quat()), Frame(vec3(1.0f), quat()), Frame(vec3(2.0f), quat())});
        {
            std::ofstream output(legacy.name.c_str());
            require(output && source.save_keyframes(output), "saving the legacy fixture failed");
        }
        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(legacy.name, &loaded, result), "loading a legacy file failed");
        require(result.generated_paths.empty(), "a legacy file invented provenance");
        require_same_path(source, loaded);

        TestFile old_format("mapple-old-format.kf");
        KeyFrameInterpolator legacy_source(camera.frame());
        require(legacy_source.add_keyframe(Frame(vec3(0.0f), quat())), "a legacy keyframe was rejected");
        require(legacy_source.add_keyframe(Frame(vec3(1.0f), quat())), "a legacy keyframe was rejected");
        {
            std::ofstream output(old_format.name.c_str());
            output << "num_key_frames: 2\n"
                   << "\tframe: 0\n"
                   << "\t\tposition: 0 0 0\n"
                   << "\t\torientation: 0 0 0 1\n"
                   << "\tframe: 1\n"
                   << "\t\tposition: 1 1 1\n"
                   << "\t\torientation: 0 0 0 1\n";
        }
        KeyFrameInterpolator old_loaded(camera.frame());
        require(mapple::read_camera_path(old_format.name, &old_loaded, result),
                "loading the original keyframe format failed");
        require(result.generated_paths.empty(), "the original keyframe format invented provenance");
        require_same_path(legacy_source, old_loaded);
        require(!old_loaded.is_path_periodic() && old_loaded.keyframe_dwell(0) == 0.0f,
                "the original keyframe format acquired new defaults");

        TestFile malformed("mapple-malformed-provenance.kf");
        {
            std::ofstream output(malformed.name.c_str());
            require(output && source.save_keyframes(output), "saving the malformed fixture failed");
            output << "\tprovenance:\n{not json}\n";
        }
        KeyFrameInterpolator fallback(camera.frame());
        require(mapple::read_camera_path(malformed.name, &fallback, result), "malformed provenance blocked keyframe loading");
        require(result.generated_paths.empty(), "malformed provenance was adopted");
        require_same_path(source, fallback);

        TestFile incompatible("mapple-incompatible-provenance.kf");
        {
            std::ofstream output(incompatible.name.c_str());
            require(output && source.save_keyframes(output), "saving the incompatible fixture failed");
            output << "\tprovenance:\n"
                   << "{\"version\": 999, \"keyframes\": " << source.number_of_keyframes()
                   << ", \"playback\": {}, \"paths\": []}\n";
        }
        KeyFrameInterpolator incompatible_loaded(camera.frame());
        require(mapple::read_camera_path(incompatible.name, &incompatible_loaded, result),
                "incompatible provenance blocked keyframe loading");
        require(result.generated_paths.empty(), "incompatible provenance was adopted");
        require_same_path(source, incompatible_loaded);

        TestFile edited("mapple-edited-provenance.kf");
        CameraPathBuilder builder(&camera, &source);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f), vec3(5.0f, 1.0f, 5.0f)));
        CameraPathBuilder::Settings settings;
        settings.ground_up_axis = CameraPathBuilder::GroundUpAxis::Y;
        builder.set_settings(settings);
        CameraPathBuilder::Source input = CameraPathBuilder::Source::walk({
                {vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f)}
        });
        const std::vector<Frame> generated = builder.frames_of(input);
        source.delete_path();
        add_frames(source, generated);
        source.set_keyframe_position(1, source.keyframe_position(1) + vec3(0.0f, 0.5f, 0.0f));
        mapple::GeneratedPath path;
        path.pending = false;
        path.source = input;
        path.built_settings = settings;
        path.first = 0;
        path.last = static_cast<int>(generated.size()) - 1;
        capture_context(path, builder, camera);
        require(mapple::save_camera_path(edited.name, source, {path}), "saving edited provenance failed");
        KeyFrameInterpolator edited_loaded(camera.frame());
        require(mapple::read_camera_path(edited.name, &edited_loaded, result), "edited provenance blocked keyframe loading");
        require(result.generated_paths.empty(), "provenance overwrote a manually edited generated pose");
        require_same_path(source, edited_loaded);

        TestFile distant_edit("mapple-distant-edit-provenance.kf");
        const vec3 offset(1000000.0f, 0.0f, 0.0f);
        builder.set_scene_box(Box3(vec3(-5.0f, -1.0f, -5.0f) + offset, vec3(5.0f, 1.0f, 5.0f) + offset));
        CameraPathBuilder::Source distant_input = CameraPathBuilder::Source::walk({
                {vec3(-2.0f, 0.0f, 0.0f) + offset,
                 vec3(0.0f, 0.0f, 0.0f) + offset,
                 vec3(2.0f, 0.0f, 0.0f) + offset}
        });
        const std::vector<Frame> distant_generated = builder.frames_of(distant_input);
        source.delete_path();
        add_frames(source, distant_generated);
        source.set_keyframe_position(1, source.keyframe_position(1) + vec3(10.0f, 0.0f, 0.0f));
        path.source = distant_input;
        path.built_settings = settings;
        path.scene_box = builder.scene_box();
        path.scene_radius = camera.scene_radius();
        path.view_up = camera.up_vector();
        path.has_build_context = true;
        path.last = static_cast<int>(distant_generated.size()) - 1;
        require(mapple::save_camera_path(distant_edit.name, source, {path}),
                "saving the distant-edit fixture failed");
        KeyFrameInterpolator distant_loaded(camera.frame());
        require(mapple::read_camera_path(distant_edit.name, &distant_loaded, result),
                "the distant-edit fixture blocked keyframe loading");
        require(result.generated_paths.empty(), "a large world coordinate hid a manually edited generated pose");
        require_same_path(source, distant_loaded);
    }

    void test_refused_build_fallback() {
        TestFile file("mapple-refused-build.kf");
        Camera camera;
        KeyFrameInterpolator source(camera.frame());
        add_frames(source, {Frame(vec3(0.0f), quat()), Frame(vec3(1.0f), quat()), Frame(vec3(2.0f), quat())});

        mapple::GeneratedPath path;
        path.source = CameraPathBuilder::Source::walk({{{0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}}});
        path.first = 0;
        path.last = 2;
        path.pending = false;
        path.has_build_context = true;
        path.scene_radius = 1.0f;
        path.build_failed = true;
        require(mapple::save_camera_path(file.name, source, {path}), "saving the refused-build fixture failed");

        KeyFrameInterpolator loaded(camera.frame());
        mapple::LoadedCameraPath result;
        require(mapple::read_camera_path(file.name, &loaded, result), "the refused-build fixture blocked keyframe loading");
        require(result.generated_paths.empty(), "a refused rebuild produced non-restorable provenance");
        require_same_path(source, loaded);
    }

} // namespace

int main() {
    return easy3d::test::run("camera path file", [] {
        test_stream_io();
        test_positive_scene_box_without_playback();
        test_orientation_edits_preserve_provenance();
        test_edited_keyframe_round_trip();
        test_walk_and_manual_gap();
        test_view_up_without_scene_box();
        test_flight();
        test_multiple_blocks();
        test_orbit();
        test_fallbacks();
        test_refused_build_fallback();
    });
}
