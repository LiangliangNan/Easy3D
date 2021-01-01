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

#include <easy3d/core/model.h>
#include <easy3d/renderer/walk_throuth.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/progress.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>


using namespace easy3d;

DialogWalkThrough::DialogWalkThrough(MainWindow *window)
	: Dialog(window)
{
	setupUi(this);

	connect(doubleSpinBoxCharacterHeightFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterHeightFactor(double)));
	connect(doubleSpinBoxCharacterDistanceFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterDistanceFactor(double)));
    connect(doubleSpinBoxInterpolationSpeed, SIGNAL(valueChanged(double)), this, SLOT(setInterpolationSpeed(double)));

    connect(importCameraPathButton, SIGNAL(clicked()), this, SLOT(importCameraPathFromFile()));
    connect(exportCameraPathButton, SIGNAL(clicked()), this, SLOT(exportCameraPathToFile()));
    connect(checkBoxShowCameraPath, SIGNAL(toggled(bool)), this, SLOT(showCameraPath(bool)));

    connect(addKeyframeButton, SIGNAL(clicked()), this, SLOT(addKeyFrame()));

    connect(radioButtonWalkingMode, SIGNAL(toggled(bool)), this, SLOT(setWalkingMode(bool)));

	connect(previousPositionButton, SIGNAL(clicked()), this, SLOT(goToPreviousPosition()));
	connect(nextPositionButton, SIGNAL(clicked()), this, SLOT(goToNextPosition()));
    connect(removeLastPositionButton, SIGNAL(clicked()), this, SLOT(removeLastPosition()));

    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));

    connect(clearCameraPathButton, SIGNAL(clicked()), this, SLOT(clearPath()));
    connect(previewButton, SIGNAL(toggled(bool)), this, SLOT(preview(bool)));
    connect(recordButton, SIGNAL(toggled(bool)), this, SLOT(record(bool)));

    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    easy3d::connect(&walkThrough()->path_modified, this, &DialogWalkThrough::newPositionAdded);

    progress_ = new ProgressLogger;

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(radioButtonFreeMode);
    group->addButton(radioButtonWalkingMode);
    radioButtonWalkingMode->setChecked(true);
}


DialogWalkThrough::~DialogWalkThrough()
{
    delete progress_;
}


void DialogWalkThrough::newPositionAdded() {
    disconnect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));
    int num = walkThrough()->num_positions();
    if (num == 1) // range is [0, 0]
        horizontalSliderPreview->setEnabled(false);
    else {
        horizontalSliderPreview->setEnabled(true);
        horizontalSliderPreview->setRange(0, std::max(0, num - 1));
    }
    int pos = walkThrough()->current_position();
    horizontalSliderPreview->setValue(pos);
    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));
}


easy3d::WalkThrough* DialogWalkThrough::walkThrough() {
    return viewer_->walkThrough();
}


easy3d::KeyFrameInterpolator* DialogWalkThrough::interpolator() {
    return walkThrough()->interpolator();
}


void DialogWalkThrough::showEvent(QShowEvent* e) {
	doubleSpinBoxCharacterHeightFactor->setValue(walkThrough()->height_factor());
	doubleSpinBoxCharacterDistanceFactor->setValue(walkThrough()->third_person_forward_factor());

#ifdef HAS_FFMPEG
    std::string name = "./video.mp4";
    if (viewer_->currentModel())
        name = file_system::replace_extension(viewer_->currentModel()->name(), "mp4");
#else
    std::string name = "./video.png";
    if (viewer_->currentModel())
        name = file_system::replace_extension(viewer_->currentModel()->name(), "png");
#endif

    lineEditOutputFile->setText(QString::fromStdString(name));
	QDialog::showEvent(e);
}


void DialogWalkThrough::closeEvent(QCloseEvent* e) {
    QDialog::closeEvent(e);
	viewer_->update();
}


void DialogWalkThrough::addKeyFrame() {
    easy3d::Frame *frame = viewer_->camera()->frame();
    walkThrough()->add_key_frame(*frame);
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
    interpolator()->setInterpolationSpeed(s);
    viewer_->update();
}


void DialogWalkThrough::setWalkingMode(bool b) {
    addKeyframeButton->setEnabled(!b);

    labelCharacterHeight->setEnabled(b);
    labelCharacterDistanceToEye->setEnabled(b);
    doubleSpinBoxCharacterHeightFactor->setEnabled(b);
    doubleSpinBoxCharacterDistanceFactor->setEnabled(b);
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
    if (walkThrough()->num_positions() == 0 && interpolator()->numberOfKeyFrames() == 0) {
        LOG(WARNING) << "nothing to clear (path is empty)";
        return;
    }

    if (QMessageBox::warning(
            viewer_, "Please confirm!",
            "This will delete the previously defined animation path, which cannot be undone.\n"
            "You may export the path to a file before you delete it."
            "\nContinue to delete?", QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        walkThrough()->delete_path();
        viewer_->update();
	}
}


void DialogWalkThrough::enableAllButtons(bool b) {
    labelCharacterHeight->setEnabled(b);
    labelCharacterDistanceToEye->setEnabled(b);
    doubleSpinBoxCharacterHeightFactor->setEnabled(b);
    doubleSpinBoxCharacterDistanceFactor->setEnabled(b);
    previousPositionButton->setEnabled(b);
    nextPositionButton->setEnabled(b);
    removeLastPositionButton->setEnabled(b);
    horizontalSliderPreview->setEnabled(b);
    previewButton->setEnabled(b);
    recordButton->setEnabled(b);
    clearCameraPathButton->setEnabled(b);
    update();
    QApplication::processEvents();
}


void DialogWalkThrough::browse() {
    std::string dir;
    if (viewer_->currentModel())
        dir = file_system::parent_directory(viewer_->currentModel()->name());
    const QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Choose a file name"), QString::fromStdString(dir),
                                                    tr("Supported formats (*.png *.mp4)")
    );
    lineEditOutputFile->setText(fileName);
}


void DialogWalkThrough::preview(bool b) {
    if (walkThrough()->num_positions() == 0 && interpolator()->numberOfKeyFrames() == 0) {
        previewButton->setChecked(false);
        return;
    }

    auto interpolationStopped = [this]() -> void { emit animationStopped(); };

    if (b) {
        enableAllButtons(false);
        previewButton->setEnabled(true);

        easy3d::connect(&interpolator()->interpolation_stopped, 0, interpolationStopped);
        QObject::connect(this, &DialogWalkThrough::animationStopped, this, &DialogWalkThrough::resetUIAfterAnimationStopped);

        walkThrough()->animate();
        LOG(INFO) << "animation started...";
    }
    else {
        enableAllButtons(true);

        easy3d::disconnect(&interpolator()->interpolation_stopped, 0);
        QObject::disconnect(this, &DialogWalkThrough::animationStopped, this, &DialogWalkThrough::resetUIAfterAnimationStopped);

        interpolator()->stopInterpolation();
        LOG(INFO) << "animation finished";
    }

    viewer_->update();
}


void DialogWalkThrough::record(bool b) {
    if (b) {
        if (walkThrough()->num_positions() == 0 && interpolator()->numberOfKeyFrames() == 0) {
            recordButton->setChecked(false);
            return;
        }

        const QString file = lineEditOutputFile->text();
        const int fps = spinBoxFPS->value();
        const int bitrate = spinBoxBitRate->value();

        enableAllButtons(false);
        recordButton->setEnabled(true);
        viewer_->recordAnimation(file, fps, bitrate, true, progress_);
        enableAllButtons(true);
        recordButton->setChecked(false);
    }
    else
        recordButton->setChecked(false);
}


void DialogWalkThrough::resetUIAfterAnimationStopped() {
    previewButton->setChecked(false);
    recordButton->setChecked(false);
    enableAllButtons(true);
}


void DialogWalkThrough::showCameraPath(bool b) {
    walkThrough()->set_path_visible(b);
    if (b) {
        const int count = interpolator()->numberOfKeyFrames();
        float radius = viewer_->camera()->sceneRadius();
        for (int i=0; i<count; ++i) {
            radius = std::max( radius,
                               distance(viewer_->camera()->sceneCenter(), interpolator()->keyFrame(i).position())
            );
        }
        viewer_->camera()->setSceneRadius(radius);
    }
    else {
        Box3 box;
        for (auto m : viewer_->models())
            box.add_box(m->bounding_box());
        viewer_->camera()->setSceneBoundingBox(box.min(), box.max());
    }
    viewer_->update();
}


void DialogWalkThrough::exportCameraPathToFile() {
    if (walkThrough()->num_positions() == 0 && interpolator()->numberOfKeyFrames() == 0) {
        LOG(INFO) << "nothing can be exported (path is empty)";
        return;
    }

    std::string name = "./animation.path";
    if (viewer_->currentModel())
        name = file_system::replace_extension(viewer_->currentModel()->name(), "path");

    QString suggested_name = QString::fromStdString(name);
    const QString fileName = QFileDialog::getSaveFileName(
            this,
            "Export camera path to file",
            suggested_name,
            "Camera state (*.path)\n"
            "All formats (*.*)"
    );

    if (!fileName.isEmpty())
        interpolator()->save_keyframes(fileName.toStdString());
}


void DialogWalkThrough::importCameraPathFromFile() {
    std::string dir = "./";
    if (viewer_->currentModel())
        dir = file_system::parent_directory(viewer_->currentModel()->name());
    QString suggested_dir = QString::fromStdString(dir);
    const QString fileName = QFileDialog::getOpenFileName(
            this,
            "Import camera path from file",
            suggested_dir,
            "Camera path (*.path)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    interpolator()->read_keyframes(fileName.toStdString());
    if (walkThrough()->is_path_visible()) {
        // update scene radius to make sure the path is within the view frustum
        int num = interpolator()->numberOfKeyFrames();
        float radius = viewer_->camera()->sceneRadius();
        for (int i = 0; i < num; ++i) {
            radius = std::max(
                    radius,
                    distance(viewer_->camera()->sceneCenter(), interpolator()->keyFrame(i).position())
            );
        }
        viewer_->camera()->setSceneRadius(radius);
    }
    update();
}
