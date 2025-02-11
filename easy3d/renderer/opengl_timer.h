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

#ifndef EASY3D_RENDERER_OPENGL_TIMER_H
#define EASY3D_RENDERER_OPENGL_TIMER_H

namespace easy3d {

    /**
     * \brief Accurate timing of GPU operations.
     *
     * \class OpenGLTimer easy3d/renderer/opengl_timer.h
     *
     * \details Because the GPU is based on an asynchronously processed command stream that is implicitly generated
     *      by OpenGL calls, it is not possible to determine the amount of time spent processing a set of OpenGL calls
     *      by timing the calls themselves at the application level. Instead, the timing must be done via an
     *      asynchronous timer system. This timer is started and stopped at the beginning and end of the OpenGL
     *      commands of interest.  However, the timing results must be queried at some point later, as the pipeline
     *      must complete before the timer values are known. This class implements such a timer that behaves like a
     *      CPU timer (See StopWatch).
     *
     * Usage example:
     *      \code
     *          OpenGLTimer t;
     *          // opengl calls ...
     *          draw();
     *          fprintf(stdout, "\rTime: %2.4f ms    ", t.time());
     *      \endcode
     */
    class OpenGLTimer
    {
    public:
        /**
         * \brief Constructor.
         * \param start_timing Whether to start the timer immediately.
         */
        explicit OpenGLTimer(bool start_timing = true);
        /**
         * \brief Destructor.
         */
        virtual ~OpenGLTimer();

        /**
         * \brief Start the timer. The next OpenGL call will be the first timed.
         * \note This must be called from a thread with the OpenGL context bound.
         */
        void start();

        /**
         * \brief Check if the timer is running.
         * \return \c true if the timer is running, \c false otherwise.
         */
        bool is_running() const { return running_; }

        /**
         * \brief Stop the timer. The previous OpenGL call will be the last timed.
         * \note This must be called from a thread with the OpenGL context bound.
         */
        void stop();

        /**
         * \brief Get the GPU time consumed since the last start.
         * \return The GPU time in milliseconds.
         */
        double time();

    protected:
        unsigned int query_id_; ///< The query ID for the OpenGL timer.
        bool running_;          ///< Indicates whether the timer is running.
    };

}


#endif  // EASY3D_RENDERER_OPENGL_TIMER_H
