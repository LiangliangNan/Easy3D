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

class PaintCanvas;
class DialogSnapshot;
class WidgetPointsDrawable;
class WidgetLinesDrawable;
class WidgetTrianglesDrawable;
class QDockWidget;

class MainWindow : public QMainWindow
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
    void deleteProperty();
    void computeHeightField();
    void computeSurfaceMeshCurvatures();

    // point cloud
    void pointCloudDownsampling();
    void pointCloudEstimateNormals();
    void pointCloudReorientNormals();
    void pointCloudNormalizeNormals();
    void pointCloudPoissonSurfaceReconstruction();
    void pointCloudRansacPrimitiveExtraction();

    // surface mesh
    void surfaceMeshReportTopologyStatistics();
    void surfaceMeshExtractConnectedComponents();
    void surfaceMeshTriangulation();
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

    // about
    void onAboutMapple();
    void showManual();

    void operationModeChanged(QAction* act);

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

    void showDialog(QDialog*);

    bool okToContinue();
    void readSettings();
    void writeSettings();
    void updateWindowTitle();

    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

private:
    PaintCanvas*   viewer_;

    QStringList recentFiles_;
    QString		curDataDirectory_;

    enum { MaxRecentFiles = 5 };
    QAction *actionsRecentFile[MaxRecentFiles],
        *actionSeparator;

    DialogSnapshot*             dialogSanpshot_;
    QDockWidget*                dialogCommand_;

    WidgetPointsDrawable*       widgetPointsDrawable_;
    WidgetLinesDrawable*        widgetLinesDrawable_;
    WidgetTrianglesDrawable*    widgetTrianglesDrawable_;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
