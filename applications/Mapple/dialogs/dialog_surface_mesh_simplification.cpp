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


#include "dialog_surface_mesh_simplification.h"

#include <QIntValidator>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/renderer/renderer.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshSimplification::DialogSurfaceMeshSimplification(MainWindow *window)
        : Dialog(window) {
    setupUi(this);

    QValidator *validator = new QIntValidator(3, 999999999, this);
    lineEditVertexNumber->setValidator(validator);

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        const int num = static_cast<int>(mesh->n_vertices() * 0.5f);
        lineEditVertexNumber->setText(QString("%1").arg(num));
    }

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshSimplification::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    const int normal_deviation = 180;
    const int aspect_ratio = 10;

    const int expected_vertex_number = lineEditVertexNumber->text().toInt();
    SurfaceMeshSimplification ss(mesh);
    ss.initialize(aspect_ratio, 0.0, 0.0, normal_deviation, 0.0);
    ss.simplify(expected_vertex_number);

    mesh->renderer()->update();
    viewer_->update();
    window_->updateUi();
}

