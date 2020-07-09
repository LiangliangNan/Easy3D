/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/util/dialogs.h>
#include <easy3d/util/file_system.h>
#include <3rd_party/portable_file_dialogs/portable_file_dialogs.h>


namespace easy3d {

    // https://github.com/samhocevar/portable-file-dialogs
    std::vector<std::string> dialog::open(
            const std::string& title,
            const std::string& default_directory,
            const std::vector<std::string>& filters,
            bool multiple)
    {
        // Set verbosity to false
        pfd::settings::verbose(false);

        // if the provided path doesn't exist, let the OS decide.
        const std::string& path = file_system::is_directory(default_directory) ?
                    default_directory : "";
        auto f = pfd::open_file(title, file_system::convert_to_native_style(path), filters, multiple);
        return f.result();
    }


    std::string dialog::open(
            const std::string& title,
            const std::string& default_directory,
            const std::vector<std::string>& filters
            )
    {
        const auto& file_names = open(title, default_directory, filters, false);
        if (!file_names.empty())
            return file_names[0];
        else
            return "";
    }


    std::string dialog::save(
            const std::string& title,
            const std::string& default_file_name,
            const std::vector<std::string>& filters,
            bool warn_overwrite
            )
    {
        // Set verbosity to false
        pfd::settings::verbose(false);
        auto f = pfd::save_file(title, file_system::convert_to_native_style(default_file_name), filters, !warn_overwrite);
        return f.result();
    }


    std::string dialog::open_folder(
            const std::string& title,
            const std::string& default_directory
            )
    {
        // Set verbosity to false
        pfd::settings::verbose(false);

        // if the provided path doesn't exist, let the OS decide.
        const std::string& path = file_system::is_directory(default_directory) ?
                    default_directory : "";
        return pfd::select_folder(title, file_system::convert_to_native_style(path)).result();
    }


    void dialog::notify(
            const std::string& title,
            const std::string& message,
            Type type
            )
    {
        // Set verbosity to false
        pfd::settings::verbose(false);

        // Notification
        pfd::notify(title, message, pfd::icon(type));
    }


    dialog::Response dialog::message(
            const std::string& title,
            const std::string& message,
            Choice choice,
            Type type)
    {
        // Set verbosity to false
        pfd::settings::verbose(false);

        // Show message box
        auto value = pfd::message(title, message, pfd::choice(choice), pfd::icon(type));
        return static_cast<Response>(value.result());
    }


#ifdef HAS_OSDIALOG
    // https://github.com/AndrewBelt/osdialog
    std::string FileDialog::open(const std::string& file_name, const std::string& filters) {
        osdialog_filters* filters_c = nullptr;
        if (!filters.empty())
            filters_c = osdialog_filters_parse(filters.c_str());

        std::string result;
        char *filepath_c = osdialog_file(OSDIALOG_OPEN, nullptr, file_name.c_str(), filters_c);
        if (filepath_c) {
            result = std::string(filepath_c);
            std::free(filepath_c);
            osdialog_filters_free(filters_c);
        }
        return result;
    };
#endif



#ifdef HAS_TINY_FILE_DIALOGS

    std::string FileDialog::open(
            const std::vector<std::string> & filetypes,
            const std::string& default_path /* = ""*/
            )
    {
        auto result = open(filetypes, false, default_path);
        return result.empty() ? "" : result.front();
    }


   std::vector<std::string> FileDialog::open(
            const std::vector<std::string>& filetypes,
            bool multiple,
            const std::string& default_path /* = ""*/
            )
   {
       const char** filters = nullptr;
       if (!filetypes.empty()) {
          filters = new const char*[filetypes.size()];
          for (std::size_t i =0; i<filetypes.size(); ++i)
              filters[i] = filetypes[i].c_str();
       }

       std::string types_string;
       for (std::size_t i=0; i<filetypes.size(); ++i)
           types_string = types_string + filetypes[i] + ((i == filetypes.size() - 1) ? "" : "; ");
       types_string = "Supported formats (" + types_string + ")";


       const char* buffer = tinyfd_openFileDialog(
                   "Please select the file(s) to open",
                   file_system::convert_to_native_style(default_path).c_str(),
                   static_cast<int>(filetypes.size()),
                   filters,
                   types_string.c_str(),
                   multiple
                   );

       std::vector<std::string> files;

       if (buffer) {
           const char* p = buffer;
           bool skip_empty_fields = true;
           const char seperator = '|';
           std::string sequence(p);
           std::string::size_type pos = sequence.find_last_of('\n');
           if (pos != std::string::npos)
               sequence = sequence.substr(0, pos);
           std::size_t length = sequence.length();
           std::size_t start = 0;
           while (start < length) {
               std::size_t end = sequence.find(seperator, start);
               if (end == std::string::npos) {
                   end = length;
               }
               if (!skip_empty_fields || (end - start > 0)) {
                   files.push_back(sequence.substr(start, end - start));
               }
               start = end + 1;
           }
           p += strlen( p );
       }

       if (filters)
           delete [] filters;

       return files;
   }


    std::string FileDialog::save(
            const std::vector<std::string>& filetypes,
            const std::string& default_file_name /* = ""*/
            )
    {
        const char** filters = nullptr;
        if (!filetypes.empty()) {
           filters = new const char*[filetypes.size()];
           for (std::size_t i =0; i<filetypes.size(); ++i)
               filters[i] = filetypes[i].c_str();
        }

        std::string types_string;
        for (std::size_t i=0; i<filetypes.size(); ++i)
            types_string = types_string + filetypes[i] + ((i == filetypes.size() - 1) ? "" : "; ");
        types_string = "Supported formats (" + types_string + ")";

        const char* file = tinyfd_saveFileDialog(
                    "Please specify the file name to save",
                    default_file_name.c_str(),
                    static_cast<int>(filetypes.size()),
                    filters,
                    types_string.c_str()
                    );

        if (filters)
            delete [] filters;

        if (file)
            return std::string(file);
        else
            return "";
    }




    // c is the current color
    vec3 ColorDialog::pick(const vec3& c) {
        unsigned char color[] = {
            static_cast<unsigned char>(c.x * 255), static_cast<unsigned char>(c.y * 255), static_cast<unsigned char>(c.z * 255)
        };
        unsigned char result[3];
        tinyfd_colorChooser("Choose a color", nullptr, color, result);
        return vec3(result[0]/255.0f, result[1]/255.0f, result[2]/255.0f);
    }

#endif

}
