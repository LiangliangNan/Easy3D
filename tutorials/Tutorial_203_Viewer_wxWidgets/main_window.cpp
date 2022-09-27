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
        fileMenu->Append(wxID_OPEN, "&Open...");
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