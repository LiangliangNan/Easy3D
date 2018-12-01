/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <memory>
#include <iostream>
#include <algorithm>

#include "mapple.h"
#include <easy3d/guiviewer/window.h>
#include <easy3d/guiviewer/plugin_examples.h>


using namespace easy3d;

int main(int /* argc */, char ** /* argv */) {
    try {
#if 1
        Viewer app("Easy3D_Viewer", 80, 3, 2);
        //BasicViewer app("Easy3D_BasicViewer", 80, 3, 2);
#else
		Mapple app(40, 3, 2);

		Window win(&app, "easy3d Demo Window");
		PluginSmartBox smartbox(&win);
		PluginPolyFit  polyfit(&win);

#if 1
		Window win2(&app, "Another Window");
		PluginSmartBox smartbox2(&win2);
#endif
#endif

		app.run();

    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
		std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
