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



#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <string>

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>

#include <easy3d/util/progress.h>
#include <easy3d/core/types.h>


namespace Ui {
    class MainWindow;
}

namespace easy3d {
    class Model;
}

class PaintCanvas;
class WidgetPointsDrawable;
class WidgetLinesDrawable;
class WidgetTrianglesDrawable;

class MainWindow : public QMainWindow, public easy3d::ProgressClient
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    PaintCanvas* viewer() { return viewer_; }

    void setCurrentFile(const QString &fileName);

    void updateUi(); // entire ui: window tile, rendering panel, model panel
    void updateRenderingPanel();    // rendering panel only

    void setShowSelectedOnly(bool b);

    void showPointUnderMouse(const easy3d::vec3& p, bool found);

public slots:
    void enableCameraManipulation();

private slots:

    // file
    bool onOpen();
    bool onSave();
    void onOpenRecentFile();
    void onClearRecentFiles();

    // view
    void saveSnapshot();
    void setBackgroundColor();

    // common edit
    void addGaussianNoise();

    // property
    void manipulateProperties();
    void computeHeightField();
    void computeSurfaceMeshCurvatures();

    // point cloud
    void pointCloudDownsampling();
    void pointCloudEstimateNormals();
    void pointCloudReorientNormals();
    void pointCloudNormalizeNormals();
    void pointCloudPoissonSurfaceReconstruction();
    void pointCloudRansacPrimitiveExtraction();
    void pointCloudDelaunayTriangulation2D();
    void pointCloudDelaunayTriangulation3D();

    // surface mesh
    void surfaceMeshReportTopologyStatistics();
    void surfaceMeshExtractConnectedComponents();
    void surfaceMeshPlanarPartition();
    void surfaceMeshTriangulation();

    void surfaceMeshOrientSurface();
    void surfaceMeshReverseOrientation();
    void surfaceMeshDetectDuplicatedFaces();
    void surfaceMeshRemoveDuplicatedFaces();
    void surfaceMeshDetectSelfIntersections();
    void surfaceMeshRemeshSelfIntersections();

    void surfaceMeshSampling();
    void surfaceMeshSubdivisionCatmullClark();
    void surfaceMeshSubdivisionLoop();
    void surfaceMeshSubdivisionSqrt3();
    void surfaceMeshSmoothing();
    void surfaceMeshFairing();
    void surfaceMeshHoleFilling();
    void surfaceMeshSimplification();
    void surfaceMeshRemeshing();
    void surfaceMeshParameterization();
    void surfaceMeshGeodesic();
    void surfaceMeshStitchCoincidentEdges();
    void surfaceMeshRemoveIsolatedVertices();
    void surfaceMeshCreateMeshFromText();

    // user interaction
    void operationModeChanged(QAction* act);

    // status bar
    void updateStatusBar();
    void cancelTask();

    // about
    void onAbout();
    void showManual();

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *) override;
    void closeEvent(QCloseEvent *event) override;

private:
    // Open a file with file name given. On success, the model has been added to the viewer
    easy3d::Model* open(const std::string& file_name);

private:
    void createActionsForFileMenu();
    void createActionsForViewMenu();
    void createActionsForEditMenu();
    void createActionsForPropertyMenu();
    void createActionsForSelectMenu();
    void createActionsForPointCloudMenu();
    void createActionsForSurfaceMeshMenu();

    void createStatusBar();

    bool okToContinue();
    void readSettings();
    void writeSettings();
    void updateWindowTitle();

    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    void notify(std::size_t value, bool show_text = true, bool update_viewer = true) override ;

private:
    PaintCanvas*   viewer_;

    QStringList recentFiles_;
    QString		curDataDirectory_;

    enum { MaxRecentFiles = 5 };
    QAction *actionsRecentFile[MaxRecentFiles],
        *actionSeparator;

    QProgressBar*	progress_bar_;
    QLabel *labelStatusInfo_,
            *labelPointUnderMouse_,
            *labelNumFaces_,
            *labelNumVertices_,
            *labelNumEdges_;

    WidgetPointsDrawable*       widgetPointsDrawable_;
    WidgetLinesDrawable*        widgetLinesDrawable_;
    WidgetTrianglesDrawable*    widgetTrianglesDrawable_;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
