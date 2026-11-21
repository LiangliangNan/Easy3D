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

#ifndef EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
#define EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H

#include <easy3d/core/types.h>
#include "easy3d/util/signal.h"
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
     *      A keyframe is defined by a Frame and a time, expressed in seconds. The time has to be monotonously
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
     *      During the interpolation, the KeyFrameInterpolator emits a \c frame_interpolated Signal which will usually
     *      be connected to the viewer's update() method. The interpolation is stopped when duration has reached.
     *      Another Signal interpolation_stopped will be emitted when the interpolation reaches its end or when the
     *      stop_interpolation() method was triggered.
     *
     *      Note that a Camera has a keyframe_interpolator() method, that can be used to drive the Camera along a
     *      path, or to restore a saved position (a path made of a single keyframe).
     *
     * \note The default duration of any two consecutive keyframes is the same. So for a smoother animation, it is
     *      suggested to regularly sample the viewpoints (as equally as possible).
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
     */
    class KeyFrameInterpolator
    {
    public:
        /**
         * \brief Creates a KeyFrameInterpolator, with \p frame as associated frame().
         * \details The frame() can be set or changed using set_frame(). The interpolation_speed() and
         *      interpolation_period() are set to their default values.
         * \param frame The frame to be associated with the KeyFrameInterpolator.
         */
        explicit KeyFrameInterpolator(Frame *frame = nullptr);

        /** Virtual destructor. Clears the keyframe path. */
        virtual ~KeyFrameInterpolator();

        /*! \name Path creation and modification*/
        //@{
    public:
        /**
         * \brief Appends a new keyframe to the path.
         * \details Same as add_keyframe(const Frame&, float), except that the keyframe_time() is automatically set
         *      to previous keyframe_time() plus one second (or 0.0 if there is no previous keyframe).
         * \param frame The frame to be added as a keyframe.
         * \return \c true if the keyframe has been successfully added.
         */
        bool add_keyframe(const Frame& frame);

        /**
         * \brief Appends a new keyframe to the path, with its associated \p time (in seconds).
         * \details The path will use the current \p frame state.
         * \param frame The frame to be added as a keyframe.
         * \param time The time associated with the keyframe.
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
         * \param index The index of the keyframe.
         * \param t The time to be set for the keyframe.
         * \note The time \c t have to be monotonously increasing over keyframes.
         * \sa keyframe_time()
         */
        void set_keyframe_time(std::size_t index, float t);

        /**
         * \brief Sets the position of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \param pos The position to be set for the keyframe.
         * \sa keyframe_time()
         */
        void set_keyframe_position(std::size_t index, const vec3& pos);

        /**
         * \brief Sets the orientation of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \param q The orientation to be set for the keyframe.
         * \sa keyframe_time()
         */
        void set_keyframe_orientation(std::size_t index, const quat& q);

        /**
         * \brief Removes all keyframes from the path.
         * \details Upon return, the number_of_keyframes() will return 0.
         */
        void delete_path();
        //@}

        /*! \name Associated Frame */
        //@{
    public:
        /**
         * \brief Returns the associated Frame and that is interpolated by the KeyFrameInterpolator.
         * \details The returned Frame was set using set_frame() or with the KeyFrameInterpolator constructor.
         *      When is_interpolation_started(), this Frame's position and orientation will regularly be updated
         *      by a timer, so that they follow the KeyFrameInterpolator path.
         * \return The associated Frame.
         * \sa set_frame()
         */
        Frame* frame() const { return frame_; }

    public:
        /**
         * \brief Sets the frame() associated to the KeyFrameInterpolator.
         * \param frame The frame to be associated with the KeyFrameInterpolator.
         */
        void set_frame(Frame* const frame) { frame_ = frame; }
        //@}

        /*! \name Access keyframe and path parameters */
        //@{
    public:
        /**
         * \brief Returns the number of keyframes used by the interpolation.
         * \details Use add_keyframe() to add new keyframes.
         * \return The number of keyframes.
         */
        std::size_t number_of_keyframes() const { return keyframes_.size(); }

        /**
         * \brief Returns the Frame associated with the keyframe at \p index.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \return The Frame associated with the keyframe.
         * \sa keyframe_time()
         */
        Frame keyframe(std::size_t index) const;

        /**
         * \brief Returns the time corresponding to the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \return The time corresponding to the keyframe.
         * \sa set_keyframe_time().
         */
        float keyframe_time(std::size_t index) const;

        /**
         * \brief Returns the position of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \return The position of the keyframe.
         * \sa set_keyframe_position()
         */
        const vec3& keyframe_position(std::size_t index) const;

        /**
         * \brief Returns the orientation of the \p index-th keyframe.
         * \details The \p index has to be in the range [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \return The orientation of the keyframe.
         * \sa set_keyframe_orientation()
         */
        const quat& keyframe_orientation(std::size_t index) const;

        /**
         * \brief Returns the duration of the KeyFrameInterpolator path, expressed in seconds.
         * \details The duration simply corresponds to last_time() - first_time(). It returns 0.0 if the path has less
         *      than 2 keyframes.
         * \return The duration of the path.
         * \sa keyframe_time().
         */
        float duration() const;

        /**
         * \brief Returns the time corresponding to the first keyframe, expressed in seconds.
         * \details Returns 0.0 if the path is empty.
         * \return The time of the first keyframe.
         * \sa last_time(), duration(), and keyframe_time().
         */
        float first_time() const;

        /**
         * \brief Returns the time corresponding to the last keyframe, expressed in seconds.
         * \details Returns 0.0 if the path is empty.
         * \return The time of the last keyframe.
         * \sa first_time(), duration(), and keyframe_time().
         */
        float last_time() const;
        //@}

        /*! \name Interpolation parameters */
        //@{
    public:

        /** Interpolation method */
        enum Method {
            FITTING,        ///< Fit a curve to the keyframes
            INTERPOLATION   ///< Interpolate between keyframes
        };
        /**
         * \brief Returns the interpolation method.
         * \details Default value is \c Interpolation (cubic spline interpolation).
         * \return The interpolation method.
         */
        Method interpolation_method() const { return interpolation_method_; }

        /**
         * \brief Returns the current interpolation speed.
         * \details Default value is 1.0, which means keyframe_time() will be matched during the interpolation
         *      (provided that your main loop is fast enough).
         * \return The interpolation speed.
         */
        float interpolation_speed() const { return interpolation_speed_; }

        /**
         * \brief Returns the current interpolation period, expressed in milliseconds.
         * \details The update of the frame() state will be done by a timer at this period when
         *      is_interpolation_started(). This period (multiplied by interpolation_speed()) will be added to be the
         *      next frame's time.
         *      The interpolation_period() multiplied by frame_rate() is always equal to 1.0.
         * \return The interpolation period in milliseconds.
         */
        int interpolation_period() const { return static_cast<int>(1000.0 / fps_); }

        /**
         * \brief Returns the desired frame rate. Default value is 30.
         * \details The frame_rate() multiplied by interpolation_period() is always equal to 1.0.
         * \return The frame rate.
         */
        int frame_rate() const { return fps_; }

    public:
        /**
         * \brief Sets the interpolation_method().
         * \param m The interpolation method.
         */
        void set_interpolation_method(Method m);

        /**
         * \brief Sets the interpolation_speed().
         * \param speed Positive finite interpolation speed. Changing it stops playback.
         */
        void set_interpolation_speed(float speed);

        /**
         * \brief Sets the desired frame rate.
         * \param fps Frame rate in 1..1000. Changing it stops playback.
         */
        void set_frame_rate(int fps);
        //@}

        /*! \name Interpolation */
        //@{
    public:
        /**
         * \brief Returns whether the interpolation is being performed.
         * \details Use start_interpolation(), stop_interpolation() or toggleInterpolation() to modify this state.
         * \return \c true if the interpolation is started, \c false otherwise.
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
         * \brief Start interpolation without creating a timer or worker thread.
         * \details The host must call advance_interpolation() at
         * interpolation_period() intervals. Stop and restart preserve the next
         * sample; completion resets to the first. Use a fresh interpolator for
         * manual mode: switching an instance that used timer playback is rejected.
         */
        void start_manual_interpolation();
        /**
         * \brief Advance manual playback by one interpolation sample.
         * \return True when a sample was applied; false when manual playback is inactive.
         */
        bool advance_interpolation();

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
        /**
         * \brief Computes and returns all the interpolated frames.
         * \return A vector of interpolated frames.
         */
        const std::vector<Frame>& interpolate();

        /**
         * \brief Returns the duration of the path, in seconds.
         * \details This is the timeline interpolate() samples: the travel time between
         *      consecutive keyframes plus the dwell time of every keyframe. At an
         *      interpolation speed of 1.0 it is also the duration of the playback and of a
         *      recorded video, which is what a UI shows next to "Record Video".
         * \return The duration in seconds (0 for an empty path).
         * \sa keyframe_dwell(), duration()
         */
        float total_duration() const;

    private:
        // Where each keyframe sits along the path, as a fraction of the path's total: 0 at the
        // first keyframe, 1 at the last.
        //
        // length_fraction() is the fraction of the path's LENGTH at which a keyframe sits: it
        // is the position spline's own parameter, and the pose must be read with it.
        std::vector<float> length_fraction() const;

        // Cinematic speed profile within one travel segment: fraction of the segment's length
        // covered at time fraction w (in [0,1]). stop_start/stop_end say whether the camera is
        // at rest at the segment's ends.
        static float segment_fraction(float w, bool stop_start, bool stop_end);

    public:
        //@}

        /*! \name Path drawing */
        //@{
    public:

        /**
         * \brief Draws the virtual 3D cameras for the keyframes.
         * \details Draws immediately, after the scene, which is only available on OpenGL;
         *      use update_cameras_drawable() to draw them as scene geometry on both backends.
         * \param camera The current camera used by the viewer.
         * \param camera_width Controls the size of the cameras. A good value can be 5% of the scene radius, or
         *      10% of the character height (in walking mode), for instance.
         * \param color The color of the cameras.
         */
        void draw_cameras(const Camera* camera, float camera_width, const vec4& color = vec4(0.5f, 0.8f, 0.5f, 1.0f));

        /**
         * \brief Draws the interpolated camera path.
         * \details Draws immediately, after the scene, which is only available on OpenGL;
         *      use update_path_drawable() to draw the path as scene geometry on both backends.
         * \param camera The current camera used by the viewer.
         * \param thickness The thickness of the path.
         * \param color The color of the path.
         */
        void draw_path(const Camera* camera, float thickness = 2.0f, const vec4& color = vec4(0.15f, 0.45f, 1.0f, 1.0f));

        /**
         * \brief Fills \p drawable with the interpolated camera path, for drawing it as
         *      scene geometry.
         * \details The polyline takes part in the normal scene pass, so scene surfaces
         *      occlude it and it is drawn like any other drawable on both backends. Add the
         *      drawable to the viewer's scene (e.g. with Viewer::add_drawable()) and refresh
         *      it when the keyframes change. The line uses a cylinder impostor, so
         *      \p thickness is the on-screen width in pixels.
         * \param drawable The drawable to fill; its buffers, color and line style are replaced.
         * \param thickness The thickness of the path, in pixels.
         * \param color The color of the path.
         * \return \c false, leaving \p drawable untouched, if the path has fewer than two
         *      interpolated samples (fewer than two keyframes, or interpolation failed).
         */
        bool update_path_drawable(LinesDrawable& drawable, float thickness = 2.0f,
                                  const vec4& color = vec4(0.15f, 0.45f, 1.0f, 1.0f));

        /**
         * \brief Fills \p drawable with the keyframe cameras as solid glyphs, for drawing
         *      them as scene geometry.
         * \details Unlike draw_cameras(), which draws after the scene, a drawable filled
         *      here takes part in the normal depth pass: the back side of each glyph is
         *      hidden behind its front side, and scene surfaces occlude the glyphs (and
         *      the glyphs occlude them) on both backends. Add the drawable to the viewer's
         *      scene (e.g. with Viewer::add_drawable()) and refresh it when the keyframes,
         *      the camera's projection, or \p camera_width changes. Each glyph is a solid
         *      view frustum with its apex at the keyframe position, opening along that
         *      keyframe's view direction, with an up arrow on its front face; \p camera_width
         *      sets the front face's width, the screen aspect its height and the field of
         *      view its depth. Because the glyphs are ordinary triangles, they can also be
         *      picked: glyph \c i owns the triangles
         *      [i * camera_glyph_triangles(), (i + 1) * camera_glyph_triangles()).
         * \param drawable The drawable to fill; its buffers and color are replaced.
         * \param camera The current camera; its field of view and screen aspect size the glyphs.
         * \param camera_width Controls the size of the glyphs, in world units. A good value
         *      can be 5% of the scene radius, or 10% of the character height (in walking mode).
         * \param color The color of the glyphs.
         * \return \c false, leaving \p drawable untouched, if the path has no keyframe, if
         *      \p camera is null, or if \p camera_width is not a positive finite number.
         */
        bool update_cameras_drawable(TrianglesDrawable& drawable, const Camera* camera, float camera_width,
                                     const vec4& color = vec4(0.2f, 0.7f, 0.3f, 1.0f)) const;

        /**
         * \brief The number of triangles one camera glyph occupies in update_cameras_drawable().
         * \details The glyph mesh has a fixed topology, so the count is the same for every
         *      keyframe and does not depend on camera_width, the camera's field of view, the
         *      screen aspect or the number of keyframes; those only move the glyph vertices.
         *      In a drawable filled by update_cameras_drawable(), glyph \c i owns the
         *      triangles [i * camera_glyph_triangles(), (i + 1) * camera_glyph_triangles()),
         *      so a picked triangle \c t belongs to keyframe \c t / camera_glyph_triangles().
         *      draw_cameras() renders the same glyph, so the count describes its geometry too.
         * \return The glyph's triangle count (7 with the current mesh). Call this instead of
         *      hard-coding the number.
         * \note The mapping assumes the drawable's element buffer holds nothing but these
         *      glyphs, i.e. it was filled by update_cameras_drawable() alone. Refill the
         *      drawable whenever the keyframes, the camera's projection or the glyph width
         *      change; a stale drawable maps picked triangles to the old keyframes.
         * \sa update_cameras_drawable()
         */
        static std::size_t camera_glyph_triangles();

        //@}

        /*! \name Dwell time */
        //@{
    public:
        /**
         * \brief Sets the dwell time of the \p index-th keyframe.
         * \details The camera pauses for \p seconds at this keyframe before proceeding
         *      to the next one. The \p index has to be in the range
         *      [0, number_of_keyframes()-1].
         * \param index The index of the keyframe.
         * \param seconds Seconds to pause at the keyframe (0 = no pause).
         * \sa keyframe_dwell()
         */
        void set_keyframe_dwell(std::size_t index, float seconds);

        /**
         * \brief Returns the dwell time of the \p index-th keyframe, in seconds.
         * \param index The index of the keyframe.
         * \return The dwell time in seconds.
         * \sa set_keyframe_dwell()
         */
        float keyframe_dwell(std::size_t index) const;
        //@}

        /*! \name File io */
        //@{
    public:
        /**
         * \brief Saves the camera path to a file.
         * \param file_name The name of the file.
         * \return \c true if the keyframes were successfully saved.
         */
        bool save_keyframes(const std::string& file_name) const;
        /**
         * \brief Reads the camera path from a file.
         * \param file_name The name of the file.
         * \return \c true if the keyframes were successfully read.
         */
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
            /**
             * \brief Returns the dwell time at this keyframe, in seconds.
             * \details The camera pauses for dwell_time() seconds at this keyframe
             *      before proceeding to the next one. Default is 0 (no pause).
             * \return The dwell time in seconds.
             * \sa set_dwell_time()
             */
            float dwell_time() const { return dwell_time_; }
            /**
             * \brief Sets the dwell time at this keyframe, in seconds.
             * \param seconds Seconds to pause at this keyframe before moving on.
             * \sa dwell_time()
             */
            void set_dwell_time(float seconds) { dwell_time_ = seconds; }
            void flip_if_needed(const quat& prev); // flip its orientation if needed
        private:
            vec3 p_;
            quat q_;
            float time_;
            float dwell_time_ = 0.0f; //!< Seconds to pause at this keyframe (0 = no pause).
        };

        // Appends the keyframe cameras as glyph triangles to the given buffers: the glyph
        // mesh is created once (shape::create_camera) and instanced per keyframe with that
        // keyframe's Frame matrix. Every glyph contributes the same triangle range, and
        // glyph i occupies the contiguous indices [i * camera_glyph_triangles(), ...), which
        // is what maps a picked triangle back to its keyframe.
        void append_camera_glyphs(float camera_width, float fov, float aspect,
                                  std::vector<vec3>& points, std::vector<unsigned int>& indices) const;

        // Releases the drawables that draw_path()/draw_cameras() build lazily.
        void drop_immediate_drawables();

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
        bool manual_playback_ = false;
        bool timer_mode_used_ = false;
        int last_stopped_index_;

        // is path valid? Adding new keyframes or editing a keyframe invalidates the path
        bool pathIsValid_;

        LinesDrawable* path_drawable_;
        TrianglesDrawable* cameras_drawable_;

    public:
        Signal<> frame_interpolated;    ///< Emitted when a frame is interpolated.
        Signal<> interpolation_stopped; ///< Emitted when the interpolation is stopped.
    };

}


#endif // EASY3D_RENDERER_KEY_FRAME_INTERPOLATOR_H
