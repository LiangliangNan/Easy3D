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


#include <easy3d/util/dialogs.h>

#include <cstring>

#include <3rd_party/tinyfiledialogs/tinyfiledialogs.h>


namespace easy3d {


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

       const char* buffer = tinyfd_openFileDialog(
                   "Please select the file(s) to open",
                   default_path.c_str(),
                   static_cast<int>(filetypes.size()),
                   filters,
                   "All supported formats",
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

        const char* file = tinyfd_saveFileDialog(
                    "Please specify the file name to save",
                    default_file_name.c_str(),
                    static_cast<int>(filetypes.size()),
                    filters,
                    "All supported formats"
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

}
