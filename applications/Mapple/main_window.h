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
class WidgetPointCloudRenderer;
class WidgetSurfaceMeshRenderer;
class WidgetSurfaceMeshTexturing;
class WidgetScalarField;
class WidgetVectorField;

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

    // topology
    void reportTopologyStatistics();
    void detectDuplicatedFaces();
    void removeDuplicatedFaces();
    void detectSelfIntersections();
    void remeshSelfIntersections();

    // orientation
    void estimatePointCloudNormals();
    void reorientPointCloudNormals();

    // conversion
    void poissonSurfaceReconstruction();
    void ransacPrimitiveExtraction();
    void samplingSurfaceMesh();

    // about
    void onAboutMapple();

    void onCurrentModelChanged();

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
    WidgetPointCloudRenderer*       widgetCloudRenderer_;
    WidgetSurfaceMeshRenderer*      widgetMeshRenderer_;
    WidgetSurfaceMeshTexturing*     widgetMeshTexturing_;
    WidgetScalarField*              widgetScalarField_;
    WidgetVectorField*              widgetVectorField_;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
