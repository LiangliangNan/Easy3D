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
class DialogPoissonReconstruction;
class DialogRansacPrimitiveExtraction;
class DialogPointCloudSimplification;
class DialogSurfaceMeshSampling;
class DialogGaussianNoise;

class WidgetPointsDrawable;
class WidgetLinesDrawable;
class WidgetTrianglesDrawable;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    PaintCanvas* viewer() { return viewer_; }

    void setCurrentFile(const QString &fileName);
    void currentModelChanged();

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
    void computeHeightField();
    void computeMeanCurvature();
    void computeMaxAbsoluteCurvature();
    void computeGaussianCurvature();
    void computePrincipleCurvatures();

    // point cloud
    void downsampling();
    void estimatePointCloudNormals();
    void reorientPointCloudNormals();
    void normalizePointCloudNormals();
    void poissonSurfaceReconstruction();
    void ransacPrimitiveExtraction();

    // surface mesh
    void reportTopologyStatistics();
    void extractConnectedComponents();
    void detectDuplicatedFaces();
    void removeDuplicatedFaces();
    void detectSelfIntersections();
    void remeshSelfIntersections();
    void samplingSurfaceMesh();
    void subdivisionCatmullClark();
    void subdivisionLoop();
    void subdivisionSqrt3();


    // about
    void onAboutMapple();

    void operationModeChanged(QAction* act);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *) override;
    void closeEvent(QCloseEvent *event) override;

private:
    // Open a file with file name given
    // NOTE: model will not be added to the viewer
    easy3d::Model* open(const std::string& file_name, bool create_default_drawables = true);

private:
    void createActionsForFileMenu();
    void createActionsForViewMenu();
    void createActionsForEditMenu();
    void createActionsForPropertyMenu();
    void createActionsForSelectMenu();
    void createActionsForPointCloudMenu();
    void createActionsForSurfaceMeshMenu();

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

    DialogSnapshot*                 dialogSanpshot_;
    DialogPoissonReconstruction*    dialogPoissonReconstruction_;
    DialogRansacPrimitiveExtraction*dialogRansacPrimitiveExtraction_;
    DialogPointCloudSimplification* dialogPointCloudSimplification_;
    DialogSurfaceMeshSampling*      dialogSurfaceMeshSampling_;
    DialogGaussianNoise*            dialogGaussianNoise_;

    WidgetPointsDrawable*       widgetPointsDrawable_;
    WidgetLinesDrawable*        widgetLinesDrawable_;
    WidgetTrianglesDrawable*    widgetTrianglesDrawable_;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
