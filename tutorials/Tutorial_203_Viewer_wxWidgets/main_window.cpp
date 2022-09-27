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

#include "main_window.h"
#include "viewer.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


namespace easy3d {

    wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
                    EVT_MENU(wxID_OPEN, MainWindow::OnMenuFileOpen)
                    EVT_MENU(wxID_EXIT, MainWindow::OnMenuFileExit)
                    EVT_MENU(wxID_HELP, MainWindow::OnMenuHelpAbout)
    wxEND_EVENT_TABLE()

    // MainWindow constructor
    MainWindow::MainWindow(wxFrame *frame, const wxString &title, const wxPoint &pos, const wxSize &size, long style)
            : wxFrame(frame, wxID_ANY, title, pos, size, style) {
        SetIcon(wxIcon("Resources/sample.xpm"));

        // Make the "File" menu
        wxMenu *fileMenu = new wxMenu;
        fileMenu->Append(wxID_OPEN, "&Open...\tCTRL-O");
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

        viewer_ = new Viewer(this, wxID_ANY, wxDefaultPosition, GetClientSize(), wxDEFAULT_FRAME_STYLE);
    }

    // File|Open... command
    void MainWindow::OnMenuFileOpen(wxCommandEvent & WXUNUSED(event)) {
        wxString filename = wxFileSelector("Choose a file", "", "", "",
                                           "Surface Mesh (*.ply;*.obj;*.off;*.stl;*.sm;*.geojson;*.trilist)|"
                                           "*.ply;*.obj;*.off;*.stl;*.sm;*.geojson;*.trilist|"
                                           "Point Cloud (*.ply;*.bin;*.ptx;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg;*.ptx)|"
                                           "*.ply;*.bin;*.ptx;*.las;*.laz;*.xyz;*.bxyz;*.vg;*.bvg;*.ptx|"
                                           "Polyhedral Mesh (*.plm;*.pm;*.mesh)|"
                                           "*.plm;*.pm;*.mesh|"
                                           "Graph (*.ply)|*.ply",
                                           wxFD_OPEN);
        if (!filename.IsEmpty()) {
            auto model = viewer_->add_model(filename.ToStdString(), true);
            if (model) {
                viewer_->fit_screen(model);
            }
        }
    }

    // File|Exit command
    void MainWindow::OnMenuFileExit(wxCommandEvent & WXUNUSED(event)) {
        // true is to force the frame to close
        Close(true);
    }

    // Help|About command
    void MainWindow::OnMenuHelpAbout(wxCommandEvent & WXUNUSED(event)) {
        wxMessageBox("Easy3D viewer based on wxWidgets");
    }

}