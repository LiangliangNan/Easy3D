#ifndef DIALOG_POISSON_RECONSTRUCTION_H
#define DIALOG_POISSON_RECONSTRUCTION_H

#include "dialog.h"

namespace Ui {
    class DialogPoissonReconstruction;
}

class PaintCanvas;
class MainWindow;
class QDockWidget;

class DialogPoissonReconstruction : public Dialog
{
    Q_OBJECT

public:
    explicit DialogPoissonReconstruction(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~DialogPoissonReconstruction();

private Q_SLOTS:
    void showHint(bool);
    void resetParameters();
    void reconstruct();
    void trim();

private:
    Ui::DialogPoissonReconstruction *ui;

    int			default_octree_depth_;
    int         default_samples_per_node_;
    double		default_trim_value_;
    double		default_area_ratio_;

    std::string density_attr_name_;
};

#endif // DIALOG_POISSON_RECONSTRUCTION_H
