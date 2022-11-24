/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/


#include "dialogs/dialog_gaussian_noise.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/gaussian_noise.h>
#include <easy3d/renderer/renderer.h>

#include <QIntValidator>
#include "paint_canvas.h"
#include "main_window.h"


using namespace easy3d;

DialogGaussianNoise::DialogGaussianNoise(MainWindow *window)
        : Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    suggested_ratio_ = 0.01;

    default_sigma_ = "0.001";
    lineEditGaussianNoiseSigma->setValidator(new QDoubleValidator(0.00001, 1.0, 5, this));
    lineEditGaussianNoiseSigma->setText(default_sigma_);

    labelGaussianNoiseRadiusBBox->setText(QString("%1 * Bounding Sphere Radius").arg(suggested_ratio_));

    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    connect(computeBBoxButton, SIGNAL(clicked()), this, SLOT(computeBBox()));
}


void DialogGaussianNoise::apply() {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const float sigma = lineEditGaussianNoiseSigma->text().toFloat();
    if (dynamic_cast<SurfaceMesh *>(model)) {
        GaussianNoise::apply(dynamic_cast<SurfaceMesh *>(model), sigma);
        model->renderer()->update();
        viewer_->update();
    } else if (dynamic_cast<PointCloud *>(model)) {
        GaussianNoise::apply(dynamic_cast<PointCloud *>(model), sigma);
        model->renderer()->update();
        viewer_->update();
    }
}


void DialogGaussianNoise::computeBBox() {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const float radius = model->bounding_box().radius();
    const std::string &str = std::to_string(radius * suggested_ratio_);
    lineEditGaussianNoiseRadiusBBox->setText(QString::fromStdString(str));
}
