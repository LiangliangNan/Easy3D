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

#ifndef EASY3D_UTIL_LOGGING_H
#define EASY3D_UTIL_LOGGING_H


#include <easy3d/util/easylogging++.h>


// to have the same syntax as glog
#define LOG_FIRST_N LOG_N_TIMES

// to have LOG_IF_EVERY_N
#define LOG_IF_EVERY_N(n, condition, LEVEL)  if (condition) \
    CLOG_EVERY_N(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

// to have LOG_IF_FIRST_N
#define LOG_IF_FIRST_N(n, condition, LEVEL)  if (condition) \
CLOG_N_TIMES(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

// for logging the counter number
#define COUNTER     ELPP_COUNTER->hitCounts()


namespace easy3d {

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
    namespace logging {

        /**
         * @brief Initializes the logging module.
         * @param info_to_stderr \c ture to log messages at a the INFO level to stderr. \c false only log at WARNING,
         *      ERROR, FATAL levels to stderr.
         * @param log_file A string specifying the full path to the log file.
         *      If \p log_file is a valid path: log messages will be written to this file in addition to stderr.
         *      If \p log_file is empty: no log file will be created.
         *      If \p log_file is "default": creat a log file with a title in the form "ApplicationName.log" next to
         *      the executable file.
         * @param verbosity_threshold A \c VLOG(level) with \c level <= \c verbosity_threshold will be written into the
         *      log file (if specified). Value must be in the range [0, 9]. Default is 0 (
         * @note This initialization is optional. If not called, log messages will be written to stderr only.
         */
        void initialize(bool info_to_stderr = false, const std::string &log_file = "", int verbosity_threshold = 0);

        /// Returns the full path of the log file (empty if no log file has been created).
        std::string log_file();

        /// Base class for a logger (that can log messages to whatever)
        /// Users should subclass Logger and override send() to do whatever they want.
        /// \class Logger easy3d/util/logging.h
        class Logger {
        public:
            Logger();
            /// writes the log message \p msg (and may also other given information).
            virtual void send(el::Level level, const std::string& msg) = 0;
        };




        DOCTEST_CLANG_SUPPRESS_WARNING_WITH_PUSH("-Wdeprecated-declarations")
        void color_to_stream(std::ostream& s, Color::Enum code) {
            static_cast<void>(s);    // for DOCTEST_CONFIG_COLORS_NONE or DOCTEST_CONFIG_COLORS_WINDOWS
            static_cast<void>(code); // for DOCTEST_CONFIG_COLORS_NONE
#ifdef DOCTEST_CONFIG_COLORS_ANSI
            if(g_no_colors ||
           (isatty(STDOUT_FILENO) == false && getContextOptions()->force_colors == false))
            return;

        auto col = "";
        // clang-format off
            switch(code) { //!OCLINT missing break in switch statement / unnecessary default statement in covered switch statement
                case Color::Red:         col = "[0;31m"; break;
                case Color::Green:       col = "[0;32m"; break;
                case Color::Blue:        col = "[0;34m"; break;
                case Color::Cyan:        col = "[0;36m"; break;
                case Color::Yellow:      col = "[0;33m"; break;
                case Color::Grey:        col = "[1;30m"; break;
                case Color::LightGrey:   col = "[0;37m"; break;
                case Color::BrightRed:   col = "[1;31m"; break;
                case Color::BrightGreen: col = "[1;32m"; break;
                case Color::BrightWhite: col = "[1;37m"; break;
                case Color::Bright: // invalid
                case Color::None:
                case Color::White:
                default:                 col = "[0m";
            }
        // clang-format on
        s << "\033" << col;
#endif // DOCTEST_CONFIG_COLORS_ANSI

#ifdef DOCTEST_CONFIG_COLORS_WINDOWS
            if(g_no_colors ||
           (isatty(fileno(stdout)) == false && getContextOptions()->force_colors == false))
            return;

#define DOCTEST_SET_ATTR(x) SetConsoleTextAttribute(g_stdoutHandle, x | g_origBgAttrs)

        // clang-format off
        switch (code) {
            case Color::White:       DOCTEST_SET_ATTR(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); break;
            case Color::Red:         DOCTEST_SET_ATTR(FOREGROUND_RED);                                      break;
            case Color::Green:       DOCTEST_SET_ATTR(FOREGROUND_GREEN);                                    break;
            case Color::Blue:        DOCTEST_SET_ATTR(FOREGROUND_BLUE);                                     break;
            case Color::Cyan:        DOCTEST_SET_ATTR(FOREGROUND_BLUE | FOREGROUND_GREEN);                  break;
            case Color::Yellow:      DOCTEST_SET_ATTR(FOREGROUND_RED | FOREGROUND_GREEN);                   break;
            case Color::Grey:        DOCTEST_SET_ATTR(0);                                                   break;
            case Color::LightGrey:   DOCTEST_SET_ATTR(FOREGROUND_INTENSITY);                                break;
            case Color::BrightRed:   DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_RED);               break;
            case Color::BrightGreen: DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_GREEN);             break;
            case Color::BrightWhite: DOCTEST_SET_ATTR(FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); break;
            case Color::None:
            case Color::Bright: // invalid
            default:                 DOCTEST_SET_ATTR(g_origFgAttrs);
        }
            // clang-format on
#endif // DOCTEST_CONFIG_COLORS_WINDOWS
        }


        std::ostream& operator<<(std::ostream& s, const String& in) { return s << in.c_str(); }

        namespace {
            void color_to_stream(std::ostream&, Color::Enum) DOCTEST_BRANCH_ON_DISABLED({}, ;)
    } // namespace

    namespace Color {
        std::ostream& operator<<(std::ostream& s, Color::Enum code) {
            color_to_stream(s, code);
            return s;
        }
    } // namespace Color


    void printVersion() {
            std::cout << PROJECT_VERSION << std::endl;
        if(opt.no_version == false)
            s << Color::Cyan << "[doctest] " << Color::None << "doctest version is \""
              << DOCTEST_VERSION_STR << "\"\n";
    }

};

}



#endif  // EASY3D_UTIL_LOGGING_H
