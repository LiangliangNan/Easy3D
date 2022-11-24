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


#include "dialog_surface_mesh_smoothing.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshSmoothing::DialogSurfaceMeshSmoothing(MainWindow *window)
        : Dialog(window) {
    setupUi(this);

    comboBoxScheme->addItem("Explicit Smoothing");
    comboBoxScheme->addItem("Implicit Smoothing");
    comboBoxScheme->setCurrentIndex(0);

    connect(comboBoxScheme, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setSmoothingScheme(const QString &)));

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshSmoothing::setSmoothingScheme(const QString &s) {
    if (s == "Explicit Smoothing") {
        labelIterations->setEnabled(true);
        spinBoxIterations->setEnabled(true);
    }
    else {
        labelIterations->setEnabled(false);
        spinBoxIterations->setEnabled(false);
    }
}


void DialogSurfaceMeshSmoothing::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    const bool uniform_laplace = checkBoxUniformLaplace->isChecked();

    SurfaceMeshSmoothing smoother(mesh);

    if (comboBoxScheme->currentText() == "Explicit Smoothing") {
        const int iter = spinBoxIterations->value();
        smoother.explicit_smoothing(iter, uniform_laplace);
    }
    else {    // Implicit Smoothing
        const float timestep = 0.001f;

        // does the mesh have a boundary?
        bool has_boundary = false;
        for (auto v: mesh->vertices())
            if (mesh->is_border(v))
                has_boundary = true;

        // only re-scale if we don't have a (fixed) boundary
        const bool rescale = !has_boundary;
        const float scene_radius = viewer_->camera()->sceneRadius();
        const float dt = uniform_laplace ? timestep : timestep * scene_radius * scene_radius;
        smoother.implicit_smoothing(dt, uniform_laplace, rescale);
    }

    mesh->renderer()->update();
    viewer_->update();
}

