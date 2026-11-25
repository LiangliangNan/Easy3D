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

#ifndef EASY3D_MAPPLE_SETTINGS_H
#define EASY3D_MAPPLE_SETTINGS_H

#include <cstddef>
#include <string>
#include <vector>


/**
 * \brief State Mapple carries across runs: viewer size and position, panel layout, view toggles,
 *      recording defaults, and the file history (last directory, recent files).
 * \details load() runs at start-up and save() at exit. The file is a plain
 *      key=value text file under the per-user configuration directory, so a
 *      failed write never affects the session. Rendering parameters are not
 *      stored here: they live in easy3d::setting and are read and written as
 *      JSON through setting::load()/save() (see AppSettings::rendering_file_path()).
 */
struct AppSettings {
    // Viewer size and position.
    int viewer_width = 800;
    int viewer_height = 600;
    int viewer_pos_x = 100;
    int viewer_pos_y = 100;
    // Show or hide side and log panels.
    bool show_rendering_panel = true;
    bool show_models_panel = true;
    bool show_log_panel = false;
    // Side-panel widths and log-panel height, in logical window pixels.
    float left_width = 250.0f;
    float right_width = 300.0f;
    float log_height = 170.0f;

    bool pick_readout = true;
    bool log_auto_scroll = true;
    // Models panel view toggles: fit the camera when the active model changes, and
    // show only the active model.
    bool models_auto_focus = false;
    bool models_selected_only = false;
    // Camera Path: the rule that decides the up direction of generated Walk and Flight
    // paths (an mapple::CameraPathBuilder::GroundUpAxis value, 0 = Auto), so a scene that
    // needed another rule does not silently revert to Auto next session.
    int ground_up_axis = 0;
    // Camera Path: whether generated Walk and Flight tracks keep their height above the
    // ground under them (true) or keep the whole track at the first point's altitude
    // (false). Kept with the up rule, for the same reason.
    bool follow_ground_slopes = true;
    // Camera Path recording defaults, both kept across runs. The window offers only
    // counts this viewer can render, and a stored count it cannot render is resolved
    // when the recording starts.
    int video_bitrate_mbps = 8;
    int video_samples = 4;

    // Directory the File dialogs start in; empty until a file is opened or saved.
    std::string data_directory;
    // Newest first, at most max_recent_files, only files that still exist.
    std::vector<std::string> recent_files;

    static constexpr std::size_t max_recent_files = 5;

    /// Per-user settings file, e.g. ~/Library/Application Support/Mapple/settings.ini.
    /// The previous app's directory (Mapple-imgui) is copied over once on first run.
    static std::string file_path();

    /// Per-user rendering settings file (easy3d::setting JSON), next to file_path().
    static std::string rendering_file_path();

    /// Reads the settings file and the rendering settings. Returns false (keeping the
    /// defaults) when the settings file does not exist; missing keys keep their default
    /// and malformed entries are skipped.
    bool load();

    /// Writes the settings file, creating its directory. Returns false on failure.
    bool save() const;

    /// Makes \p file the newest entry, dropping older duplicates, and updates
    /// data_directory to its parent directory.
    void remember_file(const std::string &file);

    /// Drops \p file from the recent list (e.g. after it failed to load).
    void forget_file(const std::string &file);

    void clear_recent_files() { recent_files.clear(); }
};


#endif // EASY3D_MAPPLE_SETTINGS_H
