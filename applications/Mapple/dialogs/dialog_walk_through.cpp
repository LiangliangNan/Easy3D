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

#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>


using namespace easy3d;

DialogWalkThrough::DialogWalkThrough(MainWindow *window)
	: Dialog(window)
{
	setupUi(this);

	spinBoxFPS->setValue(interpolator()->frame_rate());
	doubleSpinBoxInterpolationSpeed->setValue(interpolator()->interpolation_speed());

	connect(doubleSpinBoxCharacterHeightFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterHeightFactor(double)));
	connect(doubleSpinBoxCharacterDistanceFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterDistanceFactor(double)));

    connect(spinBoxFPS, SIGNAL(valueChanged(int)), this, SLOT(setFrameRate(int)));
    connect(doubleSpinBoxInterpolationSpeed, SIGNAL(valueChanged(double)), this, SLOT(set_interpolation_speed(double)));

    connect(importCameraPathButton, SIGNAL(clicked()), this, SLOT(importCameraPathFromFile()));
    connect(exportCameraPathButton, SIGNAL(clicked()), this, SLOT(exportCameraPathToFile()));
    connect(checkBoxShowCameraPath, SIGNAL(toggled(bool)), this, SLOT(showCameraPath(bool)));

    connect(radioButtonWalkingMode, SIGNAL(toggled(bool)), this, SLOT(setWalkingMode(bool)));

	connect(previousPositionButton, SIGNAL(clicked()), this, SLOT(goToPreviousPosition()));
	connect(nextPositionButton, SIGNAL(clicked()), this, SLOT(goToNextPosition()));
    connect(removeLastPositionButton, SIGNAL(clicked()), this, SLOT(removeLastPosition()));

    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));

    connect(clearCameraPathButton, SIGNAL(clicked()), this, SLOT(clearPath()));
    connect(previewButton, SIGNAL(toggled(bool)), this, SLOT(preview(bool)));
    connect(recordButton, SIGNAL(toggled(bool)), this, SLOT(record(bool)));

    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    easy3d::connect(&walkThrough()->path_modified, this, &DialogWalkThrough::keyFrameAdded);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(radioButtonFreeMode);
    group->addButton(radioButtonWalkingMode);
    radioButtonWalkingMode->setChecked(true);
}


DialogWalkThrough::~DialogWalkThrough()
{
}


void DialogWalkThrough::keyFrameAdded() {
    disconnect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToPosition(int)));
    int num = interpolator()->number_of_keyframes();
    if (num == 1) // range is [0, 0]
        horizontalSliderPreview->setEnabled(false);
    else {
        horizontalSliderPreview->setEnabled(true);
        horizontalSliderPreview->setRange(0, std::max(0, num - 1));
    }
    int pos = walkThrough()->current_keyframe_index();
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
    if (radioButtonWalkingMode->isChecked())
        walkThrough()->set_status(easy3d::WalkThrough::WALKING_MODE);
    else
        walkThrough()->set_status(easy3d::WalkThrough::FREE_MODE);

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
    walkThrough()->set_status(easy3d::WalkThrough::STOPPED);
    QDialog::closeEvent(e);
	viewer_->update();
}


void DialogWalkThrough::setCharacterHeightFactor(double h) {
    walkThrough()->set_height_factor(h);
    DLOG(WARNING) << "TODO: allow to modify the last keyframe (camera position and orientation) here" << std::endl;
    viewer_->update();
}


void DialogWalkThrough::setCharacterDistanceFactor(double d) {
    walkThrough()->set_third_person_forward_factor(d);
    DLOG(WARNING) << "TODO: allow to modify the last keyframe (camera position and orientation) here" << std::endl;
    viewer_->update();
}


void DialogWalkThrough::set_interpolation_speed(double s) {
    interpolator()->set_interpolation_speed(s);
    viewer_->update();
}


void DialogWalkThrough::setFrameRate(int fps) {
    interpolator()->set_frame_rate(fps);
    viewer_->update();
}


void DialogWalkThrough::setWalkingMode(bool b) {
    labelCharacterHeight->setEnabled(b);
    labelCharacterDistanceToEye->setEnabled(b);
    doubleSpinBoxCharacterHeightFactor->setEnabled(b);
    doubleSpinBoxCharacterDistanceFactor->setEnabled(b);

    if (b)
        walkThrough()->set_status(easy3d::WalkThrough::WALKING_MODE);
    else
        walkThrough()->set_status(easy3d::WalkThrough::FREE_MODE);
}


void DialogWalkThrough::goToPreviousPosition()
{
    int pos = walkThrough()->current_keyframe_index();
    if (pos <= 0)  // if not started (or at the 1st keyframe), move to the start view point
        walkThrough()->move_to(0);
    else
        walkThrough()->move_to(pos - 1);
    viewer_->update();
    LOG(INFO) << "moved to position " << walkThrough()->current_keyframe_index();
}


void DialogWalkThrough::goToNextPosition()
{
    int pos = walkThrough()->current_keyframe_index();
    if (pos >= interpolator()->number_of_keyframes() - 1)  // if already at the end, move to the last view point
        walkThrough()->move_to(interpolator()->number_of_keyframes() - 1);
    else
        walkThrough()->move_to(pos + 1);
    viewer_->update();
    LOG(INFO) << "moved to position " << walkThrough()->current_keyframe_index();
}


void DialogWalkThrough::removeLastPosition() {
    if (interpolator()->number_of_keyframes() == 0) {
        LOG(INFO) << "no position can be removed (path is empty)";
    }
    else {
        int pos = walkThrough()->current_keyframe_index();
        if (pos == interpolator()->number_of_keyframes() - 1)  // currently viewing at the last position
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
    if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
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


void DialogWalkThrough::browse() {
    std::string suggested_name;
    if (viewer_->currentModel())
        suggested_name = file_system::replace_extension(viewer_->currentModel()->name(), "mp4");
    const QString fileName = QFileDialog::getSaveFileName(this,
                                                          tr("Choose a file name"), QString::fromStdString(suggested_name),
                                                          tr("Supported formats (*.png *.mp4)")
    );
    if (!fileName.isEmpty())
        lineEditOutputFile->setText(fileName);
}


void DialogWalkThrough::preview(bool b) {
#if 1
    // this single line works: very efficient (in another thread without overhead).
    walkThrough()->preview();
    viewer_->update();

#elif 0 // this also works. But std::this_thread::sleep_for() is not efficient and frame rate is low.
    if (b) {
        if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
            recordButton->setChecked(false);
            return;
        }
        static int last_stopped_index = 0;
        const auto &frames = interpolator()->interpolate();

        setEnabled(false);
        LOG(INFO) << "preview started...";

        ProgressLogger progress(frames.size(), true);
        for (int id = last_stopped_index; id < frames.size(); ++id) {
            if (progress.is_canceled()) {
                last_stopped_index = id;
                LOG(INFO) << "preview cancelled";
                break;
            }
            const auto &f = frames[id];
            viewer_->camera()->frame()->setPositionAndOrientation(f.position(), f.orientation());
            const int interval = interpolator()->interpolation_period() / interpolator()->interpolation_speed();
            std::cout << "interval: " << interval << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (id == frames.size() - 1)  // reaches the end frame
                last_stopped_index = 0;

            progress.next();
        }

        if (last_stopped_index == 0)
            LOG(INFO) << "preview finished";

        setEnabled(true);
        previewButton->setChecked(false);
    }
    else
        recordButton->setChecked(false);

#else

    // this also handles the UI, but too complicated. I don't like it.
//    auto interpolationStopped = [this]() -> void { emit animationStopped(); };
//    auto currentFrameFinished = [this]() -> void { emit oneFrameFinished(); };
//
//    if (b) {
//        if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
//            previewButton->setChecked(false);
//            return;
//        }
//
//        easy3d::connect(&interpolator()->interpolation_stopped, 0, interpolationStopped);
//        easy3d::connect(&interpolator()->current_frame_finished, 0, currentFrameFinished);
//        QObject::connect(this, &DialogWalkThrough::animationStopped, this, &DialogWalkThrough::resetUIAfterAnimationStopped);
//        QObject::connect(this, &DialogWalkThrough::oneFrameFinished, this, &DialogWalkThrough::onOneFrameFinished);
//
//        setEnabled(false);
//
//        LOG(INFO) << "preview started...";
//    }
//    else {
//        easy3d::disconnect(&interpolator()->interpolation_stopped, 0);
//        easy3d::disconnect(&interpolator()->current_frame_finished, 0);
//        QObject::disconnect(this, &DialogWalkThrough::animationStopped, this, &DialogWalkThrough::resetUIAfterAnimationStopped);
//        QObject::disconnect(this, &DialogWalkThrough::oneFrameFinished, this, &DialogWalkThrough::onOneFrameFinished);
//
//        interpolator()->stop_interpolation();
//        LOG(INFO) << "animation finished";
//
//        setEnabled(true);
//        previewButton->setChecked(false);
//    }
#endif
}


void DialogWalkThrough::record(bool b) {
    if (b) {
        if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
            recordButton->setChecked(false);
            return;
        }

        if (previewButton->isChecked())
            previewButton->setChecked(false);

        // make sure the path is not visible in recording
        const bool visible = walkThrough()->is_path_visible();
        if (visible)
            walkThrough()->set_path_visible(false);

        const QString file = lineEditOutputFile->text();
        const int fps = spinBoxFPS->value();
        const int bitrate = spinBoxBitRate->value();

        setEnabled(false);
        viewer_->recordAnimation(file, fps, bitrate, true);
        setEnabled(true);
        recordButton->setChecked(false);

        // restore
        if (visible)
            walkThrough()->set_path_visible(true);
    }
    else
        recordButton->setChecked(false);
}


void DialogWalkThrough::showCameraPath(bool b) {
    walkThrough()->set_path_visible(b);
    if (b) {
        const int count = interpolator()->number_of_keyframes();
        float radius = viewer_->camera()->sceneRadius();
        for (int i=0; i<count; ++i) {
            radius = std::max( radius,
                               distance(viewer_->camera()->sceneCenter(), interpolator()->keyframe(i).position())
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
    if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
        LOG(INFO) << "nothing can be exported (path is empty)";
        return;
    }

    std::string name = "./keyframes.kf";
    if (viewer_->currentModel())
        name = file_system::replace_extension(viewer_->currentModel()->name(), "kf");

    QString suggested_name = QString::fromStdString(name);
    const QString fileName = QFileDialog::getSaveFileName(
            this,
            "Export keyframes to file",
            suggested_name,
            "Keyframe file (*.kf)\n"
            "All formats (*.*)"
    );

    if (!fileName.isEmpty()) {
        if (interpolator()->save_keyframes(fileName.toStdString()))
            LOG(INFO) << "keyframes saved to file";
    }
}


void DialogWalkThrough::importCameraPathFromFile() {
    std::string dir = "./";
    if (viewer_->currentModel())
        dir = file_system::parent_directory(viewer_->currentModel()->name());
    QString suggested_dir = QString::fromStdString(dir);
    const QString fileName = QFileDialog::getOpenFileName(
            this,
            "Import keyframes from file",
            suggested_dir,
            "Keyframe file (*.kf)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    if (interpolator()->read_keyframes(fileName.toStdString())) {
        LOG(INFO) << interpolator()->number_of_keyframes() << " keyframes loaded";
        if (walkThrough()->is_path_visible()) {
            // update scene radius to make sure the path is within the view frustum
            int num = interpolator()->number_of_keyframes();
            float radius = viewer_->camera()->sceneRadius();
            for (int i = 0; i < num; ++i) {
                radius = std::max(
                        radius,
                        distance(viewer_->camera()->sceneCenter(), interpolator()->keyframe(i).position())
                );
            }
            viewer_->camera()->setSceneRadius(radius);
            viewer_->update();
        }
    }

    update();
}
