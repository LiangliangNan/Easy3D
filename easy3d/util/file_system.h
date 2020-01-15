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

#ifndef EASY3D_UTIL_FILE_SYSTEM_H
#define EASY3D_UTIL_FILE_SYSTEM_H

#include <string>
#include <vector>
#include <fstream>


namespace easy3d {

    /**
     * @brief A very basic file_system implementation.
     * @attention Functions are not sufficiently tested.
     * @todo [Liangliang] Comprehensive tests.
     *
     * @related ghc::filesystem - A C++17-like filesystem implementation for C++11/C++147/C++17
     *          https://github.com/gulrak/filesystem/blob/master/include/ghc/filesystem.hpp
     */
    namespace file_system {

        /**
         * @brief Tests if 'path' is an existing file.
         * @param path The full path of a file (including file extention).
         * @return ture If the file 'path' exists.
         */
        bool    is_file(const std::string& path);

        /**
         * @brief Tests if 'path' is an existing directory.
         * @param path The full path of a directory.
         * @return ture If the directory 'path' exists.
         */
        bool	is_directory(const std::string& path);

        /**
         * @brief Creates a directory entitled 'path'.
         * @param path The full path of the directory to be created.
         * @return ture If the directory has been successfully created or already exists.
         */
        bool	create_directory(const std::string& path);

        /**
         * @brief Deletes the file 'path'.
         * @param path The full path of a file.
         * @return ture If the file has been successfully deleted or doesn't exist.
         * @todo [Liangliang] Test if path should be absolute.
         */
        bool	delete_file(const std::string& path);

        /**
         * @brief Deletes the directory 'path' (and its contents will be deleted recursively).
         * @param path The full path of a directory.
         * @return ture If the directory has been successfully deleted or doesn't exist.
         * @todo [Liangliang] Test if path should be absolute.
         */
        bool	delete_directory(const std::string& path);

        /**
         * @brief Deletes the contents of the directory 'path' (the directory will not be deleted).
         * @param path The full path of a directory.
         * @return ture If the contents of directory have been successfully deleted or don't exist.
         * @todo [Liangliang] Test if path should be absolute.
         */
        bool	delete_contents(const std::string& path);

        /**
         * @brief Query the entries of a directory 'path' (including subdirectories and files).
         * @param path The full path of a directory.
         * @param entries Return the entries of 'path'. Result strings are simple names (i.e.,
         *        without the 'path' part).
         * @param recursive Perform the query recursively if true.
         * @related get_files(), get_sub_directories().
         */
        void	get_directory_entries(const std::string& path,
                                      std::vector<std::string>& entries,
                                      bool recursive);

        /**
         * @brief Query file entries of a directory 'path'.
         * @param path The full path of a directory.
         * @param files Return the file entries of 'path'. Result strings are simple names (i.e.,
         *        without the 'path' part).
         * @param recursive Perform the query recursively if true.
         * @related get_directory_entries(), get_sub_directories().
         */
        void	get_files(const std::string& path, std::vector<std::string>& files, bool recursive);

        /**
         * @brief Query subdirectory entries of a directory 'path'.
         * @param path The full path of a directory.
         * @param subs Return the subdirectory entries of 'path'. Result strings are only the
         *        names of the subdirectories (i.e., without the 'path' part).
         * @param recursive Perform the query recursively if true.
         * @related get_directory_entries(), get_files().
         */
        void	get_sub_directories(const std::string& path,
                                    std::vector<std::string>& subs,
                                    bool recursive);

        /**
         * @brief Query the current working directory.
         * @return The string representing the current working directory.
         */
        std::string current_working_directory() ;

        /**
         * @brief Set the current working directory.
         * @param path The string representing the new current working directory.
         * @return true on success.
         */
        bool    set_current_working_directory(const std::string& path);

        /**
         * @brief Query the home path for the current user.
         * @return The string representing the home directory of the current user.
         */
        std::string home_directory();

        /**
         * @brief Query the name of *this* executable.
         * @return The string representing the full path of this executable, e.g., C:/a/b/c.exe
         */
        std::string executable();

        /**
         * @brief Query the directory where the executable file is located.
         * @return The string representing the directory in which the executable resides.
         */
        std::string executable_directory();

        /**
         * @brief Rename the file from 'old_name' to 'new_name'.
         * @param old_name The full path of the file to be renamed.
         * @param new_name The full path of the file's new name.
         * @return true on success.
         */
        bool    rename_file(const std::string& old_name, const std::string& new_name);

        /**
         * @brief Query the time stamp of a file or directory.
         * @param path The full path of the file or directory.
         * @return The time stamp.
         */
        time_t	time_stamp(const std::string& path);

        /**
         * @brief Query the time stamp of a file or directory as a string.
         * @param path The full path of the file or directory.
         * @return The string of the time stamp.
         */
        std::string time_string(const std::string& path);

        /**
         * @brief Query the size of the file.
         * @param path The full path of the file.
         * @return The size of the file (in bytes).
         */
        std::ifstream::pos_type file_size(const std::string& path);

        /**
         * @brief Query the parent path from full name of a file or directory
         *        (e.g., /a/b/c.Ext => /a/b)
         * @param path The full path of the file or directory.
         * @return The string of the parent path.
         */
        std::string parent_directory(const std::string& path) ;

        /**
         * @brief Query the file extension without dot (e.g., /a/b/c.Ext => Ext).
         * @param path The full path of a file.
         * @param lower Converts the string to lower case if true.
         * @return The string of the file extension.
         */
        std::string extension(const std::string& path, bool lower = true) ;

        /**
         * @brief Gets file name without path but with extension (e.g, /a/b/c.Ext => c.Ext)
         * @param path The full path of a file.
         * @return The string of the file name without path but with extension.
         */
        std::string simple_name(const std::string& path) ;

        /**
         * @brief Gets file name without path and last extension
         *        (e.g., c:/file.ext1.ext2 => file.ext1; /a/b/c.Ext => c).
         * @param path The full path of a file.
         * @return The string of the file name without path and last extension
         */
        std::string base_name(const std::string& path) ;

        /**
         * @brief Gets file path without last extension
         *        (e.g., /a/b/c.Ext => /a/b/c; file.ext1.ext2 => file.ext1).
         * @param path The full path of a file.
         * @return The string of the file path without last extension.
         */
        std::string name_less_extension(const std::string& path);

        /**
         * @brief Gets file path without all extensions
         *        (e.g., /a/b/c.Ext => /a/b/c; file.ext1.ext2 => file).
         * @param path The full path of a file.
         * @return The string of the file path without all extensions.
         */
        std::string name_less_all_extensions(const std::string& path);

        /**
        * @brief Replaces the extension of the given file with 'ext'. If the file name does not
        *        have an extension, the expected new extension is appended.
        * @param path The file name.
        * @param ext The expected new extension.
        * @return The string of the new file name/
        */
        std::string replace_extension(std::string const& path, const std::string& ext);

        /**
        * @brief Gets root part of a path ("/" or "C:"), or an empty string if none found.
        * @param path The path.
        * @return The string representing the root part of the path.
        */
        std::string path_root(const std::string& path);

        /**
        * @brief Tests if path is absolute, as !get_path_root(path).empty(). .
        * @param path The path.
        * @return true if the path is absolute.
        */
        /** Tests if path is absolute, as !get_path_root(path).empty(). */
        bool    is_absolute_path(const std::string& path);

        /**
         * @brief Compute the relative path from 'from' to 'to'.
         * @param from The 'from' directory.
         * @param to The 'to' directory.
         * @return If 'to' is in a subdirectory of 'from' then this function returns the subpath,
         *         otherwise it just returns the file name.
         * @attention The function does \b not automatically resolve paths as the system does, so
         *            be careful to give canonical paths. However, the function interprets slashes
         *            ('/') and backslashes ('\') as they were equal.
         */
        std::string relative_path(const std::string& from, const std::string& to);

        /**
         * @brief Converts to absolute path (i.e., removes .. and . from a path string).
         * @param path The path string.
         * @return The absolute path.
         */
        std::string absolute_path(const std::string& path);

        /**
         * @brief Converts the path to Windows style, i.e., forward slashes (/) to back slashes (\\).
         * @param path The path string.
         * @return The path in Windows style.
         * @related convert_to_unix_style(), convert_to_native_style().
         */
        std::string convert_to_windows_style(const std::string& path);

        /**
         * @brief Converts the path to Unix style, i.e., back slashes (\\) to forward slashes (/).
         * @param path The path string.
         * @return The path in Unix style.
         * @related convert_to_windows_style(), convert_to_native_style().
         */
        std::string convert_to_unix_style(const std::string& path);

        /**
         * @brief Convert sthe path to contain only the current platform's path separators.
         * @param path The path string.
         * @return The path in native style.
         * @related convert_to_windows_style(), convert_to_unix_style().
         */
        std::string convert_to_native_style(const std::string& path);

        /**
         * @brief Gets the path separator of the current platform.
         * @return The path separator in native style.
         */
        char native_path_separator();

        /**
         * @brief Checks sif the path contains only the current platform's path separators.
         * @return true if the path contains only the current platform's path separators.
         */
        bool is_native_style(const std::string& path);

        /**
         * @brief Makes a copy of an existing file.
         * @param original The file name of the original file.
         * @param copy The file name of the copy
         * @return true on success.
         */
        bool copy_file(const std::string& original, const std::string& copy);

        /**
         * @brief Checks if a file contains string 'str'.
         * @param filename The string of the file name.
         * @param str The string to be checked.
         * @return true if the file contains the string.
         */
        bool file_contains_string(const std::string& filename, const std::string& str);

        /**
         * @brief Reads the contents of a file into a string.
         * @param filename The string of the file name.
         * @param data The destination string.
         */
        void read_file_to_string(const std::string& filename, std::string& str);

        /**
         * @brief Writes the string into a file.
         * @param str The string.
         * @param filename The string of the file name.
         */
        void write_string_to_file(const std::string& str, const std::string& filename);

    }


} // namespace easy3d


#endif	// EASY3D_UTIL_FILE_SYSTEM_H
