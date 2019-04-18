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

#ifndef EASY3D_FILE_H
#define EASY3D_FILE_H

#include <string>
#include <vector>

// "OpenSceneGraph - <osgDB/FileNameUtils>" has great implementation and documentation

namespace easy3d {

    namespace file {

        bool		is_file(const std::string& filename);
        bool		delete_file(const std::string& filename);

        bool		is_directory(const std::string& filename);
        bool		create_directory(const std::string& path); // Warning: path should be absolute.

        bool		delete_directory(const std::string& path); // all content will be deleted first
        bool		delete_contents(const std::string& path);	 // delete contents only

        std::string parent_directory(const std::string& path);

        // results do not contain the 'dir' part
        void		get_directory_entries(const std::string& dir, std::vector<std::string>& entries, bool recursive);

        // results contain the 'dir' part
        void		get_files(const std::string& dir, std::vector<std::string>& files, bool recursive = false);
        void		get_sub_directories(const std::string& dir, std::vector<std::string>& subs, bool recursive = false);

        std::string get_current_working_directory() ;
        bool		set_current_working_directory(const std::string& path);

        /** Determines the home path for the current user. */
        std::string get_home_directory(void);

        bool		rename_file(const std::string& old_name, const std::string& new_name);

        time_t		get_time_stamp(const std::string& file_or_dir);
        std::string get_time_string(const std::string& file_or_dir);

        /** Gets the parent path from full name (Ex: /a/b/c.Ext => /a/b). */
        std::string dir_name(const std::string& file_name) ;
        /** Gets the extension without dot (Ex: /a/b/c.Ext => Ext). */
        std::string extension(const std::string& file_name, bool lower_case = true) ;

        /** Gets file name with extension (Ex: /a/b/c.Ext => c.Ext). */
        std::string simple_name(const std::string& file_name) ;
        /** Gets file name without path and last extension (Ex: c:/file.ext1.ext2 => file.ext1; /a/b/c.Ext => c). */
        std::string base_name(const std::string& file_name) ;

        /** Gets file path without last extension (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file.ext1). */
        std::string name_less_extension(const std::string& file_name);
        /** Gets file path without all extensions (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file). */
        std::string name_less_all_extensions(const std::string& file_name);

        /**
        * Replaces extension of the given file with 'ext'. If the file name
        * does not have an extension, the given extension is appended.
        */
        std::string replace_extension(std::string const& file_name, std::string const& ext);

        /** Gets root part of a path ("/" or "C:"), or an empty string if none found. */
        std::string get_path_root(const std::string& path);
        /** Tests if path is absolute, as !get_path_root(path).empty(). */
        bool		is_absolute_path(const std::string& path);
        /** If 'to' is in a subdirectory of 'from' then this function returns the subpath, otherwise it just returns the file name.
        * The function does \b not automagically resolve paths as the system does, so be careful to give canonical paths.
        * However, the function interprets slashes ('/') ans backslashes ('\') as they were equal.
        */
        std::string get_relative_path(const std::string& from, const std::string& to);
        /** Removes .. and . dirs in a path */
        std::string get_absolute_path(const std::string& path);

        std::string convert_to_windows_style(const std::string& path);
        /** Converts back slashes (\) to forward slashes (/). */
        std::string convert_to_unix_style(const std::string& path);

        /** Get the path separator for the current platform. */
        char get_native_path_separator();
        /** Check if the path contains only the current platform's path separators. */
        bool is_native_style(const std::string& path);
        /** Convert the path to contain only the current platform's path separators. */
        std::string convert_to_native_style(const std::string& path);

        bool copy_file(const std::string& original, const std::string& copy);
        bool file_contains_string(const std::string& file_name, const std::string& x);

        void read_file_to_string(const std::string& filename, std::string& data);
		void write_string_to_file(const std::string& data, const std::string& filename);

    }


} // namespace easy3d


#endif	// EASY3D_FILE_H
