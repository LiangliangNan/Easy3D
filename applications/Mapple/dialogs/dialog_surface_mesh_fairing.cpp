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


#include "dialog_surface_mesh_fairing.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/renderer/renderer.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshFairing::DialogSurfaceMeshFairing(MainWindow *window)
        : Dialog(window) {
    setupUi(this);

    comboBoxCriterion->addItem("Minimize Area");
    comboBoxCriterion->addItem("Minimize Curvature");
    comboBoxCriterion->addItem("Minimize Curvature Variation");

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshFairing::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    SurfaceMeshFairing fair(mesh);
    if (comboBoxCriterion->currentText() == "Minimize Area") {
        LOG(INFO) << "fairing by minimizing area ...";
        fair.minimize_area(); // Minimize Area
    }
    else if (comboBoxCriterion->currentText() == "Minimize Curvature") {
        LOG(INFO) << "fairing by minimizing curvature ...";
        fair.minimize_curvature();
    }
    else if (comboBoxCriterion->currentText() == "Minimize Curvature Variation") {
        LOG(INFO) << "fairing by minimizing curvature variation...";
        fair.fair(3);
    }

    mesh->renderer()->update();
    viewer_->update();
}

