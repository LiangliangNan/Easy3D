/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
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

	// Abstract class for plugins. 
	// A plugin typically implements some methods. It may contain several GUI 
	// elements (e.g., buttons for executing commands, input boxes for methods 
	// or rendering parameters) and is always attached to a panel.

	class Panel;

	class Plugin
	{
	public:
		Plugin(Panel * panel, const std::string& title);
		virtual ~Plugin() {}

		const std::string& title() const { return title_; }

	public:
        virtual bool draw() const = 0;

		virtual void cleanup() { }

	protected:
		Panel *		panel_;
		std::string	title_;
	};

}

#endif	// _EASY3D_PLUGIN_H_
