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


#ifndef EASY3D_VIEWER_VIEWER_INPUT_H
#define EASY3D_VIEWER_VIEWER_INPUT_H

namespace easy3d {
    /**
 * \brief Key, modifier, and mouse-button identifiers used by viewer input APIs.
 * \details These values are shared by the supported graphics backends. Do not
 * change the explicitly assigned values: client code may use them in bindings
 * and event callbacks.
 */
    struct ViewerInput {
        /**
     * \brief Keyboard key identifiers used in events and shortcut bindings.
     * \details Character-key values use their ASCII codes; the remaining
     * identifiers follow the corresponding GLFW key values where applicable.
     * A key event reports a key identifier, not text input; use
     * ViewerBase::char_input_event() to receive entered characters.
     * \see ViewerBase, Viewer
     */
        enum Key {
            /// No recognized key.
            KEY_UNKNOWN = -1,
            /// Number-row keys, with values matching ASCII '0' through '9'.
            KEY_0 = 48,
            KEY_1 = 49,
            KEY_2 = 50,
            KEY_3 = 51,
            KEY_4 = 52,
            KEY_5 = 53,
            KEY_6 = 54,
            KEY_7 = 55,
            KEY_8 = 56,
            KEY_9 = 57,
            /// Letter keys, with values matching uppercase ASCII 'A' through 'Z'.
            KEY_A = 65,
            KEY_B = 66,
            KEY_C = 67,
            KEY_D = 68,
            KEY_E = 69,
            KEY_F = 70,
            KEY_G = 71,
            KEY_H = 72,
            KEY_I = 73,
            KEY_J = 74,
            KEY_K = 75,
            KEY_L = 76,
            KEY_M = 77,
            KEY_N = 78,
            KEY_O = 79,
            KEY_P = 80,
            KEY_Q = 81,
            KEY_R = 82,
            KEY_S = 83,
            KEY_T = 84,
            KEY_U = 85,
            KEY_V = 86,
            KEY_W = 87,
            KEY_X = 88,
            KEY_Y = 89,
            KEY_Z = 90,
            /// Common control and navigation keys.
            KEY_ESCAPE = 256,
            KEY_ENTER = 257,
            KEY_DELETE = 261,
            KEY_RIGHT = 262,
            KEY_LEFT = 263,
            KEY_DOWN = 264,
            KEY_UP = 265,
            /// Function keys F1 through F9.
            KEY_F1 = 290,
            KEY_F2 = 291,
            KEY_F3 = 292,
            KEY_F4 = 293,
            KEY_F5 = 294,
            KEY_F6 = 295,
            KEY_F7 = 296,
            KEY_F8 = 297,
            KEY_F9 = 298,
            /// Additional printable-key identifiers using ASCII values.
            KEY_SPACE = 32,
            KEY_COMMA = 44 /* , */,
            KEY_MINUS = 45 /* - */,
            KEY_PERIOD = 46 /* . */,
            KEY_SLASH = 47 /* / */,
            KEY_SEMICOLON = 59 /* ; */,
            KEY_EQUAL = 61 /* = */,
            KEY_LEFT_BRACKET = 91 /* [ */,
            KEY_BACKSLASH = 92 /* \ */,
            KEY_RIGHT_BRACKET = 93 /* ] */,
            /// Physical modifier keys. Events report them as Key values and also
            /// through the corresponding ViewerInput::Modifier bit mask.
            KEY_LEFT_SHIFT = 340,
            KEY_LEFT_CONTROL = 341,
            KEY_LEFT_ALT = 342,
            KEY_LEFT_SUPER = 343,
            KEY_RIGHT_SHIFT = 344,
            KEY_RIGHT_CONTROL = 345,
            KEY_RIGHT_ALT = 346,
            KEY_RIGHT_SUPER = 347
        };

        /**
     * \brief The key modifiers. Currently only Shift, Ctrl, and Alt are supported.
     */
        enum Modifier {
            /// No modifier key is pressed.
            MODIF_NONE = 0x0000,
            /// Shift modifier.
            MODIF_SHIFT = 0x0001,
#ifdef __APPLE__ // To have the same shortcut behavior on macOS and other platforms (i.e., Windows and Linux)
            /// Control modifier (normalized to the platform's shortcut value).
            MODIF_CTRL = 0x0008,
#else
            /// Control modifier (normalized to the platform's shortcut value).
            MODIF_CTRL = 0x0002,
#endif
            /// Alt modifier.
            MODIF_ALT = 0x0004
        };

        /// Mouse button identifiers used by viewer callbacks and event handlers.
        enum Button {
            /// Left mouse button.
            BUTTON_LEFT = 0,
            /// Right mouse button.
            BUTTON_RIGHT = 1,
            /// Middle mouse button.
            BUTTON_MIDDLE = 2
        };
    };
} // namespace easy3d
#endif
