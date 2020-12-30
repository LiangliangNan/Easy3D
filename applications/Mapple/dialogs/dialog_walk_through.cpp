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

#include "dialog_walk_through.h"
#include "paint_canvas.h"
#include "main_window.h"
#include <easy3d/renderer/walk_throuth.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/util/logging.h>

#include <QFileDialog>
#include <QMessageBox>


using namespace easy3d;

DialogWalkThrough::DialogWalkThrough(MainWindow *window)
	: Dialog(window)
{
	setupUi(this);

	connect(doubleSpinBoxCharacterHeightFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterHeightFactor(double)));
	connect(doubleSpinBoxCharacterDistanceFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterDistanceFactor(double)));
    connect(doubleSpinBoxInterpolationSpeed, SIGNAL(valueChanged(double)), this, SLOT(setInterpolationSpeed(double)));

	connect(previousPositionButton, SIGNAL(clicked()), this, SLOT(goToPreviousPosition()));
	connect(nextPositionButton, SIGNAL(clicked()), this, SLOT(goToNextPosition()));
    connect(removeLastPositionButton, SIGNAL(clicked()), this, SLOT(removeLastPosition()));

    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));

    connect(startAnimationButton, SIGNAL(clicked()), this, SLOT(startAnimation()));
    connect(exportAnimationButton, SIGNAL(clicked()), this, SLOT(exportAnimation()));
    connect(clearPathButton, SIGNAL(clicked()), this, SLOT(clearPath()));

    easy3d::connect(&walkThrough()->path_modified, this, &DialogWalkThrough::newPositionAdded);
}


DialogWalkThrough::~DialogWalkThrough()
{
}


void DialogWalkThrough::newPositionAdded() {
    disconnect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));
    int num = walkThrough()->num_positions();
    horizontalSliderPreview->setRange(0, std::max(0, num - 1));
    int pos = walkThrough()->current_position();
    horizontalSliderPreview->setValue(pos);
    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));
}


easy3d::WalkThrough* DialogWalkThrough::walkThrough() {
    return viewer_->walkThrough();
}


void DialogWalkThrough::showEvent(QShowEvent* e) {
	doubleSpinBoxCharacterHeightFactor->setValue(walkThrough()->height_factor());
	doubleSpinBoxCharacterDistanceFactor->setValue(walkThrough()->third_person_forward_factor());
	QDialog::showEvent(e);
}


void DialogWalkThrough::closeEvent(QCloseEvent* e) {
    QDialog::closeEvent(e);
	viewer_->update();
}


void DialogWalkThrough::setCharacterHeightFactor(double h) {
    walkThrough()->set_height_factor(h);
    viewer_->update();
}


void DialogWalkThrough::setCharacterDistanceFactor(double d) {
    walkThrough()->set_third_person_forward_factor(d);
    viewer_->update();
}


void DialogWalkThrough::setInterpolationSpeed(double s) {
    walkThrough()->interpolator()->setInterpolationSpeed(s);
    viewer_->update();
}


void DialogWalkThrough::goToPreviousPosition()
{
    int pos = walkThrough()->current_position();
    int new_pos = walkThrough()->move_to(pos - 1);
    viewer_->update();

    if (new_pos == pos)
        LOG(INFO) << "no previous position found (current position is " << new_pos << ")";
    else
        LOG(INFO) << "moved to position " << new_pos;
}


void DialogWalkThrough::goToNextPosition()
{
    int pos = walkThrough()->current_position();
    int new_pos = walkThrough()->move_to(pos + 1);
    viewer_->update();

    if (new_pos == pos)
        LOG(INFO) << "no next position found (current position is " << new_pos << ")";
    else
        LOG(INFO) << "moved to position " << new_pos;
}


void DialogWalkThrough::removeLastPosition() {
    if (walkThrough()->num_positions() == 0) {
        LOG(INFO) << "no position can be removed (path is empty)";
    }
    else {
        int pos = walkThrough()->current_position();
        if (pos == walkThrough()->num_positions() - 1)  // currently viewing at the last position
            pos = walkThrough()->move_to(pos - 1);  // move to the previous position
        walkThrough()->delete_last_position();
        viewer_->update();

        LOG(INFO) << "last position removed (current position is " << pos << ")";
    }
}


void DialogWalkThrough::goToPosition(int p) {
    walkThrough()->move_to(p, false);
    viewer_->update();
}


void DialogWalkThrough::clearPath() {
    if (walkThrough()->num_positions() == 0 && walkThrough()->interpolator()->numberOfKeyFrames() == 0) {
        LOG(INFO) << "nothing to clear (path is empty)";
        return;
    }

    if (QMessageBox::warning(
            viewer_, "Please confirm!",
            "This will delete the previously defined animation path, which cannot be undone.\n"
            "You may export the path to a file (using the \'Camera\' menu) before you delete it."
            "\nContinue to delete?", QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        walkThrough()->delete_path();
        viewer_->update();
	}
}


void DialogWalkThrough::startAnimation() {
    if (walkThrough()->num_positions() == 0 && walkThrough()->interpolator()->numberOfKeyFrames() == 0) {
        LOG(INFO) << "nothing to animate (path is empty)";
        return;
    }
    walkThrough()->animate();
    viewer_->update();
}


void DialogWalkThrough::exportAnimation() {
    if (walkThrough()->num_positions() == 0 && walkThrough()->interpolator()->numberOfKeyFrames() == 0) {
        LOG(INFO) << "nothing can be exported (path is empty)";
        return;
    }
    window_->exportCamaraPathToFile();
}
