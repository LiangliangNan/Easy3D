#include "dialogs/dialog_gaussian_noise.h"


#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/gaussian_noise.h>
#include <easy3d/viewer/renderer.h>

#include <QIntValidator>
#include "paint_canvas.h"
#include "main_window.h"

#include "ui_dialog_gaussian_noise.h"


using namespace easy3d;

DialogGaussianNoise::DialogGaussianNoise(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::DialogGaussianNoise)
{
    ui->setupUi(this);
    viewer_ = dynamic_cast<MainWindow *>(parent)->viewer();
//    setWindowFlags(Qt::WindowStaysOnTopHint);

    suggested_ratio_ = 0.01f;

    default_sigma_ = "0.001";
    ui->lineEditGaussianNoiseSigma->setValidator(new QDoubleValidator(0.00001, 1.0, 5, this));
    ui->lineEditGaussianNoiseSigma->setText(default_sigma_);

    ui->labelGaussianNoiseRadiusBBox->setText(QString("%1 * Bounding Sphere Radius").arg(suggested_ratio_));

    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->computeBBoxButton, SIGNAL(clicked()), this, SLOT(computeBBox()));
}


DialogGaussianNoise::~DialogGaussianNoise() {

}


void DialogGaussianNoise::apply() {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const float sigma = ui->lineEditGaussianNoiseSigma->text().toFloat();
    if (dynamic_cast<SurfaceMesh *>(model)) {
        GaussianNoise::apply(dynamic_cast<SurfaceMesh *>(model), sigma);
        renderer::update_buffer(dynamic_cast<SurfaceMesh *>(model), model->triangles_drawable("faces"));
        viewer_->update();
    }
    else if (dynamic_cast<PointCloud *>(model)) {
        GaussianNoise::apply(dynamic_cast<PointCloud *>(model), sigma);
        renderer::update_buffer(dynamic_cast<PointCloud *>(model), model->points_drawable("vertices"));
        viewer_->update();
    }
}


void DialogGaussianNoise::computeBBox() {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const Box3 &box = model->bounding_box();
    float radius = box.diagonal() * 0.5f;

    const std::string &str = std::to_string(radius * suggested_ratio_);
    ui->lineEditGaussianNoiseRadiusBBox->setText(QString::fromStdString(str));
}
