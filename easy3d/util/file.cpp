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

#include <easy3d/util/file.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <ctime>	    // for get_time_string()

#ifdef WIN32
#include <Windows.h>
#include <io.h>
#include <direct.h>   // for _mkdir
#include <sys/stat.h> // for _stat64
#include <ShlObj.h>   // for SHGetFolderPathA
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#endif

#include <easy3d/util/string.h>

/*
some code are modified from or inspired by:
"OpenSceneGraph - <osgDB/FileNameUtils>"
"OpenSceneGraph - <osgDB/FileSystem>"
*/


namespace easy3d {

    namespace file {

        const char UNIX_PATH_SEPARATOR = '/';
        const char WINDOWS_PATH_SEPARATOR = '\\';

        static const char * const PATH_SEPARATORS = "/\\";


        //_______________________OS-dependent functions__________________________


        bool is_file(const std::string& filename) {
    #ifdef _WIN32
            struct _stat statbuf;
            if (::_stat(filename.c_str(), &statbuf) < 0)  // use '_wstat()' for Multi-Byte Character Set
                return false;

            if (!(statbuf.st_mode & _S_IFREG))
                return false;
    #else // _WIN32
            struct stat statbuf;
            if (::stat(filename.c_str(), &statbuf) < 0)
                return false;

            if (!S_ISREG(statbuf.st_mode))
                return false;
    #endif // _WIN32

            return true;
        }


        bool is_directory(const std::string& path) {
            if (path == get_path_root(path)) // already the root of the path
                return true;

    #ifdef _WIN32
            struct _stat statbuf;
            if (::_stat(path.c_str(), &statbuf) < 0)  // use '_wstat()' for Multi-Byte Character Set
                return false;

            if (!(statbuf.st_mode & _S_IFDIR))
                return false;
    #else // _WIN32
            struct stat statbuf;
            if (::stat(path.c_str(), &statbuf) < 0)
                return false;

            if (!S_ISDIR(statbuf.st_mode))
                return false;
    #endif // _WIN32

            return true;
        }


        bool create_directory(const std::string& dir) {
            if (is_directory(dir)) {
                std::cout << "directory \'" << dir << "\' already exists" << std::endl;
                return true;
            }

            // make sure the parent directory exists
            const std::string& parent_dir = parent_directory(dir);
            if (!is_directory(parent_dir))
                create_directory(parent_dir);

    #ifdef WIN32
            if (::_mkdir(dir.c_str()) == 0) { // use '_wmkdir()' for Multi-Byte Character Set
    #else
            if (::mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP) == 0) {
    #endif
                return true;
            }

            //std::cerr << "could not create directory: " << dir << std::endl;
            return false;
        }

        bool delete_contents(const std::string& path) {
            if (!is_directory(path))
                return true; // no need to delete

            std::vector<std::string> entries;
            get_directory_entries(path, entries, false);
            for (unsigned int i = 0; i < entries.size(); ++i) {
                const std::string& entry = path + "/" + entries[i];
                if (is_directory(entry)) {
                    if (!delete_directory(entry)) {
                        std::cerr << "could not delete subdirectory \'" << entry << "\'" << std::endl;
                        return false;
                    }
                }
                else {
                    if (!delete_file(entry)) {
                        std::cerr << "could not delete file \'" << entry << "\'" << std::endl;
                        return false;
                    }
                }
            }
            return true;
        }

        bool delete_directory(const std::string& path) {
            if (!is_directory(path))
                return true; // no need to delete

            // delete contents first
            delete_contents(path);

    #ifdef WIN32
            return (::_rmdir(path.c_str()) == 0);
    #else
            return (::rmdir(path.c_str()) == 0);
    #endif
        }

        bool delete_file(const std::string& filename) {
    #ifdef WIN32
            return (::_unlink(filename.c_str()) == 0);  // you can also use "remove()"
    #else
            return (::unlink(filename.c_str()) == 0);  // you can also use "remove()"
    #endif
        }


        std::string get_current_working_directory() {
            char buff[1024];
#ifdef WIN32
            return std::string(::_getcwd(buff, 4096));
#else
            return std::string(::getcwd(buff, 4096));
#endif
        }


        bool set_current_working_directory(const std::string& path) {
    #ifdef WIN32
            return (::_chdir(path.c_str()) == 0); // // use '_wchdir()' for Multi-Byte Character Set
    #else
            return (::chdir(path.c_str()) == 0); // // use '_wchdir()' for Multi-Byte Character Set
    #endif
        }

        std::string get_home_directory() {
            char home_path[2048] = { 0 };

            if (*home_path != 0)
                return home_path;

            // TODO: Use HOME environment variable?

    #ifdef _WIN32
            // SHGetFolderPathA seems to expect non-wide chars
            // http://msdn.microsoft.com/en-us/library/bb762181(VS.85).aspx
            // FIXME: Max length of home path?
            if (!SUCCEEDED(::SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, home_path)))
                std::cerr << "Cannot determine home directory" << std::endl;
    #else // _WIN32
            uid_t user_id = ::geteuid();
            struct passwd* user_info = ::getpwuid(user_id);
            if (user_info == nullptr || user_info->pw_dir == nullptr)
                std::cerr << "Cannot determine home directory" << std::endl;
            std::strncpy(home_path, user_info->pw_dir, PATH_MAX);
    #endif // _WIN32

            return home_path;
        }

        bool rename_file(const std::string& old_name, const std::string& new_name) {
            if (is_file(new_name)) {
                return false;
            }
            return (::rename(old_name.c_str(), new_name.c_str()) == 0);
        }


        time_t get_time_stamp(const std::string& file_or_dir) {
            struct stat buffer;
            if (!stat(file_or_dir.c_str(), &buffer))
                return (buffer.st_mtime);
            return 0;
        }

        std::string get_time_string(const std::string& file_or_dir) {
            time_t stamp = get_time_stamp(file_or_dir);
            if (stamp != 0) {
                struct tm* timeinfo = localtime(&stamp);
                std::string tstr = asctime(timeinfo);
                //return tstr;
                return tstr.substr(0, tstr.length() - 1); // discard the terminating null-character
            }
            else
                return "Unknown. Error occurred.";
        }

        std::string parent_directory(const std::string& path) {
            return dir_name(path); // treat it as a file name;
        }


        void get_directory_entries(const std::string& dir, std::vector<std::string>& contents) {
            if (!is_directory(dir)) {
                std::cerr << "directory \'" << dir << " \' does not exist" << std::endl;
            }

    #if defined(WIN32) && !defined(__CYGWIN__)

            std::string path = dir + "/*.*";
            _finddata_t data;
            intptr_t handle = _findfirst(path.c_str(), &data);
            if (handle != -1) {
                do {
                    std::string name = data.name;
                    if (name != "." && name != "..") // "." and ".." seems always there
                        contents.push_back(name);
                } while (_findnext(handle, &data) != -1);

                _findclose(handle);
            }
    #else
            DIR *handle = opendir(dir.c_str());
            if (handle)
            {
                dirent *rc;
                while ((rc = readdir(handle)) != nullptr)
                {
                    contents.push_back(rc->d_name);
                }
                closedir(handle);
            }
    #endif
        }


        //_______________________OS-independent functions__________________________


        std::string extension(const std::string& file_name, bool lowercase) {
            std::string::size_type dot = file_name.find_last_of('.');
            std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
            if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
                return std::string("");

            std::string ext(file_name.begin() + dot + 1, file_name.end());
            if (lowercase)
                return string::to_lowercase(ext);
            else
                return ext;
        }

        std::string base_name(const std::string& file_path) {
            std::string simpleName = simple_name(file_path);
            return name_less_extension(simpleName);
        }

        std::string dir_name(const std::string& file_name) {
            std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
            if (slash == std::string::npos)
                return std::string();
            else
                return std::string(file_name, 0, slash);
        }

        std::string simple_name(const std::string& file_name) {
            std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
            if (slash == std::string::npos)
                return file_name;
            else
                return std::string(file_name.begin() + slash + 1, file_name.end());
        }


        // strip one level of extension from the filename.
        std::string name_less_extension(const std::string& file_name)
        {
            std::string::size_type dot = file_name.find_last_of('.');
            std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);        // Finds forward slash *or* back slash
            if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
                return file_name;

            return std::string(file_name.begin(), file_name.begin() + dot);
        }


        // strip all extensions from the filename.
        std::string name_less_all_extensions(const std::string& file_name) {
            // Finds start serach position: from last slash, or the begining of the string if none found
            std::string::size_type startPos = file_name.find_last_of(PATH_SEPARATORS);  // Finds forward slash *or* back slash
            if (startPos == std::string::npos)
                startPos = 0;
            std::string::size_type dot = file_name.find_first_of('.', startPos);        // Finds *FIRST* dot from start pos
            if (dot == std::string::npos)
                return file_name;

            return std::string(file_name.begin(), file_name.begin() + dot);
        }

        std::string replace_extension(std::string const& file_name, std::string const& ext)
        {
            std::size_t slashpos = file_name.find_last_of(PATH_SEPARATORS);
            if (slashpos == std::string::npos)
                slashpos = 0;

            std::size_t dotpos = file_name.find_last_of('.');
            if (dotpos == std::string::npos || dotpos < slashpos)
                return file_name + "." + ext;

            return file_name.substr(0, dotpos) + "." + ext;
        }

        std::string get_path_root(const std::string& path) {
            // Test for unix root
            if (path.empty())
                return "";
            if (path[0] == '/')
                return "/";
            // Now test for Windows root
            if (path.length() < 2)
                return "";
            if (path[1] == ':') {
                // We should check that path[0] is a letter, but as ':' is invalid in paths in other cases, that's not a problem.
                return path.substr(0, 2);
            }

            return "";
        }

        bool is_absolute_path(const std::string& path) {
            if (path.empty())
                return false;
            // Unix-like OS: /path/to/file
            if (path.front() == '/')
                return true;
            // Windows: \\server\user\file
            if (path.size() > 1 && path[0] == '\\' && path[1] == '\\') {
                return true;
            }
            // Windows: X:\path\to\file
            if (path.size() > 2 && ::isalpha(path[0]) && path[1] == ':' &&
                path[2] == '\\') {
                return true;
            }
            return false;

            //#ifdef _WIN32
            //		// test for Windows
            //		// We should check that path[0] is a letter, but as ':' is invalid in paths in other cases, that's not a problem.
            //		return path.size() >= 2 && path[1] == ':';
            //		//return path.size() >= 2 && std::isalpha(path[0]) && path[1] == ':';
            //#else
            //		// Test for unix
            //		return path.size() >= 1 && path[0] == '/';
            //#endif
        }

        // See Qt's QDir::absoluteFilePath(), QDir::relativeFilePath() ...
        std::string get_relative_path(const std::string& from, const std::string& to) {
            std::cerr << "not implemented yet. Returning 'to' unchanged." << std::endl;
            return simple_name(to);
        }


        std::string get_absolute_path(const std::string& path)
        {
    #if defined(WIN32)  && !defined(__CYGWIN__)
            const int max_path_len = 2048;
            char retbuf[max_path_len];

            if (_fullpath(retbuf, path.c_str(), max_path_len) != 0)
                return retbuf;
            else {
                std::cerr << "invalid path. Returning 'path' unchanged." << std::endl;
                return path;
            }
    #else

            char resolved_path[PATH_MAX];
            char* result = realpath(path.c_str(), resolved_path);

            if (result)
                return std::string(resolved_path);
            else
                return path;

    #endif
        }


        std::string convert_to_windows_style(const std::string& path) {
            std::string new_fileName(path);

            std::string::size_type slash = 0;
            while ((slash = new_fileName.find_first_of(UNIX_PATH_SEPARATOR, slash)) != std::string::npos)
            {
                new_fileName[slash] = WINDOWS_PATH_SEPARATOR;
            }
            return new_fileName;
        }

        std::string convert_to_unix_style(const std::string& path) {
            std::string new_fileName(path);

            std::string::size_type slash = 0;
            while ((slash = new_fileName.find_first_of(WINDOWS_PATH_SEPARATOR, slash)) != std::string::npos)
            {
                new_fileName[slash] = UNIX_PATH_SEPARATOR;
            }

            return new_fileName;
        }

        char get_native_path_separator() {
    #if defined(WIN32) && !defined(__CYGWIN__)
            return WINDOWS_PATH_SEPARATOR;
    #else
            return UNIX_PATH_SEPARATOR;
    #endif
        }

        bool is_native_style(const std::string& path) {
    #if defined(WIN32) && !defined(__CYGWIN__)
            return path.find(UNIX_PATH_SEPARATOR) == std::string::npos; // return true if no unix style slash exist
    #else
            return path.find(WINDOWS_PATH_SEPARATOR) == std::string::npos; // return true if no windows style backslash exist
    #endif
        }


        std::string convert_to_native_style(const std::string& path) {
    #if defined(WIN32) && !defined(__CYGWIN__)
            return convert_to_windows_style(path);
    #else
            return convert_to_unix_style(path);
    #endif
        }


        void get_directory_entries(
            const std::string& dir, std::vector<std::string>& result, bool recursive
        )
        {
            get_directory_entries(dir, result);
            if (recursive) {
                for (unsigned int i = 0; i < result.size(); i++) {
                    std::string path = dir + "/" + result[i];
                    if (is_directory(path)) {
                        std::vector<std::string> entries;
                        get_directory_entries(path, entries);
                        for (unsigned int j = 0; j < entries.size(); ++j)
                            result.push_back(result[i] + "/" + entries[j]);
                    }
                }
            }
        }

        void get_files(const std::string& dir, std::vector<std::string>& result, bool recursive) {
            std::vector<std::string> entries;
            get_directory_entries(dir, entries, recursive);
            for (unsigned int i = 0; i < entries.size(); i++) {
                std::string name = dir + "/" + entries[i];
                if (is_file(name)) {
                    result.push_back(name);
                }
            }
        }

        void get_sub_directories(const std::string& dir, std::vector<std::string>& result, bool recursive) {
            std::vector<std::string> entries;
            get_directory_entries(dir, entries, recursive);
            for (unsigned int i = 0; i < entries.size(); i++) {
                std::string name = dir + "/" + entries[i];
                if (is_directory(name)) {
                    result.push_back(name);
                }
            }
        }

        bool copy_file(const std::string& original, const std::string& copy) {
            std::ifstream in(original.c_str());
            if (in.fail()) {
                std::cerr << "could not open file \'" << original << "\'" << std::endl;
                return false;
            }
            std::ofstream out(copy.c_str());
            if (in.fail()) {
                std::cerr << "could not open file \'" << copy << "\'" << std::endl;
                return false;
            }

            std::string line;
            while (in.good()) {
                getline(in, line);
                out << line << std::endl;
            }

            return true;
        }

        bool file_contains_string(const std::string& file_name, const std::string& x) {
            std::ifstream in(file_name.c_str());
            std::string buff;
            while (in) {
                getline(in, buff);
                if (buff.find(x) != std::string::npos)
                    return true;
            }
            return false;
        }


        void read_file_to_string(const std::string& filename, std::string& data) {
            std::ifstream in(filename.c_str(), std::fstream::binary);
            if (in.fail()) {
                std::cerr << "Could not open file \'" << filename << "\'" << std::endl;
                return;
            }

            in.seekg(0, std::ios::end);
            std::size_t length = in.tellg();
            in.seekg(0, std::ios::beg);
            data.resize(length);
            in.read(&(data[0]), length);
            in.close();
        }

        void write_string_to_file(const std::string& data, const std::string& filename) {
			std::ofstream out(filename.c_str(), std::ios::binary);
			if (out.fail()) {
				std::cerr << "Could not open file \'" << filename << "\'" << std::endl;
				return;
			}
			out.write(data.c_str(), data.length());
			out.close();
        }

    }


} // namespace easy3

