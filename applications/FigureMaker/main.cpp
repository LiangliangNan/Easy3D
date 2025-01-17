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


#include "viewer.h"
#include <easy3d/util/initializer.h>

using namespace easy3d;


// This application allows to render a set of (well aligned) models in a grid layout, convenient for making figures.

int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize(false, true, true);

    // create a 1 by 1 layout by default
    FigureMaker viewer(1, 1, APP_TITLE);

    // usage
    viewer.set_usage(
        "Usage:\n"
        "--------------------------------------\n"
        "  Ctrl + o: add model(s) to the viewer\n"
        "  ------------------------------------\n"
        "  s: snapshot\n"
        "  ------------------------------------\n"
        "  f: fit model to screen\n"
        "  l: toggle lighting\n"
        "  d: toggle dividing lines\n"
        "--------------------------------------\n"
    );

    // run the viewer
    return viewer.run();
}