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


#include "dialog_surface_mesh_remeshing.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_features.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/renderer/renderer.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshRemeshing::DialogSurfaceMeshRemeshing(MainWindow *window)
        : Dialog(window) {
    setupUi(this);

    comboBoxScheme->addItem("Uniform Remeshing");
    comboBoxScheme->addItem("Adaptive Remeshing");
    comboBoxScheme->setCurrentIndex(0);

    connect(comboBoxScheme, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setRemeshingScheme(const QString &)));

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshRemeshing::setRemeshingScheme(const QString &s) {
    if (s == "Uniform Remeshing") {
        labelEdgeLength->setEnabled(true);
        doubleSpinBoxEdgeLength->setEnabled(true);
    } else {
        labelEdgeLength->setEnabled(false);
        doubleSpinBoxEdgeLength->setEnabled(false);
    }
}


void DialogSurfaceMeshRemeshing::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    if (checkBoxUseFeatures->isChecked()) {
        const int feature_angle = spinBoxDihedralAngle->value();
        SurfaceMeshFeatures sf(mesh);
        sf.clear();
        sf.detect_angle(feature_angle);
        sf.detect_boundary();
    }

    if (comboBoxScheme->currentText() == "Uniform Remeshing") {
        float len(0.0f);
        for (auto eit : mesh->edges())
            len += distance(mesh->position(mesh->vertex(eit, 0)),
                            mesh->position(mesh->vertex(eit, 1)));
        len /= (float) mesh->n_edges();
        SurfaceMeshRemeshing(mesh).uniform_remeshing(len * doubleSpinBoxEdgeLength->value());
    } else { // Adaptive remeshing
        auto bb = mesh->bounding_box().diagonal_length();
        SurfaceMeshRemeshing(mesh).adaptive_remeshing(
                0.001 * bb,  // min length
                0.100 * bb,  // max length
                0.001 * bb); // approx. error
    }

    mesh->renderer()->update();
    viewer_->update();
    window_->updateUi();
}

