/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <string>
#include <iostream>
#include <stdexcept>

#ifdef HAS_QT5

#include <QDir>
#include <QTime>
#include <QApplication>
#include <QSplashScreen>
#include <QSurfaceFormat>
#include <QStyleFactory>

#include <easy3d/viewer/setting.h>

#include "main_window.h"


int main(int argc, char *argv[])
{
    // Note: Calling QSurfaceFormat::setDefaultFormat() before constructing the
    //       QApplication instance is mandatory on some platforms(for example, macOS)
    //       when an OpenGL core profile context is requested. This is to ensure
    //       that resource sharing between contexts stays functional as all internal
    //       contexts are created using the correct version and profile.
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // Commented to let Qt choose the most suitable OpenGL implementation
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
#ifdef _WIN32   // to have similar style as on macOS.
    app.setStyle(QStyleFactory::create("Fusion"));
#endif

     QDir workingDir = QCoreApplication::applicationDirPath();
#ifdef __APPLE__
    // This makes sure that our "working directory" is not within the application bundle
    if ( workingDir.dirName() == "MacOS" ) {
        workingDir.cdUp();
        workingDir.cdUp();
        workingDir.cdUp();
    }
#endif
    QDir::setCurrent(workingDir.absolutePath());

    int status = 0;
    try {
        MainWindow win;
        win.show();
        status = app.exec();
    }
    catch (const std::exception& e) {
        const std::string msg =
                std::string("Oh sorry, ViewerQt crashed.\n") +
                "Error message: " + e.what() + ".\n"
                "Please contact me (liangliang.nan@gmail.com) for more information.";
        std::cerr << msg << std::endl;
    }

    return status;
}

#else

int main(int argc, char *argv[])
{
    std::cerr << "The ViewerQt example requires Qt5 which is not availabe/found on this machine" << std::endl;
    return false;
}

#endif
