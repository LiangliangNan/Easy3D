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


#include "dialog_point_cloud_ransac_primitive_extraction.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogPointCloudRansacPrimitiveExtraction::DialogPointCloudRansacPrimitiveExtraction(MainWindow *window)
        : Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    selected_only_ = false;

    // default value
    default_min_support_ = 1000;
    default_distance_threshold_ = 0.005;
    default_bitmap_resolution_ = 0.02;
    default_normal_threshold_ = 0.8;
    default_overlook_probability_ = 0.001;

    reset();

    connect(pushButtonReset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(pushButtonExtract, SIGNAL(clicked()), this, SLOT(extract()));
}


void DialogPointCloudRansacPrimitiveExtraction::reset() {
    spinBoxMinimumSupport->setValue(default_min_support_);
    doubleSpinBoxDistanceThreshold->setValue(default_distance_threshold_);
    doubleSpinBoxBitmapResolution->setValue(default_bitmap_resolution_);
    doubleSpinBoxNormalThreshold->setValue(default_normal_threshold_);
    doubleSpinBoxOverlookProbability->setValue(default_overlook_probability_);
}


void DialogPointCloudRansacPrimitiveExtraction::extract() {
    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (!cloud)
        return;

    int min_support = spinBoxMinimumSupport->value();
    double dist_thresh = doubleSpinBoxDistanceThreshold->value();
    double bitmap_reso = doubleSpinBoxBitmapResolution->value();
    double normal_thresh = doubleSpinBoxNormalThreshold->value();
    double overlook_prob = doubleSpinBoxOverlookProbability->value();

    PrimitivesRansac ransac;
    if (checkBoxPlane->isChecked()) ransac.add_primitive_type(PrimitivesRansac::PLANE);
    if (checkBoxCylinder->isChecked()) ransac.add_primitive_type(PrimitivesRansac::CYLINDER);
    if (checkBoxSphere->isChecked()) ransac.add_primitive_type(PrimitivesRansac::SPHERE);
    if (checkBoxCone->isChecked()) ransac.add_primitive_type(PrimitivesRansac::CONE);
    if (checkBoxTorus->isChecked()) ransac.add_primitive_type(PrimitivesRansac::TORUS);

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
    } else {
        int num = ransac.detect(cloud, min_support, dist_thresh, bitmap_reso, normal_thresh, overlook_prob);
        LOG(INFO) << num << " primitives extracted";
    }

    auto segments = cloud->vertex_property<int>("v:primitive_index");
    const std::string color_name = "v:color-segments";
    auto coloring = cloud->vertex_property<vec3>(color_name, vec3(0, 0, 0));
    Renderer::color_from_segmentation(cloud, segments, coloring);

    auto vertices = cloud->renderer()->get_points_drawable("vertices");
    vertices->set_property_coloring(State::VERTEX, color_name);
    vertices->update();

    viewer_->update();
    window_->updateRenderingPanel();
}
