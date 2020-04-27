#include "dialog_surface_mesh_curvature.h"

#include <easy3d/algo/surface_mesh_curvature.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "ui_dialog_surface_mesh_curvature.h"


using namespace easy3d;

DialogSurfaceMeshCurvature::DialogSurfaceMeshCurvature(MainWindow *window, QDockWidget* dockWidgetCommand) :
        Dialog(window, dockWidgetCommand),
        ui(new Ui::DialogSurfaceMeshCurvature) {
    ui->setupUi(this);

    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    bestSize();
}


DialogSurfaceMeshCurvature::~DialogSurfaceMeshCurvature() {
    delete ui;
}


void DialogSurfaceMeshCurvature::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    const unsigned int iter_smooth = ui->spinBoxPostSmoothingIterations->value();
    const bool two_ring = ui->checkBoxUseTwoRingNeighborhood->isChecked();
    LOG(INFO) << "post smoothing iterations: " << iter_smooth << ". use 2-ring neighbors: " << (two_ring ? "true" : "false");

    SurfaceMeshCurvature analyzer(mesh);
    analyzer.analyze_tensor(iter_smooth, two_ring);

    analyzer.compute_mean_curvature();
    analyzer.compute_gauss_curvature();
    analyzer.compute_max_abs_curvature();

    mesh->update();
    viewer_->update();
    window_->updateRenderingPanel();
}