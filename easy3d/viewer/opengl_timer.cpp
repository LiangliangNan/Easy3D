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

#include <easy3d/viewer/opengl_timer.h>

#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/opengl_error.h>

#include <cassert>


namespace easy3d {

    OpenGLTimer::OpenGLTimer(bool start_timing /* = true*/)
        : query_id_(0)
        , running_(false)
    {
        glGenQueries(1, &query_id_); easy3d_debug_log_gl_error;

        if (start_timing)
            start();
    }


    OpenGLTimer::~OpenGLTimer() {
        if (query_id_) {
            glDeleteQueries(1, &query_id_); easy3d_debug_log_gl_error;
        }
    }


    void OpenGLTimer::start() {
        assert(!running_);
        glBeginQuery(GL_TIME_ELAPSED, query_id_);	easy3d_debug_log_gl_error;
        running_ = true;
    }


    void OpenGLTimer::stop() {
        assert(running_);
        glEndQuery(GL_TIME_ELAPSED);
        running_ = false;
    }

    double OpenGLTimer::time() {
        if (running_)
            stop();

        // check and wait until the result is available
        GLuint queryReady = 0;
        do {
            // check whether the passed samples counter is immediately available. If a delay
            // would occur waiting for the query result, GL_FALSE is returned. Otherwise,
            // GL_TRUE is returned, which also indicates that the results of all previous
            // queries are available as well.
            glGetQueryObjectuiv(query_id_, GL_QUERY_RESULT_AVAILABLE, &queryReady);
        } while (!queryReady);

        GLuint query_time = 0;
        glGetQueryObjectuiv(query_id_, GL_QUERY_RESULT, &query_time);   easy3d_debug_log_gl_error

        return static_cast<double>(query_time) / 1000000.0; // in milliseconds
    }

}
