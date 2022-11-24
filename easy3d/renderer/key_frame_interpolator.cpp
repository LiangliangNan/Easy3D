/********************************************************************
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
 ********************************************************************/

/** ----------------------------------------------------------
 *
 * The code in this file is adapted from libQGLViewer with significant
 * modifications and enhancement.
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 *      http://libqglviewer.com/
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/key_frame_interpolator.h>

#include <chrono>
#include <thread>

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

            keyframes_.emplace_back(Keyframe(frame, time));
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


    const vec3& KeyFrameInterpolator::keyframe_position(std::size_t index) const {
        return keyframes_.at(index).position();
    }


    const quat& KeyFrameInterpolator::keyframe_orientation(std::size_t index) const {
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


    void  KeyFrameInterpolator::set_interpolation_method(Method m) {
        interpolation_method_ = m;
        pathIsValid_ = false;
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
            // interval in ms (0.9 to approximately compensate the overhead the timer thread and viewer update)
            const int interval = static_cast<int>(1000.0f / static_cast<float>(frame_rate()) * 0.9f);
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


    void KeyFrameInterpolator::draw_cameras(const Camera *camera, float camera_width, const vec4 &color) {
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

        if (!cameras_drawable_) {
            std::vector<vec3> points;
            std::vector<unsigned int> indices;

            // camera representation
            for (const auto& frame : keyframes_) {
                std::vector<vec3> cam_points;
                std::vector<unsigned int> cam_indices;
                shape::create_camera(cam_points, cam_indices, camera_width, camera->fieldOfView(),
                                      static_cast<float>(camera->screenHeight()) / static_cast<float>(camera->screenWidth()));
                unsigned int offset = static_cast<unsigned int>(points.size());
                for (auto id : cam_indices)
                    indices.push_back(offset + id);
                const mat4 &m = Frame(frame.position(), frame.orientation()).matrix();
                for (auto &p : cam_points)
                    points.push_back(m * p);
            }

            if (points.size() > 1) {
                cameras_drawable_ = new TrianglesDrawable;
                cameras_drawable_->update_vertex_buffer(points);
                cameras_drawable_->update_element_buffer(indices);
                cameras_drawable_->set_uniform_coloring(vec4(0.2f, 0.7f, 0.3f, 1.0f));
            }
        }

        if (cameras_drawable_)
            cameras_drawable_->draw(camera);
    }


    void KeyFrameInterpolator::draw_path(const Camera *camera, float thickness, const vec4 &color) {
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
            for (std::size_t i = 0; i < interpolated_path_.size() - 1; ++i) {
                points.push_back(interpolated_path_[i].position());
                points.push_back(interpolated_path_[i + 1].position());
            }
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
        else if (keyframes_.size() == 1) {   // only one keyframe
            interpolated_path_.emplace_back(Frame(keyframes_[0].position(), keyframes_[0].orientation()));
            return interpolated_path_;
        }
        else if (keyframes_.size() == 2) {   // only two keyframe: linear interpolation
            interpolated_path_.clear();
            const float interval = interpolation_speed() * static_cast<float>(interpolation_period()) / 1000.0f;
            const int num_frames = static_cast<int>(duration() / interval + 1);
            for (int i=0; i<num_frames; ++i) {
                const float w = static_cast<float>(i) / static_cast<float>(num_frames - 1);
                const vec3 pos = (1.0f - w) * keyframes_[0].position() + w * keyframes_[1].position();
                const auto orient = quat::slerp(keyframes_[0].orientation(), keyframes_[1].orientation(), w);
                interpolated_path_.emplace_back(Frame(pos, orient.normalized()));
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

        LOG_IF(keyframes_.size() > 2, INFO) << "interpolating " << keyframes_.size() << " keyframes...";

        interpolated_path_.clear();
        const float interval = interpolation_speed() * static_cast<float>(interpolation_period()) / 1000.0f;
        const int num_frames = static_cast<int>(duration() / interval + 1);

        if (interpolation_method_ == INTERPOLATION) {
            // we choose the accumulated path length as parameter, so to have equal intervals.
            std::vector<float> parameters(keyframes_.size());
            std::vector<vec3> positions(keyframes_.size());
            std::vector<vec4> orientations(keyframes_.size());
            float dist(0.0f);
            for (int i=0; i<keyframes_.size(); ++i) {
                positions[i] = keyframes_[i].position();
                const quat& orient = keyframes_[i].orientation();
                for (unsigned char j=0; j<4; ++j)
                    orientations[i][j] = orient[j];
                if (i > 0)
                    dist += distance(positions[i-1], positions[i]);
                parameters[i] = dist;
            }

            // spine interpolation
            typedef SplineCurveInterpolation<vec3> PosFitter;
            PosFitter pos_fitter;
            pos_fitter.set_boundary(PosFitter::second_deriv, 0, PosFitter::second_deriv, 0, false);
            pos_fitter.set_points(parameters, positions);

            typedef SplineCurveInterpolation<vec4> OrientFitter;
            OrientFitter orient_fitter;
            orient_fitter.set_boundary(OrientFitter::second_deriv, 0, OrientFitter::second_deriv, 0, false);
            orient_fitter.set_points(parameters, orientations);

            for (int i = 0; i < num_frames; ++i) {
                const float u = static_cast<float>(i) / static_cast<float>(num_frames - 1);
                const vec3 pos = pos_fitter.eval_f(u);
                const vec4 q = orient_fitter.eval_f(u);
                quat orient;
                for (unsigned char j=0; j<4; ++j)
                    orient[j] = q[j];
                orient.normalize();
                interpolated_path_.emplace_back(Frame(pos, orient));
            }
        }
        else {
            std::vector<vec3> positions(keyframes_.size());
            std::vector<vec4> orientations(keyframes_.size());
            for (std::size_t i=0; i<keyframes_.size(); ++i) {
                positions[i] = keyframes_[i].position();
                const quat& orient = keyframes_[i].orientation();
                for (unsigned char j=0; j<4; ++j)
                    orientations[i][j] = orient[j];
            }

            // spine fitting
            const int order = 3;  // Smoothness of the spline (min 2)
            typedef SplineCurveFitting <vec3> PosFitter;
            PosFitter pos_fitter(order, PosFitter::eOPEN_UNIFORM);
            pos_fitter.set_ctrl_points(positions);
            const std::vector<float> parameters = pos_fitter.get_equally_spaced_parameters(num_frames);

            typedef SplineCurveFitting <vec4> OrientFitter;
            OrientFitter orient_fitter(order, OrientFitter::eOPEN_UNIFORM);
            orient_fitter.set_ctrl_points(orientations);

            for (int i = 0; i < num_frames; ++i) {
                const float u = parameters[i];
                const vec3 pos = pos_fitter.eval_f(u);
                const vec4 q = orient_fitter.eval_f(u);
                quat orient;
                for (unsigned char j=0; j<4; ++j)
                    orient[j] = q[j];
                orient.normalize();
                interpolated_path_.emplace_back(Frame(pos, orient));
            }
        }

        LOG_IF(keyframes_.size() > 2, INFO)
                        << "keyframe interpolation done: "
                        << interpolated_path_.size() << " frames, "
                        << string::time(duration() / interpolation_speed() * 1000)
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

