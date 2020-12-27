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
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/camera.h>   // for scene radius (to draw cameras in a proper size)

#include <easy3d/util/timer.h>


namespace easy3d {


    KeyFrameInterpolator::KeyFrameInterpolator(Frame *frame)
            : frame_(frame), period_(40)   // 25 frames per second
            , interpolationTime_(0.0), interpolationSpeed_(1.0), interpolationStarted_(false),
              loopInterpolation_(false), pathIsValid_(false), valuesAreValid_(true), currentFrameValid_(false),
              path_drawable_(nullptr), cameras_drawable_(nullptr) {
        if (keyFrames_.size() < 2)
            return;

        for (int i = 0; i < 4; ++i)
            currentFrame_[i] = keyFrames_.begin();
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
        interpolateAtTime(interpolationTime());

        interpolationTime_ += interpolationSpeed() * interpolationPeriod() / 1000.0;

        if (interpolationTime() > keyFrames_.back()->time()) {
            if (loopInterpolation())
                setInterpolationTime(keyFrames_.front()->time() + interpolationTime_ - keyFrames_.back()->time());
            else {
                // Make sure last KeyFrame is reached and displayed
                interpolateAtTime(keyFrames_.back()->time());
                stopInterpolation();
            }
            timer_.stop();
        } else if (interpolationTime() < keyFrames_.front()->time()) {
            if (loopInterpolation())
                setInterpolationTime(keyFrames_.back()->time() - keyFrames_.front()->time() + interpolationTime_);
            else {
                // Make sure first KeyFrame is reached and displayed
                interpolateAtTime(keyFrames_.front()->time());
                stopInterpolation();
            }
            timer_.stop();
        }
    }


    void KeyFrameInterpolator::startInterpolation(int period) {
        if (period >= 0)
            setInterpolationPeriod(period);

        if (!keyFrames_.empty()) {
            if ((interpolationSpeed() > 0.0) && (interpolationTime() >= keyFrames_.back()->time()))
                setInterpolationTime(keyFrames_.front()->time());
            if ((interpolationSpeed() < 0.0) && (interpolationTime() <= keyFrames_.front()->time()))
                setInterpolationTime(keyFrames_.back()->time());
            interpolationStarted_ = true;
            update();
            timer_.set_interval(interpolationPeriod(), &KeyFrameInterpolator::update, this);
        }
    }


    void KeyFrameInterpolator::stopInterpolation() {
        timer_.stop();
        interpolationStarted_ = false;
    }


    void KeyFrameInterpolator::resetInterpolation() {
        stopInterpolation();
        setInterpolationTime(firstTime());
    }


    void KeyFrameInterpolator::addKeyFrame(const Frame &frame, double time) {
        if (keyFrames_.empty())
            interpolationTime_ = time;

        if ((!keyFrames_.empty()) && (keyFrames_.back()->time() > time))
            LOG(ERROR) << "Error in KeyFrameInterpolator::addKeyFrame: time is not monotone";
        else
            keyFrames_.push_back(new KeyFrame(frame, time));

        valuesAreValid_ = false;
        pathIsValid_ = false;
        currentFrameValid_ = false;
        resetInterpolation();
    }


    void KeyFrameInterpolator::addKeyFrame(const Frame &frame) {
        double time;
        if (keyFrames_.empty())
            time = 0.0;
        else
            time = keyFrames_.back()->time() + 1.0;

        addKeyFrame(frame, time);
    }


    void KeyFrameInterpolator::deletePath() {
        stopInterpolation();
        for (auto f : keyFrames_) {
            delete f;
        }
        keyFrames_.clear();
        pathIsValid_ = false;
        valuesAreValid_ = false;
        currentFrameValid_ = false;

        delete path_drawable_;
        path_drawable_ = nullptr;

        delete cameras_drawable_;
        cameras_drawable_ = nullptr;
    }


    void KeyFrameInterpolator::draw_path(const Camera *cam, float scale) {
        if (keyFrames_.empty())
            return;

        const int nbSteps = 30;
        if (!pathIsValid_) {
            path_.clear();

            if (path_drawable_) {
                delete path_drawable_;
                path_drawable_ = nullptr;
            }
            if (cameras_drawable_) {
                delete cameras_drawable_;
                cameras_drawable_ = nullptr;
            }

            if (!valuesAreValid_)
                updateModifiedFrameValues();

            if (keyFrames_.front() == keyFrames_.back())
                path_.push_back(Frame(keyFrames_.front()->position(), keyFrames_.front()->orientation()));
            else {
                static Frame fr;
                KeyFrame *kf_[4];
                kf_[0] = keyFrames_.front();
                kf_[1] = kf_[0];
                int index = 1;
                kf_[2] = (index < keyFrames_.size()) ? keyFrames_.at(index) : nullptr;
                index++;
                kf_[3] = (index < keyFrames_.size()) ? keyFrames_.at(index) : nullptr;

                while (kf_[2]) {
                    vec3 diff = kf_[2]->position() - kf_[1]->position();
                    vec3 v1 = 3.0 * diff - 2.0 * kf_[1]->tgP() - kf_[2]->tgP();
                    vec3 v2 = -2.0 * diff + kf_[1]->tgP() + kf_[2]->tgP();

                    // cout << kf_[0]->time() << " , " << kf_[1]->time() << " , " << kf_[2]->time() << " , " << kf_[3]->time() << endl;
                    for (int step = 0; step < nbSteps; ++step) {
                        double alpha = step / static_cast<double>(nbSteps);
                        fr.setPosition(kf_[1]->position() + alpha * (kf_[1]->tgP() + alpha * (v1 + alpha * v2)));
                        fr.setOrientation(
                                quat::squad(kf_[1]->orientation(), kf_[1]->tgQ(), kf_[2]->tgQ(), kf_[2]->orientation(),
                                            alpha));
                        path_.push_back(fr);
                    }

                    // Shift
                    kf_[0] = kf_[1];
                    kf_[1] = kf_[2];
                    kf_[2] = kf_[3];
                    index++;
                    kf_[3] = (index < keyFrames_.size()) ? keyFrames_.at(index) : nullptr;
                }
                // Add last KeyFrame
                path_.push_back(Frame(kf_[1]->position(), kf_[1]->orientation()));
            }
            pathIsValid_ = true;
        }

        if (!path_drawable_) {
            std::vector<vec3> points;

            // the path
            for (std::size_t i = 0; i < path_.size() - 1; ++i) {
                points.push_back(path_[i].position());
                points.push_back(path_[i + 1].position());
            }

            if (points.size() > 1) {
                path_drawable_ = new LinesDrawable;
                path_drawable_->update_vertex_buffer(points);
                path_drawable_->set_uniform_coloring(vec4(1.0f, 0.67f, 0.5f, 1.0f));
                path_drawable_->set_line_width(2);
                path_drawable_->set_impostor_type(LinesDrawable::CYLINDER);
            }
        }

        if (!cameras_drawable_) {
            std::vector<vec3> points;

            // camera representation
            for (std::size_t i = 0; i < keyFrames_.size(); ++i) {
                std::vector<vec3> cam_points;
                opengl::prepare_camera(cam_points, cam->sceneRadius() * 0.03f * scale,
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
    double KeyFrameInterpolator::adjust_scene_radius(Camera *cam) const {
        // update scene bounding box to make sure the path is within the view frustum
        double radius = cam->sceneRadius();
        for (const auto frame : keyFrames_) {
            float dist = distance(cam->sceneCenter(), frame->position());
            if (dist > radius)
                radius = dist;
        }
        cam->setSceneRadius(radius);
        return radius;
    }


    void KeyFrameInterpolator::updateModifiedFrameValues() {
        quat prevQ = keyFrames_.front()->orientation();
        KeyFrame *kf;
        for (int i = 0; i < keyFrames_.size(); ++i) {
            kf = keyFrames_.at(i);
            kf->flipOrientationIfNeeded(prevQ);
            prevQ = kf->orientation();
        }

        KeyFrame *prev = keyFrames_.front();
        kf = keyFrames_.front();
        int index = 1;
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
        valuesAreValid_ = true;
    }


    Frame KeyFrameInterpolator::keyFrame(int index) const {
        const KeyFrame *const kf = keyFrames_.at(index);
        return Frame(kf->position(), kf->orientation());
    }


    double KeyFrameInterpolator::keyFrameTime(int index) const {
        return keyFrames_.at(index)->time();
    }


    double KeyFrameInterpolator::duration() const {
        return lastTime() - firstTime();
    }


    double KeyFrameInterpolator::firstTime() const {
        if (keyFrames_.empty())
            return 0.0;
        else
            return keyFrames_.front()->time();
    }


    double KeyFrameInterpolator::lastTime() const {
        if (keyFrames_.empty())
            return 0.0;
        else
            return keyFrames_.back()->time();
    }


    void KeyFrameInterpolator::updateCurrentKeyFrameForTime(double time) {
        // Assertion: times are sorted in monotone order.
        // Assertion: keyFrames_ is not empty

        // TODO: Special case for loops when closed path is implemented !!
        if (!currentFrameValid_)
            // Recompute everything from scratch
            currentFrame_[1] = keyFrames_.begin(); // points to the first one

        while ((*currentFrame_[1])->time() > time) {
            currentFrameValid_ = false;
            if (currentFrame_[1] == keyFrames_.begin()) // alreay the first one
                break;
            --currentFrame_[1]; // points to the previous one
        }

        if (!currentFrameValid_)
            currentFrame_[2] = currentFrame_[1];

        while ((*currentFrame_[2])->time() < time) {
            currentFrameValid_ = false;
            if (*currentFrame_[2] == keyFrames_.back()) // already the last one
                break;
            ++currentFrame_[2];// points to the next one
        }

        if (!currentFrameValid_) {
            currentFrame_[1] = currentFrame_[2];
            if ((currentFrame_[1] != keyFrames_.begin()) && (time < (*currentFrame_[2])->time()))
                --currentFrame_[1];

            currentFrame_[0] = currentFrame_[1];
            if (currentFrame_[0] != keyFrames_.begin())
                --currentFrame_[0];

            currentFrame_[3] = currentFrame_[2];

            if ((*currentFrame_[3]) != keyFrames_.back()) // has next
                ++currentFrame_[3];

            currentFrameValid_ = true;
            splineCacheIsValid_ = false;
        }

        // cout << "Time = " << time << " : " << currentFrame_[0]->peekNext()->time() << " , " <<
        // next(currentFrame_[1])->time() << " , " << next(currentFrame_[2])->time() << " , " <<  currentFrame_[3]->peekNext()->time() << endl;
    }


    void KeyFrameInterpolator::updateSplineCache() {
        const vec3 &delta = (*currentFrame_[2])->position() - (*currentFrame_[1])->position();
        v1 = 3.0 * delta - 2.0 * (*currentFrame_[1])->tgP() - (*currentFrame_[2])->tgP();
        v2 = -2.0 * delta + (*currentFrame_[1])->tgP() + (*currentFrame_[2])->tgP();
        splineCacheIsValid_ = true;
    }


    void KeyFrameInterpolator::interpolateAtTime(double time) {
        setInterpolationTime(time);

        if ((keyFrames_.empty()) || (!frame()))
            return;

        if (!valuesAreValid_)
            updateModifiedFrameValues();

        updateCurrentKeyFrameForTime(time);

        if (!splineCacheIsValid_)
            updateSplineCache();

        double alpha;
        const double dt = (*currentFrame_[2])->time() - (*currentFrame_[1])->time();
        if (dt == 0.0)
            alpha = 0.0;
        else
            alpha = (time - (*currentFrame_[1])->time()) / dt;

        // Linear interpolation - debug
        // vec3 pos = alpha*((*currentFrame_[2])->position()) + (1.0-alpha)*((*currentFrame_[1])->position());
        vec3 pos = (*currentFrame_[1])->position() + alpha * ((*currentFrame_[1])->tgP() + alpha * (v1 + alpha * v2));
        quat q = quat::squad((*currentFrame_[1])->orientation(), (*currentFrame_[1])->tgQ(),
                             (*currentFrame_[2])->tgQ(), (*currentFrame_[2])->orientation(), alpha);
        frame()->setPositionAndOrientationWithConstraint(pos, q);
    }

#ifndef DOXYGEN

    //////////// KeyFrame private class implementation /////////
    KeyFrameInterpolator::KeyFrame::KeyFrame(const Frame &fr, double t)
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


}


#endif //DOXYGEN
