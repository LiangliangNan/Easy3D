#ifndef EASY3D_WXWIDGETS_MAIN_WINDOW_H
#define EASY3D_WXWIDGETS_MAIN_WINDOW_H

#include <wx/frame.h>


namespace easy3d {

    class Viewer;

    class MainWindow : public wxFrame {
    public:
        MainWindow(wxFrame *frame, const wxString &title, const wxPoint &pos,
                   const wxSize &size, long style = wxDEFAULT_FRAME_STYLE);

        Viewer *viewer() { return viewer_; }

    private:
        void OnMenuFileOpen(wxCommandEvent &event);

        void OnMenuFileExit(wxCommandEvent &event);

        void OnMenuHelpAbout(wxCommandEvent &event);

    private:
        Viewer *viewer_;

    wxDECLARE_EVENT_TABLE();
    };

}

#endif // #ifndef EASY3D_VIEWER_WXWIDGETS_MAIN_WINDOW_H
