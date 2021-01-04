/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include "main_window.h"

#include <string>
#include <iostream>
#include <stdexcept>

#include <QDir>
#include <QTime>
#include <QApplication>
#include <QSplashScreen>
#ifdef _WIN32
#include <QStyleFactory>
#endif
#include <QSurfaceFormat>
#include <QElapsedTimer>
#include <QException>

#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


// reimplemented from QApplication so we can throw exceptions in slots/event handler
class Mapple : public QApplication
{
public:
    Mapple(int& argc, char ** argv) : QApplication(argc, argv) { }
    virtual ~Mapple() { }
    virtual bool notify(QObject * receiver, QEvent * event) {
        try {
            return QApplication::notify(receiver, event);
        } catch(QException& e) {
            LOG(ERROR) << "an exception was thrown: " << e.what();
        }
        return false;
    }
};


int main(int argc, char *argv[])
{
    //Locale management
    {
        //Force 'English' locale so as to get a consistent behavior everywhere
        QLocale locale = QLocale(QLocale::English);
        locale.setNumberOptions(QLocale::c().numberOptions());
        QLocale::setDefault(locale);

#ifdef Q_OS_UNIX
        //We reset the numeric locale for POSIX functions
        //See http://qt-project.org/doc/qt-5/qcoreapplication.html#locale-settings
        setlocale(LC_NUMERIC, "C");
#endif
    }

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
#ifndef NDEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    QSurfaceFormat::setDefaultFormat(format);

    // Commented to let Qt choose the most suitable OpenGL implementation
	//QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    //QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

#ifdef __APPLE__
    //QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

    Mapple app(argc, argv);
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

    logging::initialize(true, "", google::GLOG_INFO);

#ifdef NDEBUG
    // splash screen
    const std::string file = resource::directory() + "/images/overview.jpg";
    QPixmap pixmap(QString::fromStdString(file));
    QSplashScreen splash(pixmap.scaledToWidth(600), Qt::WindowStaysOnTopHint);
    QElapsedTimer splashTimer;
    splashTimer.start();
    splash.show();
    splash.showMessage("  Starting Mapple...");
    QApplication::processEvents();

    //we want the splash screen to be visible for a minimum amount of time
    while (splashTimer.elapsed() < 500) {   // in milliseconds
        splash.raise();
        QApplication::processEvents(); //to let the system breath!
    }
#endif

    try {
        MainWindow win;
        win.show();

#ifdef NDEBUG
        splash.finish(&win);
        QApplication::processEvents();
#endif

        return app.exec();
    }
    catch (const std::exception& e) {
        const std::string msg =
                std::string("Oh sorry, Mapple crashed.\n") +
                "Error message: " + e.what() + ".\n"
                "Please contact me (liangliang.nan@gmail.com) for more information.";
        LOG(ERROR) << msg;
    }

    return EXIT_FAILURE;
}
