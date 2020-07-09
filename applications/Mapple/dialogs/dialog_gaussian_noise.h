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


#ifndef DIALOG_GAUSSIAN_NOISE_H
#define DIALOG_GAUSSIAN_NOISE_H

#include "dialog.h"
#include "ui_dialog_gaussian_noise.h"


class DialogGaussianNoise : public Dialog, public Ui::DialogGaussianNoise {
Q_OBJECT

public:
    DialogGaussianNoise(MainWindow *window);
    ~DialogGaussianNoise();

private Q_SLOTS:
    void apply();
    void computeBBox();

private:
    QString default_sigma_;
    double suggested_ratio_;
};

#endif
