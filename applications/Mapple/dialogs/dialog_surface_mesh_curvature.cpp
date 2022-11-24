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


#include "dialog_surface_mesh_curvature.h"

#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/renderer/renderer.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshCurvature::DialogSurfaceMeshCurvature(MainWindow *window)
        : Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshCurvature::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    const unsigned int iter_smooth = spinBoxPostSmoothingIterations->value();
    const bool two_ring = checkBoxUseTwoRingNeighborhood->isChecked();
    LOG(INFO) << "post smoothing iterations: " << iter_smooth << ". use 2-ring neighbors: "
              << (two_ring ? "true" : "false");

    SurfaceMeshCurvature analyzer(mesh);
    analyzer.analyze_tensor(iter_smooth, two_ring);

    analyzer.compute_mean_curvature();
    analyzer.compute_gauss_curvature();
    analyzer.compute_max_abs_curvature();

    mesh->renderer()->update();
    viewer_->update();
    window_->updateRenderingPanel();
}