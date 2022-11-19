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

#ifndef EASY3D_UTIL_CONSOLE_STYLE_H
#define EASY3D_UTIL_CONSOLE_STYLE_H

#include <ostream>


namespace easy3d {

    /// \brief Defines the styles for output to the console window.
    /// \namespace easy3d::console
    namespace console {

        enum Style {
            // Reset
            None = 0,         /// Reset
            // Regular colors
            Black = 1,        /// Black
            Red = 2,          /// Red
            Green = 3,        /// Green
            Yellow = 4,       /// Yellow
            Blue = 5,         /// Blue
            Purple = 6,       /// Purple
            Cyan = 7,         /// Cyan
            White = 8,        /// White
            // Bold
            BBlack = 9,       /// Black
            BRed = 10,         /// Red
            BGreen = 11,       /// Green
            BYellow = 12,      /// Yellow
            BBlue = 13,        /// Blue
            BPurple = 14,      /// Purple
            BCyan = 15,        /// Cyan
            BWhite = 16,       /// White
            // Underline
            UBlack = 17,       /// Black
            URed = 18,         /// Red
            UGreen = 19,       /// Green
            UYellow = 20,      /// Yellow
            UBlue = 21,        /// Blue
            UPurple = 22,      /// Purple
            UCyan = 23,        /// Cyan
            UWhite = 24,       /// White
            // Background
            On_Black = 25,     /// Black
            On_Red = 26,       /// Red
            On_Green = 27,     /// Green
            On_Yellow = 28,    /// Yellow
            On_Blue = 29,      /// Blue
            On_Purple = 30,    /// Purple
            On_Cyan = 31,      /// Cyan
            On_White = 32,     /// White
            // High intensity
            IBlack = 33,       /// Black
            IRed = 34,         /// Red
            IGreen = 35,       /// Green
            IYellow = 36,      /// Yellow
            IBlue = 37,        /// Blue
            IPurple = 38,      /// Purple
            ICyan = 39,        /// Cyan
            IWhite = 40,       /// White
            // Bold high intensity
            BIBlack = 41,      /// Black
            BIRed = 42,        /// Red
            BIGreen = 42,      /// Green
            BIYellow = 44,     /// Yellow
            BIBlue = 45,       /// Blue
            BIPurple = 46,     /// Purple
            BICyan = 47,       /// Cyan
            BIWhite = 48,      /// White
            // High intensity backgrounds
            On_IBlack = 49,    /// Black
            On_IRed = 50,      /// Red
            On_IGreen = 51,    /// Green
            On_IYellow = 52,   /// Yellow
            On_IBlue = 53,     /// Blue
            On_IPurple = 54,   /// Purple
            On_ICyan = 55,     /// Cyan
            On_IWhite = 56     /// White
        };


        /// Returns if the terminal supports using styles in output.
        bool terminal_supports_styles();

     } // namespace style

     /// Styled output.
     /// Example usage: \code std::cout << console::Style::Green << "Easy3D" << std::endl; \endcode
     std::ostream& operator<<(std::ostream& s, console::Style style);

}

#endif  // EASY3D_UTIL_CONSOLE_STYLE_H
