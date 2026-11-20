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

/** ----------------------------------------------------------
 *
 * The code in this file is adapted from libQGLViewer with significant
 * modifications and enhancement.
 *		- libQGLViewer (Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 *      http://libqglviewer.com/
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/key_frame_interpolator.h>

#include <chrono>
#include <thread>
#include <limits>
#include <stdexcept>
#include <cmath>

#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>   // for drawing the camera path drawables
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <easy3d/util/string.h>  // for formatting time string


namespace easy3d {


    KeyFrameInterpolator::KeyFrameInterpolator(Frame *frame)
            : frame_(frame)
            , fps_(30)
            , interpolation_method_(INTERPOLATION)
            , interpolation_speed_(1.0f)
            , interpolation_started_(false)
            , last_stopped_index_(0)
            , pathIsValid_(false)
            , path_drawable_(nullptr)
            , cameras_drawable_(nullptr)
    {
    }


    KeyFrameInterpolator::~KeyFrameInterpolator() {
        delete_path();
    }


    bool KeyFrameInterpolator::add_keyframe(const Frame &frame) {
        // interval of each consecutive keyframes is 1.0 sec.
        float time = 0.0f;
        if (keyframes_.empty())
            time = 0.0;
        else
            time = keyframes_.back().time() + 1.0f;

        return add_keyframe(frame, time);
    }


    bool KeyFrameInterpolator::add_keyframe(const Frame &frame, float time) {
        if ((!keyframes_.empty()) && (keyframes_.back().time() >= time)) {
            LOG(ERROR) << "time is not monotone";
            return false;
        }
        else {
            // detect and eliminate duplicated camera positions
            if (!keyframes_.empty()) {
                const float sd = distance2(keyframes_.back().position(), frame.position());
                if (sd < epsilon_sqr<float>()) {
                    LOG(WARNING) << "could not add keyframe: camera position too close to the previous one (distance: "
                                 << std::sqrt(sd) << ")";
                    return false;
                }
            }

            keyframes_.emplace_back(frame, time);
            pathIsValid_ = false;
            last_stopped_index_ = 0; // may not be valid anymore
            stop_interpolation();
            return true;
        }
    }


    void KeyFrameInterpolator::delete_last_keyframe() {
        stop_interpolation();
        keyframes_.pop_back();
        pathIsValid_ = false;
        last_stopped_index_ = 0; // may not be valid anymore
    }


    void KeyFrameInterpolator::set_keyframe_time(std::size_t index, float t) {
        stop_interpolation();
        keyframes_.at(index).set_time(t);
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_keyframe_position(std::size_t index, const vec3& pos) {
        stop_interpolation();
        keyframes_.at(index).set_position(pos);
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_keyframe_orientation(std::size_t index, const quat& q) {
        stop_interpolation();
        keyframes_.at(index).set_orientation(q);
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_keyframe_dwell(std::size_t index, float seconds) {
        stop_interpolation();
        keyframes_.at(index).set_dwell_time(seconds);
        pathIsValid_ = false;
    }


    float KeyFrameInterpolator::keyframe_dwell(std::size_t index) const {
        return keyframes_.at(index).dwell_time();
    }


    void KeyFrameInterpolator::set_speed_profile(SpeedProfile profile) {
        stop_interpolation();
        speed_profile_ = profile;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::delete_path() {
        stop_interpolation();
        keyframes_.clear();
        interpolated_path_.clear();
        pathIsValid_ = false;
        last_stopped_index_ = 0;

        drop_immediate_drawables();
    }


    Frame KeyFrameInterpolator::keyframe(std::size_t index) const {
        const Keyframe& kf = keyframes_.at(index);
        return {kf.position(), kf.orientation()};
    }


    float KeyFrameInterpolator::keyframe_time(std::size_t index) const {
        return keyframes_.at(index).time();
    }


    const vec3& KeyFrameInterpolator::keyframe_position(std::size_t index) const {
        return keyframes_.at(index).position();
    }


    const quat& KeyFrameInterpolator::keyframe_orientation(std::size_t index) const {
        return keyframes_.at(index).orientation();
    }


    float KeyFrameInterpolator::duration() const {
        return last_time() - first_time();
    }


    float KeyFrameInterpolator::first_time() const {
        if (keyframes_.empty())
            return 0.0f;
        return keyframes_.front().time();
    }


    float KeyFrameInterpolator::last_time() const {
        if (keyframes_.empty())
            return 0.0f;
        return keyframes_.back().time();
    }


    void  KeyFrameInterpolator::set_interpolation_method(Method m) {
        interpolation_method_ = m;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_interpolation_speed(float speed) {
        if (!std::isfinite(speed) || speed<=0) throw std::invalid_argument("interpolation speed must be positive and finite");
        stop_interpolation();
        interpolation_speed_ = speed;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_frame_rate(int fps) {
        if (fps<=0 || fps>1000) throw std::invalid_argument("interpolation frame rate must be in 1..1000");
        stop_interpolation();
        fps_ = fps;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::start_interpolation() {
        if (manual_playback_) stop_interpolation();
        manual_playback_ = false;
        auto animation = [this]() {
            // interval in ms (0.9 to approximately compensate the overhead the timer thread and viewer update)
            const int interval = static_cast<int>(1000.0f / static_cast<float>(frame_rate()) * 0.9f);
            for (int id = last_stopped_index_; id < interpolated_path_.size(); ++id) {
                if (timer_.is_stopped()) {
                    last_stopped_index_ = id;
                    break;
                }
                const auto &f = interpolated_path_[id];
                frame()->set_position_and_orientation(f.position(), f.orientation());
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (id == interpolated_path_.size() - 1)  // reaches the end frame
                    last_stopped_index_ = 0;
                frame_interpolated.send();
            }
            interpolation_stopped.send();
            interpolation_started_ = false;
        };

        if (keyframes_.empty())
            return;

        if (!pathIsValid_)
            interpolate();

        // all done in another thread.
        interpolation_started_ = true;
        timer_mode_used_ = true;
        timer_.set_timeout(0, animation);
    }


    void KeyFrameInterpolator::start_manual_interpolation() {
        if (timer_mode_used_) throw std::logic_error("manual interpolation requires a fresh, timer-free interpolator");
        stop_interpolation();
        if (!frame_ || keyframes_.empty()) return;
        if (!pathIsValid_) interpolate();
        if (interpolated_path_.empty()) return;
        if (last_stopped_index_ < 0 || size_t(last_stopped_index_) >= interpolated_path_.size()) last_stopped_index_=0;
        manual_playback_=true;
        interpolation_started_=true;
    }

    bool KeyFrameInterpolator::advance_interpolation() {
        if (!manual_playback_ || !interpolation_started_) return false;
        if (!frame_) {stop_interpolation();return false;}
        const auto& sample=interpolated_path_.at(size_t(last_stopped_index_));
        frame_->set_position_and_orientation(sample.position(),sample.orientation());
        const bool finished=++last_stopped_index_>=int(interpolated_path_.size());
        if (finished) {last_stopped_index_=0;interpolation_started_=false;manual_playback_=false;}
        frame_interpolated.send();
        if (finished) interpolation_stopped.send();
        return true;
    }

    void KeyFrameInterpolator::stop_interpolation() {
        timer_.stop();
        interpolation_started_ = false;
        manual_playback_ = false;
    }


    void KeyFrameInterpolator::draw_cameras(const Camera *camera, float camera_width, const vec4 &color) {
        if (keyframes_.empty())
            return;
        if (!pathIsValid_) {
            drop_immediate_drawables();
            interpolated_path_ = interpolate();
            pathIsValid_ = true;
        }
        if (interpolated_path_.empty()) // interpolation may have failed.
            return;

        if (!cameras_drawable_) {
            std::vector<vec3> points;
            std::vector<unsigned int> indices;
            append_camera_glyphs(camera_width, camera->field_of_view(),
                                 static_cast<float>(camera->screen_height()) / static_cast<float>(camera->screen_width()),
                                 points, indices);
            if (points.size() > 1) {
                cameras_drawable_ = new TrianglesDrawable;
                cameras_drawable_->update_vertex_buffer(points);
                cameras_drawable_->update_element_buffer(indices);
                cameras_drawable_->set_uniform_coloring(color);
            }
        }

        if (cameras_drawable_) {
            cameras_drawable_->set_uniform_coloring(color);
            cameras_drawable_->draw(camera);
        }
    }


    bool KeyFrameInterpolator::update_cameras_drawable(TrianglesDrawable &drawable, const Camera *camera,
                                                       float camera_width, const vec4 &color) const {
        if (keyframes_.empty() || !camera || !(camera_width > 0.0f) || !std::isfinite(camera_width))
            return false;
        std::vector<vec3> points;
        std::vector<unsigned int> indices;
        append_camera_glyphs(camera_width, camera->field_of_view(),
                             static_cast<float>(camera->screen_height()) / static_cast<float>(camera->screen_width()),
                             points, indices);
        drawable.update_vertex_buffer(points);
        drawable.update_element_buffer(indices);
        drawable.set_uniform_coloring(color);
        return true;
    }


    std::size_t KeyFrameInterpolator::camera_glyph_triangles() {
        // The angle only moves the glyph vertices; the triangle count is the same.
        std::vector<vec3> points;
        std::vector<unsigned int> indices;
        shape::create_camera(points, indices, 1.0f, 0.8f, 1.0f);
        return indices.size() / 3;
    }


    void KeyFrameInterpolator::drop_immediate_drawables() {
        delete path_drawable_;
        path_drawable_ = nullptr;
        delete cameras_drawable_;
        cameras_drawable_ = nullptr;
    }


    void KeyFrameInterpolator::append_camera_glyphs(float camera_width, float fov, float aspect,
                                                    std::vector<vec3> &points,
                                                    std::vector<unsigned int> &indices) const {
        if (keyframes_.empty())
            return;
        std::vector<vec3> glyph_points;
        std::vector<unsigned int> glyph_indices;
        shape::create_camera(glyph_points, glyph_indices, camera_width, fov, aspect);
        points.reserve(points.size() + glyph_points.size() * keyframes_.size());
        indices.reserve(indices.size() + glyph_indices.size() * keyframes_.size());
        for (const auto &frame : keyframes_) {
            const mat4 m = Frame(frame.position(), frame.orientation()).matrix();
            const auto offset = static_cast<unsigned int>(points.size());
            for (const auto &p : glyph_points)
                points.push_back(m * p);
            for (auto id : glyph_indices)
                indices.push_back(offset + id);
        }
    }


    namespace {
        // Consecutive sample pairs, ready for a LinesDrawable.
        std::vector<vec3> path_segments(const std::vector<Frame> &samples) {
            std::vector<vec3> points;
            if (samples.size() < 2)
                return points;
            points.reserve((samples.size() - 1) * 2);
            for (std::size_t i = 0; i + 1 < samples.size(); ++i) {
                points.push_back(samples[i].position());
                points.push_back(samples[i + 1].position());
            }
            return points;
        }

        // Smoothstep ease-in/out: 3w^2 - 2w^3. Flat derivative at both ends.
        inline float ease_smoothstep(float w) {
            return 3.0f * w * w - 2.0f * w * w * w;
        }

        // Remap the within-segment parameter w (0 = at start keyframe, 1 = at end keyframe)
        // according to the speed profile. slow_start/slow_end request extra flattening at
        // the corresponding end (used by CINEMATIC next to keyframes with dwell).
        inline float ease_parameter(float w, KeyFrameInterpolator::SpeedProfile profile,
                                    bool slow_start, bool slow_end) {
            if (profile == KeyFrameInterpolator::SpeedProfile::CONSTANT)
                return w;
            float w_eased = ease_smoothstep(w);  // EASE_IN_OUT base
            if (profile == KeyFrameInterpolator::SpeedProfile::CINEMATIC) {
                if (slow_start)
                    w_eased = 1.0f - ease_smoothstep(1.0f - w_eased);  // extra flat at start
                if (slow_end)
                    w_eased = ease_smoothstep(w_eased);                // extra flat at end
            }
            return w_eased;
        }
    } // namespace


    bool KeyFrameInterpolator::update_path_drawable(LinesDrawable &drawable, float thickness, const vec4 &color) {
        if (!pathIsValid_)
            drop_immediate_drawables();
        const std::vector<vec3> points = path_segments(interpolate());
        if (points.empty())
            return false;
        drawable.update_vertex_buffer(points);
        drawable.set_uniform_coloring(color);
        drawable.set_line_width(thickness);
        // drawable.set_impostor_type(LinesDrawable::CYLINDER);
        return true;
    }


    void KeyFrameInterpolator::draw_path(const Camera *camera, float thickness, const vec4 &color) {
        if (keyframes_.empty())
            return;
        if (!pathIsValid_) {
            drop_immediate_drawables();
            interpolated_path_ = interpolate();
            pathIsValid_ = true;
        }
        if (interpolated_path_.empty()) // interpolation may have failed.
            return;

        if (!path_drawable_) {
            auto points = path_segments(interpolated_path_);
            if (points.size() > 1) {
                path_drawable_ = new LinesDrawable;
                path_drawable_->update_vertex_buffer(points);
                path_drawable_->set_uniform_coloring(color);
                path_drawable_->set_line_width(thickness);
                path_drawable_->set_impostor_type(LinesDrawable::CYLINDER);
            }
        }

        if (path_drawable_)
            path_drawable_->draw(camera);
    }


    bool KeyFrameInterpolator::save_keyframes(const std::string& file_name) const {
        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        output << "\tnum_key_frames: " << keyframes_.size() << std::endl;

        for (std::size_t id = 0; id < keyframes_.size(); ++id) {
            output << "\tframe: " << id << std::endl;
            const Keyframe& kf = keyframes_[id];
            output << "\t\tposition: " << kf.position() << std::endl;
            output << "\t\torientation: " << kf.orientation() << std::endl;
        }

        return !keyframes_.empty();
    }


    bool KeyFrameInterpolator::read_keyframes(const std::string& file_name) {
        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        // clean
        delete_path();

        // load path from file
        std::string dummy;
        int num_key_frames;
        input >> dummy >> num_key_frames;
        for (int j = 0; j < num_key_frames; ++j) {
            int frame_id;
            input >> dummy >> frame_id;
            vec3 pos;
            quat orient;
            input >> dummy >> pos >> dummy >> orient;
            add_keyframe(Frame(pos, orient));
        }

        return !keyframes_.empty();
    }


    const std::vector<Frame>& KeyFrameInterpolator::interpolate() {
        if (pathIsValid_ || keyframes_.empty()) // already fitted or no keyframe
            return interpolated_path_;

        interpolated_path_.clear();
        const std::size_t n = keyframes_.size();
        if (n == 1) {   // only one keyframe
            LOG(INFO) << "only 1 keyframe: interpolation not possible";
            interpolated_path_.emplace_back(keyframes_[0].position(), keyframes_[0].orientation());
            pathIsValid_=true;last_stopped_index_=0;
            return interpolated_path_;
        }

        if (frame_rate()<=0 || frame_rate()>1000 || !std::isfinite(interpolation_speed()) || interpolation_speed()<=0)
            throw std::invalid_argument("interpolation requires frame rate 1..1000 and positive finite speed");
        const float interval = interpolation_speed() * static_cast<float>(interpolation_period()) / 1000.0f;

        // Total effective duration = travel time between keyframes + dwell time at each keyframe.
        float total_dwell = 0.0f, total_travel = 0.0f;
        for (std::size_t i = 0; i < n; ++i) {
            total_dwell += keyframes_[i].dwell_time();
            if (i + 1 < n)
                total_travel += keyframes_[i + 1].time() - keyframes_[i].time();
        }
        const float total = total_dwell + total_travel;
        const double count = double(total) / interval + 1;
        if (!std::isfinite(count) || count>std::numeric_limits<int>::max())
            throw std::invalid_argument("interpolation path has too many samples");
        const int num_frames = std::max(2,static_cast<int>(count));

        if (n == 2) {   // only two keyframes: linear interpolation with optional end dwells
            LOG(INFO) << "only two keyframes: linear interpolation";
            // Timeline: dwell at keyframe 0, travel 0->1, dwell at keyframe 1.
            const float dwell0 = keyframes_[0].dwell_time();
            const float travel = keyframes_[1].time() - keyframes_[0].time();
            const float dwell1 = keyframes_[1].dwell_time();
            const float seg0_end = dwell0;
            const float seg1_end = seg0_end + travel;
            for (int i = 0; i < num_frames; ++i) {
                const float t_k = std::min(static_cast<float>(i) * interval, total);
                vec3 pos;
                quat orient;
                if (t_k <= seg0_end) {
                    pos = keyframes_[0].position();
                    orient = keyframes_[0].orientation();
                } else if (t_k <= seg1_end) {
                    const float w = (t_k - seg0_end) / travel;
                    // Apply speed profile within the travel segment.
                    const bool slow_start = dwell0 > 0.0f;
                    const bool slow_end = dwell1 > 0.0f;
                    const float w_eased = ease_parameter(w, speed_profile_, slow_start, slow_end);
                    pos = (1.0f - w_eased) * keyframes_[0].position() + w_eased * keyframes_[1].position();
                    orient = quat::slerp(keyframes_[0].orientation(), keyframes_[1].orientation(), w_eased);
                } else {
                    pos = keyframes_[1].position();
                    orient = keyframes_[1].orientation();
                }
                interpolated_path_.emplace_back(pos, orient.normalized());
            }
            pathIsValid_ = true;
            last_stopped_index_ = 0; // not valid anymore
            return interpolated_path_;
        }

        quat prevQ = keyframes_.front().orientation();
        for (auto& frame : keyframes_) {
            frame.flip_if_needed(prevQ);
            prevQ = frame.orientation();
        }

        LOG_IF(keyframes_.size() > 1, INFO) << "interpolating " << keyframes_.size() << " keyframes...";

        // Orientation: squad tangents at every keyframe (shared by both methods).
        std::vector<quat> tangents(n);
        for (std::size_t i = 0; i < n; ++i) {
            if (i == 0)
                tangents[i] = keyframes_[0].orientation();
            else if (i == n - 1)
                tangents[i] = keyframes_[n - 1].orientation();
            else
                tangents[i] = quat::squad_tangent(
                    keyframes_[i - 1].orientation(),
                    keyframes_[i].orientation(),
                    keyframes_[i + 1].orientation());
        }

        // Position fitter (method-specific).
        typedef SplineCurveInterpolation<Vec, 3, float> PosInterpFitter;
        typedef SplineCurveFitting <Vec, 3, float> PosFitFitter;
        PosInterpFitter pos_interp_fitter;
        PosFitFitter pos_fit_fitter(3, PosFitFitter::eOPEN_UNIFORM);
        std::vector<float> arc_params;      // INTERPOLATION: accumulated arc length per keyframe
        std::vector<vec3> arc_positions;    // INTERPOLATION: keyframe positions
        float largest_t = 0.0f;             // INTERPOLATION: total arc length
        if (interpolation_method_ == INTERPOLATION) {
            arc_params.resize(n);
            arc_positions.resize(n);
            float dist(0.0f);
            for (std::size_t i = 0; i < n; ++i) {
                arc_positions[i] = keyframes_[i].position();
                if (i > 0)
                    dist += distance(arc_positions[i-1], arc_positions[i]);
                arc_params[i] = dist;
            }
            pos_interp_fitter.set_boundary(PosInterpFitter::second_deriv, 0, PosInterpFitter::second_deriv, 0);
            pos_interp_fitter.set_points(arc_params, arc_positions);
            largest_t = arc_params.back();
        } else {
            std::vector<vec3> positions(n);
            for (std::size_t i = 0; i < n; ++i)
                positions[i] = keyframes_[i].position();
            pos_fit_fitter.set_ctrl_points(positions);
        }

        // Walk the timeline: dwell at keyframe i, then travel to keyframe i+1.
        // t_cursor tracks the global time (dwell + travel) at the start of each segment.
        interpolated_path_.reserve(num_frames);
        float t_cursor = 0.0f;
        for (int k = 0; k < num_frames; ++k) {
            const float t_k = std::min(static_cast<float>(k) * interval, total);

            // Advance the segment cursor while t_k is past this segment's end.
            std::size_t seg = 0;
            float seg_start = 0.0f;
            while (seg < n) {
                const float seg_dwell = keyframes_[seg].dwell_time();
                const float seg_travel = (seg + 1 < n) ? (keyframes_[seg + 1].time() - keyframes_[seg].time()) : 0.0f;
                const float seg_end = seg_start + seg_dwell + seg_travel;
                if (t_k <= seg_end) break;
                seg_start = seg_end;
                ++seg;
            }

            if (seg >= n) {
                // t_k == total: end on the last keyframe.
                interpolated_path_.emplace_back(keyframes_[n-1].position(), keyframes_[n-1].orientation());
                continue;
            }

            const float dwell_dur = keyframes_[seg].dwell_time();
            const float dwell_end = seg_start + dwell_dur;

            if (t_k <= dwell_end) {
                // Pause at keyframe seg.
                interpolated_path_.emplace_back(keyframes_[seg].position(), keyframes_[seg].orientation());
            } else if (seg + 1 < n) {
                // Travel from keyframe seg to seg+1.
                const float travel_start = dwell_end;
                const float travel_dur = keyframes_[seg + 1].time() - keyframes_[seg].time();
                const float w = (t_k - travel_start) / travel_dur;

                // Apply the speed profile: ease-in/out within each travel segment, with
                // extra slowing near keyframes that have a non-zero dwell time (CINEMATIC).
                const bool slow_start = keyframes_[seg].dwell_time() > 0.0f;
                const bool slow_end = keyframes_[seg + 1].dwell_time() > 0.0f;
                const float w_eased = ease_parameter(w, speed_profile_, slow_start, slow_end);

                // Global travel fraction (0 at keyframe 0, 1 at keyframe n-1).
                // Cumulative travel time up to this segment plus the eased travel within it.
                float cum_travel = 0.0f;
                for (std::size_t i = 0; i < seg; ++i)
                    cum_travel += keyframes_[i + 1].time() - keyframes_[i].time();
                const float u_global = (cum_travel + w_eased * travel_dur) / total_travel;

                vec3 pos;
                if (interpolation_method_ == INTERPOLATION) {
                    // Arc-length spline parameter in [0,1].
                    pos = pos_interp_fitter.eval_f(u_global);
                } else {
                    // FITTING: map w_eased linearly within the B-spline parameter range for this
                    // keyframe-to-keyframe transition. eOPEN_UNIFORM passes through the
                    // endpoints, so u = seg/(n-1) is approximately keyframe seg's position.
                    const float u = static_cast<float>(seg) / static_cast<float>(n - 1)
                                  + w_eased * (1.0f / static_cast<float>(n - 1));
                    pos = pos_fit_fitter.eval_f(u);
                }

                const quat orient = quat::squad(
                    keyframes_[seg].orientation(),
                    tangents[seg],
                    tangents[seg + 1],
                    keyframes_[seg + 1].orientation(),
                    w_eased);

                interpolated_path_.emplace_back(pos, orient);
            } else {
                // Last keyframe: only dwell remains (handled by the dwell branch above).
                interpolated_path_.emplace_back(keyframes_[seg].position(), keyframes_[seg].orientation());
            }
        }

        LOG_IF(keyframes_.size() > 2, INFO)
                        << "keyframe interpolation done: "
                        << interpolated_path_.size() << " frames, "
                        << string::time(total / interpolation_speed() * 1000)
                        << " (at speed " << interpolation_speed() << "x)";

        pathIsValid_ = true;
        last_stopped_index_ = 0; // not valid anymore
        return interpolated_path_;
    }



    // \cond

    //////////// Keyframe private class implementation /////////
    KeyFrameInterpolator::Keyframe::Keyframe(const Frame &fr, float t)
            : time_(t) {
        p_ = fr.position();
        q_ = fr.orientation();
    }


    void KeyFrameInterpolator::Keyframe::flip_if_needed(const quat &prev) {
        if (quat::dot(prev, q_) < 0.0f)
            q_.negate();
    }

    // \endcond
}

