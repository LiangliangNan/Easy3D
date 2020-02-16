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

#include <easy3d/viewer/key_frame_interpolator.h>
#include <easy3d/viewer/frame.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/camera.h>   // for scene raduis (to draw cameras in a proper size)

#include <easy3d/util/timer.h>


namespace easy3d {


    /*! Creates a KeyFrameInterpolator, with \p frame as associated frame().

      The frame() can be set or changed using setFrame().

      interpolationTime(), interpolationSpeed() and interpolationPeriod() are set to their default
      values. */
    KeyFrameInterpolator::KeyFrameInterpolator(Frame* frame)
        : frame_(frame)
        , period_(40)   // 25 frames per second
        , interpolationTime_(0.0)
        , interpolationSpeed_(1.0)
        , interpolationStarted_(false)
        , closedPath_(false)
        , loopInterpolation_(false)
        , pathIsValid_(false)
        , valuesAreValid_(true)
        , currentFrameValid_(false)
        // #CONNECTION# Values cut pasted initFromDOMElement()
        , path_drawable_(nullptr)
        , cameras_drawable_(nullptr)
    {
        if (keyFrame_.size() < 2)
            return;

        for (int i=0; i<4; ++i)
            currentFrame_[i] = keyFrame_.begin();
    }

    /*! Virtual destructor. Clears the keyFrame path. */
    KeyFrameInterpolator::~KeyFrameInterpolator()
    {
        deletePath();
    }

    /*! Sets the frame() associated to the KeyFrameInterpolator. */
    void KeyFrameInterpolator::setFrame(Frame* const frame)
    {
        frame_ = frame;
    }

    /*! Updates frame() state according to current interpolationTime(). Then adds
      interpolationPeriod()*interpolationSpeed() to interpolationTime().

      This internal method is called by a timer when interpolationIsStarted(). It can be used for
      debugging purpose. stopInterpolation() is called when interpolationTime() reaches firstTime() or
      lastTime(), unless loopInterpolation() is \c true. */
    void KeyFrameInterpolator::update()
    {
        interpolateAtTime(interpolationTime());

        interpolationTime_ += interpolationSpeed() * interpolationPeriod() / 1000.0;

        if (interpolationTime() > keyFrame_.back()->time())
        {
            if (loopInterpolation())
                setInterpolationTime(keyFrame_.front()->time() + interpolationTime_ - keyFrame_.back()->time());
            else
            {
                // Make sure last KeyFrame is reached and displayed
                interpolateAtTime(keyFrame_.back()->time());
                stopInterpolation();
            }
            timer_.stop();
        }
        else if (interpolationTime() < keyFrame_.front()->time())
        {
            if (loopInterpolation())
                setInterpolationTime(keyFrame_.back()->time() - keyFrame_.front()->time() + interpolationTime_);
            else
            {
                // Make sure first KeyFrame is reached and displayed
                interpolateAtTime(keyFrame_.front()->time());
                stopInterpolation();
            }
            timer_.stop();
        }
    }


    /*! Starts the interpolation process.

      A timer is started with an interpolationPeriod() period that updates the frame()'s position and
      orientation. interpolationIsStarted() will return \c true until stopInterpolation() or
      toggleInterpolation() is called.

      If \p period is positive, it is set as the new interpolationPeriod(). The previous
      interpolationPeriod() is used otherwise (default).

      If interpolationTime() is larger than lastTime(), interpolationTime() is reset to firstTime()
      before interpolation starts (and inversely for negative interpolationSpeed()).

      Use setInterpolationTime() before calling this method to change the starting interpolationTime().

      See the <a href="../examples/keyFrames.html">keyFrames example</a> for an illustration.

      You may also be interested in QCanvas::animate() and QCanvas::startAnimation().

      \attention The keyFrames must be defined (see addKeyFrame()) \e before you startInterpolation(),
      or else the interpolation will naturally immediately stop. */
    void KeyFrameInterpolator::startInterpolation(int period)
    {
        if (period >= 0)
            setInterpolationPeriod(period);

        if (!keyFrame_.empty())
        {
            if ((interpolationSpeed() > 0.0) && (interpolationTime() >= keyFrame_.back()->time()))
                setInterpolationTime(keyFrame_.front()->time());
            if ((interpolationSpeed() < 0.0) && (interpolationTime() <= keyFrame_.front()->time()))
                setInterpolationTime(keyFrame_.back()->time());
             interpolationStarted_ = true;
            update();
            timer_.set_interval(interpolationPeriod(), &KeyFrameInterpolator::update, this);
        }
    }


    /*! Stops an interpolation started with startInterpolation(). See interpolationIsStarted() and toggleInterpolation(). */
    void KeyFrameInterpolator::stopInterpolation()
    {
        timer_.stop();
        interpolationStarted_ = false;
    }


    /*! Stops the interpolation and resets interpolationTime() to the firstTime().

    If desired, call interpolateAtTime() after this method to actually move the frame() to
    firstTime(). */
    void KeyFrameInterpolator::resetInterpolation()
    {
        stopInterpolation();
        setInterpolationTime(firstTime());
    }


    /*! Appends a new keyFrame to the path, with its associated \p time (in seconds).

      The path will use the current \p frame state. If you want the path to change when \p frame is
      modified, you need to pass a \e pointer to the Frame instead (see addKeyFrame(const Frame*,
      double)).

      The keyFrameTime() have to be monotonously increasing over keyFrames. */
    void KeyFrameInterpolator::addKeyFrame(const Frame& frame, double time)
    {
        if (keyFrame_.empty())
            interpolationTime_ = time;

        if ( (!keyFrame_.empty()) && (keyFrame_.back()->time() > time) )
            LOG(ERROR) << "Error in KeyFrameInterpolator::addKeyFrame: time is not monotone";
        else
            keyFrame_.push_back(new KeyFrame(frame, time));

        valuesAreValid_ = false;
        pathIsValid_ = false;
        currentFrameValid_ = false;
        resetInterpolation();
    }

    /*! Appends a new keyFrame to the path.

     Same as addKeyFrame(const Frame& frame, double), except that the keyFrameTime() is automatically set
     to previous keyFrameTime() plus one second (or 0.0 if there is no previous keyFrame). */
    void KeyFrameInterpolator::addKeyFrame(const Frame& frame)
    {
        double time;
        if (keyFrame_.empty())
            time = 0.0;
        else
            time = keyFrame_.back()->time() + 1.0;

        addKeyFrame(frame, time);
    }

    /*! Removes all keyFrames from the path. The numberOfKeyFrames() is set to 0. */
    void KeyFrameInterpolator::deletePath()
    {
        stopInterpolation();
        for (auto f : keyFrame_) {
            delete f;
        }
        keyFrame_.clear();
        pathIsValid_ = false;
        valuesAreValid_ = false;
        currentFrameValid_ = false;

        if (path_drawable_) {
            delete path_drawable_;
            path_drawable_ = nullptr;
        }

        if (cameras_drawable_) {
            delete cameras_drawable_;
            cameras_drawable_ = nullptr;
        }
    }


    /*! Draws the path used to interpolate the frame().

      \p mask controls what is drawn: if (mask & 1) (default), the position path is drawn. If (mask &
      2), a camera representation is regularly drawn and if (mask & 4), an oriented axis is regularly
      drawn. Examples:

      \code
      drawPath();  // Simply draws the interpolation path
      drawPath(3); // Draws path and cameras
      drawPath(5); // Draws path and axis
      \endcode

      In the case where camera or axis is drawn, \p nbFrames controls the number of objects (axis or
      camera) drawn between two successive keyFrames. When \p nbFrames=1, only the path KeyFrames are
      drawn. \p nbFrames=2 also draws the intermediate orientation, etc. The maximum value is 30. \p
      nbFrames should divide 30 so that an object is drawn for each KeyFrame. Default value is 6.

      \p scale (default=1.0) controls the scaling of the camera and axis drawing. A value of
      QCanvas::sceneRadius() should give good results.

      See the <a href="../examples/keyFrames.html">keyFrames example</a> for an illustration.

      The color of the path is the current \c glColor().

      \attention The OpenGL state is modified by this method: GL_LIGHTING is disabled and line width set
      to 2. Use this code to preserve your current OpenGL state:
      \code
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      drawPathModifyGLState(mask, nbFrames, scale);
      glPopAttrib();
      \endcode */
    void KeyFrameInterpolator::drawPath(const Camera* cam, int mask, int nbFrames, float scale)
    {
        if (keyFrame_.empty())
            return;

        const int nbSteps = 30;
        if (!pathIsValid_)
        {
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

            if (keyFrame_.front() == keyFrame_.back())
                path_.push_back(Frame(keyFrame_.front()->position(), keyFrame_.front()->orientation()));
            else
            {
                static Frame fr;
                KeyFrame* kf_[4];
                kf_[0] = keyFrame_.front();
                kf_[1] = kf_[0];
                int index = 1;
                kf_[2] = (index < keyFrame_.size()) ? keyFrame_.at(index) : nullptr;
                index++;
                kf_[3] = (index < keyFrame_.size()) ? keyFrame_.at(index) : nullptr;

                while (kf_[2])
                {
                    vec3 diff = kf_[2]->position() - kf_[1]->position();
                    vec3 v1 = 3.0 * diff - 2.0 * kf_[1]->tgP() - kf_[2]->tgP();
                    vec3 v2 = -2.0 * diff + kf_[1]->tgP() + kf_[2]->tgP();

                    // cout << kf_[0]->time() << " , " << kf_[1]->time() << " , " << kf_[2]->time() << " , " << kf_[3]->time() << endl;
                    for (int step=0; step<nbSteps; ++step)
                    {
                        double alpha = step / static_cast<double>(nbSteps);
                        fr.setPosition(kf_[1]->position() + alpha * (kf_[1]->tgP() + alpha * (v1+alpha*v2)));
                        fr.setOrientation(quat::squad(kf_[1]->orientation(), kf_[1]->tgQ(), kf_[2]->tgQ(), kf_[2]->orientation(), alpha));
                        path_.push_back(fr);
                    }

                    // Shift
                    kf_[0] = kf_[1];
                    kf_[1] = kf_[2];
                    kf_[2] = kf_[3];
                    index++;
                    kf_[3] = (index < keyFrame_.size()) ? keyFrame_.at(index) : nullptr;
                }
                // Add last KeyFrame
                path_.push_back(Frame(kf_[1]->position(), kf_[1]->orientation()));
            }
            pathIsValid_ = true;
        }

        if (!path_drawable_) {
            std::vector<vec3> points;

            // the path
            for (std::size_t i=0; i<path_.size() - 1; ++i) {
                points.push_back(path_[i].position());
                points.push_back(path_[i+1].position());
            }

            if (points.size() > 1) {
                path_drawable_ = new LinesDrawable;
                path_drawable_->update_vertex_buffer(points);
                path_drawable_->set_default_color(vec3(1.0f, 0.67f, 0.5f));
                path_drawable_->set_line_width(2);
                path_drawable_->set_impostor_type(LinesDrawable::CYLINDER);
            }
        }

        if (!cameras_drawable_) {
            std::vector<vec3> points;

            // camera representation
            for (std::size_t i=0; i<keyFrame_.size(); ++i) {
                std::vector<vec3> cam_points;
                opengl::prepare_camera(cam_points, cam->sceneRadius() * 0.03f * scale, static_cast<float>(cam->screenHeight())/cam->screenWidth());
                const mat4& m = Frame(keyFrame_[i]->position(), keyFrame_[i]->orientation()).matrix();
                for (auto& p : cam_points) {
                    points.push_back(m * p);
                }
            }

            if (points.size() > 1) {
                cameras_drawable_ = new LinesDrawable;
                cameras_drawable_->update_vertex_buffer(points);
                cameras_drawable_->set_default_color(vec3(0.0f, 0.0f, 1.0f));
                cameras_drawable_->set_line_width(2);
            }
        }

        if (path_drawable_)
            path_drawable_->draw(cam);
        if (cameras_drawable_)
            cameras_drawable_->draw(cam);
    }

    void KeyFrameInterpolator::updateModifiedFrameValues()
    {
        quat prevQ = keyFrame_.front()->orientation();
        KeyFrame* kf;
        for (int i=0; i<keyFrame_.size(); ++i)
        {
            kf = keyFrame_.at(i);
            kf->flipOrientationIfNeeded(prevQ);
            prevQ = kf->orientation();
        }

        KeyFrame* prev = keyFrame_.front();
        kf = keyFrame_.front();
        int index = 1;
        while (kf)
        {
            KeyFrame* next = (index < keyFrame_.size()) ? keyFrame_.at(index) : nullptr;
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

    /*! Returns the Frame associated with the keyFrame at index \p index.

     See also keyFrameTime(). \p index has to be in the range 0..numberOfKeyFrames()-1.

     \note If this keyFrame was defined using a pointer to a Frame (see addKeyFrame(const Frame*
      const)), the \e current pointed Frame state is returned. */
    Frame KeyFrameInterpolator::keyFrame(int index) const
    {
        const KeyFrame* const kf = keyFrame_.at(index);
        return Frame(kf->position(), kf->orientation());
    }

    /*! Returns the time corresponding to the \p index keyFrame.

     See also keyFrame(). \p index has to be in the range 0..numberOfKeyFrames()-1. */
    double KeyFrameInterpolator::keyFrameTime(int index) const
    {
        return keyFrame_.at(index)->time();
    }

    /*! Returns the duration of the KeyFrameInterpolator path, expressed in seconds.

     Simply corresponds to lastTime() - firstTime(). Returns 0.0 if the path has less than 2 keyFrames.
     See also keyFrameTime(). */
    double KeyFrameInterpolator::duration() const
    {
        return lastTime() - firstTime();
    }

    /*! Returns the time corresponding to the first keyFrame, expressed in seconds.

    Returns 0.0 if the path is empty. See also lastTime(), duration() and keyFrameTime(). */
    double KeyFrameInterpolator::firstTime() const
    {
        if (keyFrame_.empty())
            return 0.0;
        else
            return keyFrame_.front()->time();
    }

    /*! Returns the time corresponding to the last keyFrame, expressed in seconds.

    Returns 0.0 if the path is empty. See also firstTime(), duration() and keyFrameTime(). */
    double KeyFrameInterpolator::lastTime() const
    {
        if (keyFrame_.empty())
            return 0.0;
        else
            return keyFrame_.back()->time();
    }


    void KeyFrameInterpolator::updateCurrentKeyFrameForTime(double time)
    {
        // Assertion: times are sorted in monotone order.
        // Assertion: keyFrame_ is not empty

        // TODO: Special case for loops when closed path is implemented !!
        if (!currentFrameValid_)
            // Recompute everything from scrach
            currentFrame_[1] = keyFrame_.begin(); // points to the first one

        while ((*currentFrame_[1])->time() > time)
        {
            currentFrameValid_ = false;
            if (currentFrame_[1] == keyFrame_.begin()) // alreay the first one
                break;
            --currentFrame_[1]; // points to the previous one
        }

        if (!currentFrameValid_)
            currentFrame_[2] = currentFrame_[1];

        while ((*currentFrame_[2])->time() < time)
        {
            currentFrameValid_ = false;
            if (*currentFrame_[2] == keyFrame_.back()) // already the last one
                break;
            ++currentFrame_[2];// points to the next one
        }

        if (!currentFrameValid_)
        {
            currentFrame_[1] = currentFrame_[2];
            if ((currentFrame_[1] != keyFrame_.begin()) && (time < (*currentFrame_[2])->time()))
                --currentFrame_[1];

            currentFrame_[0] = currentFrame_[1];
            if (currentFrame_[0] != keyFrame_.begin())
                --currentFrame_[0];

            currentFrame_[3] = currentFrame_[2];

            if ((*currentFrame_[3]) != keyFrame_.back()) // has next
                ++currentFrame_[3];

            currentFrameValid_ = true;
            splineCacheIsValid_ = false;
        }

        // cout << "Time = " << time << " : " << currentFrame_[0]->peekNext()->time() << " , " <<
        // next(currentFrame_[1])->time() << " , " << next(currentFrame_[2])->time() << " , " <<  currentFrame_[3]->peekNext()->time() << endl;
    }

    void KeyFrameInterpolator::updateSplineCache()
    {
        const vec3& delta = (*currentFrame_[2])->position() - (*currentFrame_[1])->position();
        v1 = 3.0 * delta - 2.0 * (*currentFrame_[1])->tgP() - (*currentFrame_[2])->tgP();
        v2 = -2.0 * delta + (*currentFrame_[1])->tgP() + (*currentFrame_[2])->tgP();
        splineCacheIsValid_ = true;
    }



    /*! Interpolate frame() at time \p time (expressed in seconds). interpolationTime() is set to \p
      time and frame() is set accordingly.

      If you simply want to change interpolationTime() but not the frame() state, use
      setInterpolationTime() instead.

      Emits the interpolated() signal and makes the frame() emit the Frame::interpolated() signal. */
    void KeyFrameInterpolator::interpolateAtTime(double time)
    {
       setInterpolationTime(time);

        if ((keyFrame_.empty()) || (!frame()))
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
        vec3 pos = (*currentFrame_[1])->position() + alpha * ((*currentFrame_[1])->tgP() + alpha * (v1+alpha*v2));
        quat q = quat::squad((*currentFrame_[1])->orientation(), (*currentFrame_[1])->tgQ(),
                (*currentFrame_[2])->tgQ(), (*currentFrame_[2])->orientation(), alpha);
        frame()->setPositionAndOrientationWithConstraint(pos, q);
    }

    #ifndef DOXYGEN

    //////////// KeyFrame private class implementation /////////
    KeyFrameInterpolator::KeyFrame::KeyFrame(const Frame& fr, double t)
        : time_(t)
    {
        p_ = fr.position();
        q_ = fr.orientation();
    }

    void KeyFrameInterpolator::KeyFrame::computeTangent(const KeyFrame* const prev, const KeyFrame* const next)
    {
        tgP_ = 0.5 * (next->position() - prev->position());
        tgQ_ = quat::squad_tangent(prev->orientation(), q_, next->orientation());
    }

    void KeyFrameInterpolator::KeyFrame::flipOrientationIfNeeded(const quat& prev)
    {
        if (quat::dot(prev, q_) < 0.0f)
            q_.negate();
    }


}



#endif //DOXYGEN
