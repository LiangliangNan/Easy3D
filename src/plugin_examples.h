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

#ifndef _EASY3D_PLUGIN_EXAMPLES_H_
#define _EASY3D_PLUGIN_EXAMPLES_H_

#include "plugin.h"


class PluginSmartBox : public Plugin
{
public:
	PluginSmartBox(Window* win): Plugin(win, "Plugin - SmartBox") {}

	bool draw();
};


class PluginPolyFit : public Plugin
{
public:
	PluginPolyFit(Window* win) : Plugin(win, "Plugin - PolyFit") {}

	bool draw();
};

#endif // _EASY3D_PLUGIN_EXAMPLES_H_
