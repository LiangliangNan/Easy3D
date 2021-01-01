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
 * the code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/key_frame_interpolator.h>

#include <fstream>
#include <chrono>
#include <thread>

#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/camera.h>   // for scene radius (to draw cameras in a proper size)


namespace easy3d {


    // allow to save interpolated frames as keyframes (so I can visualize them)
//#define DEBUG_INTERPOLATED_FRAMES
#ifdef DEBUG_INTERPOLATED_FRAMES
void save_interpolation(const std::vector<easy3d::Frame>& frames) {
    std::string file = "interpolated_frames.path";
    std::ofstream output(file.c_str());
    if (output.fail())
        std::cout << "could not open file: " << file << std::endl;
    output << "\tnum_key_frames: " << frames.size() << std::endl;
    for (std::size_t id = 0; id < frames.size(); ++id) {
        output << "\tframe: " << id << std::endl;
        const easy3d::Frame& frame = frames[id];
        output << "\t\tposition: " << frame.position() << std::endl;
        output << "\t\torientation: " << frame.orientation() << std::endl;
    }
}
#endif

    KeyFrameInterpolator::KeyFrameInterpolator(Frame *frame)
            : frame_(frame), period_(40)   // 25 frames per second
            , interpolationSpeed_(1.0), interpolationStarted_(false), start_position_(0), pathIsValid_(false),
              path_drawable_(nullptr), cameras_drawable_(nullptr) {
        if (keyFrames_.size() < 2)
            return;
    }


    KeyFrameInterpolator::~KeyFrameInterpolator() {
        deletePath();
    }


    void KeyFrameInterpolator::setFrame(Frame *const frame) {
        frame_ = frame;
    }


    void KeyFrameInterpolator::startInterpolation() {
        if (keyFrames_.empty())
            return;

        if (!pathIsValid_)
            interpolate();

        // all done in another thread.
        interpolationStarted_ = true;
        timer_.set_timeout(0, [this]() {
                               for (int id = start_position_; id < interpolated_path_.size(); ++id) {
                                   if (timer_.is_stopped()) {
                                       start_position_ = id;
                                       break;
                                   }
                                   const auto &f = interpolated_path_[id];
                                   frame()->setPositionAndOrientation(f.position(), f.orientation());
                                   const int interval = interpolationPeriod() / interpolationSpeed(); // interval in ms
                                   std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                                   if (id == interpolated_path_.size() - 1)  // reaches the end frame
                                       start_position_ = 0;
                               }
                               interpolation_stopped.send();
                               interpolationStarted_ = false;
                           }
        );
    }


    void KeyFrameInterpolator::stopInterpolation() {
        timer_.stop();
        interpolationStarted_ = false;
    }


    void KeyFrameInterpolator::addKeyFrame(const Frame &frame, float time) {
        if ((!keyFrames_.empty()) && (keyFrames_.back()->time() > time))
            LOG(ERROR) << "time is not monotone";
        else
            keyFrames_.push_back(new KeyFrame(frame, time));

        pathIsValid_ = false;
        stopInterpolation();
    }


    void KeyFrameInterpolator::addKeyFrame(const Frame &frame) {
        float time = 0.0f;
        if (keyFrames_.empty())
            time = 0.0;
        else
            time = keyFrames_.back()->time() + 1.0f;

        addKeyFrame(frame, time);
    }


    void KeyFrameInterpolator::deletePath() {
        stopInterpolation();
        for (auto f : keyFrames_) {
            delete f;
        }
        keyFrames_.clear();
        interpolated_path_.clear();
        pathIsValid_ = false;

        delete path_drawable_;
        path_drawable_ = nullptr;

        delete cameras_drawable_;
        cameras_drawable_ = nullptr;
    }


    void KeyFrameInterpolator::draw_path(const Camera *cam, float camera_width) {
        if (keyFrames_.empty())
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
            for (std::size_t i = 0; i < keyFrames_.size(); ++i) {
                std::vector<vec3> cam_points;
                opengl::prepare_camera(cam_points, camera_width,
                                       static_cast<float>(cam->screenHeight()) / cam->screenWidth());
                const mat4 &m = Frame(keyFrames_[i]->position(), keyFrames_[i]->orientation()).matrix();
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


    bool KeyFrameInterpolator::save_keyframes(const std::string &file_name) const {
        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            std::cerr << "unable to open \'" << file_name << "\'" << std::endl;
            return false;
        }

        output << "\tnum_key_frames: " << keyFrames_.size() << std::endl;

        for (std::size_t id = 0; id < keyFrames_.size(); ++id) {
            output << "\tframe: " << id << std::endl;
            const KeyFrame* frame = keyFrames_[id];
            output << "\t\tposition: " << frame->position() << std::endl;
            output << "\t\torientation: " << frame->orientation() << std::endl;
        }

        return keyFrames_.size() > 0;
    }


    bool KeyFrameInterpolator::read_keyframes(const std::string &file_name) {
        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            std::cerr << "unable to open \'" << file_name << "\'" << std::endl;
            return false;
        }

        // clean
        deletePath();

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
            addKeyFrame(Frame(pos, orient));
        }

        return keyFrames_.size() > 0;
    }


    void KeyFrameInterpolator::updateModifiedFrameValues() {
        quat prevQ = keyFrames_.front()->orientation();
        KeyFrame *kf = nullptr;
        for (std::size_t i = 0; i < keyFrames_.size(); ++i) {
            kf = keyFrames_.at(i);
            kf->flipOrientationIfNeeded(prevQ);
            prevQ = kf->orientation();
        }

        KeyFrame *prev = keyFrames_.front();
        kf = keyFrames_.front();
        std::size_t index = 1;
        while (kf) {
            KeyFrame *next = (index < keyFrames_.size()) ? keyFrames_.at(index) : nullptr;
            index++;
            if (next)
                kf->computeTangent(prev, next);
            else
                kf->computeTangent(prev, kf);
            prev = kf;
            kf = next;
        }
    }


    Frame KeyFrameInterpolator::keyFrame(std::size_t index) const {
        const KeyFrame *const kf = keyFrames_.at(index);
        return Frame(kf->position(), kf->orientation());
    }


    float KeyFrameInterpolator::keyFrameTime(std::size_t index) const {
        return keyFrames_.at(index)->time();
    }


    float KeyFrameInterpolator::duration() const {
        return lastTime() - firstTime();
    }


    float KeyFrameInterpolator::firstTime() const {
        if (keyFrames_.empty())
            return 0.0f;
        else
            return keyFrames_.front()->time();
    }


    float KeyFrameInterpolator::lastTime() const {
        if (keyFrames_.empty())
            return 0.0f;
        else
            return keyFrames_.back()->time();
    }


    Frame KeyFrameInterpolator::interpolate(float time) const {
        if (!pathIsValid_)
            const_cast<KeyFrameInterpolator*>(this)->updateModifiedFrameValues();

        std::vector<KeyFrame *>::const_iterator relatedFrames[4];
        getRelatedKeyFramesForTime(time, relatedFrames);

        vec3 v1, v2;
        computeSpline(relatedFrames, v1, v2);

        float alpha = 0.0f;
        const float dt = (*relatedFrames[2])->time() - (*relatedFrames[1])->time();
        if (std::abs(dt) < epsilon<float>())
            alpha = 0.0f;
        else
            alpha = (time - (*relatedFrames[1])->time()) / dt;

#if 0   // linear interpolation - not smooth
        vec3 pos = alpha*((*relatedFrames[2])->position()) + (1.0f-alpha)*((*relatedFrames[1])->position());
        quat a = (*relatedFrames[1])->orientation();
        quat b = (*relatedFrames[2])->orientation();
        quat q = quat(
                alpha * b[0] + (1.0f -alpha) * a[0],
                alpha * b[1] + (1.0f -alpha) * a[1],
                alpha * b[2] + (1.0f -alpha) * a[2],
                alpha * b[3] + (1.0f -alpha) * a[3]
        );
        q.normalize();

#else   // spline interpolation
        const vec3 pos =
                (*relatedFrames[1])->position() + alpha * ((*relatedFrames[1])->tgP() + alpha * (v1 + alpha * v2));
        const quat q = quat::squad((*relatedFrames[1])->orientation(), (*relatedFrames[1])->tgQ(),
                                   (*relatedFrames[2])->tgQ(), (*relatedFrames[2])->orientation(), alpha);
#endif
        Frame f;
        f.setPosition(pos);
        f.setOrientation(q);
        return f;
    }


#ifndef DOXYGEN

    //////////// KeyFrame private class implementation /////////
    KeyFrameInterpolator::KeyFrame::KeyFrame(const Frame &fr, float t)
            : time_(t) {
        p_ = fr.position();
        q_ = fr.orientation();
    }

    void KeyFrameInterpolator::KeyFrame::computeTangent(const KeyFrame *const prev, const KeyFrame *const next) {
#if 1   // distance(prev, cur) and distance(cur, next) can have a big difference.
        // we compensate this
        float sd_prev = distance2(prev->position(), position());
        float sd_next = distance2(next->position(), position());
        if (sd_prev < sd_next) {
            vec3 new_next = position() + (next->position() - position()).normalize() * std::sqrt(sd_prev);
            tgP_ = 0.5 * (new_next - prev->position());
        }
        else {
            vec3 new_prev = position() +(prev->position() - position()).normalize() * std::sqrt(sd_next);
            tgP_ = 0.5 * (next->position() - new_prev);
        }
#else
        tgP_ = 0.5 * (next->position() - prev->position());
#endif
        tgQ_ = quat::squad_tangent(prev->orientation(), q_, next->orientation());
    }

    void KeyFrameInterpolator::KeyFrame::flipOrientationIfNeeded(const quat &prev) {
        if (quat::dot(prev, q_) < 0.0f)
            q_.negate();
    }


    void KeyFrameInterpolator::getRelatedKeyFramesForTime(float time, std::vector<KeyFrame *>::const_iterator *relatedFrames) const {
        // Assertion: times are sorted in monotone order.
        // Assertion: keyFrames_ is not empty

        // TODO: Special case for loops when closed path is implemented !!
        // Recompute everything from scratch
        relatedFrames[1] = keyFrames_.begin(); // points to the first one

        while ((*relatedFrames[1])->time() > time) {
            if (relatedFrames[1] == keyFrames_.begin()) // already the first one
                break;
            --relatedFrames[1]; // points to the previous one
        }

        relatedFrames[2] = relatedFrames[1];
        while ((*relatedFrames[2])->time() < time) {
            if (*relatedFrames[2] == keyFrames_.back()) // already the last one
                break;
            ++relatedFrames[2];// points to the next one
        }

        relatedFrames[1] = relatedFrames[2];
        if ((relatedFrames[1] != keyFrames_.begin()) && (time < (*relatedFrames[2])->time()))
            --relatedFrames[1];

        relatedFrames[0] = relatedFrames[1];
        if (relatedFrames[0] != keyFrames_.begin())
            --relatedFrames[0];

        relatedFrames[3] = relatedFrames[2];

        if ((*relatedFrames[3]) != keyFrames_.back()) // has next
            ++relatedFrames[3];

//        if (std::abs(round(time) - time) < 1e-4) {
//            std::cout << "time: " << time << "\n\trelated frames: "
//                      << relatedFrames[0] - keyFrames_.begin() << ", "
//                      << relatedFrames[1] - keyFrames_.begin() << ", "
//                      << relatedFrames[2] - keyFrames_.begin() << ", "
//                      << relatedFrames[3] - keyFrames_.begin() << std::endl;
//        }
    }


    void KeyFrameInterpolator::computeSpline(const std::vector<KeyFrame*>::const_iterator* relatedFrames, vec3& v1, vec3& v2) const {
        const vec3 &delta = (*relatedFrames[2])->position() - (*relatedFrames[1])->position();
        v1 = 3.0 * delta - 2.0 * (*relatedFrames[1])->tgP() - (*relatedFrames[2])->tgP();
        v2 = -2.0 * delta + (*relatedFrames[1])->tgP() + (*relatedFrames[2])->tgP();
    }


    const std::vector<Frame>& KeyFrameInterpolator::interpolate() {
        interpolated_path_.clear();
        if (keyFrames_.empty())
            return interpolated_path_;

        const float interval = interpolationSpeed() * interpolationPeriod() / 1000.0f;
        for (float time = firstTime(); time < lastTime() + interval; time += interval) {
            const Frame f = interpolate(time);
            interpolated_path_.push_back(f);
        }

#ifdef DEBUG_INTERPOLATED_FRAMES
        save_interpolation(interpolated_path_);
#endif
        return interpolated_path_;
    }

}


#endif //DOXYGEN
