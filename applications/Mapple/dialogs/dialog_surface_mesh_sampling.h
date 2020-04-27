#ifndef DIALOG_SURFACE_SAMPLING_H
#define DIALOG_SURFACE_SAMPLING_H

#include "dialog.h"

namespace Ui {
    class DialogSurfaceMeshSampling;
}

class DialogSurfaceMeshSampling : public Dialog {
Q_OBJECT

public:
    explicit DialogSurfaceMeshSampling(MainWindow *window, QDockWidget* dockWidgetCommand);

    ~DialogSurfaceMeshSampling();

private Q_SLOTS:

    void apply();

private:
    Ui::DialogSurfaceMeshSampling *ui;
};

#endif // DIALOG_SURFACE_SAMPLING_H
