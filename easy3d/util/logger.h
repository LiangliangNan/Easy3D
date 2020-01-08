/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_UTIL_LOGGER_H
#define EASY3D_UTIL_LOGGER_H

#include <string>
#include <cstdio>
#include <cstdarg>


namespace easy3d {

    enum easy3d_log_level {
        LOG_NONE  = 0,
        LOG_FATAL = 1,
        LOG_ERROR = 2,
        LOG_WARN  = 3,
        LOG_INFO  = 4,
        LOG_DEBUG = 5
    };

    // TODO: have similar interface as glog, e.g.,
    //   LOG(INFO) << "Current working directory: " << dir;

    class Logger
    {
    public:
        /**
         * Sets the logging level. All messages with lower priority will be ignored.
         * @param level Logging level
         */
        static void set_level(int level) { instance().log_level_ = level; }

        /**
         * Returns the currently set logging level.
         * @return current logging level
         */
        static int log_level() { return instance().log_level_; }

        /**
         * Sets the logging destination
         * @param name Filename or an empty string for console
         */
        static void set_destination(const std::string& name) { instance()._set_destination(name); }

        /**
         * Print log message
         * @param level Log level
         * @param fmt Message format
         * @return
         */
        static Logger& log(int level, const char* format, ...)
        {
            va_list arglist;
            va_start(arglist, format);
            instance()._log(level, format, arglist);
            va_end(arglist);
            return instance();
        }

    #define LOG_METHOD(NAME, LEVEL)                             \
        static Logger& NAME(const char* format, ...)                \
        {                                                       \
            return log(LEVEL, format);                          \
        }

        LOG_METHOD(fatal, LOG_FATAL)
        LOG_METHOD(error, LOG_ERROR)
        LOG_METHOD(warn,  LOG_WARN)
        LOG_METHOD(info,  LOG_INFO)
        LOG_METHOD(debug, LOG_DEBUG)

    private:
        Logger() : stream(stdout), log_level_(LOG_WARN) {}

        ~Logger()
        {
            if ((stream!=nullptr)&&(stream!=stdout)) {
                fclose(stream);
            }
        }

        static Logger& instance()
        {
            static Logger logger;
            return logger;
        }

        void _set_destination(const std::string& name)
        {
            if (name.empty()) {
                stream = stdout;
            }
            else {
                stream = fopen(name.c_str(), "w");
                if (stream == nullptr) {
                    stream = stdout;
                }
            }
        }

        Logger& _log(int level, const char* format, va_list arglist)
        {
            if (level > log_level_ ) return instance();
            int ret = vfprintf(stream, format, arglist);
            return instance();
        }

        template <typename T>
        friend Logger& operator<<(Logger& logger, const T& msg);

    private:
        FILE*   stream;
        int     log_level_;
    };


    template <typename T>
    Logger& operator<<(Logger& logger, const T& msg)
    {
        logger.stream << msg;
        return logger;
    }

#define LOG(LEVEL)                  \
    Logger& Logger::##log(LEVEL)    \
    {                               \
        return Logger::instance();  \
    }


}

#endif //EASY3D_UTIL_LOGGER_H
