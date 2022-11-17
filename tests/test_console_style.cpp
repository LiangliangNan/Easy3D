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

#include <iostream>

#include "easy3d/util/version.h"
#include <easy3d/util/console_style.h>

using namespace easy3d;


int test_console_style() {
    if (!console::terminal_supports_styles()) {
        std::cerr << "WARNING: console styles not supported on your system.\n\t"
                     "Maybe you didn't run the application from the console?" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << console::Style::Red << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BRed << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::URed << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::IRed << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BIRed << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_Red << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_IRed << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::None << "Easy3D version: " << version() << std::endl;

    std::cout << console::Style::Green << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BGreen << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::UGreen << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::IGreen << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BIGreen << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_Green << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_IGreen << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::None << "Easy3D version: " << version() << std::endl;

    std::cout << console::Style::Blue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BBlue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::UBlue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::IBlue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BIBlue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_Blue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_IBlue << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::None << "Easy3D version: " << version() << std::endl;

    std::cout << console::Style::Purple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BPurple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::UPurple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::IPurple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::BIPurple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_Purple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::On_IPurple << "Easy3D version: " << version() << std::endl;
    std::cout << console::Style::None << "Easy3D version: " << version() << std::endl;

    std::cout << console::Style::Red   << "Easy3D major: " << version_major() << std::endl;
    std::cout << console::Style::Green << "Easy3D minor: " << version_minor() << std::endl;
    std::cout << console::Style::Blue  << "Easy3D patch: " << version_patch() << std::endl;

    // restore the default
    std::cout << console::Style::None;

    return EXIT_SUCCESS;
}