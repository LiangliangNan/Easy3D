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
#include <mutex>

#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/camera.h>   // for scene radius (to draw cameras in a proper size)


namespace easy3d {


    KeyFrameInterpolator::KeyFrameInterpolator(Frame *frame)
            : frame_(frame), period_(40)   // 25 frames per second
            , interpolationTime_(0.0), interpolationSpeed_(1.0), interpolationStarted_(false),
              loopInterpolation_(false), pathIsValid_(false),
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


    /*! Updates frame() state according to current interpolationTime(). Then adds
      interpolationPeriod()*interpolationSpeed() to interpolationTime().

      This internal method is called by a timer when interpolationIsStarted(). It can be used for
      debugging purpose. stopInterpolation() is called when interpolationTime() reaches firstTime() or
      lastTime(), unless loopInterpolation() is \c true. */
    void KeyFrameInterpolator::update() {
#if 0 // animate using only the keyframes (i.e., no interpolation)
        static int idx = 0;
        if (idx >= numberOfKeyFrames())
            return;
        Frame f = keyFrame(idx++);
        frame()->setPositionAndOrientation(f.position(), f.orientation());
        if (idx == numberOfKeyFrames()) {
            idx = 0; // so I can restart again
            stopInterpolation();
            end_reached.send();
        }
        return;
#endif

        static std::mutex mutex;
        mutex.try_lock();

        interpolateAtTime(interpolationTime());

        interpolationTime_ += interpolationSpeed() * interpolationPeriod() / 1000.0f;

        if (interpolationTime() > keyFrames_.back()->time()) {
            if (loopInterpolation())
                setInterpolationTime(keyFrames_.front()->time() + interpolationTime_ - keyFrames_.back()->time());
            else {
                // Make sure last KeyFrame is reached and displayed
                interpolateAtTime(keyFrames_.back()->time());
                stopInterpolation();
            }
            end_reached.send();
        } else if (interpolationTime() < keyFrames_.front()->time()) {
            if (loopInterpolation())
                setInterpolationTime(keyFrames_.back()->time() - keyFrames_.front()->time() + interpolationTime_);
            else {
                // Make sure first KeyFrame is reached and displayed
                interpolateAtTime(keyFrames_.front()->time());
                stopInterpolation();
            }
            end_reached.send();
        }
        mutex.unlock();
    }


    void KeyFrameInterpolator::startInterpolation() {
        if (keyFrames_.empty())
            return;

        if ((interpolationSpeed() > 0.0f) && (interpolationTime() >= keyFrames_.back()->time()))
            setInterpolationTime(keyFrames_.front()->time());
        if ((interpolationSpeed() < 0.0f) && (interpolationTime() <= keyFrames_.front()->time()))
            setInterpolationTime(keyFrames_.back()->time());
        interpolationStarted_ = true;
        update();  // for the starting view point
        timer_.set_interval(interpolationPeriod(), this, &KeyFrameInterpolator::update);
    }


    void KeyFrameInterpolator::stopInterpolation() {
        timer_.stop();
        interpolationStarted_ = false;
    }


    void KeyFrameInterpolator::resetInterpolation() {
        stopInterpolation();
        setInterpolationTime(firstTime());
    }


    void KeyFrameInterpolator::addKeyFrame(const Frame &frame, float time) {
        if (keyFrames_.empty())
            interpolationTime_ = time;

        if ((!keyFrames_.empty()) && (keyFrames_.back()->time() > time))
            LOG(ERROR) << "time is not monotone";
        else
            keyFrames_.push_back(new KeyFrame(frame, time));

        pathIsValid_ = false;
        resetInterpolation();
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
            interpolated_path_ = compute_all();
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


    // adjusts the scene radius so that the entire camera path is within the view frustum.
    float KeyFrameInterpolator::adjust_scene_radius(Camera *cam) const {
        // update scene bounding box to make sure the path is within the view frustum
        float radius = cam->sceneRadius();
        for (const auto frame : keyFrames_) {
            float dist = distance(cam->sceneCenter(), frame->position());
            if (dist > radius)
                radius = dist;
        }
        cam->setSceneRadius(radius);
        return radius;
    }


    void KeyFrameInterpolator::save_path(const std::string &file_name) const {
        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            std::cerr << "unable to open \'" << file_name << "\'" << std::endl;
            return;
        }

        output << "\tnum_key_frames: " << keyFrames_.size() << std::endl;

        for (std::size_t id = 0; id < keyFrames_.size(); ++id) {
            output << "\tframe: " << id << std::endl;
            const KeyFrame* frame = keyFrames_[id];
            output << "\t\tposition: " << frame->position() << std::endl;
            output << "\t\torientation: " << frame->orientation() << std::endl;
        }
    }


    void KeyFrameInterpolator::read_path(const std::string &file_name) {
        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            std::cerr << "unable to open \'" << file_name << "\'" << std::endl;
            return;
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


    Frame KeyFrameInterpolator::compute_at_time(float time) const {
        if (!pathIsValid_)
            const_cast<KeyFrameInterpolator*>(this)->updateModifiedFrameValues();

        std::vector<KeyFrame *>::const_iterator relatedFrames[4];
        getRelatedKeyFramesForTime(time, relatedFrames);

        vec3 v1, v2;
        computeSpline(relatedFrames, v1, v2);

        float alpha = 0.0f;
        const float dt = (*relatedFrames[2])->time() - (*relatedFrames[1])->time();
        if (dt == 0.0f)
            alpha = 0.0f;
        else
            alpha = (time - (*relatedFrames[1])->time()) / dt;

        const vec3 pos =
                (*relatedFrames[1])->position() + alpha * ((*relatedFrames[1])->tgP() + alpha * (v1 + alpha * v2));
        const quat q = quat::squad((*relatedFrames[1])->orientation(), (*relatedFrames[1])->tgQ(),
                                   (*relatedFrames[2])->tgQ(), (*relatedFrames[2])->orientation(), alpha);
        Frame f;
        f.setPosition(pos);
        f.setOrientation(q);
        return f;
    }


    void KeyFrameInterpolator::interpolateAtTime(float time) {
        std::cout << "time: " << time << std::endl;
        const Frame f = compute_at_time(time);
        vec3 pos = f.position();
        quat ori = f.orientation();
        frame()->setPositionAndOrientationWithConstraint(pos, ori);
    }

#ifndef DOXYGEN

    //////////// KeyFrame private class implementation /////////
    KeyFrameInterpolator::KeyFrame::KeyFrame(const Frame &fr, float t)
            : time_(t) {
        p_ = fr.position();
        q_ = fr.orientation();
    }

    void KeyFrameInterpolator::KeyFrame::computeTangent(const KeyFrame *const prev, const KeyFrame *const next) {
        tgP_ = 0.5 * (next->position() - prev->position());
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
    }


    void KeyFrameInterpolator::computeSpline(const std::vector<KeyFrame*>::const_iterator* relatedFrames, vec3& v1, vec3& v2) const {
        const vec3 &delta = (*relatedFrames[2])->position() - (*relatedFrames[1])->position();
        v1 = 3.0 * delta - 2.0 * (*relatedFrames[1])->tgP() - (*relatedFrames[2])->tgP();
        v2 = -2.0 * delta + (*relatedFrames[1])->tgP() + (*relatedFrames[2])->tgP();
    }


    std::vector<Frame> KeyFrameInterpolator::compute_all() const {
        std::vector<Frame> frames;
        if (keyFrames_.empty())
            return frames;

        const float interval = interpolationSpeed() * interpolationPeriod() / 1000.0f;
        for (float time = firstTime(); time < lastTime() + interval; time += interval) {
            const Frame f = compute_at_time(time);
            frames.push_back(f);
        }
        return frames;
    }

}


#endif //DOXYGEN
