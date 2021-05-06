/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/


#include "dialogs/dialog_point_cloud_simplification.h"

#include <easy3d/kdtree/kdtree_search_eth.h>
#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>
#include <QButtonGroup>
#include <QIntValidator>

#include "paint_canvas.h"
#include "main_window.h"


using namespace easy3d;


DialogPointCloudSimplification::DialogPointCloudSimplification(MainWindow *window)
        : Dialog(window), kdtree_(nullptr) {
    setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // default value
    lineEditDistanceThreshold->setText("0.01");
    lineEditAverageSpacing->setText("unknown");
    lineEditExpectedPointNumber->setText("100000");
    lineEditExpectedPointNumber->setValidator(new QIntValidator(1, 1000000000, this));

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(radioButtonExpectedPointNumber, 0);
    buttonGroup->addButton(radioButtonDistanceThreshold, 1);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(strategyChanged(int)));
    strategyChanged(0);

    connect(buttonComputeAvgSpacing, SIGNAL(clicked()), this, SLOT(computeAvgSpacing()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    connect(queryButton, SIGNAL(clicked()), this, SLOT(query()));
}


DialogPointCloudSimplification::~DialogPointCloudSimplification() {
    if (kdtree_)
        delete kdtree_;
}


void DialogPointCloudSimplification::closeEvent(QCloseEvent *e) {
    points_to_remove_.clear();
    QDialog::closeEvent(e);
}


void DialogPointCloudSimplification::showEvent(QShowEvent *e) {
    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        int num = cloud->n_vertices();
        lineEditExpectedPointNumber->setText(QString("%1").arg(num));
    }
    QDialog::showEvent(e);
}


void DialogPointCloudSimplification::strategyChanged(int id) {
    if (id == 0) {
        lineEditExpectedPointNumber->setDisabled(false);
        lineEditDistanceThreshold->setDisabled(true);
        checkBoxUniform->setDisabled(true);
        lineEditAverageSpacing->setDisabled(true);
        buttonComputeAvgSpacing->setDisabled(true);
    } else {
        lineEditExpectedPointNumber->setDisabled(true);
        lineEditDistanceThreshold->setDisabled(false);
        checkBoxUniform->setDisabled(false);
        lineEditAverageSpacing->setDisabled(false);
        buttonComputeAvgSpacing->setDisabled(false);
    }
}


void DialogPointCloudSimplification::constructKdTree() {
    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        if (kdtree_)
            delete kdtree_;
        kdtree_ = new KdTreeSearch_ETH;
        kdtree_->begin();
        kdtree_->add_point_cloud(cloud);
        kdtree_->end();
    }
}


void DialogPointCloudSimplification::computeAvgSpacing() {
    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        constructKdTree();

        if (kdtree_) {
            float as = PointCloudSimplification::average_spacing(cloud, kdtree_, 6);
            lineEditAverageSpacing->setText(QString("%1").arg(as));
            show();
        }
    }
}


void DialogPointCloudSimplification::query() {
    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        if (radioButtonExpectedPointNumber->isChecked()) {
            unsigned int expected_number = lineEditExpectedPointNumber->text().toInt();
            points_to_remove_ = PointCloudSimplification::uniform_simplification(cloud, expected_number);
        } else {
            float threshold = lineEditDistanceThreshold->text().toFloat();
            if (checkBoxUniform->isChecked()) {
                if (!kdtree_)
                    constructKdTree();
                points_to_remove_ = PointCloudSimplification::uniform_simplification(cloud, threshold, kdtree_);
            } else
                points_to_remove_ = PointCloudSimplification::grid_simplification(cloud, threshold);
        }
        LOG(INFO) << cloud->n_vertices() - points_to_remove_.size() << " points will remain";
    }
}


void DialogPointCloudSimplification::apply() {
    PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud) {
        if (points_to_remove_.size() == 0) {
            LOG(INFO) << "please query points that can be removed";
            return;
        }

        int old_num = cloud->n_vertices();
        for (auto id : points_to_remove_)
            cloud->delete_vertex(PointCloud::Vertex(id));

        cloud->collect_garbage();

        if (!points_to_remove_.empty()) {
            points_to_remove_.clear();
            int new_num = cloud->n_vertices();
            LOG(INFO) << old_num - new_num << " points removed. " << new_num << " points remain";

            cloud->renderer()->update();
            viewer_->update();
        }

        delete kdtree_;
        kdtree_ = 0;
    }
}
