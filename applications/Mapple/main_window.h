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

private slots:

    // file
    bool onOpen();
    bool onSave();
    void onOpenRecentFile();
    void onClearRecentFiles();

    // view
    void saveSnapshot();
    void setBackgroundColor();

    // edit
    void downsampling();
    void addGaussianNoise();

    // property
    void computeHeightField();

    // topology
    void reportTopologyStatistics();
    void detectDuplicatedFaces();
    void removeDuplicatedFaces();
    void detectSelfIntersections();
    void remeshSelfIntersections();

    // orientation
    void estimatePointCloudNormals();
    void reorientPointCloudNormals();
    void normalizePointCloudNormals();

    // conversion
    void poissonSurfaceReconstruction();
    void ransacPrimitiveExtraction();
    void samplingSurfaceMesh();

    // about
    void onAboutMapple();

    void onCurrentModelChanged();

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
    void createActions();

    void createActionsForFileMenu();
    void createActionsForViewMenu();
    void createActionsForEditMenu();
    void createActionsForPropertyMenu();
    void createActionsForTopologyMenu();
    void createActionsForOrientationMenu();
    void createActionsForConversionMenu();

    bool okToContinue();
    void readSettings();
    void writeSettings();
    void updateWindowTitle();

    void setCurrentFile(const QString &fileName);

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
