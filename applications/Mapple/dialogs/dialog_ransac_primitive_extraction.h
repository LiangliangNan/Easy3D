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


#ifndef DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
#define DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H


#include "dialog.h"
#include "ui_dialog_ransac_primitive_extraction.h"


class DialogRansacPrimitiveExtraction : public Dialog, public Ui::DialogRansacPrimitiveExtraction
{
    Q_OBJECT

public:
    explicit DialogRansacPrimitiveExtraction(MainWindow *window);
    ~DialogRansacPrimitiveExtraction();

    void setWorkOnSelectedPoints(bool b) { selected_only_ = b; }

private Q_SLOTS:
    void extract();
    void reset();

private:
    bool	selected_only_;

    int		default_min_support_;
    double	default_distance_threshold_;
    double	default_bitmap_resolution_;
    double	default_normal_threshold_;
    double	default_overlook_probability_;
};

#endif // DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
