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

#ifndef EASY3D_DIALOGS_H
#define EASY3D_DIALOGS_H

#include <string>
#include <vector>


namespace easy3d
{

    namespace dialog {

        /**
         * \brief Open a native file open dialog.
         * \param title The title of the dialog, e.g., "Please choose the file(s) to open".
         * \param default_path The default file path, e.g., resource::directory() + "/data/".
         * \param filters The accepted file filters, e.g.,
         *              const std::vector<std::string> filters = {
         *                  "Mesh Files (*.obj *.ply)", "*.obj *.ply",
         *                  "Point Cloud Files (*.bin *.xyz *.ply)", "*.bin *.xyz *.ply",
         *                  "All Files (*.*)", "*"
         *              };
         * \param multiple Allow choosing multiple files if it is true.
         * @returns The selected file names.
         */
        std::vector<std::string> open(
                const std::string& title,
                const std::string& default_directory,
                const std::vector<std::string>& filters,
                bool multiple
                );

        /**
         * \brief Open a native file open dialog.
         * \param title The title of the dialog, e.g., "Please choose the file(s) to open".
         * \param default_path The default file path, e.g., resource::directory() + "/data/".
         * \param filters The accepted file filters. For each filter, the string you see in the file
         *                dialog and the actual filter always come in pairs, e.g.,
         *                      const std::vector<std::string> filters = {
         *                          "Mesh Files (*.obj *.ply)", "*.obj *.ply",
         *                          "Point Cloud Files (*.bin *.xyz *.ply)", "*.bin *.xyz *.ply",
         *                          "All Files (*.*)", "*"
         *                      };
         * @return The selected file name (empty if no file was selected).
         * @todo   A filter needs only one string (the filter can be extracted from the string).
         */
        std::string open(
                const std::string& title = "Please choose the file(s) to open",
                const std::string& default_directory = "",
                const std::vector<std::string>& filters = { "All Files (*.*)", "*" }
                );

        // -----------------------------------------------------------------------------

        /**
         * \brief Open a native file save dialog.
         * \param title The title of the dialog, e.g., "Please choose a file name".
         * \param default_file_name The suggested file name, e.g., /project/data/bunny.ply"
         * \param filters The accepted file filters, e.g.,
         *              const std::vector<std::string> filters = {
         *                  "Mesh Files (*.obj *.ply)", "*.obj *.ply",
         *                  "Point Cloud Files (*.bin *.xyz *.ply)", "*.bin *.xyz *.ply",
         *                  "All Files (*.*)", "*.*"
         *              };
         * \param warn_overwrite If set true (default value), prompt a confirm dialog if a
		 *						 file with the same name already exists.
         * @return The chosen file name (empty if no file name was chosen).
         */
        std::string save(
                const std::string& title = "Please choose a file name",
                const std::string& default_file_name = "",
                const std::vector<std::string>& filters = { "All Files (*.*)", "*" },
                bool warn_overwrite = true
                );

        // -----------------------------------------------------------------------------

        /**
         * \brief Open a folder dialog.
         * \param title The title of the dialog, e.g., "Please choose a folder".
         * \param default_path The suggested file path, e.g., /project/data/"
         * @return The chosen folder (empty if no folder was chosen).
         */
        std::string open_folder(
                const std::string& title = "Please choose a folder",
                const std::string& default_directory = ""
                );

        // -----------------------------------------------------------------------------

        enum class Type {
             info = 0,
             warning,
             error,
             question
        };

        enum class Response {
            cancel = -1,
            ok,
            yes,
            no,
            abort,
            retry,
            ignore,
        };

        enum class Choice {
            ok = 0,
            ok_cancel,
            yes_no,
            yes_no_cancel,
            retry_cancel,
            abort_retry_ignore,
        };

        /**
         * @brief Notification.
         * @param title The title of the notification dialog.
         * @param message The message string.
         * @param type The notification type. Default is 'info'. See 'Type'.
         */

        void notify(
                const std::string& title,
                const std::string& message,
                Type type = Type::info
                );

        // -----------------------------------------------------------------------------

        /**
         * @brief Message box.
         * @param title The title of the message box.
         * @param message The message string.
         * @param choice The options for the user. Defauly is 'ok_cancel'. See 'Choice'.
         * @param type The message type. Default is 'info'. See 'Type'.
         * @return The user's response. See "Response'.
         */
        Response message(
                const std::string& title,
                const std::string& message,
                Choice choice = Choice::ok_cancel,
                Type type = Type::info
                );


#ifdef HAS_OSDIALOG
        // usage
        //        const std::string filters =
        //                "Source:off,obj,ply;"
        //                "Header:xyz,ply";
        //        const std::string& file_name = FileDialog::open("", filters);
        static std::string open(const std::string& file_name, const std::string& filters) ;
#endif

#ifdef HAS_TINY_FILE_DIALOGS

        /**
         * \brief Open a native file open dialog.
         * \param filetypes The accepted file extensions, e.g., "{"*.jpg","*.png"}"
         * \param default_path The default file path.
         */
        static std::string open(
                const std::vector<std::string> & filetypes,
                const std::string& default_path = ""
                );

        /**
         * \brief Open a native file open dialog, which allows multiple selection.
         * \param filetypes The accepted file extensions, e.g., "{"*.jpg","*.png"}"
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

#endif	// EASY3D_DIALOGS_H

