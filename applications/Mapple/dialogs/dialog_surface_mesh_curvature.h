#ifndef DIALOG_SURFACE_CURVATURE_H
#define DIALOG_SURFACE_CURVATURE_H

#include <QDialog>

namespace Ui {
    class DialogSurfaceMeshCurvature;
}

class PaintCanvas;
class MainWindow;

class DialogSurfaceMeshCurvature : public QDialog {
Q_OBJECT

public:
    explicit DialogSurfaceMeshCurvature(QWidget *parent = nullptr);

    ~DialogSurfaceMeshCurvature();

private Q_SLOTS:

    void apply();

private:
    Ui::DialogSurfaceMeshCurvature *ui;

    PaintCanvas *viewer_;
    MainWindow *main_window_;
};

#endif // DIALOG_SURFACE_CURVATURE_H
