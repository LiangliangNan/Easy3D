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
#include <QFileOpenEvent>

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
    Mapple(int &argc, char **argv) : QApplication(argc, argv) { }

    virtual ~Mapple() { }

    bool notify(QObject * receiver, QEvent * event) override {
        try {
            return QApplication::notify(receiver, event);
        } catch(QException& e) {
            LOG(ERROR) << "an exception was thrown: " << e.what();
        }
        catch(...) {
            LOG(ERROR) << "an unknown exception was thrown";
        }
        return false;
    }

    bool event(QEvent *e) override {
        switch (e->type()) {
            case QEvent::FileOpen:
                filesToOpen.push_back(static_cast<QFileOpenEvent *>(e)->file());
                return true;
            default:
                return QApplication::event(e);
        }
    }

    QStringList filesToOpen;
};



int main(int argc, char *argv[])
{
    // initialize logging at the very beginning to make sure everything will be logged into the log file.
    logging::initialize(true, "default", 9);

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
        if (!logging::log_file().empty())
            LOG(INFO) << "use log file: " << logging::log_file();

        win.show();

#ifdef NDEBUG
        splash.finish(&win);
#endif

        // process the application QEvent::FileOpen events
        QApplication::processEvents();
        QStringList fileNames = app.filesToOpen;
        // open files requested by the arguments
        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                QString name(argv[i]);
                QFileInfo info(name);
                if (info.isFile())
                    fileNames.push_back(name);
            }
        }

        if (!fileNames.empty()) {
            win.openFiles(fileNames);
            QCoreApplication::processEvents();
        }

        return app.exec();
    }
    catch (const std::exception &e) {
        LOG(ERROR) << "Oh sorry, Mapple crashed.\nError message: " << e.what() << ".\n"
                   << "Please contact Liangliang (liangliang.nan@gmail.com) for more information.";
    }
    catch (...) {
        LOG(ERROR) << "Oh sorry, Mapple crashed.\n"
                   << "Please contact Liangliang (liangliang.nan@gmail.com) for more information.";
    }

    return EXIT_FAILURE;
}
