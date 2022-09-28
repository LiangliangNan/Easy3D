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

#include <wx/app.h>
#include "main_window.h"


using namespace easy3d;

class Easy3DApp : public wxApp {
public:
    virtual bool OnInit() wxOVERRIDE {
        if (!wxApp::OnInit())
            return false;
        MainWindow *window = new MainWindow(NULL, "Tutorial_203_Viewer_wxWidgets", wxDefaultPosition, wxSize(800, 600));
        window->Show(true);
        return true;
    }
};



#if 0

#ifdef VIEWER_WITH_CONSOLE_WINDOW
wxIMPLEMENT_APP_CONSOLE(Easy3DApp);
#else
wxIMPLEMENT_APP(Easy3DApp);
#endif

#else

#ifdef VIEWER_WITH_CONSOLE_WINDOW
int main(int argc, char **argv) {
	wxDisableAsserts();
	return wxEntry(argc, argv);
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow) {
	wxDisableAsserts(); 
	return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#endif

wxAppInitializer wxTheAppInitializer([]() -> wxAppConsole * { return new Easy3DApp; });

#endif
