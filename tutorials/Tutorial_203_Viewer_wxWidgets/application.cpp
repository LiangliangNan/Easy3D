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

#include "application.h"
#include "window.h"


namespace easy3d {

    class AppImpl : public wxApp {
    public:
        AppImpl(const std::string &title, int w, int h) : title_(title), width_(w), height_(h) {}

        bool OnInit() wxOVERRIDE {
            if (!wxApp::OnInit())
                return false;
            new Window(nullptr, title_, wxDefaultPosition, wxSize(width_, height_));
            return true;
        }
    private:
        std::string title_;
        int width_;
        int height_;
    };


    Application::Application(const std::string &title, int width, int height) {
        wxDisableAsserts();
        wxApp::SetInstance(new AppImpl(title, width, height));
    }


    int Application::run(int argc, char **argv) {
        return wxEntry(argc, argv);
    }

}