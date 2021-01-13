/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

/** ----------------------------------------------------------
 *
 * The code in this file is adapted from Surface_mesh with significant
 * modifications and enhancement.
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 *      http://libqglviewer.com/
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/key_frame_interpolator.h>

#include <fstream>
#include <chrono>
#include <thread>

#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/camera.h>   // for drawing the camera path drawables
#include <easy3d/util/string.h>  // for formating time string


namespace easy3d {


    KeyFrameInterpolator::KeyFrameInterpolator(Frame *frame)
            : frame_(frame)
            , fps_(30)
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


    void KeyFrameInterpolator::add_keyframe(const Frame &frame) {
        // interval of each consecutive keyframes is 1.0 sec.
        float time = 0.0f;
        if (keyframes_.empty())
            time = 0.0;
        else
            time = keyframes_.back().time() + 1.0f;

        add_keyframe(frame, time);
    }


    void KeyFrameInterpolator::add_keyframe(const Frame &frame, float time) {
        if ((!keyframes_.empty()) && (keyframes_.back().time() >= time))
            LOG(ERROR) << "time is not monotone";
        else {
            keyframes_.emplace_back(Keyframe(frame, time));
        }

        pathIsValid_ = false;
        last_stopped_index_ = 0; // may not be valid any more
        stop_interpolation();
    }


    void KeyFrameInterpolator::delete_last_keyframe() {
        stop_interpolation();
        keyframes_.pop_back();
        pathIsValid_ = false;
        last_stopped_index_ = 0; // may not be valid any more
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


    void KeyFrameInterpolator::delete_path() {
        stop_interpolation();
        keyframes_.clear();
        interpolated_path_.clear();
        pathIsValid_ = false;
        last_stopped_index_ = 0;

        delete path_drawable_;
        path_drawable_ = nullptr;

        delete cameras_drawable_;
        cameras_drawable_ = nullptr;
    }


    Frame KeyFrameInterpolator::keyframe(std::size_t index) const {
        const Keyframe& kf = keyframes_.at(index);
        return Frame(kf.position(), kf.orientation());
    }


    float KeyFrameInterpolator::keyframe_time(std::size_t index) const {
        return keyframes_.at(index).time();
    }


    const vec3& KeyFrameInterpolator::keyframe_position(std::size_t index) {
        return keyframes_.at(index).position();
    }


    const quat& KeyFrameInterpolator::keyframe_orientation(std::size_t index) {
        return keyframes_.at(index).orientation();
    }


    float KeyFrameInterpolator::duration() const {
        return lastTime() - firstTime();
    }


    float KeyFrameInterpolator::firstTime() const {
        if (keyframes_.empty())
            return 0.0f;
        else
            return keyframes_.front().time();
    }


    float KeyFrameInterpolator::lastTime() const {
        if (keyframes_.empty())
            return 0.0f;
        else
            return keyframes_.back().time();
    }


    void KeyFrameInterpolator::set_interpolation_speed(float speed) {
        interpolation_speed_ = speed;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::set_frame_rate(int fps) {
        fps_ = fps;
        pathIsValid_ = false;
    }


    void KeyFrameInterpolator::start_interpolation() {
        auto animation = [this]() {
            // interval in ms. (0.9 to approximately compensate the overhead the timer thread and viewer update)
            const int interval = 1000.0f / frame_rate() * 0.9f;
            for (int id = last_stopped_index_; id < interpolated_path_.size(); ++id) {
                if (timer_.is_stopped()) {
                    last_stopped_index_ = id;
                    break;
                }
                const auto &f = interpolated_path_[id];
                frame()->setPositionAndOrientation(f.position(), f.orientation());
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
        timer_.set_timeout(0, animation);
    }


    void KeyFrameInterpolator::stop_interpolation() {
        timer_.stop();
        interpolation_started_ = false;
    }


    void KeyFrameInterpolator::draw_path(const Camera *cam, float camera_width) {
        if (keyframes_.empty())
            return;
        if (!pathIsValid_) {
            if (path_drawable_) {
                delete path_drawable_;
                path_drawable_ = nullptr;
            }
            if (cameras_drawable_) {
                delete cameras_drawable_;
                cameras_drawable_ = nullptr;
            }
            interpolated_path_ = interpolate();
            pathIsValid_ = true;
        }
        if (interpolated_path_.empty()) // interpolation may have failed.
            return;

        if (!path_drawable_) {
            std::vector<vec3> points;

            // the path
            for (std::size_t i = 0; i < interpolated_path_.size() - 1; ++i) {
                points.push_back(interpolated_path_[i].position());
                points.push_back(interpolated_path_[i + 1].position());
            }
            if (points.size() > 1) {
                path_drawable_ = new LinesDrawable;
                path_drawable_->update_vertex_buffer(points);
                path_drawable_->set_uniform_coloring(vec4(1.0f, 0.2f, 0.2f, 1.0f));
                path_drawable_->set_line_width(2);
                path_drawable_->set_impostor_type(LinesDrawable::CYLINDER);
            }
        }

        if (!cameras_drawable_) {
            std::vector<vec3> points;

            // camera representation
            for (std::size_t i = 0; i < keyframes_.size(); ++i) {
                std::vector<vec3> cam_points;
                opengl::prepare_camera(cam_points, camera_width,
                                       static_cast<float>(cam->screenHeight()) / cam->screenWidth());
                const mat4 &m = Frame(keyframes_[i].position(), keyframes_[i].orientation()).matrix();
                for (auto &p : cam_points) {
                    points.push_back(m * p);
                }
            }

            if (points.size() > 1) {
                cameras_drawable_ = new LinesDrawable;
                cameras_drawable_->update_vertex_buffer(points);
                cameras_drawable_->set_uniform_coloring(vec4(0.0f, 0.0f, 1.0f, 1.0f));
                cameras_drawable_->set_line_width(2);
            }
        }

        if (path_drawable_)
            path_drawable_->draw(cam);
        if (cameras_drawable_)
            cameras_drawable_->draw(cam);
    }


    bool KeyFrameInterpolator::save_keyframes(std::ostream& output) const {
        if (output.fail())
            return false;

        output << "\tnum_key_frames: " << keyframes_.size() << std::endl;

        for (std::size_t id = 0; id < keyframes_.size(); ++id) {
            output << "\tframe: " << id << std::endl;
            const Keyframe& kf = keyframes_[id];
            output << "\t\tposition: " << kf.position() << std::endl;
            output << "\t\torientation: " << kf.orientation() << std::endl;
        }

        return keyframes_.size() > 0;
    }


    bool KeyFrameInterpolator::read_keyframes(std::istream& input) {
        if (input.fail())
            return false;

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

        return keyframes_.size() > 0;
    }


    void KeyFrameInterpolator::update_keyframe_values(std::vector<Keyframe> &keyframes) {
        LOG_IF(keyframes.size() < 2, WARNING) << "computing frame values for only 2 keyframes";

        quat prevQ = keyframes.front().orientation();
        for (std::size_t i = 0; i < keyframes.size(); ++i) {
            Keyframe& kf = keyframes.at(i);
            kf.flip_if_needed(prevQ);
            prevQ = kf.orientation();
        }

        // the first keyframe
        keyframes[0].compute_tangent(keyframes[0], keyframes[1]);
        // the last keyframe
        keyframes.back().compute_tangent(keyframes[keyframes.size()-1], keyframes.back());
        // the other keyframes
        for (std::size_t i=1; i<keyframes.size()-1; ++i)
            keyframes[i].compute_tangent(keyframes[i-1], keyframes[i+1]);
    }


#ifndef DOXYGEN

    //////////// Keyframe private class implementation /////////
    KeyFrameInterpolator::Keyframe::Keyframe(const Frame &fr, float t)
            : time_(t) {
        p_ = fr.position();
        q_ = fr.orientation();
    }


    void KeyFrameInterpolator::Keyframe::compute_tangent(const Keyframe& prev, const Keyframe& next) {
#if 1
        // distance(prev, cur) and distance(cur, next) can have a big difference.
        // we compensate this
        const float sd_prev = distance2(prev.position(), position());
        const float sd_next = distance2(next.position(), position());
        if (sd_prev < sd_next) {
            const vec3 new_next = position() + (next.position() - position()).normalize() * std::sqrt(sd_prev);
            tgP_ = 0.5f * (new_next - prev.position());
        }
        else {
            const vec3 new_prev = position() +(prev.position() - position()).normalize() * std::sqrt(sd_next);
            tgP_ = 0.5f * (next.position() - new_prev);
        }
#else   // standard CatmullRom fitting
        tgP_ = 0.5f * (next.position() - prev.position());
#endif
        tgQ_ = quat::squad_tangent(prev.orientation(), q_, next.orientation());
    }


    void KeyFrameInterpolator::Keyframe::flip_if_needed(const quat &prev) {
        if (quat::dot(prev, q_) < 0.0f)
            q_.negate();
    }


    void KeyFrameInterpolator::get_keyframes_at_time(float time, const std::vector<Keyframe> &keyframes,
                                                     std::vector<Keyframe>::const_iterator *related) const {
        // Assertion: times are sorted in monotone order.
        // Assertion: keyframes_ is not empty

        // TODO: Special case for loops when closed path is implemented !!
        // Recompute everything from scratch
        related[1] = keyframes.begin(); // points to the first one

        while (related[1]->time() > time) {
            if (related[1] == keyframes.begin()) // already the first one
                break;
            --related[1]; // points to the previous one
        }

        related[2] = related[1];
        while (related[2]->time() < time) {
            if (related[2] == keyframes.end() - 1) // already the last one
                break;
            ++related[2];// points to the next one
        }

        related[1] = related[2];
        if ((related[1] != keyframes.begin()) && (time < related[2]->time()))
            --related[1];

        related[0] = related[1];
        if (related[0] != keyframes.begin())
            --related[0];

        related[3] = related[2];

        if ((related[3]) != keyframes.end() - 1) // has next
            ++related[3];
    }


    void KeyFrameInterpolator::compute_spline(const std::vector<Keyframe>::const_iterator *related, vec3 &v1,
                                              vec3 &v2) const {
        const vec3 &delta = related[2]->position() - related[1]->position();
        v1 = 3.0f * delta - 2.0f * related[1]->tgP() - related[2]->tgP();
        v2 = -2.0f * delta + related[1]->tgP() + related[2]->tgP();
    }


    const std::vector<Frame>& KeyFrameInterpolator::interpolate(bool smoothing) {
        if (keyframes_.size() == 1) {   // only one keyframe
            interpolated_path_.emplace_back(Frame(keyframes_[0].position(), keyframes_[0].orientation()));
            return interpolated_path_;
        }
        else if (pathIsValid_ || keyframes_.empty()) // already fitted or no keyframe
            return interpolated_path_;

        if (smoothing && keyframes_.size() > 2)
            adjust_keyframe_times(keyframes_, true);

        LOG_IF(keyframes_.size() > 2, INFO) << "interpolating " << keyframes_.size() << " keyframes";
        do_interpolate(interpolated_path_, keyframes_);
        LOG_IF(keyframes_.size() > 2, INFO)
                        << "keyframe interpolation done: "
                        << interpolated_path_.size() << " frames, "
                        << string::time(duration() / interpolation_speed() * 1000)
                        << " (at speed " << interpolation_speed() << "x)";

        if (smoothing && interpolated_path_.size() > 2) { // more iterations do not provide further improvement
            std::vector<Keyframe> as_key_frames;
            for (std::size_t i = 0; i < interpolated_path_.size(); ++i)
                as_key_frames.emplace_back(Keyframe(interpolated_path_[i], i));
            as_key_frames.front().set_time(firstTime());
            as_key_frames.back().set_time(lastTime());

            adjust_keyframe_times(as_key_frames, false);
            do_interpolate(interpolated_path_, as_key_frames);
        }

        pathIsValid_ = true;
        last_stopped_index_ = 0; // not valid any more
        return interpolated_path_;
    }

#if 0  // allow to use two different methods: CatmullRom (interpolation) and BSpline (fitting)

    auto interpolate_CatmullRom = [](float u, const vec3 &P0, const vec3 &P1, const vec3 &P2, const vec3 &P3) -> vec3 {
        vec3 point(0, 0, 0);
        point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 2;
        point += u * u * (2 * P0 - 5 * P1 + 4 * P2 - P3) / 2;
        point += u * ((-1) * P0 + P2) / 2;
        point += P1;
        return point;
    };

    auto interpolate_BSpline = [](float u, const vec3 &P0, const vec3 &P1, const vec3 &P2, const vec3 &P3) -> vec3 {
        vec3 point(0, 0, 0);
        point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 6;
        point += u * u * (3 * P0 - 6 * P1 + 3 * P2) / 6;
        point += u * ((-3) * P0 + 3 * P2) / 6;
        point += (P0 + 4 * P1 + P2) / 6;
        return point;
    };

    void KeyFrameInterpolator::do_interpolate(std::vector<Frame>& frames, const std::vector<Keyframe>& keyframes) const {
        frames.clear();
        if (keyframes.size() < 2)
            return;

        const float interval = interpolation_speed() * interpolation_period() / 1000.0f;
        for (float time = firstTime(); time < lastTime() + interval; time += interval) {
            std::vector<Keyframe>::const_iterator related[4];
            get_keyframes_at_time(time, keyframes, related);

            float alpha = 0.0f;
            const float dt = related[2]->time() - related[1]->time();
            if (std::abs(dt) < epsilon<float>())
                alpha = 0.0f;
            else
                alpha = (time - related[1]->time()) / dt;

            // spline interpolation
            const vec3 pos = interpolate_BSpline(alpha, related[0]->position(), related[1]->position(), related[2]->position(), related[3]->position());
            const quat q = quat::squad(
                    related[1]->orientation(),
                    quat::squad_tangent(related[0]->orientation(), related[1]->orientation(),related[2]->orientation()),
                    quat::squad_tangent(related[1]->orientation(), related[2]->orientation(),related[3]->orientation()),
                    related[2]->orientation(),
                    alpha
            );

            frames.emplace_back(Frame(pos, q));
        }
    }

#else

    void KeyFrameInterpolator::do_interpolate(std::vector<Frame>& frames, const std::vector<Keyframe>& keyframes) const {
        frames.clear();

        const_cast<KeyFrameInterpolator *>(this)->update_keyframe_values(
                const_cast<std::vector<Keyframe> & >(keyframes)
        );

        const float interval = interpolation_speed() * interpolation_period() / 1000.0f;
        for (float time = firstTime(); time < lastTime() + interval; time += interval) {
            std::vector<Keyframe>::const_iterator related[4];
            get_keyframes_at_time(time, keyframes, related);

            vec3 v1, v2;
            compute_spline(related, v1, v2);

            float alpha = 0.0f;
            const float dt = related[2]->time() - related[1]->time();
            if (std::abs(dt) < epsilon<float>())
                alpha = 0.0f;
            else
                alpha = (time - related[1]->time()) / dt;

            // spline interpolation
            const vec3 pos =
                    related[1]->position() + alpha * (related[1]->tgP() + alpha * (v1 + alpha * v2));
            const quat q = quat::squad(related[1]->orientation(), related[1]->tgQ(),
                                       related[2]->tgQ(), related[2]->orientation(), alpha);

            frames.emplace_back(Frame(pos, q));
        }
    }

#endif

    void KeyFrameInterpolator::adjust_keyframe_times(std::vector<Keyframe>& keyframes, bool slower_turning) {
        if (keyframes.size() < 2)
            return;

        const float duration = keyframes.back().time() - keyframes.front().time();

        if (slower_turning) {   // make turning slower
            // use step-distance-weighted keyframe timing + turning times.
            // the time needed for turning from current orientation to the orientation of the next frame.
            //  - 0 degree: 0 second
            //  - 90 degrees: 1.0 second
            // so we use 1 - acos(angle) to approximate this time
            auto turning_time = [](const Keyframe &frame, const Keyframe &prev) -> float {
                const float cos_angle = std::abs(quat::dot(frame.orientation(), prev.orientation()));
                return (1.0f - cos_angle) * 10;
            };

            float path_length = 0.0f;
            for (std::size_t i = 1; i < keyframes.size(); ++i)
                path_length += distance(keyframes[i - 1].position(), keyframes[i].position());

            const float t0 = keyframes[0].time();
            double time = t0;
            for (std::size_t i = 1; i < keyframes.size(); ++i) {
                const float time_distance =
                        distance(keyframes[i].position(), keyframes[i - 1].position()) / path_length * duration;
                const float time_turning = turning_time(keyframes[i], keyframes[i - 1]);
                time += (time_distance + time_turning);
                keyframes[i].set_time(time);
            }

            // then normalize so the duration is preserved
            const float scale = duration / (keyframes.back().time() - keyframes.front().time());
            for (std::size_t i = 0; i < keyframes.size(); ++i) {
                float t = keyframes[i].time() * scale;
                keyframes[i].set_time(t);
            }
        }
        else {  // use stride-length weighted keyframe timing
            std::vector<float> sub_lengths(keyframes.size(), 0.0f);
            float path_length = 0.0f;
            for (std::size_t i = 1; i < keyframes.size(); ++i) {
                path_length += distance(keyframes[i - 1].position(), keyframes[i].position());
                sub_lengths[i] = path_length;
            }
            if (path_length < epsilon_sqr<float>())
                path_length = 1.0f;

            const float t0 = keyframes[0].time();
            const float scale = duration / path_length;
            for (std::size_t i = 1; i < keyframes.size() - 1; ++i) { // don't modify the first and the last keyframes
                const float t = t0 + sub_lengths[i] * scale;
                keyframes[i].set_time(t);
            }
        }
    }

}


#endif //DOXYGEN
