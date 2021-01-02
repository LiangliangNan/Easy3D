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

#include "dialog_snapshot.h"

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

    computeImageSize();;

    connect(viewer_, SIGNAL(resized()), this, SLOT(computeImageSize()));

    connect(doubleSpinBoxImageScale, SIGNAL(valueChanged(double)), this, SLOT(computeImageSize()));
    connect(pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


DialogSnapshot::~DialogSnapshot() {
}


void DialogSnapshot::computeImageSize() {
    double scale = doubleSpinBoxImageScale->value();
    int w = static_cast<int>(viewer_->width() * viewer_->dpi_scaling() * scale);
    int h = static_cast<int>(viewer_->height() * viewer_->dpi_scaling() * scale);
    spinBoxImageWidth->setValue(w);
    spinBoxImageHeight->setValue(h);
}


void DialogSnapshot::saveSnapshot(const QString& fileName) {
    // disable ui to prevent the rendering from being modified.
    window_->setEnabled(false);

    // Hide closed dialog
    QApplication::processEvents();

    int w = spinBoxImageWidth->value();
    int h = spinBoxImageHeight->value();
    int samples = spinBoxSamples->value();
    viewer_->saveSnapshot(w, h, samples, fileName,
                          checkBoxUseWhiteBackground->isChecked(),
                          checkBoxExpandFrustum->isChecked()
    );

    if (checkBoxSaveWindowState) {
        const auto state_file = file_system::replace_extension(fileName.toStdString(), "view");
        viewer_->saveStateToFile(state_file);
    }

    // restore the ui
    window_->setEnabled(true);
}
