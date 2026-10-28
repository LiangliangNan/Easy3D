// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VIEWER_INPUT_H
#define EASY3D_VIEWER_VIEWER_INPUT_H

namespace easy3d {
// Input identifiers shared by the OpenGL and Vulkan viewer counterparts.
struct ViewerInput {
    enum Key { // Do NOT modify the values!!!
        // the unknown key
        KEY_UNKNOWN = -1,
        // the number keys
        KEY_0 = 48, KEY_1 = 49, KEY_2 = 50, KEY_3 = 51, KEY_4 = 52, KEY_5 = 53, KEY_6 = 54, KEY_7 = 55,
        KEY_8 = 56, KEY_9 = 57,
        // the character keys
        KEY_A = 65, KEY_B = 66, KEY_C = 67, KEY_D = 68, KEY_E = 69, KEY_F = 70, KEY_G = 71, KEY_H = 72,
        KEY_I = 73, KEY_J = 74, KEY_K = 75, KEY_L = 76, KEY_M = 77, KEY_N = 78, KEY_O = 79, KEY_P = 80,
        KEY_Q = 81, KEY_R = 82, KEY_S = 83, KEY_T = 84, KEY_U = 85, KEY_V = 86, KEY_W = 87, KEY_X = 88,
        KEY_Y = 89, KEY_Z = 90,
        // the functional keys
        KEY_RIGHT = 262, KEY_LEFT = 263, KEY_DOWN = 264, KEY_UP = 265,
        KEY_F1 = 290,   KEY_F2 = 291,   KEY_F3 = 292,   KEY_F4 = 293,   KEY_F5 = 294,
        KEY_F6 = 295,   KEY_F7 = 296,   KEY_F8 = 297,   KEY_F9 = 298,
        // some printable keys
        KEY_SPACE = 32,
        KEY_COMMA = 44/* , */,  KEY_MINUS     = 45/* - */,  KEY_PERIOD = 46/* . */,
        KEY_SLASH = 47/* / */,  KEY_SEMICOLON = 59/* ; */,  KEY_EQUAL  = 61/* = */,
        KEY_LEFT_BRACKET = 91/* [ */,   KEY_BACKSLASH = 92/* \ */, KEY_RIGHT_BRACKET = 93/* ] */
    };

    /**
     * \brief The key modifiers. Currently only Shift, Ctrl, and Alt are supported.
     */
    enum Modifier { /* Do NOT modify the values!!! */
        MODIF_NONE = 0x0000,
        MODIF_SHIFT = 0x0001,
#ifdef __APPLE__    // To have the same shortcut behavior on macOS and other platforms (i.e., Windows and Linux)
        MODIF_CTRL = 0x0008,
#else
        MODIF_CTRL = 0x0002,
#endif
        MODIF_ALT = 0x0004
    };

    /**
     * \brief Mouse buttons.
     */
    enum Button { // Do NOT modify the values!!!
        BUTTON_LEFT = 0,
        BUTTON_RIGHT = 1,
        BUTTON_MIDDLE = 2
    };

};
}
#endif
