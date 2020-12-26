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

#ifndef EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
#define EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H

#include <easy3d/core/types.h>
#include <easy3d/util/timer.h>

#include <vector>

namespace easy3d {

    class Frame;
    class Camera;
    class LinesDrawable;

    /*! \brief A keyFrame Catmull-Rom Frame interpolator.
      \class KeyFrameInterpolator easy3d/renderer/key_frame_interpolator.h

      \details A KeyFrameInterpolator holds keyFrames (that define a path) and a pointer to a Frame of your
      application (which will be interpolated). When the user startInterpolation(), the
      KeyFrameInterpolator regularly updates the frame() position and orientation along the path.

      Here is a typical utilization example (see also the <a href="../examples/keyFrames.html">keyFrames
      example</a>):
      \code


      init()
      {
        // The KeyFrameInterpolator kfi is given the Frame that it will drive over time.
        kfi = new KeyFrameInterpolator( new Frame() );
        kfi->addKeyFrame( Frame( vec3(1,0,0), Quaternion() ) );
        kfi->addKeyFrame( new Frame( vec3(2,1,0), Quaternion() ) );
        // ...and so on for all the keyFrames.

        // Ask for a display update after each update of the KeyFrameInterpolator
        connect(kfi, SIGNAL(interpolated()), SLOT(update()));

        kfi->startInterpolation();
      }

      draw()
      {
        glPushMatrix();
        glMultMatrixd( kfi->frame()->matrix() );
        // Draw your object here. Its position and orientation are interpolated.
        glPopMatrix();
      }
      \endcode

      The keyFrames are defined by a Frame and a time, expressed in seconds. The Frame can be provided
      as a const reference or as a pointer to a Frame (see the addKeyFrame() methods). In the latter
      case, the path will automatically be updated when the Frame is modified (using the
      Frame::modified() signal).

      The time has to be monotonously increasing over keyFrames. When interpolationSpeed() equals 1.0
      (default value), these times correspond to actual user's seconds during interpolation (provided
      that your main loop is fast enough). The interpolation is then real-time: the keyFrames will be
      reached at their keyFrameTime().

      <h3>Interpolation details</h3>

      When the user startInterpolation(), a timer is started which will update the frame()'s position
      and orientation every interpolationPeriod() milliseconds. This update increases the
      interpolationTime() by interpolationPeriod() * interpolationSpeed() milliseconds.

      Note that this mechanism ensures that the number of interpolation steps is constant and equal to
      the total path duration() divided by the interpolationPeriod() * interpolationSpeed(). This is
      especially useful for benchmarking or movie creation (constant number of snapshots).

      During the interpolation, the KeyFrameInterpolator emits an interpolated() signal, which will
      usually be connected to the QCanvas::update() slot. The interpolation is stopped when
      interpolationTime() is greater than the lastTime() (unless loopInterpolation() is \c true) and the
      endReached() signal is then emitted.

      Note that a QCamera has QCamera::keyFrameInterpolator(), that can be used to drive the QCamera along a
      path, or to restore a saved position (a path made of a single keyFrame). Press Alt+Fx to define a
      new keyFrame for path x. Pressing Fx plays/pauses path interpolation. See QCanvas::pathKey() and
      the <a href="../keyboard.html">keyboard page</a> for details.

      \attention If a Constraint is attached to the frame() (see Frame::constraint()), it should be
      deactivated before interpolationIsStarted(), otherwise the interpolated motion (computed as if
      there was no constraint) will probably be erroneous.

      <h3>Retrieving interpolated values</h3>

      This code defines a KeyFrameInterpolator, and displays the positions that will be followed by the
      frame() along the path:
      \code
      KeyFrameInterpolator kfi( new Frame() );
      // calls to kfi.addKeyFrame() to define the path.

      const double deltaTime = 0.04; // output a position every deltaTime seconds
      for (double time=kfi.firstTime(); time<=kfi.lastTime(); time += deltaTime)
      {
        kfi.interpolateAtTime(time);
        cout << "t=" << time << "\tpos=" << kfi.frame()->position() << endl;
      }
      \endcode
      You may want to temporally disconnect the \c kfi interpolated() signal from the
      QCanvas::update() slot before calling this code. \nosubgrouping */
    class KeyFrameInterpolator
    {
    public:
        /*! Creates a KeyFrameInterpolator, with \p frame as associated frame().
         * \details The frame() can be set or changed using setFrame().
         * interpolationTime(), interpolationSpeed() and interpolationPeriod() are set to their default values. */
        KeyFrameInterpolator(Frame* fr=nullptr);

        /*! Virtual destructor. Clears the keyFrame path. */
        virtual ~KeyFrameInterpolator();

        /*! @name Path creation */
        //@{
    public:
        /*! Appends a new keyFrame to the path.
         * Same as addKeyFrame(const Frame& frame, double), except that the keyFrameTime() is automatically set
         * to previous keyFrameTime() plus one second (or 0.0 if there is no previous keyFrame). */
        void addKeyFrame(const Frame& frame);

        /*! Appends a new keyFrame to the path, with its associated \p time (in seconds).
         * The path will use the current \p frame state.
         * \attention The keyFrameTime() have to be monotonously increasing over keyFrames. */
        void addKeyFrame(const Frame& frame, double time);

        /*! Removes all keyFrames from the path. The numberOfKeyFrames() is set to 0. */
        void deletePath();
        //@}

        /*! @name Associated Frame */
        //@{
    public:
        /*! Returns the associated Frame and that is interpolated by the KeyFrameInterpolator.

        When interpolationIsStarted(), this Frame's position and orientation will regularly be updated
        by a timer, so that they follow the KeyFrameInterpolator path.

        Set using setFrame() or with the KeyFrameInterpolator constructor. */
        Frame* frame() const { return frame_; }

    public:
        /*! Sets the frame() associated to the KeyFrameInterpolator. */
        void setFrame(Frame* const frame);
        //@}

        /*! @name Path parameters */
        //@{
    public:
        /*! Returns the number of keyFrames used by the interpolation. Use addKeyFrame() to add new keyFrames. */
        std::size_t numberOfKeyFrames() const { return keyFrames_.size(); }

        /**
         * \brief Returns the Frame associated with the keyFrame at index \p index.
         * See also keyFrameTime(). \p index has to be in the range [0, numberOfKeyFrames()-1].
         * \note If this keyFrame was defined using a pointer to a Frame (see addKeyFrame(const Frame* const)),
         *      the \e current pointed Frame state is returned.
         */
        Frame keyFrame(int index) const;

        /*! Returns the time corresponding to the \p index keyFrame.
         * See also keyFrame(). \p index has to be in the range [0, numberOfKeyFrames()-1].
         */
        double keyFrameTime(int index) const;

        /*! Returns the duration of the KeyFrameInterpolator path, expressed in seconds.
         * Simply corresponds to lastTime() - firstTime(). Returns 0.0 if the path has less than 2 keyFrames.
         * \sa keyFrameTime(). */
        double duration() const;

        /*! Returns the time corresponding to the first keyFrame, expressed in seconds.
         * Returns 0.0 if the path is empty.
         * \sa lastTime(), duration() and keyFrameTime(). */
        double firstTime() const;

        /*! Returns the time corresponding to the last keyFrame, expressed in seconds.
         * Returns 0.0 if the path is empty.
         * \sa firstTime(), duration() and keyFrameTime(). */
        double lastTime() const;
        //@}

        /*! @name Interpolation parameters */
        //@{
    public:
        /*! Returns the current interpolation time (in seconds) along the KeyFrameInterpolator path.

        This time is regularly updated when interpolationIsStarted(). Can be set directly with
        setInterpolationTime() or interpolateAtTime(). */
        double interpolationTime() const { return interpolationTime_; }
        /*! Returns the current interpolation speed.

        Default value is 1.0, which means keyFrameTime() will be matched during the interpolation
        (provided that your main loop is fast enough).

        A negative value will result in a reverse interpolation of the keyFrames. See also
        interpolationPeriod(). */
        double interpolationSpeed() const { return interpolationSpeed_; }
        /*! Returns the current interpolation period, expressed in milliseconds.

        The update of the frame() state will be done by a timer at this period when
        interpolationIsStarted().

        This period (multiplied by interpolationSpeed()) is added to the interpolationTime() at each
        update, and the frame() state is modified accordingly (see interpolateAtTime()). Default value
        is 40 milliseconds. */
        int interpolationPeriod() const { return period_; }
        /*! Returns \c true when the interpolation is played in an infinite loop.

        When \c false (default), the interpolation stops when interpolationTime() reaches firstTime()
        (with negative interpolationSpeed()) or lastTime().

        interpolationTime() is otherwise reset to firstTime() (+ interpolationTime() - lastTime()) (and
        inversely for negative interpolationSpeed()) and interpolation continues.

        In both cases, the endReached() signal is emitted. */
        bool loopInterpolation() const { return loopInterpolation_; }

    public:
        /*! Sets the interpolationTime().

        \attention The frame() state is not affected by this method. Use this function to define the
        starting time of a future interpolation (see startInterpolation()). Use interpolateAtTime() to
        actually interpolate at a given time. */
        void setInterpolationTime(double time) { interpolationTime_ = time; }
        /*! Sets the interpolationSpeed(). Negative or null values are allowed. */
        void setInterpolationSpeed(double speed) { interpolationSpeed_ = speed; }
        /*! Sets the interpolationPeriod(). */
        void setInterpolationPeriod(int period) { period_ = period; }
        /*! Sets the loopInterpolation() value. */
        void setLoopInterpolation(bool loop=true) { loopInterpolation_ = loop; }
        //@}

        /*! @name Interpolation */
        //@{
    public:
        /*! Returns \c true when the interpolation is being performed. Use startInterpolation(),
        stopInterpolation() or toggleInterpolation() to modify this state. */
        bool interpolationIsStarted() const { return interpolationStarted_; }
    public:

        /*! Starts the interpolation process.
         *
         * A timer is started with an interpolationPeriod() period that updates the frame()'s position and
         * orientation. interpolationIsStarted() will return \c true until stopInterpolation() or
         * toggleInterpolation() is called.
         *
         * If \p period is positive, it is set as the new interpolationPeriod(). The previous interpolationPeriod() is
         * used otherwise (default).
         *
         * If interpolationTime() is larger than lastTime(), interpolationTime() is reset to firstTime() before
         * interpolation starts (and inversely for negative interpolationSpeed()).
         *
         * Use setInterpolationTime() before calling this method to change the starting interpolationTime().
         *
         * \attention The keyFrames must be defined (see addKeyFrame()) \e before you startInterpolation(), or else
         * the interpolation will naturally immediately stop. */
        void startInterpolation(int period = -1);

        /*! Stops an interpolation started with startInterpolation().
         * \sa startInterpolation(), interpolationIsStarted(), and toggleInterpolation(). */
        void stopInterpolation();

        /*! Stops the interpolation and resets interpolationTime() to the firstTime().
         * If desired, call interpolateAtTime() after this method to actually move the frame() to firstTime(). */
        void resetInterpolation();

        /*! Calls startInterpolation() or stopInterpolation(), depending on interpolationIsStarted(). */
        void toggleInterpolation() { if (interpolationIsStarted()) stopInterpolation(); else startInterpolation(); }

        /*! Interpolate frame() at time \p time (expressed in seconds). interpolationTime() is set
         * to \p time and frame() is set accordingly.
         *
         * If you simply want to change interpolationTime() but not the frame() state, use setInterpolationTime()
         * instead.  */
        virtual void interpolateAtTime(double time);
        //@}

        /*! @name Path drawing */
        //@{
    public:
        /*! Draws the path used to interpolate the frame().
         *
         * \p mask controls what is drawn: if (mask & 1) (default), the position path is drawn. If (mask & 2),
         * a camera representation is regularly drawn and if (mask & 4), an oriented axis is regularly drawn.
         * Examples:
         *      \code
         *      drawPath();  // Simply draws the interpolation path
         *      drawPath(3); // Draws path and cameras
         *      drawPath(5); // Draws path and axis
         *      \endcode
         *
         * In the case where camera or axis is drawn, \p nbFrames controls the number of objects (axis or
         * camera) drawn between two successive keyFrames. When \p nbFrames=1, only the path KeyFrames are
         * drawn. \p nbFrames=2 also draws the intermediate orientation, etc. The maximum value is 30. \p
         * nbFrames should divide 30 so that an object is drawn for each KeyFrame. Default value is 6.
         *
         * \p scale (default=1.0) controls the scaling of the camera and axis drawing. A value of
         * Canvas::sceneRadius() should give good results.
         *
         * The rendering state can be changes by calling the path/cameras drawable's related methods. */
        virtual void drawPath(const Camera* cam, int mask=1, int nbFrames=6, float scale=1.0);

        /// adjusts the scene radius so that the entire camera path is within the view frustum.
        /// \related The adjusted scene radius.
        double adjust_scene_radius(Camera* cam) const;
        //@}

    private:
        virtual void update();
        virtual void on_frame_modified() { valuesAreValid_ = false; pathIsValid_ = false; splineCacheIsValid_ = false; }

    private:
        // Copy constructor and operator= are declared private and undefined
        // Prevents everyone from trying to use them
         KeyFrameInterpolator(const KeyFrameInterpolator& kfi);
         KeyFrameInterpolator& operator=(const KeyFrameInterpolator& kfi);

        void updateCurrentKeyFrameForTime(double time);
        void updateModifiedFrameValues();
        void updateSplineCache();

    #ifndef DOXYGEN
        // Internal private KeyFrame representation
        class KeyFrame
        {
        public:
            KeyFrame(const Frame& fr, double t);

            vec3 position() const { return p_; }
            quat orientation() const { return q_; }
            vec3 tgP() const { return tgP_; }
            quat tgQ() const { return tgQ_; }
            double time() const { return time_; }
            void flipOrientationIfNeeded(const quat& prev);
            void computeTangent(const KeyFrame* const prev, const KeyFrame* const next);
        private:
            vec3 p_, tgP_;
            quat q_, tgQ_;
            double time_;
        };

    #endif

        // Key Frames
        mutable std::vector<KeyFrame*> keyFrames_;
        std::vector<Frame> path_;

        std::vector<KeyFrame*>::iterator currentFrame_[4];

        // Associated frame
        Frame* frame_;

        // Rhythm
        Timer timer_;
        int period_;
        double interpolationTime_;
        double interpolationSpeed_;
        bool interpolationStarted_;

        // Make a loop?
        bool loopInterpolation_;

        // Cached values and flags
        bool pathIsValid_;
        bool valuesAreValid_;
        bool currentFrameValid_;
        bool splineCacheIsValid_;
        vec3 v1, v2;

        LinesDrawable* path_drawable_;
        LinesDrawable* cameras_drawable_;
    };

}



#endif // EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
