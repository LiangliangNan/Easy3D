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

#ifndef EASY3D_QT_VIEWER_WINDOW_H
#define EASY3D_QT_VIEWER_WINDOW_H

#include <string>
#include <QMainWindow>


namespace Ui {
    class Window;
}

namespace easy3d {

    class Model;
    class Viewer;

    class Window : public QMainWindow {
    Q_OBJECT
    public:
        explicit Window(QWidget *parent = nullptr);
        ~Window() override = default;

        Viewer *viewer() { return viewer_; }

    public slots:

        // file
        bool onOpen();
        bool onSave();
        void onOpenRecentFile();
        void onClearRecentFiles();
        void onCurrentModelChanged();

        // view
        void saveSnapshot();
        void setBackgroundColor();

        // topology
        void reportTopologyStatistics();

        // about
        void onAbout();

    protected:
        void dragEnterEvent(QDragEnterEvent *) override;
        void dropEvent(QDropEvent *) override;
        void closeEvent(QCloseEvent *) override;

    private:

        // open a file (given the file name) and add the model to the viewer
        // for visualization. It will also create the default drawables for
        // visualizing the model.
        easy3d::Model *open(const std::string &file_name);

    private:
        void createActions();
        void createActionsForFileMenu();
        void createActionsForViewMenu();
        void createActionsForTopologyMenu();
        bool okToContinue();
        void readSettings();
        void writeSettings();
        void updateWindowTitle();
        void setCurrentFile(const QString &fileName);
        void updateRecentFileActions();
        QString strippedName(const QString &fullFileName);

    private:
        Viewer *viewer_;

        QStringList recentFiles_;
        QString curDataDirectory_;

        enum {
            MaxRecentFiles = 5
        };
        QAction *actionsRecentFile[MaxRecentFiles],
                *actionSeparator;

    private:
        Ui::Window *ui;

    };

}

#endif // EASY3D_QT_VIEWER_WINDOW_H
