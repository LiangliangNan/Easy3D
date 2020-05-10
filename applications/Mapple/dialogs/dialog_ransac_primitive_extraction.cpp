/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include "dialog_ransac_primitive_extraction.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "ui_dialog_ransac_primitive_extraction.h"


using namespace easy3d;

DialogRansacPrimitiveExtraction::DialogRansacPrimitiveExtraction(MainWindow *window, QDockWidget* dockWidgetCommand) :
    Dialog(window, dockWidgetCommand),
    ui(new Ui::DialogRansacPrimitiveExtraction)
{
    ui->setupUi(this);

    // default value
    default_min_support_ = 1000;
    default_distance_threshold_ = 0.005;
    default_bitmap_resolution_ = 0.02;
    default_normal_threshold_ = 0.8;
    default_overlook_probability_ = 0.001;

    reset();

    connect(ui->pushButtonReset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(ui->pushButtonExtract, SIGNAL(clicked()), this, SLOT(extract()));

    bestSize();
}

DialogRansacPrimitiveExtraction::~DialogRansacPrimitiveExtraction()
{
    delete ui;
}

void DialogRansacPrimitiveExtraction::reset()
{
    ui->spinBoxMinimumSupport->setValue(default_min_support_);
    ui->doubleSpinBoxDistanceThreshold->setValue(default_distance_threshold_);
    ui->doubleSpinBoxBitmapResolution->setValue(default_bitmap_resolution_);
    ui->doubleSpinBoxNormalThreshold->setValue(default_normal_threshold_);
    ui->doubleSpinBoxOverlookProbability->setValue(default_overlook_probability_);
}


void DialogRansacPrimitiveExtraction::extract() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer_->currentModel());
    if (!cloud)
        return;

    int min_support = ui->spinBoxMinimumSupport->value();
    double dist_thresh = ui->doubleSpinBoxDistanceThreshold->value();
    double bitmap_reso = ui->doubleSpinBoxBitmapResolution->value();
    double normal_thresh = ui->doubleSpinBoxNormalThreshold->value();
    double overlook_prob = ui->doubleSpinBoxOverlookProbability->value();

    PrimitivesRansac ransac;
    if (ui->checkBoxPlane->isChecked())		ransac.add_primitive_type(PrimitivesRansac::PLANE);
    if (ui->checkBoxCylinder->isChecked())	ransac.add_primitive_type(PrimitivesRansac::CYLINDER);
    if (ui->checkBoxSphere->isChecked())	ransac.add_primitive_type(PrimitivesRansac::SPHERE);
    if (ui->checkBoxCone->isChecked())		ransac.add_primitive_type(PrimitivesRansac::CONE);
    if (ui->checkBoxTorus->isChecked())		ransac.add_primitive_type(PrimitivesRansac::TORUS);

    if (selected_only_) {
        auto selected = cloud->get_vertex_property<bool>("v:select");
        if (!selected) {
            LOG(WARNING) << "no points have been selected";
            return;
        }
        std::vector<int> indices;
        for (auto v : cloud->vertices()) {
            if (selected[v])
                indices.push_back(v.idx());
        }

        int num = ransac.detect(cloud, indices, min_support, dist_thresh, bitmap_reso, normal_thresh, overlook_prob);
        LOG(INFO) << num << " primitives extracted from the selected points";
    }
    else {
        int num = ransac.detect(cloud, min_support, dist_thresh, bitmap_reso, normal_thresh, overlook_prob);
        LOG(INFO) << num << " primitives extracted";
    }

    auto vertices = cloud->drawable("vertices");
    const std::string name = "v:color-segments";

    renderer::colorize_segmentation(cloud, "v:primitive_index", name);
    vertices->set_property_coloring(State::VERTEX, name);
    vertices->update();

    viewer_->update();
    window_->updateRenderingPanel();
}
