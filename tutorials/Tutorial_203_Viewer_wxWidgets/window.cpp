/********************************************************************
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
 ********************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "window.h"
#include "viewer.h"

#include <easy3d/core/model.h>
#include <easy3d/util/file_system.h>


namespace easy3d {

    wxDEFINE_EVENT(SAVE_SNAPSHOT, wxCommandEvent);

    wxBEGIN_EVENT_TABLE(Window, wxFrame)
                    EVT_MENU(wxID_OPEN, Window::menuFileOpen)
                    EVT_MENU(wxID_SAVE, Window::menuFileSave)
                    EVT_MENU(SAVE_SNAPSHOT, Window::menuFileSnapshot)
                    EVT_MENU(wxID_EXIT, Window::menuFileExit)
                    EVT_MENU(wxID_HELP, Window::menuHelpAbout)
    wxEND_EVENT_TABLE()

    // Window constructor
    Window::Window(wxFrame *parent, const wxString &title, const wxPoint &pos, const wxSize &size, long style)
            : wxFrame(parent, wxID_ANY, title, pos, size, style) {
#ifdef WIN32
        SetIcon(wxICON(sample));
#else
        SetIcon(wxIcon("Resources/sample.xpm"));
#endif

        // Make the "File" menu
        wxMenu *fileMenu = new wxMenu;
        fileMenu->Append(wxID_OPEN, "&Open...\tCTRL-O");
        fileMenu->Append(wxID_SAVE, "&Save...\tCTRL-S");
        fileMenu->AppendSeparator();
        fileMenu->Append(SAVE_SNAPSHOT, "&Snapshot...\tS");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "E&xit\tALT-X");
        // Make the "Help" menu
        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(wxID_HELP, "&About");

        wxMenuBar *menuBar = new wxMenuBar;
        menuBar->Append(fileMenu, "&File");
        menuBar->Append(helpMenu, "&Help");
        SetMenuBar(menuBar);
        Show(true);

        wxGLAttributes glAttrib;
        glAttrib.PlatformDefaults().RGBA().DoubleBuffer().Depth(24).Stencil(8).SampleBuffers(1).Samplers(4).EndList();
        viewer_ = new Viewer(this, glAttrib, wxID_ANY, wxDefaultPosition, GetClientSize(), wxDEFAULT_FRAME_STYLE, title);
    }

    // File|Open... command
    void Window::menuFileOpen(wxCommandEvent & WXUNUSED(event)) {
        const std::string &title = "Choose a file";
        wxString filename = wxFileSelector(title, "", "", "",
                                           "Surface Mesh (*.ply;*.obj;*.off;*.stl;*.sm;*.geojson;*.trilist)|"
                                           "*.ply;*.obj;*.off;*.stl;*.sm;*.geojson;*.trilist|"
                                           "Point Cloud (*.ply;*.bin;*.ptx;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg;*.ptx)|"
                                           "*.ply;*.bin;*.ptx;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg;*.ptx|"
                                           "Polyhedral Mesh (*.plm;*.pm;*.mesh)|"
                                           "*.plm;*.pm;*.mesh|"
                                           "Graph (*.ply)|*.ply",
                                           wxFD_OPEN);

        auto model = viewer_->add_model(filename.ToStdString(), true);
        if (model)
            viewer_->fit_screen(model);
    }

    // File|Save... command
    void Window::menuFileSave(wxCommandEvent & WXUNUSED(event)) {
        const Model *m = viewer_->current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return;
        }

        std::string name = m->name();
        if (file_system::extension(name).empty()) // no extension?
            name += ".ply"; // default to ply

        const std::string default_path = file_system::parent_directory(name);
        const std::string default_name = file_system::base_name(name);
        const std::string default_ext = file_system::extension(name);

        const std::string &title = "Please specify a file name";
        wxString filename = wxFileSelector(title, default_path, default_name, default_ext,
                                           "Surface Mesh (*.ply;*.obj;*.off;*.stl;*.sm)|"
                                           "*.ply;*.obj;*.off;*.stl;*.sm|"
                                           "Point Cloud (*.ply;*.bin;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg)|"
                                           "*.ply;*.bin;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg|"
                                           "Polyhedral Mesh (*.plm;*.pm;*.mesh)|"
                                           "*.plm;*.pm;*.mesh|"
                                           "Graph (*.ply)|*.ply",
                                           wxFD_SAVE);
        viewer_->save_current_model(filename.ToStdString());
    }

    // File|Snapshot... command
    void Window::menuFileSnapshot(wxCommandEvent &event) {
        const std::string &title = "Please specify an image file name";
        std::string name("untitled.png");

        if (viewer_->current_model())
            name = file_system::replace_extension(viewer_->current_model()->name(), "png");

        const std::string default_path = file_system::parent_directory(name);
        const std::string default_name = file_system::base_name(name);
        const std::string default_ext = file_system::extension(name);

        wxString filename = wxFileSelector(title, default_path, default_name, default_ext,
                                           "Image Files (*.png;*.jpg;*.bmp;*.ppm;*.tga)|"
                                           "*.png;*.jpg;*.bmp;*.ppm;*.tga",
                                           wxFD_SAVE);

        viewer_->snapshot(filename.ToStdString(), true);
    }

    // File|Exit command
    void Window::menuFileExit(wxCommandEvent & WXUNUSED(event)) {
        // true is to force the frame to close
        Close(true);
    }

    // Help|About command
    void Window::menuHelpAbout(wxCommandEvent & WXUNUSED(event)) {
        wxMessageBox("Easy3D viewer based on wxWidgets");
    }

}