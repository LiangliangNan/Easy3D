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


#include "dialog_surface_mesh_from_text.h"

#include <easy3d/algo/text_mesher.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>

#include <QFileDialog>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshFromText::DialogSurfaceMeshFromText(MainWindow *window) :
        Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    font_path_ = resource::directory() + "/fonts/";

    connect(toolButtonTFontFile, SIGNAL(clicked()), this, SLOT(setFontFile()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshFromText::setFontFile() {
    const std::string dir = resource::directory() + "/fonts";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a font file"), QString::fromStdString(dir),
                                                    tr("True Type Font (*.ttf)")
    );

    if (fileName.isEmpty())
        return;

    const std::string &file_name = fileName.toStdString();
    font_path_ = file_system::parent_directory(file_name);
    const std::string &font_file = file_system::simple_name(file_name);
    lineEditFontFile->setText(QString::fromStdString(font_file));
}


void DialogSurfaceMeshFromText::apply() {
    const std::string &text = lineEditText->text().toStdString();
    if (text.empty()) {
        LOG(WARNING) << "text is empty";
        return;
    }

    const std::string &font_file = font_path_ + "/" + lineEditFontFile->text().toStdString();
    if (!file_system::is_file(font_file)) {
        LOG(WARNING) << "font file does not exit: " << font_file;
        return;
    }

    int font_size = spinBoxFontSize->value();
    if (font_size < 5) {
        LOG(WARNING) << "font size must be >= 5";
        return;
    }

    int extrusion = spinBoxExtrusion->value();
    if (extrusion < 1) {
        LOG(WARNING) << "extrusion value must be positive";
        return;
    }

    const int bezier_steps = spinBoxBezierSteps->value();
    bool collision_free = checkBoxCollisionFree->isChecked();

    TextMesher mesher(font_file, bezier_steps);

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh && mesh->name() == text) {
        mesh->clear();
        mesher.generate(mesh, text, 0, 0, font_size, extrusion, collision_free);
        mesh->renderer()->update();
    } else {
        mesh = mesher.generate(text, 0, 0, font_size, extrusion, collision_free);
        if (mesh) {
            mesh->set_name(text);
            viewer_->addModel(mesh);
            viewer_->fitScreen(mesh);
        }
    }

    viewer_->update();
    window_->updateUi();
}