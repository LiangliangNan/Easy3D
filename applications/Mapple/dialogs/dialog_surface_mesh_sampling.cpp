#include "dialog_surface_mesh_sampling.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/logging.h>
#include <easy3d/algo/surface_mesh_sampler.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "ui_dialog_surface_mesh_sampling.h"


using namespace easy3d;

DialogSurfaceMeshSampling::DialogSurfaceMeshSampling(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::DialogSurfaceMeshSampling) {
    ui->setupUi(this);
    viewer_ = dynamic_cast<MainWindow *>(parent)->viewer();

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


DialogSurfaceMeshSampling::~DialogSurfaceMeshSampling() {
    delete ui;
}


void DialogSurfaceMeshSampling::apply() {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    int num = ui->spinBoxPointNumber->value();
    if (num < mesh->n_vertices()) {
        LOG(WARNING) << "point num must >= the num of vertices of the input mesh";
        return;
    }

    SurfaceMeshSampler sampler;
    PointCloud *cloud = sampler.apply(mesh, num);
    if (cloud) {
        viewer_->makeCurrent();
        viewer_->addModel(cloud);
        viewer_->doneCurrent();
    }
}

