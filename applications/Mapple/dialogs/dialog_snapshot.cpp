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

#include "dialog_snapshot.h"

#include <QFileDialog>

#include <easy3d/core/model.h>
#include <easy3d/util/file_system.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSnapshot::DialogSnapshot(MainWindow *window)
        : Dialog(window) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    const double scale = 1.0;
    doubleSpinBoxImageScale->setValue(scale);

    computeImageSize();

    connect(viewer_, SIGNAL(resized()), this, SLOT(computeImageSize()));

    connect(doubleSpinBoxImageScale, SIGNAL(valueChanged(double)), this, SLOT(computeImageSize()));
    connect(pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


void DialogSnapshot::computeImageSize() {
    double scale = doubleSpinBoxImageScale->value();
    int w = static_cast<int>(viewer_->width() * viewer_->dpi_scaling() * scale);
    int h = static_cast<int>(viewer_->height() * viewer_->dpi_scaling() * scale);
    spinBoxImageWidth->setValue(w);
    spinBoxImageHeight->setValue(h);
}


void DialogSnapshot::saveSnapshot() {
    // make sure the save file dialog is really hidden
    QApplication::processEvents();

    const Model* model = viewer_->currentModel();

    const bool overwrite = false;
    std::string default_file_name("untitled.png");
    if (model)
        default_file_name = file_system::replace_extension(model->name(), "png");

    QString proposedFormat = "PNG (*.png)";
    const QString fileName = QFileDialog::getSaveFileName(
            this,
            "Choose an image file name",
            QString::fromStdString(default_file_name),
            "Image formats (*.png *.jpg *.bmp *.ppm)\n"
            "PNG (*.png)\n"
            "JPG (*.jpg)\n"
            "Windows Bitmap (*.bmp)\n"
            "24bit RGB Bitmap (*.ppm)",
            &proposedFormat,
            overwrite ? QFileDialog::DontConfirmOverwrite : QFileDialog::Option()
    );

    // make sure the save file dialog is really hidden
    QApplication::processEvents();

    if (fileName.isEmpty())
        return;

    // disable ui to prevent the rendering from being modified.
    window_->setDisabled(true);

    const int w = spinBoxImageWidth->value();
    const int h = spinBoxImageHeight->value();
    const int samples = spinBoxSamples->value();
    // 0: current color; 1: white; 2: transparent.
    int bk = 0;
    if (radioButtonWhiteBackground->isChecked())
        bk = 1;
    else if (radioButtonTransparentBackground->isChecked())
        bk = 2;
    viewer_->saveSnapshot(w, h, samples, fileName, bk, checkBoxExpandFrustum->isChecked());

    if (checkBoxSaveWindowState->isChecked()) {
        const auto state_file = file_system::replace_extension(fileName.toStdString(), "view");
        std::ofstream output(state_file.c_str());
        viewer_->saveState(output);
    }

    // restore the ui
    window_->setDisabled(false);
}
