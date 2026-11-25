#include "camera_path_io.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>
#include "camera_path.h"
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/keyframe_interpolator.h>
#include <easy3d/util/logging.h>

#include <3rd_party/nlohmann/json.hpp>

using namespace easy3d;
using json = nlohmann::json;

namespace mapple {
namespace {

    constexpr int kProvenanceVersion = 1;

    json write_vec3(const vec3 &value) {
        return {value.x, value.y, value.z};
    }

    bool read_vec3(const json &value, vec3 &result) {
        if (!value.is_array() || value.size() != 3)
            return false;
        for (int i = 0; i < 3; ++i) {
            if (!value[i].is_number())
                return false;
            result[i] = value[i].get<float>();
            if (!std::isfinite(result[i]))
                return false;
        }
        return true;
    }

    const char *mode_name(CameraPath::Source::Kind kind) {
        switch (kind) {
            case CameraPath::Source::Kind::Walk:
                return "walk";
            case CameraPath::Source::Kind::Flight:
                return "flight";
            case CameraPath::Source::Kind::Orbit:
                return "orbit";
            case CameraPath::Source::Kind::None:
                break;
        }
        return "";
    }

    bool read_mode(const std::string &name, CameraPath::Source::Kind &kind) {
        if (name == "walk")
            kind = CameraPath::Source::Kind::Walk;
        else if (name == "flight")
            kind = CameraPath::Source::Kind::Flight;
        else if (name == "orbit")
            kind = CameraPath::Source::Kind::Orbit;
        else
            return false;
        return true;
    }

    const char *up_axis_name(CameraPath::GroundUpAxis axis) {
        switch (axis) {
            case CameraPath::GroundUpAxis::Auto:
                return "auto";
            case CameraPath::GroundUpAxis::X:
                return "x";
            case CameraPath::GroundUpAxis::Y:
                return "y";
            case CameraPath::GroundUpAxis::Z:
                return "z";
            case CameraPath::GroundUpAxis::View:
                return "view";
        }
        return "";
    }

    bool read_up_axis(const std::string &name, CameraPath::GroundUpAxis &axis) {
        if (name == "auto")
            axis = CameraPath::GroundUpAxis::Auto;
        else if (name == "x")
            axis = CameraPath::GroundUpAxis::X;
        else if (name == "y")
            axis = CameraPath::GroundUpAxis::Y;
        else if (name == "z")
            axis = CameraPath::GroundUpAxis::Z;
        else if (name == "view")
            axis = CameraPath::GroundUpAxis::View;
        else
            return false;
        return true;
    }

    json write_settings(const CameraPath::Settings &settings) {
        return {
                {"ground_up_axis", up_axis_name(settings.ground_up_axis)},
                {"eye_height", settings.eye_height_in_scene_heights},
                {"keyframe_spacing", settings.keyframe_spacing_in_scene_heights},
                {"follow_ground_slopes", settings.follow_ground_slopes},
                {"flight_height", settings.flight_height_in_scene_heights},
                {"orbit_radius_offset", settings.orbit_radius_offset_in_object_heights},
                {"orbit_vertical_offset", settings.orbit_vertical_offset_in_object_heights},
                {"pitch_degrees", settings.pitch_degrees},
                {"keyframes_per_loop", settings.keyframes_per_loop}
        };
    }

    bool finite(float value) {
        return std::isfinite(value);
    }

    bool read_settings(const json &value, CameraPath::Settings &settings) {
        if (!value.is_object())
            return false;
        settings = CameraPath::Settings();
        if (!read_up_axis(value.at("ground_up_axis").get<std::string>(), settings.ground_up_axis))
            return false;
        settings.eye_height_in_scene_heights = value.at("eye_height").get<float>();
        settings.keyframe_spacing_in_scene_heights = value.at("keyframe_spacing").get<float>();
        settings.follow_ground_slopes = value.at("follow_ground_slopes").get<bool>();
        settings.flight_height_in_scene_heights = value.at("flight_height").get<float>();
        settings.orbit_radius_offset_in_object_heights = value.at("orbit_radius_offset").get<float>();
        settings.orbit_vertical_offset_in_object_heights = value.at("orbit_vertical_offset").get<float>();
        settings.pitch_degrees = value.at("pitch_degrees").get<float>();
        settings.keyframes_per_loop = value.at("keyframes_per_loop").get<int>();
        return finite(settings.eye_height_in_scene_heights) &&
               settings.eye_height_in_scene_heights >= 0.0f && settings.eye_height_in_scene_heights <= 100.0f &&
               finite(settings.keyframe_spacing_in_scene_heights) &&
               settings.keyframe_spacing_in_scene_heights >= 0.01f && settings.keyframe_spacing_in_scene_heights <= 10.0f &&
               finite(settings.flight_height_in_scene_heights) &&
               settings.flight_height_in_scene_heights >= 0.0f && settings.flight_height_in_scene_heights <= 100.0f &&
               finite(settings.orbit_radius_offset_in_object_heights) &&
               std::abs(settings.orbit_radius_offset_in_object_heights) <= 100.0f &&
               finite(settings.orbit_vertical_offset_in_object_heights) &&
               std::abs(settings.orbit_vertical_offset_in_object_heights) <= 100.0f &&
               finite(settings.pitch_degrees) && std::abs(settings.pitch_degrees) <= 89.0f &&
               settings.keyframes_per_loop >= 2 && settings.keyframes_per_loop <= 99;
    }

    json write_source(const CameraPath::Source &source) {
        json strokes = json::array();
        for (const auto &stroke : source.strokes) {
            json points = json::array();
            for (const auto &point : stroke)
                points.push_back(write_vec3(point));
            strokes.push_back(std::move(points));
        }
        json aims = json::array();
        for (const auto &aim : source.aims)
            aims.push_back(write_vec3(aim));
        return {
                {"mode", mode_name(source.kind)},
                {"strokes", std::move(strokes)},
                {"anchor", write_vec3(source.anchor)},
                {"center", write_vec3(source.center)},
                {"aims", std::move(aims)}
        };
    }

    bool read_source(const json &value, CameraPath::Source &source) {
        if (!value.is_object() || !read_mode(value.at("mode").get<std::string>(), source.kind))
            return false;
        source.strokes.clear();
        const auto &strokes = value.at("strokes");
        if (!strokes.is_array())
            return false;
        for (const auto &stroke : strokes) {
            if (!stroke.is_array())
                return false;
            std::vector<vec3> points;
            for (const auto &point : stroke) {
                vec3 p;
                if (!read_vec3(point, p))
                    return false;
                points.push_back(p);
            }
            source.strokes.push_back(std::move(points));
        }
        source.aims.clear();
        const auto &aims = value.at("aims");
        if (!aims.is_array())
            return false;
        for (const auto &aim : aims) {
            vec3 p;
            if (!read_vec3(aim, p))
                return false;
            source.aims.push_back(p);
        }
        return read_vec3(value.at("anchor"), source.anchor) &&
               read_vec3(value.at("center"), source.center);
    }

    json write_playback(const KeyframeInterpolator &interpolator) {
        return {
                {"frame_rate", interpolator.frame_rate()},
                {"interpolation_speed", interpolator.interpolation_speed()},
                {"interpolation_method", interpolator.interpolation_method() == KeyframeInterpolator::INTERPOLATION
                                         ? "interpolation" : "fitting"}
        };
    }

    bool read_playback(const json &value, KeyframeInterpolator &interpolator) {
        if (!value.is_object())
            return false;
        const int fps = value.at("frame_rate").get<int>();
        const float speed = value.at("interpolation_speed").get<float>();
        const std::string method = value.at("interpolation_method").get<std::string>();
        if (fps < 1 || fps > 1000 || !finite(speed) || speed <= 0.0f ||
            (method != "interpolation" && method != "fitting"))
            return false;
        interpolator.set_interpolation_method(method == "interpolation"
                                              ? KeyframeInterpolator::INTERPOLATION
                                              : KeyframeInterpolator::FITTING);
        interpolator.set_interpolation_speed(speed);
        interpolator.set_frame_rate(fps);
        return true;
    }

    json write_generated_path(const GeneratedPath &path) {
        return {
                {"source", write_source(path.source)},
                {"settings", write_settings(path.built_settings)},
                {"scene_box_valid", path.scene_box.is_valid()},
                {"scene_min", write_vec3(path.scene_box.min_point())},
                {"scene_max", write_vec3(path.scene_box.max_point())},
                {"scene_radius", path.scene_radius},
                {"view_up", write_vec3(path.view_up)},
                {"first", path.first},
                {"last", path.last},
                {"manual_before", path.manual_before}
        };
    }

    bool valid_manual_gaps(const GeneratedPath &path) {
        if (static_cast<int>(path.manual_before.size()) >= path.span() ||
            !std::is_sorted(path.manual_before.begin(), path.manual_before.end()) ||
            std::adjacent_find(path.manual_before.begin(), path.manual_before.end()) != path.manual_before.end())
            return false;
        for (std::size_t i = 0; i < path.manual_before.size(); ++i) {
            if (path.manual_before[i] < 0 || path.manual_before[i] > path.size() ||
                path.manual_position(i) > path.last)
                return false;
        }
        return true;
    }

    bool read_generated_path(const json &value, GeneratedPath &path, int keyframe_count, int &cursor) {
        if (!value.is_object() || !read_source(value.at("source"), path.source) ||
            !read_settings(value.at("settings"), path.built_settings))
            return false;
        path.has_build_context = true;
        path.scene_box.clear();
        const bool box_valid = value.at("scene_box_valid").get<bool>();
        vec3 lo, hi;
        if (!read_vec3(value.at("scene_min"), lo) || !read_vec3(value.at("scene_max"), hi))
            return false;
        if (box_valid) {
            if (lo.x > hi.x || lo.y > hi.y || lo.z > hi.z)
                return false;
            path.scene_box = Box3(lo, hi);
        }
        path.scene_radius = value.at("scene_radius").get<float>();
        if (!read_vec3(value.at("view_up"), path.view_up) ||
            path.view_up.length2() <= std::numeric_limits<float>::epsilon() ||
            !finite(path.scene_radius) || path.scene_radius <= 0.0f)
            return false;
        path.first = value.at("first").get<int>();
        path.last = value.at("last").get<int>();
        if (path.first < cursor || path.last < path.first || path.last >= keyframe_count)
            return false;
        path.manual_before = value.at("manual_before").get<std::vector<int>>();
        if (!valid_manual_gaps(path))
            return false;
        path.pending = false;
        cursor = path.last + 1;
        return true;
    }

    bool same_position(const vec3 &a, const vec3 &b) {
        return distance2(a, b) <= 1e-10f;
    }

    std::vector<Frame> regenerate(const GeneratedPath &path) {
        Camera camera;
        camera.set_scene_radius(path.scene_radius);
        camera.set_up_vector(path.view_up);
        CameraPath camera_path(&camera, nullptr);
        camera_path.set_scene_box(path.scene_box);
        camera_path.set_settings(path.built_settings);
        return camera_path.frames_of(path.source);
    }

    bool matches_generated_path(const GeneratedPath &path, const KeyframeInterpolator &interpolator,
                                const std::vector<Frame> &generated) {
        std::vector<int> manual_positions;
        for (std::size_t i = 0; i < path.manual_before.size(); ++i)
            manual_positions.push_back(path.manual_position(i));
        std::vector<int> old_generated;
        for (int i = path.first; i <= path.last; ++i) {
            if (!std::binary_search(manual_positions.begin(), manual_positions.end(), i))
                old_generated.push_back(i);
        }

        std::size_t manual = 0;
        std::size_t accepted = 0;
        int previous = path.first - 1;
        for (std::size_t i = 0; i <= generated.size(); ++i) {
            while (manual < path.manual_before.size() && path.manual_before[manual] <= static_cast<int>(i)) {
                previous = path.manual_position(manual);
                ++manual;
            }
            if (i == generated.size())
                break;
            if (previous >= 0 && distance2(interpolator.keyframe(static_cast<std::size_t>(previous)).position(),
                                           generated[i].position()) < epsilon_sqr<float>())
                continue;
            if (accepted >= old_generated.size() ||
                !same_position(interpolator.keyframe_position(static_cast<std::size_t>(old_generated[accepted])),
                               generated[i].position()))
                return false;
            previous = old_generated[accepted++];
        }
        return accepted == old_generated.size();
    }

    bool restore_generated_paths(const json &provenance, KeyframeInterpolator *interpolator,
                                 LoadedCameraPath &loaded) {
        if (!provenance.is_object() || !provenance.at("paths").is_array() ||
            provenance.at("version").get<int>() != kProvenanceVersion ||
            provenance.at("keyframes").get<int>() != static_cast<int>(interpolator->number_of_keyframes()))
            return false;

        std::vector<GeneratedPath> paths;
        int cursor = 0;
        for (const auto &value : provenance.at("paths")) {
            GeneratedPath path;
            if (!read_generated_path(value, path, static_cast<int>(interpolator->number_of_keyframes()), cursor))
                return false;
            paths.push_back(std::move(path));
        }
        if (paths.empty())
            return false;

        for (const GeneratedPath &path : paths) {
            // Serialized orientations can contain manual aim edits; only generated
            // positions determine whether the source still reproduces this block.
            if (!matches_generated_path(path, *interpolator, regenerate(path)))
                return false;
        }

        const bool periodic = paths.size() == 1 && paths.front().source.kind == CameraPath::Source::Kind::Orbit &&
                              paths.front().first == 0 &&
                              paths.front().last == static_cast<int>(interpolator->number_of_keyframes()) - 1;
        if (provenance.find("playback") != provenance.end() &&
            !read_playback(provenance.at("playback"), *interpolator))
            return false;
        interpolator->set_path_periodic(periodic);
        loaded.generated_paths = std::move(paths);
        loaded.settings = loaded.generated_paths.back().built_settings;
        return true;
    }

    bool serializable(const KeyframeInterpolator &interpolator, const std::vector<GeneratedPath> &paths) {
        if (paths.empty())
            return false;
        const int count = static_cast<int>(interpolator.number_of_keyframes());
        int cursor = 0;
        for (const GeneratedPath &path : paths) {
            if (!path.has_build_context || path.pending || path.build_failed ||
                path.source.kind == CameraPath::Source::Kind::None ||
                path.first < cursor || path.last < path.first || path.last >= count ||
                !valid_manual_gaps(path) || !finite(path.scene_radius) ||
                path.scene_radius <= 0.0f || path.view_up.length2() <= std::numeric_limits<float>::epsilon())
                return false;
            cursor = path.last + 1;
        }
        return true;
    }

} // namespace

bool save_camera_path(const std::string &file_name,
                      const KeyframeInterpolator &interpolator,
                      const std::vector<GeneratedPath> &generated_paths) {
    std::ofstream output(file_name.c_str());
    if (output.fail() || !interpolator.save_keyframes(output)) {
        LOG(ERROR) << "could not save camera path to: " << file_name;
        return false;
    }
    if (!serializable(interpolator, generated_paths))
        return !output.fail();

    json paths = json::array();
    for (const GeneratedPath &path : generated_paths)
        paths.push_back(write_generated_path(path));
    const json provenance = {
            {"version", kProvenanceVersion},
            {"keyframes", interpolator.number_of_keyframes()},
            {"playback", write_playback(interpolator)},
            {"paths", std::move(paths)}
    };

    output << "provenance:\n" << provenance.dump(2) << '\n';
    if (!output) {
        LOG(ERROR) << "could not write camera path provenance to: " << file_name;
        return false;
    }
    return true;
}

bool read_camera_path(const std::string &file_name, KeyframeInterpolator *interpolator, LoadedCameraPath &loaded) {
    loaded.generated_paths.clear();
    loaded.settings = CameraPath::Settings();
    std::ifstream input(file_name.c_str());
    if (input.fail() || !interpolator->load_keyframes(input))
        return false;
    input.clear();
    input.seekg(0);
    if (input.fail())
        return true;
    std::string line;
    std::ostringstream body;
    bool found = false;
    while (std::getline(input, line)) {
        const auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            continue;
        const auto last = line.find_last_not_of(" \t\r\n");
        const std::string trimmed = line.substr(first, last - first + 1);
        if (!found) {
            if (trimmed == "provenance:") {
                found = true;
                continue;
            }
        } else
            body << line << '\n';
    }
    if (!found)
        return true;

    try {
        const json provenance = json::parse(body.str());
        if (!restore_generated_paths(provenance, interpolator, loaded)) {
            loaded.generated_paths.clear();
            LOG(WARNING) << "camera path provenance could not reproduce the stored positions; using the keyframes";
        }
    } catch (const std::exception &error) {
        loaded.generated_paths.clear();
        LOG(WARNING) << "ignoring invalid camera path provenance: " << error.what();
    }
    return true;
}

} // namespace mapple
