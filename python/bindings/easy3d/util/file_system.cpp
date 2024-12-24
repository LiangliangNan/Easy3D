#include <easy3d/util/file_system.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_util_file_system(pybind11::module_& m)
{
	// easy3d::file_system::is_file(const std::string &) file:easy3d/util/file_system.h line:53
	m.def("is_file", (bool (*)(const std::string &)) &easy3d::file_system::is_file, "Tests if 'path' is an existing file.\n \n\n The full path of a file (including file extension).\n \n\n ture If the file 'path' exists.\n\nC++: easy3d::file_system::is_file(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::is_directory(const std::string &) file:easy3d/util/file_system.h line:60
	m.def("is_directory", (bool (*)(const std::string &)) &easy3d::file_system::is_directory, "Tests if 'path' is an existing directory.\n \n\n The full path of a directory.\n \n\n ture If the directory 'path' exists.\n\nC++: easy3d::file_system::is_directory(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::create_directory(const std::string &) file:easy3d/util/file_system.h line:67
	m.def("create_directory", (bool (*)(const std::string &)) &easy3d::file_system::create_directory, "Creates a directory entitled 'path'.\n \n\n The full path of the directory to be created.\n \n\n ture If the directory has been successfully created or already exists.\n\nC++: easy3d::file_system::create_directory(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::delete_file(const std::string &) file:easy3d/util/file_system.h line:75
	m.def("delete_file", (bool (*)(const std::string &)) &easy3d::file_system::delete_file, "Deletes the file 'path'.\n \n\n The full path of a file.\n \n\n ture If the file has been successfully deleted or doesn't exist.\n \n\n [Liangliang] Test if path should be absolute.\n\nC++: easy3d::file_system::delete_file(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::delete_directory(const std::string &) file:easy3d/util/file_system.h line:83
	m.def("delete_directory", (bool (*)(const std::string &)) &easy3d::file_system::delete_directory, "Deletes the directory 'path' (and its contents will be deleted recursively).\n \n\n The full path of a directory.\n \n\n ture If the directory has been successfully deleted or doesn't exist.\n \n\n [Liangliang] Test if path should be absolute.\n\nC++: easy3d::file_system::delete_directory(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::delete_contents(const std::string &) file:easy3d/util/file_system.h line:91
	m.def("delete_contents", (bool (*)(const std::string &)) &easy3d::file_system::delete_contents, "Deletes the contents of the directory 'path' (the directory will not be deleted).\n \n\n The full path of a directory.\n \n\n ture If the contents of directory have been successfully deleted or don't exist.\n \n\n [Liangliang] Test if path should be absolute.\n\nC++: easy3d::file_system::delete_contents(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::get_directory_entries(const std::string &, class std::vector<std::string> &, bool) file:easy3d/util/file_system.h line:101
	m.def("get_directory_entries", (void (*)(const std::string &, class std::vector<std::string> &, bool)) &easy3d::file_system::get_directory_entries, "Queries the entries of a directory (including subdirectories and files).\n \n\n The full path of a directory.\n \n\n Return the entries of the directory. Result strings are simple names (i.e.,\n        without the  part, e.g., cat.png).\n \n\n Performs the query recursively if true.\n \n\n get_files(), get_sub_directories().\n\nC++: easy3d::file_system::get_directory_entries(const std::string &, class std::vector<std::string> &, bool) --> void", pybind11::arg("path"), pybind11::arg("entries"), pybind11::arg("recursive"));

	// easy3d::file_system::get_files(const std::string &, class std::vector<std::string> &, bool) file:easy3d/util/file_system.h line:111
	m.def("get_files", (void (*)(const std::string &, class std::vector<std::string> &, bool)) &easy3d::file_system::get_files, "Queries file entries of a directory 'path'.\n \n\n The full path of a directory.\n \n\n Return the file entries of 'path'. Result strings are simple names (i.e.,\n        without the 'path' part, e.g., cat.png).\n \n\n Perform the query recursively if true.\n \n\n get_directory_entries(), get_sub_directories().\n\nC++: easy3d::file_system::get_files(const std::string &, class std::vector<std::string> &, bool) --> void", pybind11::arg("path"), pybind11::arg("files"), pybind11::arg("recursive"));

	// easy3d::file_system::get_sub_directories(const std::string &, class std::vector<std::string> &, bool) file:easy3d/util/file_system.h line:121
	m.def("get_sub_directories", (void (*)(const std::string &, class std::vector<std::string> &, bool)) &easy3d::file_system::get_sub_directories, "Query subdirectory entries of a directory 'path'.\n \n\n The full path of a directory.\n \n\n Return the subdirectory entries of 'path'. Result strings are only the\n        names of the subdirectories (i.e., without the 'path' part).\n \n\n Perform the query recursively if true.\n \n\n get_directory_entries(), get_files().\n\nC++: easy3d::file_system::get_sub_directories(const std::string &, class std::vector<std::string> &, bool) --> void", pybind11::arg("path"), pybind11::arg("subs"), pybind11::arg("recursive"));

	// easy3d::file_system::current_working_directory() file:easy3d/util/file_system.h line:127
	m.def("current_working_directory", (std::string (*)()) &easy3d::file_system::current_working_directory, "Query the current working directory.\n \n\n The string representing the current working directory.\n\nC++: easy3d::file_system::current_working_directory() --> std::string");

	// easy3d::file_system::set_current_working_directory(const std::string &) file:easy3d/util/file_system.h line:134
	m.def("set_current_working_directory", (bool (*)(const std::string &)) &easy3d::file_system::set_current_working_directory, "Set the current working directory.\n \n\n The string representing the new current working directory.\n \n\n true on success.\n\nC++: easy3d::file_system::set_current_working_directory(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::home_directory() file:easy3d/util/file_system.h line:140
	m.def("home_directory", (std::string (*)()) &easy3d::file_system::home_directory, "Query the home path for the current user.\n \n\n The string representing the home directory of the current user.\n\nC++: easy3d::file_system::home_directory() --> std::string");

	// easy3d::file_system::executable() file:easy3d/util/file_system.h line:146
	m.def("executable", (std::string (*)()) &easy3d::file_system::executable, "Query the name of *this* executable.\n \n\n The string representing the full path of this executable, e.g., C:/a/b/c.exe\n\nC++: easy3d::file_system::executable() --> std::string");

	// easy3d::file_system::executable_directory() file:easy3d/util/file_system.h line:152
	m.def("executable_directory", (std::string (*)()) &easy3d::file_system::executable_directory, "Query the directory where the executable file is located.\n \n\n The string representing the directory in which the executable resides.\n\nC++: easy3d::file_system::executable_directory() --> std::string");

	// easy3d::file_system::rename_file(const std::string &, const std::string &) file:easy3d/util/file_system.h line:160
	m.def("rename_file", (bool (*)(const std::string &, const std::string &)) &easy3d::file_system::rename_file, "Rename the file from 'old_name' to 'new_name'.\n \n\n The full path of the file to be renamed.\n \n\n The full path of the file's new name.\n \n\n true on success.\n\nC++: easy3d::file_system::rename_file(const std::string &, const std::string &) --> bool", pybind11::arg("old_name"), pybind11::arg("new_name"));

	// easy3d::file_system::time_stamp(const std::string &) file:easy3d/util/file_system.h line:167
	m.def("time_stamp", (long (*)(const std::string &)) &easy3d::file_system::time_stamp, "Query the time stamp of a file or directory.\n \n\n The full path of the file or directory.\n \n\n The time stamp.\n\nC++: easy3d::file_system::time_stamp(const std::string &) --> long", pybind11::arg("path"));

	// easy3d::file_system::time_string(const std::string &) file:easy3d/util/file_system.h line:174
	m.def("time_string", (std::string (*)(const std::string &)) &easy3d::file_system::time_string, "Query the time stamp of a file or directory as a string.\n \n\n The full path of the file or directory.\n \n\n The string of the time stamp.\n\nC++: easy3d::file_system::time_string(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::parent_directory(const std::string &) file:easy3d/util/file_system.h line:189
	m.def("parent_directory", (std::string (*)(const std::string &)) &easy3d::file_system::parent_directory, "Query the parent path from full name of a file or directory\n        (e.g., /a/b/c.Ext => /a/b)\n \n\n The full path of the file or directory.\n \n\n The string of the parent path.\n\nC++: easy3d::file_system::parent_directory(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::extension(const std::string &, bool) file:easy3d/util/file_system.h line:197
	m.def("extension", [](const std::string & a0) -> std::string { return easy3d::file_system::extension(a0); }, "", pybind11::arg("path"));
	m.def("extension", (std::string (*)(const std::string &, bool)) &easy3d::file_system::extension, "Query the file extension without dot (e.g., /a/b/c.Ext => Ext).\n \n\n The full path of a file.\n \n\n Converts the string to lower case if true.\n \n\n The string of the file extension.\n\nC++: easy3d::file_system::extension(const std::string &, bool) --> std::string", pybind11::arg("path"), pybind11::arg("lower"));

	// easy3d::file_system::simple_name(const std::string &) file:easy3d/util/file_system.h line:204
	m.def("simple_name", (std::string (*)(const std::string &)) &easy3d::file_system::simple_name, "Gets file name without path but with extension (e.g, /a/b/c.Ext => c.Ext)\n \n\n The full path of a file.\n \n\n The string of the file name without path but with extension.\n\nC++: easy3d::file_system::simple_name(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::base_name(const std::string &) file:easy3d/util/file_system.h line:212
	m.def("base_name", (std::string (*)(const std::string &)) &easy3d::file_system::base_name, "Gets file name without path and last extension\n        (e.g., c:/file.ext1.ext2 => file.ext1; /a/b/c.Ext => c).\n \n\n The full path of a file.\n \n\n The string of the file name without path and last extension\n\nC++: easy3d::file_system::base_name(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::name_less_extension(const std::string &) file:easy3d/util/file_system.h line:220
	m.def("name_less_extension", (std::string (*)(const std::string &)) &easy3d::file_system::name_less_extension, "Gets file path without last extension\n        (e.g., /a/b/c.Ext => /a/b/c; file.ext1.ext2 => file.ext1).\n \n\n The full path of a file.\n \n\n The string of the file path without last extension.\n\nC++: easy3d::file_system::name_less_extension(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::name_less_all_extensions(const std::string &) file:easy3d/util/file_system.h line:228
	m.def("name_less_all_extensions", (std::string (*)(const std::string &)) &easy3d::file_system::name_less_all_extensions, "Gets file path without all extensions\n        (e.g., /a/b/c.Ext => /a/b/c; file.ext1.ext2 => file).\n \n\n The full path of a file.\n \n\n The string of the file path without all extensions.\n\nC++: easy3d::file_system::name_less_all_extensions(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::replace_extension(const std::string &, const std::string &) file:easy3d/util/file_system.h line:237
	m.def("replace_extension", (std::string (*)(const std::string &, const std::string &)) &easy3d::file_system::replace_extension, "Replaces the extension of the given file with 'ext'. If the file name does not\n        have an extension, the expected new extension is appended.\n \n\n The file name.\n \n\n The expected new extension.\n \n\n The string of the new file name/\n\nC++: easy3d::file_system::replace_extension(const std::string &, const std::string &) --> std::string", pybind11::arg("path"), pybind11::arg("ext"));

	// easy3d::file_system::path_root(const std::string &) file:easy3d/util/file_system.h line:244
	m.def("path_root", (std::string (*)(const std::string &)) &easy3d::file_system::path_root, "Gets root part of a path (\"/\" or \"C:\"), or an empty string if none found.\n \n\n The path.\n \n\n The string representing the root part of the path.\n\nC++: easy3d::file_system::path_root(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::is_absolute_path(const std::string &) file:easy3d/util/file_system.h line:252
	m.def("is_absolute_path", (bool (*)(const std::string &)) &easy3d::file_system::is_absolute_path, "Tests if path is absolute, as !get_path_root(path).empty(). .\n \n\n The path.\n \n\n true if the path is absolute.\n\n Tests if path is absolute, as !get_path_root(path).empty(). \n\nC++: easy3d::file_system::is_absolute_path(const std::string &) --> bool", pybind11::arg("path"));

	// easy3d::file_system::relative_path(const std::string &, const std::string &) file:easy3d/util/file_system.h line:264
	m.def("relative_path", (std::string (*)(const std::string &, const std::string &)) &easy3d::file_system::relative_path, "Compute the relative path from 'from' to 'to'.\n \n\n The 'from' directory.\n \n\n The 'to' directory.\n \n\n If 'to' is in a subdirectory of 'from' then this function returns the subpath,\n         otherwise it just returns the file name.\n \n\n The function does  automatically resolve paths as the system does, so\n            be careful to give canonical paths. However, the function interprets slashes\n            ('/') and backslashes ('\\') as they were equal.\n\nC++: easy3d::file_system::relative_path(const std::string &, const std::string &) --> std::string", pybind11::arg("from"), pybind11::arg("to"));

	// easy3d::file_system::absolute_path(const std::string &) file:easy3d/util/file_system.h line:271
	m.def("absolute_path", (std::string (*)(const std::string &)) &easy3d::file_system::absolute_path, "Converts to absolute path (i.e., removes .. and . from a path string).\n \n\n The path string.\n \n\n The absolute path.\n\nC++: easy3d::file_system::absolute_path(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::convert_to_windows_style(const std::string &) file:easy3d/util/file_system.h line:279
	m.def("convert_to_windows_style", (std::string (*)(const std::string &)) &easy3d::file_system::convert_to_windows_style, "Converts the path to Windows style, i.e., forward slashes (/) to back slashes (\\).\n \n\n The path string.\n \n\n The path in Windows style.\n \n\n convert_to_unix_style(), convert_to_native_style().\n\nC++: easy3d::file_system::convert_to_windows_style(const std::string &) --> std::string", pybind11::arg("path"));

	// easy3d::file_system::convert_to_unix_style(const std::string &) file:easy3d/util/file_system.h line:287
	m.def("convert_to_unix_style", (std::string (*)(const std::string &)) &easy3d::file_system::convert_to_unix_style, "Converts the path to Unix style, i.e., back slashes (\\) to forward slashes (/).\n \n\n The path string.\n \n\n The path in Unix style.\n \n\n convert_to_windows_style(), convert_to_native_style().\n\nC++: easy3d::file_system::convert_to_unix_style(const std::string &) --> std::string", pybind11::arg("path"));

    // easy3d::file_system::convert_to_native_style(const std::string &) file:easy3d/util/file_system.h line:295
    m.def("convert_to_native_style", (std::string (*)(const std::string &)) &easy3d::file_system::convert_to_native_style, "Convert a path string such that it uses the current platform's path separators.\n \n\n The path string.\n \n\n The path in native style.\n \n\n convert_to_windows_style(), convert_to_unix_style().\n\nC++: easy3d::file_system::convert_to_native_style(const std::string &) --> std::string", pybind11::arg("path"));

    // easy3d::file_system::native_path_separator() file:easy3d/util/file_system.h line:301
    m.def("native_path_separator", (char (*)()) &easy3d::file_system::native_path_separator, "Gets the path separator of the current platform.\n \n\n The path separator in native style.\n\nC++: easy3d::file_system::native_path_separator() --> char");

    // easy3d::file_system::is_native_style(const std::string &) file:easy3d/util/file_system.h line:307
    m.def("is_native_style", (bool (*)(const std::string &)) &easy3d::file_system::is_native_style, "Checks sif the path contains only the current platform's path separators.\n \n\n true if the path contains only the current platform's path separators.\n\nC++: easy3d::file_system::is_native_style(const std::string &) --> bool", pybind11::arg("path"));

    // easy3d::file_system::copy_file(const std::string &, const std::string &) file:easy3d/util/file_system.h line:315
    m.def("copy_file", (bool (*)(const std::string &, const std::string &)) &easy3d::file_system::copy_file, "Makes a copy of an existing file.\n \n\n The file name of the original file.\n \n\n The file name of the copy\n \n\n true on success.\n\nC++: easy3d::file_system::copy_file(const std::string &, const std::string &) --> bool", pybind11::arg("original"), pybind11::arg("copy"));

    // easy3d::file_system::file_contains_string(const std::string &, const std::string &) file:easy3d/util/file_system.h line:323
    m.def("file_contains_string", (bool (*)(const std::string &, const std::string &)) &easy3d::file_system::file_contains_string, "Checks if a file contains string 'str'.\n \n\n The string of the file name.\n \n\n The string to be checked.\n \n\n true if the file contains the string.\n\nC++: easy3d::file_system::file_contains_string(const std::string &, const std::string &) --> bool", pybind11::arg("filename"), pybind11::arg("str"));

    // easy3d::file_system::read_file_to_string(const std::string &, std::string &) file:easy3d/util/file_system.h line:330
    m.def("read_file_to_string", (void (*)(const std::string &, std::string &)) &easy3d::file_system::read_file_to_string, "Reads the contents of a file into a string.\n \n\n The string of the file name.\n \n\n The destination string.\n\nC++: easy3d::file_system::read_file_to_string(const std::string &, std::string &) --> void", pybind11::arg("filename"), pybind11::arg("str"));

}
