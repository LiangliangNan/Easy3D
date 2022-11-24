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


#include "dialog_poisson_reconstruction.h"
#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

DialogPoissonReconstruction::DialogPoissonReconstruction(MainWindow *window) :
        Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // default value
    default_octree_depth_ = 8;
    default_samples_per_node_ = 1;
    default_trim_value_ = 6.0;
    default_area_ratio_ = 0.001;

    density_attr_name_ = "v:density";

    resetParameters();
    showHint(false);

    connect(pushButtonHelp, SIGNAL(toggled(bool)), this, SLOT(showHint(bool)));
    connect(pushButtonDefault, SIGNAL(clicked()), this, SLOT(resetParameters()));
    connect(pushButtonReconstruct, SIGNAL(clicked()), this, SLOT(reconstruct()));
    connect(pushButtonTrim, SIGNAL(clicked()), this, SLOT(trim()));
}


void DialogPoissonReconstruction::resetParameters() {
    spinBoxOctreeDepth->setValue(default_octree_depth_);
    spinBoxSamplesPerNode->setValue(default_samples_per_node_);

    doubleSpinBoxTrimValue->setValue(default_trim_value_);
    doubleSpinBoxIslandAreaRatio->setValue(default_area_ratio_);
}


void DialogPoissonReconstruction::showHint(bool b) {
    if (b)
        widgetHint->show();
    else
        widgetHint->hide();
}


void DialogPoissonReconstruction::reconstruct() {
    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        int octree_depth = spinBoxOctreeDepth->value();
        double samples_per_node = spinBoxSamplesPerNode->value();

        PoissonReconstruction recon;
        recon.set_depth(octree_depth);
        recon.set_samples_per_node(samples_per_node);

        SurfaceMesh *mesh = recon.apply(cloud, density_attr_name_);
        if (mesh) {
            const std::string &name = file_system::name_less_extension(cloud->name()) + "_poisson_reconstruction.ply";
            mesh->set_name(name);
            viewer_->addModel(mesh);
            window_->updateUi();
            viewer_->update();
        }
    }
}


void DialogPoissonReconstruction::trim() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        double trim_value = doubleSpinBoxTrimValue->value();
        double area_ratio = doubleSpinBoxIslandAreaRatio->value();
        bool triangulate = false; // I can do it using my algorithm :-)

        auto density = mesh->vertex_property<float>(density_attr_name_);
        if (!density) {
            LOG(WARNING) << "no property \'density\' for trimming";
            return;
        }

        float min_density = FLT_MAX;
        float max_density = -FLT_MAX;
        for (auto v : mesh->vertices()) {
            float value = density[v];
            min_density = std::min(min_density, value);
            max_density = std::max(max_density, value);
        }

        if (trim_value <= min_density || trim_value >= max_density) {
            LOG(WARNING) << "trim value (" << trim_value
                         << ") out of density range [" << min_density << ", " << max_density << "]";
            return;
        }

        SurfaceMesh *trimmed_mesh = PoissonReconstruction::trim(mesh, density_attr_name_, trim_value, area_ratio,
                                                                triangulate);
        if (trimmed_mesh) {
            const std::string &name = file_system::name_less_extension(mesh->name()) + "_trimmed.ply";
            trimmed_mesh->set_name(name);
            viewer_->addModel(trimmed_mesh);
            window_->updateUi();
            viewer_->update();
        }
    }
}

