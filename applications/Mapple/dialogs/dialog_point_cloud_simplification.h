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


#ifndef DIALOG_POINT_CLOUD_SIMPLIFICATION_H
#define DIALOG_POINT_CLOUD_SIMPLIFICATION_H

#include <easy3d/core/point_cloud.h>

#include "dialog.h"
#include "ui_dialog_point_cloud_simplification.h"


namespace easy3d {
    class KdTreeSearch;
}

class DialogPointCloudSimplification : public Dialog, public Ui::DialogPointCloudSimplification
{
	Q_OBJECT

public:
	explicit DialogPointCloudSimplification(MainWindow *window);
	~DialogPointCloudSimplification() override;

private Q_SLOTS:
	void strategyChanged(int id);

	void computeAvgSpacing();
	void apply();
	void query();

protected:
	void closeEvent(QCloseEvent* e) override;
	void showEvent(QShowEvent* e) override;

	void constructKdTree();

private:
	easy3d::KdTreeSearch*    kdtree_;
	std::vector<easy3d::PointCloud::Vertex> points_to_remove_;
};

#endif // DIALOG_POINT_CLOUD_SIMPLIFICATION_H
