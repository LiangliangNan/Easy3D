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

/** ----------------------------------------------------------
 *
 * the code is adapted from NanoGUI with enhancement and improvement.
 *		- NanoGUI (Nov 17th, 2018)
 * The original code is available at
 * https://github.com/wjakob/nanogui/blob/master/include/nanogui/common.h
 *
 * NanoGUI is a minimalistic cross-platform widget library for OpenGL 3.x
 * or higher.
 *
 *----------------------------------------------------------*/


#ifndef EASY3D_FILE_DIALOG_H
#define EASY3D_FILE_DIALOG_H

#include <string>
#include <vector>


namespace easy3d
{
	/**
	 * \brief Open a native file open/save dialog.
	 *
	 * \param filetypes
	 *     Pairs of permissible formats with descriptions like
	 *     ``("png", "Portable Network Graphics")``.
	 *
	 * \param save
	 *     Set to ``true`` if you would like subsequent file dialogs to open
	 *     at whatever folder they were in when they close this one.
	 */
	std::string file_dialog(const std::vector< std::pair<std::string, std::string> > &filetypes, bool save);

	/**
	 * \brief Open a native file open dialog, which allows multiple selection.
	 *
	 * \param filetypes
	 *     Pairs of permissible formats with descriptions like
	 *     ``("png", "Portable Network Graphics")``.
	 *
	 * \param save
	 *     Set to ``true`` if you would like subsequent file dialogs to open
	 *     at whatever folder they were in when they close this one.
	 *
	 * \param multiple
	 *     Set to ``true`` if you would like to be able to select multiple
	 *     files at once. May not be simultaneously true with \p save.
	 */
	std::vector<std::string> file_dialog(const std::vector< std::pair<std::string, std::string> > &filetypes, bool save, bool multiple);
}

#endif	// EASY3D_FILE_DIALOG_H

