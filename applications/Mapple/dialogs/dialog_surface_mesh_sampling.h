#ifndef DIALOG_SURFACE_SAMPLING_H
#define DIALOG_SURFACE_SAMPLING_H

#include <QDialog>

namespace Ui {
    class DialogSurfaceMeshSampling;
}

class PaintCanvas;

class DialogSurfaceMeshSampling : public QDialog {
Q_OBJECT

public:
    explicit DialogSurfaceMeshSampling(QWidget *parent = nullptr);

    ~DialogSurfaceMeshSampling();

private Q_SLOTS:

    void apply();

private:
    Ui::DialogSurfaceMeshSampling *ui;

    PaintCanvas *viewer_;
};

#endif // DIALOG_SURFACE_SAMPLING_H
