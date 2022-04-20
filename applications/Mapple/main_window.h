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



#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <string>

#include <QMainWindow>

#include <easy3d/util/progress.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/types.h>


class QLabel;
class QProgressBar;
class QPushButton;

class PaintCanvas;
class WidgetPointsDrawable;
class WidgetLinesDrawable;
class WidgetTrianglesDrawable;
class WidgetModelList;

namespace Ui {
    class MainWindow;
}

namespace easy3d {
    class Model;
    class Drawable;
}


class MainWindow : public QMainWindow, public easy3d::ProgressClient, public easy3d::logging::Logger
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    int openFiles(const QStringList &fileNames);

    PaintCanvas* viewer() { return viewer_; }

    void setCurrentFile(const QString &fileName);

    void updateUi(); // entire ui: window tile, rendering panel, model panel
    void updateRenderingPanel();    // rendering panel only

    void setShowSelectedOnly(bool b);

    // ui stuff
    WidgetModelList* widgetModelList() const;
    void activeDrawableChanged(easy3d::Drawable*);
    void updateWindowTitle();

public slots:
    void enableCameraManipulation();

public slots:

    // file
    void loadModelTranslateChanged(QAction*);
    bool onOpen();
    bool onSave();
    void onOpenRecentFile();
    void onClearRecentFiles();

    // view
    void saveSnapshot();
    void setBackgroundColor();

    // camera
    void saveCameraStateToFile();
    void restoreCameraStateFromFile();

    void showPrimitivePropertyUnderMouse(bool);
    void showCoordinatesUnderMouse(bool);
    void importCameraPath();
    void exportCameraPath();
    void setShowCameraPath(bool);
    void setShowKeyframeCameras(bool);
    void animation();

    // property
    void manipulateProperties();
    void computeHeightField();
    void computeSurfaceMeshCurvatures();
    void reportTopologyStatistics();
    void generateColorPropertyFromIndexedColors();

    // edit
    void translationalRecenter();
    void addGaussianNoise();
    void applyManipulatedTransformation();
    void giveUpManipulatedTransformation();

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
    void surfaceMeshExtractConnectedComponents();
    void surfaceMeshDual();
    void surfaceMeshPlanarPartition();
    void surfaceMeshPolygonization();
    void surfaceMeshTriangulation();
    void surfaceMeshTetrahedralization();
    void surfaceMeshStitchWithReorientation();
    void surfaceMeshStitchWithoutReorientation();
    void surfaceMeshOrientClosedTriangleMesh();
    void surfaceMeshReverseOrientation();
    void surfaceMeshRemoveIsolatedVertices();
    void surfaceMeshRemoveDuplicateAndFoldingFaces();
    void surfaceMeshDetectSelfIntersections();
    void surfaceMeshRemeshSelfIntersections();
    void surfaceMeshRepairPolygonSoup();
    void surfaceMeshOrientAndStitchPolygonSoup();
    void surfaceMeshClip();
    void surfaceMeshSplit();
    void surfaceMeshSlice();
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
    void surfaceMeshCreateMeshFromText();

    // polyhedral mesh
    void polymeshExtractBoundary();

    // user interaction
    void operationModeChanged(QAction*);

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
    void createActionsForCameraMenu();
    void createActionsForPropertyMenu();
    void createActionsForEditMenu();
    void createActionsForSelectMenu();
    void createActionsForPointCloudMenu();
    void createActionsForSurfaceMeshMenu();
    void createActionsForPolyMeshMenu();

    void createStatusBar();

    bool okToContinue();
    void readSettings();
    void writeSettings();

    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    void notify(std::size_t percent, bool update_viewer) override;

    void send(el::Level level, const std::string &msg) override;

private:
    PaintCanvas*   viewer_;

    QStringList recentFiles_;
    QString		curDataDirectory_;

    enum { MaxRecentFiles = 5 };
    QAction *actionsRecentFile[MaxRecentFiles],
        *actionSeparator;

    QPushButton*    cancelTaskButton_;
    QProgressBar*	progress_bar_;
    QLabel *labelStatusInfo_,
            *labelNumFaces_,
            *labelNumVertices_,
            *labelNumEdges_,
            *labelNumCells_;

    WidgetPointsDrawable*       widgetPointsDrawable_;
    WidgetLinesDrawable*        widgetLinesDrawable_;
    WidgetTrianglesDrawable*    widgetTrianglesDrawable_;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
