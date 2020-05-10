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

#include <algorithm>
#include <QDockWidget>

#include "ui_dialog_snapshot.h"
#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSnapshot::DialogSnapshot(MainWindow *window, QDockWidget* dockWidgetCommand)
    : Dialog(window, dockWidgetCommand)
    , ui(new Ui::DialogSnapshot)
{
    double scale = 1.0;
    ui->setupUi(this);
    ui->doubleSpinBoxImageScale->setValue(scale);

    computeImageSize();;

    connect(ui->doubleSpinBoxImageScale, SIGNAL(valueChanged(double)), this, SLOT(computeImageSize()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(saveSnapshot()));

    bestSize();
 }


DialogSnapshot::~DialogSnapshot()
{
    delete ui;
}


void DialogSnapshot::computeImageSize() {
    double scale = ui->doubleSpinBoxImageScale->value();
    int w = static_cast<int>(viewer_->width() * viewer_->dpi_scaling() * scale);
    int h = static_cast<int>(viewer_->height() * viewer_->dpi_scaling() * scale);
    ui->spinBoxImageWidth->setValue(w);
    ui->spinBoxImageHeight->setValue(h);
}


void DialogSnapshot::setImageFileName(const QString& fileName) {
    fileName_ = fileName;
}


void DialogSnapshot::closeDialog() {
    dockWidgetCommand_->close();
}


void DialogSnapshot::saveSnapshot() {
    // close the save dialog
    closeDialog();

    // disable ui to prevent the rendering from being modified.
    window_->setEnabled(false);

    // Hide closed dialog
    QApplication::processEvents();

    int w = ui->spinBoxImageWidth->value();
    int h = ui->spinBoxImageHeight->value();
    int samples = ui->spinBoxSamples->value();
    viewer_->saveSnapshot(w, h, samples, fileName_,
                          ui->checkBoxUseWhiteBackground->isChecked(),
                          ui->checkBoxExpandFrustum->isChecked()
                          );

    // restore the ui
    window_->setEnabled(true);
}
