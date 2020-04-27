#ifndef DIALOG_SURFACE_CURVATURE_H
#define DIALOG_SURFACE_CURVATURE_H

#include "dialog.h"

namespace Ui {
    class DialogSurfaceMeshCurvature;
}

class DialogSurfaceMeshCurvature : public Dialog {
Q_OBJECT

public:
    explicit DialogSurfaceMeshCurvature(MainWindow *window, QDockWidget* dockWidgetCommand);

    ~DialogSurfaceMeshCurvature();

private Q_SLOTS:

    void apply();

private:
    Ui::DialogSurfaceMeshCurvature *ui;
};

#endif // DIALOG_SURFACE_CURVATURE_H
