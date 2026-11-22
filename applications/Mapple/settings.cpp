/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 \endcond */

#include "settings.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>

#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>

using namespace easy3d;

namespace {

    bool parse_bool(const std::string &value, bool &out) {
        if (value == "1" || value == "true") {
            out = true;
            return true;
        }
        if (value == "0" || value == "false") {
            out = false;
            return true;
        }
        return false;
    }

    template<typename T>
    bool parse_number(const std::string &value, T &out) {
        std::istringstream input(value);
        T parsed;
        if (!(input >> parsed))
            return false;
        out = parsed;
        return true;
    }

    std::string config_root() {
#if defined(_WIN32)
        return file_system::home_directory(); // %APPDATA%
#elif defined(__APPLE__)
        return file_system::home_directory() + "/Library/Application Support";
#else
        const char *xdg = std::getenv("XDG_CONFIG_HOME");
        return (xdg && *xdg) ? std::string(xdg) : file_system::home_directory() + "/.config";
#endif
    }

    std::string config_directory() {
        return config_root() + "/Mapple";
    }
} // namespace

std::string AppSettings::file_path() {
    return config_directory() + "/settings.ini";
}

std::string AppSettings::rendering_file_path() {
    return config_directory() + "/rendering.json";
}

bool AppSettings::load() {
    // Rendering parameters live in easy3d::setting and are independent of the INI
    // file, so they are loaded even when the INI file does not exist yet.
    const std::string rendering_path = rendering_file_path();
    std::ifstream rendering(rendering_path);
    if (rendering.is_open()) {
        try {
            if (setting::load(rendering))
                LOG(INFO) << "rendering settings loaded from " << rendering_path;
        } catch (const std::exception &e) {
            LOG(WARNING) << "could not read the rendering settings from " << rendering_path
                         << ": " << e.what();
        }
    }

    const std::string path = file_path();
    std::ifstream input(path);
    if (!input.is_open()) {
        LOG(INFO) << "no settings file yet, using defaults (" << path << ")";
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line.front() == '#')
            continue;
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos)
            continue;
        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);

        if (key == "viewer_width") {
            int v;
            if (parse_number(value, v)) viewer_width = v;
        } else if (key == "viewer_height") {
            int v;
            if (parse_number(value, v)) viewer_height = v;
        } else if (key == "viewer_pos_x") {
            int v;
            if (parse_number(value, v)) viewer_pos_x = v;
        } else if (key == "viewer_pos_y") {
            int v;
            if (parse_number(value, v)) viewer_pos_y = v;
        } else if (key == "show_rendering_panel") {
            bool v;
            if (parse_bool(value, v)) show_rendering_panel = v;
        } else if (key == "show_models_panel") {
            bool v;
            if (parse_bool(value, v)) show_models_panel = v;
        } else if (key == "show_log_panel") {
            bool v;
            if (parse_bool(value, v)) show_log_panel = v;
        } else if (key == "left_width")
            parse_number(value, left_width);
        else if (key == "right_width")
            parse_number(value, right_width);
        else if (key == "log_height")
            parse_number(value, log_height);
        else if (key == "pick_readout") {
            bool v;
            if (parse_bool(value, v)) pick_readout = v;
        } else if (key == "log_auto_scroll") {
            bool v;
            if (parse_bool(value, v)) log_auto_scroll = v;
        } else if (key == "models_auto_focus") {
            bool v;
            if (parse_bool(value, v)) models_auto_focus = v;
        } else if (key == "models_selected_only") {
            bool v;
            if (parse_bool(value, v)) models_selected_only = v;
        } else if (key == "video_bitrate_mbps")
            parse_number(value, video_bitrate_mbps);
        else if (key == "video_samples")
            parse_number(value, video_samples);
        else if (key == "data_directory")
            data_directory = value;
        else if (key.rfind("recent.", 0) == 0)
            recent_files.push_back(value);
    }

    // Files can be moved or deleted between runs; never put a dead entry on the
    // menu. save() writes newest first, so the encounter order is the menu order.
    std::vector<std::string> existing;
    for (const auto &file : recent_files) {
        if (file_system::is_file(file) && std::find(existing.begin(), existing.end(), file) == existing.end())
            existing.push_back(file);
        if (existing.size() == max_recent_files)
            break;
    }
    recent_files = existing;
    if (!data_directory.empty() && !file_system::is_directory(data_directory))
        data_directory.clear();

    LOG(INFO) << "settings loaded from " << path;
    return true;
}

bool AppSettings::save() const {
    const std::string path = file_path();
    const std::string directory = file_system::parent_directory(path);
    if (!file_system::is_directory(directory) && !file_system::create_directory(directory)) {
        LOG(WARNING) << "could not create the settings directory: " << directory;
        return false;
    }

    std::ofstream output(path);
    if (!output.is_open()) {
        LOG(WARNING) << "could not write the settings file: " << path;
        return false;
    }

    output << "# Mapple settings; rewritten when the app exits\n";
    output << "viewer_width=" << viewer_width << "\n";
    output << "viewer_height=" << viewer_height << "\n";
    output << "viewer_pos_x=" << viewer_pos_x << "\n";
    output << "viewer_pos_y=" << viewer_pos_y << "\n";
    output << "show_rendering_panel=" << (show_rendering_panel ? 1 : 0) << "\n";
    output << "show_models_panel=" << (show_models_panel ? 1 : 0) << "\n";
    output << "show_log_panel=" << (show_log_panel ? 1 : 0) << "\n";
    output << "left_width=" << left_width << "\n";
    output << "right_width=" << right_width << "\n";
    output << "log_height=" << log_height << "\n";
    output << "pick_readout=" << (pick_readout ? 1 : 0) << "\n";
    output << "log_auto_scroll=" << (log_auto_scroll ? 1 : 0) << "\n";
    output << "models_auto_focus=" << (models_auto_focus ? 1 : 0) << "\n";
    output << "models_selected_only=" << (models_selected_only ? 1 : 0) << "\n";
    output << "video_bitrate_mbps=" << video_bitrate_mbps << "\n";
    output << "video_samples=" << video_samples << "\n";
    output << "data_directory=" << data_directory << "\n";
    for (std::size_t i = 0; i < recent_files.size() && i < max_recent_files; ++i)
        output << "recent." << i << "=" << recent_files[i] << "\n";

    // The rendering parameters go through easy3d::setting's own JSON writer, so every
    // setting the library knows round-trips without the app keeping a copy.
    const std::string rendering_path = rendering_file_path();
    std::ofstream rendering(rendering_path);
    if (!rendering.is_open())
        LOG(WARNING) << "could not write the rendering settings file: " << rendering_path;
    else if (setting::save(rendering))
        LOG(INFO) << "rendering settings saved to " << rendering_path;
    else
        LOG(WARNING) << "could not write the rendering settings file: " << rendering_path;

    LOG(INFO) << "settings saved to " << path;
    return output.good();
}

void AppSettings::remember_file(const std::string &file) {
    if (file.empty())
        return;
    recent_files.erase(std::remove(recent_files.begin(), recent_files.end(), file), recent_files.end());
    recent_files.insert(recent_files.begin(), file);
    if (recent_files.size() > max_recent_files)
        recent_files.resize(max_recent_files);

    const std::string directory = file_system::parent_directory(file);
    if (file_system::is_directory(directory))
        data_directory = directory;
}

void AppSettings::forget_file(const std::string &file) {
    recent_files.erase(std::remove(recent_files.begin(), recent_files.end(), file), recent_files.end());
}
