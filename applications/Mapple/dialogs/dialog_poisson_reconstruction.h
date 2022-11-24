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


#ifndef DIALOG_POISSON_RECONSTRUCTION_H
#define DIALOG_POISSON_RECONSTRUCTION_H

#include "dialog.h"
#include "ui_dialog_poisson_reconstruction.h"


class PaintCanvas;
class MainWindow;

class DialogPoissonReconstruction : public Dialog, public Ui::DialogPoissonReconstruction
{
    Q_OBJECT

public:
    explicit DialogPoissonReconstruction(MainWindow *window);
    ~DialogPoissonReconstruction() override = default;

private Q_SLOTS:
    void showHint(bool);
    void resetParameters();
    void reconstruct();
    void trim();

private:
    int			default_octree_depth_;
    int         default_samples_per_node_;
    double		default_trim_value_;
    double		default_area_ratio_;

    std::string density_attr_name_;
};

#endif // DIALOG_POISSON_RECONSTRUCTION_H
