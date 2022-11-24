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

#ifndef EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
#define EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H

#include <easy3d/core/types.h>
#include <easy3d/core/signal.h>
#include <easy3d/util/timer.h>
#include <easy3d/renderer/frame.h>

#include <vector>


namespace easy3d {

    class Frame;
    class Camera;
    class LinesDrawable;
    class TrianglesDrawable;

    /**
     * \brief A keyframe interpolator for animation generation.
     * \class KeyFrameInterpolator easy3d/renderer/key_frame_interpolator.h
     *
     * \details A KeyFrameInterpolator holds keyframes (that define a path) and a pointer to a frame (which will be
     *      interpolated) of your application. When the user start_interpolation(), the KeyFrameInterpolator regularly
     *      updates the frame() position and orientation along the path.
     *
     *      A keyframes is defined by a Frame and a time, expressed in seconds. The time has to be monotonously
     *      increasing over keyframes. When interpolation_speed() equals 1.0 (default value), these times correspond
     *      to actual user's seconds during interpolation (provided that your main loop is fast enough).
     *      The interpolation is then real-time: the keyframes will be reached at their keyframe_time().
     *
     *      When the user start_interpolation(), a timer is started which will update the frame()'s position and
     *      orientation every interpolation_period() milliseconds. This update increases the time by
     *      interpolation_period() * interpolation_speed() milliseconds. This mechanism ensures that the number of
     *      interpolation steps is constant and equal to the total path duration() divided by the
     *      interpolation_period() * interpolation_speed(). This is especially useful for benchmarking or movie
     *      creation (constant number of snapshots).
     *
     *      During the interpolation, the KeyFrameInterpolator emits an \c frame_interpolated Signal which will usually
     *      be connected to the viewer's update() method. The interpolation is stopped when duration has reached.
     *      Another Signal interpolation_stopped will be emitted when the interpolation reaches its end or the when the
     *      stop_interpolation() method was triggered.
     *
     *      Note that a Camera has a keyframe_interpolator() method, that can be used to drive the Camera along a
     *      path, or to restore a saved position (a path made of a single keyframe).
     *      
     * \note The default duration of any two consecutive keyframes is the same. So for a smoother animation, it is
     *      suggested to regularly (as equally as possible) sample the viewpoints.
     *      
     * \attention If a Constraint is attached to the frame() (see Frame::constraint()), it should be deactivated before 
     *      is_interpolation_started(), otherwise the interpolated motion (computed as if there was no constraint) will 
     *      probably be erroneous.
     *
     * Animation example:
     *      \code
     *          // the KeyFrameInterpolator kfi is given the camera's frame that it will be driven over time.
     *          kfi = new KeyFrameInterpolator( camera()->frame() );
     *          kfi->add_keyframe(...);
     *          kfi->add_keyframe(...);
     *          // ...and so on for all the keyframes.
     *          // starts animation
     *          kfi->start_interpolation();
     *      \endcode
     *      
     * \todo Allow edit the duration for each keyframe?  (not sure due to many keyframes and can be annoying).
     */
    class KeyFrameInterpolator
    {
    public:
        /**
         * \brief Creates a KeyFrameInterpolator, with \p frame as associated frame().
         * \details The frame() can be set or changed using set_frame(). The interpolation_speed() and 
         *      interpolation_period() are set to their default values. 
         */
        explicit KeyFrameInterpolator(Frame *frame = nullptr);

        /** Virtual destructor. Clears the keyframe path. */
        virtual ~KeyFrameInterpolator();

        /*! @name Path creation and modification*/
        //@{
    public:
        /**
         * \brief Appends a new keyframe to the path.
         * \details Same as add_keyframe(const Frame&, float), except that the keyframe_time() is automatically set
         *      to previous keyframe_time() plus one second (or 0.0 if there is no previous keyframe).
         * \return \c true if the keyframe has been successfully added.
         */
        bool add_keyframe(const Frame& frame);

        /**
         * \brief Appends a new keyframe to the path, with its associated \p time (in seconds).
         * \details The path will use the current \p frame state.
         * \return \c true if the keyframe has been successfully added.
         * \attention The keyframe_time() have to be monotonously increasing over keyframes.
         */
        bool add_keyframe(const Frame& frame, float time);

        /**
         * \brief Removes the lastly added keyframe from the path.
         * \details This is the reverse operation of add_keyframe(const Frame&) and add_keyframe(const Frame&, float).
         */
        void delete_last_keyframe();

        /**
         * \brief Sets the time corresponding to the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \note The time \c t have to be monotonously increasing over keyframes.
         * \sa keyframe_time()
         */
        void set_keyframe_time(std::size_t index, float t);

        /**
         * \brief Sets the position of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa keyframe_time()
         */
        void set_keyframe_position(std::size_t index, const vec3& pos);

        /**
         * \brief Sets the orientation of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa keyframe_time()
         */
        void set_keyframe_orientation(std::size_t index, const quat& q);

        /**
         * \brief Removes all keyframes from the path.
         * \details Upon return, the number_of_keyframes() will return 0.
         */
        void delete_path();
        //@}

        /*! @name Associated Frame */
        //@{
    public:
        /**
         * \brief Returns the associated Frame and that is interpolated by the KeyFrameInterpolator.
         * \details The returned Frame was set using set_frame() or with the KeyFrameInterpolator constructor.
         *      When is_interpolation_started(), this Frame's position and orientation will regularly be updated
         *      by a timer, so that they follow the KeyFrameInterpolator path.
         * \sa set_frame()
         */
        Frame* frame() const { return frame_; }

    public:
        /** Sets the frame() associated to the KeyFrameInterpolator. */
        void set_frame(Frame* const frame) { frame_ = frame; }
        //@}

        /*! @name Access keyframe and path parameters */
        //@{
    public:
        /**
         * \brief Returns the number of keyframes used by the interpolation.
         * \details Use add_keyframe() to add new keyframes.
         */
        std::size_t number_of_keyframes() const { return keyframes_.size(); }

        /**
         * \brief Returns the Frame associated with the keyframe at \p index.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa keyframe_time()
         */
        Frame keyframe(std::size_t index) const;

        /**
         * \brief Returns the time corresponding to the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa set_keyframe_time().
         */
        float keyframe_time(std::size_t index) const;

        /**
         * \brief Returns the position of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa set_keyframe_position()
         */
        const vec3& keyframe_position(std::size_t index) const;

        /**
         * \brief Returns the orientation of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \sa set_keyframe_orientation()
         */
        const quat& keyframe_orientation(std::size_t index) const;

        /**
         * \brief Returns the duration of the KeyFrameInterpolator path, expressed in seconds.
         * \details The duration simply corresponds to lastTime() - firstTime(). It returns 0.0 if the path has less
         *      than 2 keyframes.
         * \sa keyframe_time().
         */
        float duration() const;

        /**
         * \brief Returns the time corresponding to the first keyframe, expressed in seconds.
         * \details Returns 0.0 if the path is empty.
         * \sa lastTime(), duration(), and keyframe_time().
         */
        float firstTime() const;

        /**
         * \brief Returns the time corresponding to the last keyframe, expressed in seconds.
         * \details Returns 0.0 if the path is empty.
         * \sa firstTime(), duration(), and keyframe_time().
         */
        float lastTime() const;
        //@}

        /*! @name Interpolation parameters */
        //@{
    public:

        enum Method { FITTING, INTERPOLATION };
        /**
         * \brief Returns the interpolation method.
         * \details Default value is \c Interpolation (cubic spline interpolation).
         */
        Method interpolation_method() const { return interpolation_method_; }

        /**
         * \brief Returns the current interpolation speed.
         * \details Default value is 1.0, which means keyframe_time() will be matched during the interpolation
         *      (provided that your main loop is fast enough).
         */
        float interpolation_speed() const { return interpolation_speed_; }

        /**
         * \brief Returns the current interpolation period, expressed in milliseconds.
         * \details The update of the frame() state will be done by a timer at this period when
         *      is_interpolation_started(). This period (multiplied by interpolation_speed()) will be added to be the
         *      next frame's time.
         *      The interpolation_period() multiplied by frame_rate() is always equal to 1.0.
         */
        int interpolation_period() const { return static_cast<int>(1000.0 / fps_); }

        /**
         * \brief Returns the desired frame rate. Default value is 30.
         * \details The frame_rate() multiplied by interpolation_period() is always equal to 1.0.
         */
        int frame_rate() const { return fps_; }

    public:

        /**
         * \brief Sets the interpolation_method().
         */
        void set_interpolation_method(Method m);

        /**
         * \brief Sets the interpolation_speed().
         */
        void set_interpolation_speed(float speed);

        /// Sets the desired frame rate.
        void set_frame_rate(int fps);
        //@}

        /*! @name Interpolation */
        //@{
    public:
        /**
         * \brief Returns whether the interpolation is being performed.
         * \details Use start_interpolation(), stop_interpolation() or toggleInterpolation() to modify this state.
         */
        bool is_interpolation_started() const { return interpolation_started_; }

    public:

        /**
         * \brief Starts the interpolation process.
         * \details A timer is started with an interpolation_period() period that updates the frame()'s position and
         *      orientation. is_interpolation_started() will return \c true until stop_interpolation() or
         *      toggleInterpolation() is called.
         * \note The keyframes must be defined (see add_keyframe()) \e before you start_interpolation(), or else
         *      the interpolation will naturally immediately stop.
         */
        void start_interpolation();

        /**
         * \brief Stops an interpolation started with start_interpolation().
         * \sa start_interpolation(), is_interpolation_started(), and toggleInterpolation().
         */
        void stop_interpolation();

        /**
         * \brief Calls start_interpolation() or stop_interpolation(), depending on is_interpolation_started().
         */
        void toggle_interpolation() {
            if (is_interpolation_started())
                stop_interpolation();
            else
                start_interpolation();
        }

    public:
        /// Computes and returns all the interpolated frames.
        const std::vector<Frame>& interpolate();
        //@}

        /*! @name Path drawing */
        //@{
    public:

        /**
         * \brief Draws the virtual 3D cameras for the keyframes.
         * \param camera The current camera used by the viewer.
         * \param camera_width Controls the size of the cameras. A good value can be 5% of the scene radius, or
         *      10% of the character height (in walking mode), for instance.
         */
        void draw_cameras(const Camera* camera, float camera_width, const vec4& color = vec4(0.5f, 0.8f, 0.5f, 1.0f));

        /**
         * \brief Draws the interpolated camera path.
         * \param camera The current camera used by the viewer.
         */
        void draw_path(const Camera* camera, float thickness = 2.0f, const vec4& color = vec4(1.0f, 1.0f, 0.5f, 1.0f));

        //@}

        /*! @name File io */
        //@{
    public:
        /// saves the camera path to a file
        bool save_keyframes(const std::string& file_name) const;
        /// reads camera path from a file
        bool read_keyframes(const std::string& file_name);
        //@}

    private:
        // Copy constructor and operator= are declared private and undefined
        // Prevents everyone from trying to use them
        KeyFrameInterpolator(const KeyFrameInterpolator& kfi);
        KeyFrameInterpolator& operator=(const KeyFrameInterpolator& kfi);

        // Internal private Keyframe representation
        class Keyframe
        {
        public:
            Keyframe(const Frame& fr, float t);
            const vec3& position() const { return p_; }
            const quat& orientation() const { return q_; }
            float time() const { return time_; }
            void set_time(float t) { time_ = t; }
            void set_position(const vec3& p) { p_ = p; }
            void set_orientation(const quat& q) { q_ = q; }
            void flip_if_needed(const quat& prev); // flip its orientation if needed
        private:
            vec3 p_;
            quat q_;
            float time_;
        };

    private:
        // Associated frame
        Frame* frame_;

        // Key Frames
        std::vector<Keyframe> keyframes_;
        std::vector<Frame> interpolated_path_;

        // Rhythm
        Timer<> timer_;
        int fps_;
        Method interpolation_method_;
        float interpolation_speed_;
        bool interpolation_started_;
        int last_stopped_index_;

        // is path valid? Adding new keyframes or editing a keyframe invalidates the path
        bool pathIsValid_;

        LinesDrawable* path_drawable_;
        TrianglesDrawable* cameras_drawable_;

    public:
        Signal<> frame_interpolated;
        Signal<> interpolation_stopped;
    };

}


#endif // EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
