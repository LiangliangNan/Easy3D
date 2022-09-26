#include "application.h"
#include "main_window.h"

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

using namespace easy3d;


// `Main program' equivalent, creating windows and returning main app frame
bool Easy3DApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    // Create the main frame window
    MainWindow *window = new MainWindow(NULL, "Tutorial_203_Viewer_wxWidgets", wxDefaultPosition, wxSize(800, 600));

    /* Show the frame */
    window->Show(true);

    return true;
}

wxIMPLEMENT_APP(Easy3DApp);