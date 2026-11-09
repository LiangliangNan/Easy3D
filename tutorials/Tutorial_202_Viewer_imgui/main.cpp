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

#include <easy3d/renderer/camera.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

#include "viewer.h"

/**
 * \example{lineno} Tutorial_202_Viewer_imgui/main.cpp
 *
 * \brief Adding a custom menu bar and widgets to the Easy3D Viewer with Dear ImGui.
 *
 * <b>Overview</b>
 * The default Viewer already offers a rich interaction model, but applications often need
 * their own menus, buttons, and dialogs. This tutorial shows how to layer a Dear ImGui GUI on
 * top of the Viewer by subclassing it (ViewerImGui, shown after this file). The entry point is
 * identical to Tutorial_201 except that it instantiates ViewerImGui instead of Viewer; all of
 * the GUI logic lives in the subclass.
 *
 * <b>The technique</b>
 * Dear ImGui is an immediate-mode GUI: rather than building a persistent widget tree, you
 * describe the UI every frame by calling functions like BeginMainMenuBar(), MenuItem(), and
 * Text(). Each call both renders the widget and reports whether the user interacted with it,
 * so the same code that draws a "File > Open" item also triggers the open() action when it is
 * clicked. The ViewerImGui subclass hooks into the viewer's frame at three points: init()
 * creates the ImGui context and GLFW backend, pre_draw() starts an ImGui frame, lets draw_gui()
 * emit the widgets, then converts ImGui's draw lists into Easy3D overlay draw commands, and
 * post_resize() keeps ImGui's display size in sync with the window. Mouse/keyboard events are
 * routed through callbacks so that clicks on the GUI are consumed by ImGui instead of rotating
 * the scene. See the ViewerImGui class for the full integration.
 *
 * <b>Easy3D API</b>
 * - Viewer: the base interactive viewer; subclass it and override init(), pre_draw(), and
 *   post_resize() to inject the GUI.
 * - ViewerImGui: this tutorial's subclass that owns an ImGuiContext and draws File/View/Help menus.
 * - Camera: oriented here exactly as in Tutorial_201 (view along -Z, +Y up).
 *
 * <b>Running it</b>
 * A window opens with a menu bar at the top. Use File to open/save models or quit, View to take
 * a snapshot, and Help to show an About dialog. The 3D scene underneath remains fully navigable;
 * clicks on the menus are handled by ImGui rather than the camera.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_202_Viewer_imgui/viewer.h
 * The source file of viewer class:
 * \include{lineno} Tutorial_202_Viewer_imgui/viewer.cpp
 *
 * \see Viewer, ViewerImGui, Tutorial_201_Viewer_default
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A bundled sample model shipped with Easy3D's resource data.
    const std::string file_name = resource::directory() + "/data/easy3d.ply";

    // Create the ImGui-enhanced viewer instead of the plain Viewer.
    ViewerImGui viewer(EXAMPLE_TITLE);

    // Orient the camera to a standard front view: look along -Z with +Y pointing up.
    viewer.camera()->setViewDirection(vec3(0, 0, -1));
    viewer.camera()->setUpVector(vec3(0, 1, 0));

    // Load the model into the viewer (which takes ownership); report failure if any.
    if (!viewer.add_model(file_name)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}
