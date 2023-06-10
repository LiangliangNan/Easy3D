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

#if 1

#include <easy3d/util/initializer.h>

#include "application.h"


using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize(true);

    Application app(EXAMPLE_TITLE);
    return app.run(argc, argv);
}

#else // The code below is for backup purpose.
// It just works the same as the above code, but it is less structured and requires to expose the wxWidgets headers to client code.

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/app.h>
#include "window.h"

using namespace easy3d;

class Easy3DApp : public wxApp {
public:
    virtual bool OnInit() wxOVERRIDE {
        if (!wxApp::OnInit())
            return false;
        new Window(NULL, EXAMPLE_TITLE, wxDefaultPosition, wxSize(800, 600));
        return true;
    }
};



#if 1

#ifdef WIN32_VIEWER_WITHOUT_CONSOLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow) {
	wxDisableAsserts();
	wxApp::SetInstance(new Easy3DApp);
	return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#else  // WIN32_VIEWER_WITHOUT_CONSOLE
int main(int argc, char **argv) {
	wxDisableAsserts();
	wxApp::SetInstance(new Easy3DApp);
	return wxEntry(argc, argv);
}
#endif // WIN32_VIEWER_WITHOUT_CONSOLE

#else

#ifdef WIN32_VIEWER_WITHOUT_CONSOLE
wxIMPLEMENT_APP(Easy3DApp);
#else  // WIN32_VIEWER_WITHOUT_CONSOLE
wxIMPLEMENT_APP_CONSOLE(Easy3DApp);
#endif // WIN32_VIEWER_WITHOUT_CONSOLE

#endif

#endif