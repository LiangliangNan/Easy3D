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

#include <easy3d/util/console_style.h>


#include <vector>
#include <cstring>
#include <cstdlib>  // for std::getenv

namespace easy3d
{

    namespace console {

        std::vector<std::string> styles = {
                /// Text reset
                "\033[0m",        /// Text Reset
                /// Regular colors
                "\033[0;30m",     /// Black
                "\033[0;31m",     /// Red
                "\033[0;32m",     /// Green
                "\033[0;33m",     /// Yellow
                "\033[0;34m",     /// Blue
                "\033[0;35m",     /// Purple
                "\033[0;36m",     /// Cyan
                "\033[0;37m",     /// White
                /// Bold
                "\033[1;30m",     /// Black
                "\033[1;31m",     /// Red
                "\033[1;32m",     /// Green
                "\033[1;33m",     /// Yellow
                "\033[1;34m",     /// Blue
                "\033[1;35m",     /// Purple
                "\033[1;36m",     /// Cyan
                "\033[1;37m",     /// White
                /// Underline
                "\033[4;30m",     /// Black
                "\033[4;31m",     /// Red
                "\033[4;32m",     /// Green
                "\033[4;33m",     /// Yellow
                "\033[4;34m",     /// Blue
                "\033[4;35m",     /// Purple
                "\033[4;36m",     /// Cyan
                "\033[4;37m",     /// White
                /// Background
                "\033[40m",       /// Black
                "\033[41m",       /// Red
                "\033[42m",       /// Green
                "\033[43m",       /// Yellow
                "\033[44m",       /// Blue
                "\033[45m",       /// Purple
                "\033[46m",       /// Cyan
                "\033[47m",       /// White
                /// High intensity
                "\033[0;90m",     /// Black
                "\033[0;91m",     /// Red
                "\033[0;92m",     /// Green
                "\033[0;93m",     /// Yellow
                "\033[0;94m",     /// Blue
                "\033[0;95m",     /// Purple
                "\033[0;96m",     /// Cyan
                "\033[0;97m",     /// White
                /// Bold high intensity
                "\033[1;90m",     /// Black
                "\033[1;91m",     /// Red
                "\033[1;92m",     /// Green
                "\033[1;93m",     /// Yellow
                "\033[1;94m",     /// Blue
                "\033[1;95m",     /// Purple
                "\033[1;96m",     /// Cyan
                "\033[1;97m",     /// White
                /// High intensity backgrounds
                "\033[0;100m",    /// Black
                "\033[0;101m",    /// Red
                "\033[0;102m",    /// Green
                "\033[0;103m",    /// Yellow
                "\033[0;104m",    /// Blue
                "\033[10;95m",    /// Purple
                "\033[0;106m",    /// Cyan
                "\033[0;107m",    /// White
        };


        /// Returns if the terminal supports using styles in output.
        /// code taken from glog
        bool terminal_supports_styles() {
            static bool term_supports_color = false;
            static bool already_known = false;
            if (already_known)
                return term_supports_color;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
            // on Windows TERM variable is usually not set, but the console does support colors.
            term_supports_color = true;
#else
            // On non-Windows platforms, we rely on the TERM variable.
            const char *const term = std::getenv("TERM");
            if (term != nullptr && term[0] != '\0') {
                term_supports_color =
                        !strcmp(term, "xterm") ||
                        !strcmp(term, "xterm-color") ||
                        !strcmp(term, "xterm-256color") ||
                        !strcmp(term, "screen-256color") ||
                        !strcmp(term, "konsole") ||
                        !strcmp(term, "konsole-16color") ||
                        !strcmp(term, "konsole-256color") ||
                        !strcmp(term, "screen") ||
                        !strcmp(term, "linux") ||
                        !strcmp(term, "cygwin");
            }
#endif
            already_known = true;
            return term_supports_color;
        }

    }

    /// Call this function to enable the console style (if supported)
    std::ostream& operator<<(std::ostream& s, console::Style style) {
        if (!console::terminal_supports_styles())
            return s;
        s << console::styles[style];
        return s;
    }

}
