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

#ifndef _EASY3D_PLUGIN_H_
#define _EASY3D_PLUGIN_H_

#include <string>
#include <vector>


namespace easy3d {

	// Abstract class for plugins
	// All plugins MUST be derived from this class and may implement any/all the 
	// callbacks marked `virtual` here.
	// NOTE: Return true of callbacks tells the viewer that the event has been 
	// handled and that it should not be passed to the viewer and other plugins.
	class Window;

	class Plugin
	{
	public:
		Plugin(Window * window, const std::string& title);
		virtual ~Plugin() {}

		const std::string& title() const { return title_; }

	public:
		virtual bool draw() = 0;

		virtual void cleanup() { }

	protected:
		Window *	window_;
		std::string	title_;
	};

}

#endif	// _EASY3D_PLUGIN_H_
