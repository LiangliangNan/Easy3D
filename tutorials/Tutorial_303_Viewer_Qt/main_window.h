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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <string>

#include <QMainWindow>


namespace Ui {
    class MainWindow;
}

namespace easy3d {
    class Model;
}

class ViewerQt;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    ViewerQt* viewer() { return viewer_; }

private slots:

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
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *) override;
    void closeEvent(QCloseEvent *event) override;
private:
    // Open a file with file name given
    // NOTE: model will not be added to the viewer
    easy3d::Model* open(const std::string& file_name);

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
    ViewerQt*   viewer_;

    QStringList recentFiles_;
    QString		curDataDirectory_;

    enum { MaxRecentFiles = 5 };
    QAction *actionsRecentFile[MaxRecentFiles],
        *actionSeparator;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
