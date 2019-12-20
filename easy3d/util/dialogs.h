/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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


#ifndef EASY3D_FILE_DIALOG_H
#define EASY3D_FILE_DIALOG_H

#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d
{
    class FileDialog
    {
    public:

#ifdef HAS_OSDIALOG
        // usage
        //        const std::string filters =
        //                "Source:off,obj,ply;"
        //                "Header:xyz,ply";
        //        const std::string& file_name = FileDialog::open("", filters);
        static std::string open(const std::string& file_name, const std::string& filters) ;
#endif

#if 1 // HAS_PORTABLE_FILE_DIALOGS
        /**
         * \brief Open a native file open dialog.
         * \param title The title of the dialog, e.g., "Choose a file to open".
         * \param default_path The default file path, e.g., setting::resource_directory() + "/data".
         * \param filters The accepted file filters, e.g.,
         *              const std::vector<std::string> filters = {
         *                  "Mesh Files (.obj .ply)", "*.obj *.ply",
         *                  "Point Cloud Files (.bin .xyz .ply)", "*.bin *.xyz *.ply",
         *                  "All Files", "*"
         *              };
         * \param multiple Allow choosing multiple files if it is true.
         */
        static std::vector<std::string> open(
                const std::string& title,               // e.g., "Please choose a file"
                const std::string& default_path = "",   // e.g., current working/data directory
                const std::vector<std::string>& filters = { "All Files", "*.*" },
                bool multiple = false
                );

        /**
         * \brief Open a native file save dialog.
         * \param title The title of the dialog, e.g., "Choose a file to open".
         * \param default_file_name The suggested file name, e.g., bunny.ply
         * \param filters The accepted file filters, e.g.,
         *              const std::vector<std::string> filters = {
         *                  "Mesh Files (.obj .ply)", "*.obj *.ply",
         *                  "Point Cloud Files (.bin .xyz .ply)", "*.bin *.xyz *.ply",
         *                  "All Files", "*"
         *              };
         */
        static std::string save(
                const std::string& title,               // e.g., "Please choose a file"
                const std::string& default_path = "",   // e.g., current working/data directory
                const std::vector<std::string>& filters = { "All Files", "*.*" },
                bool confirm_overwrite = true
                );
#endif


#ifdef HAS_TINY_FILE_DIALOGS

        /**
         * \brief Open a native file open dialog.
         * \param filetypes The accepted file extentions, e.g., "{"*.jpg","*.png"}"
         * \param default_path The default file path.
         */
        static std::string open(
                const std::vector<std::string> & filetypes,
                const std::string& default_path = ""
                );

        /**
         * \brief Open a native file open dialog, which allows multiple selection.
         * \param filetypes The accepted file extentions, e.g., "{"*.jpg","*.png"}"
         * \param default_path The default file path.
         * \param multiple Set to ``true`` if you would like to be able to select multiple
         *     files at once.
         */
        static std::vector<std::string> open(
                const std::vector<std::string>& filetypes,
                bool multiple,
                const std::string& default_path = ""
                );


        /**
         * \brief Open a native file save dialog.
         * \param default_file_name The suggested file name, e.g., the file's original name.
         */
        static std::string save(
                const std::vector<std::string>& filetypes,
                const std::string& default_file_name = ""
                );  
#endif

    };

#ifdef HAS_TINY_FILE_DIALOGS

    class ColorDialog {
    public:
        // c is the current color
        static vec3 pick(const vec3& c);
    };

#endif

}

#endif	// EASY3D_FILE_DIALOG_H

