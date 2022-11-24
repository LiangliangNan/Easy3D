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

#include "dialog_walk_through.h"
#include "paint_canvas.h"
#include "main_window.h"
#include "walk_through.h"

#include <easy3d/core/model.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>


using namespace easy3d;

DialogWalkThrough::DialogWalkThrough(MainWindow *window)
	: Dialog(window)
{
	setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

	comboBoxInterpolationMethod->addItem("Spline Interpolation");
    comboBoxInterpolationMethod->addItem("Spline Fitting");
    comboBoxInterpolationMethod->setCurrentIndex(0);

    auto group = new QButtonGroup(this);
    group->addButton(radioButtonFreeMode);
    group->addButton(radioButtonWalkingMode);
    group->addButton(radioButtonRotateAroundAxis);
    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(setMode(QAbstractButton *)));
    group->buttonClicked(radioButtonFreeMode);

	spinBoxFPS->setValue(interpolator()->frame_rate());
	doubleSpinBoxInterpolationSpeed->setValue(interpolator()->interpolation_speed());

    connect(comboBoxInterpolationMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(setInterpolationMethod(int)));

    connect(checkBoxFollowUp, SIGNAL(toggled(bool)), this, SLOT(setFollowUp(bool)));
	connect(doubleSpinBoxCharacterHeightFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterHeightFactor(double)));
	connect(doubleSpinBoxCharacterDistanceFactor, SIGNAL(valueChanged(double)), this, SLOT(setCharacterDistanceFactor(double)));

    connect(doubleSpinBoxZoomOutFactor, SIGNAL(valueChanged(double)), this, SLOT(setZoomOutFactor(double)));
    connect(doubleSpinBoxVerticalOffsetFactor, SIGNAL(valueChanged(double)), this, SLOT(setVerticalOffsetFactor(double)));
    connect(doubleSpinBoxPitchAngle, SIGNAL(valueChanged(double)), this, SLOT(setPitchAngle(double)));
    connect(spinBoxNumberKeyframesPerLoop, SIGNAL(valueChanged(int)), this, SLOT(setNumberKeyframesPerLoop(int)));
    connect(spinBoxNumberLoops, SIGNAL(valueChanged(int)), this, SLOT(setNumberLoops(int)));

    connect(spinBoxFPS, SIGNAL(valueChanged(int)), this, SLOT(setFrameRate(int)));
    connect(doubleSpinBoxInterpolationSpeed, SIGNAL(valueChanged(double)), this, SLOT(setInterpolationSpeed(double)));

	connect(previousKeyframeButton, SIGNAL(clicked()), this, SLOT(goToPreviousKeyframe()));
	connect(nextKeyframeButton, SIGNAL(clicked()), this, SLOT(goToNextKeyframe()));
    connect(removeLastKeyframeButton, SIGNAL(clicked()), this, SLOT(removeLastKeyframe()));

    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToKeyframe(int)));

    connect(clearCameraPathButton, SIGNAL(clicked()), this, SLOT(clearPath()));
    connect(previewButton, SIGNAL(toggled(bool)), this, SLOT(preview(bool)));
    connect(recordButton, SIGNAL(clicked()), this, SLOT(record()));

    connect(browseButton, SIGNAL(clicked()), this, SLOT(browse()));

    easy3d::connect(&walkThrough()->path_modified, this, &DialogWalkThrough::numKeyramesChanged);
}


void DialogWalkThrough::numKeyramesChanged() {
    disconnect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToKeyframe(int)));
    int num = interpolator()->number_of_keyframes();
    if (num == 1) // range is [0, 0]
        horizontalSliderPreview->setEnabled(false);
    else {
        horizontalSliderPreview->setEnabled(true);
        horizontalSliderPreview->setRange(0, std::max(0, num - 1));
    }
    int pos = walkThrough()->current_keyframe_index();
    if (pos >= 0)
        horizontalSliderPreview->setValue(pos);
    connect(horizontalSliderPreview, SIGNAL(valueChanged(int)), this, SLOT(goToKeyframe(int)));
}


WalkThrough* DialogWalkThrough::walkThrough() {
    return viewer_->walkThrough();
}


easy3d::KeyFrameInterpolator* DialogWalkThrough::interpolator() {
    return walkThrough()->interpolator();
}


void DialogWalkThrough::showEvent(QShowEvent* e) {
    auto method = interpolator()->interpolation_method();
    if (method == easy3d::KeyFrameInterpolator::INTERPOLATION)
        comboBoxInterpolationMethod->setCurrentIndex(0);
    else
        comboBoxInterpolationMethod->setCurrentIndex(1);

    if (radioButtonWalkingMode->isChecked())
        walkThrough()->set_status(WalkThrough::WALKING_MODE);
    else
        walkThrough()->set_status(WalkThrough::FREE_MODE);

    checkBoxFollowUp->setChecked(walkThrough()->follow_up());
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
//	QDialog::showEvent(e);
}


void DialogWalkThrough::closeEvent(QCloseEvent* e) {
    walkThrough()->set_status(WalkThrough::STOPPED);
    QDialog::closeEvent(e);
	viewer_->update();
}


void DialogWalkThrough::setFollowUp(bool b) {
    walkThrough()->set_follow_up(b);
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


void DialogWalkThrough::setZoomOutFactor(double v) {
    walkThrough()->set_zoom_out_factor(v, true);
    viewer_->update();
}


void DialogWalkThrough::setVerticalOffsetFactor(double v) {
    walkThrough()->set_vertical_offset_factor(v, true);
    viewer_->update();
}


void DialogWalkThrough::setPitchAngle(double v) {
    walkThrough()->set_pitch_angle(v, true);
    viewer_->update();
}


void DialogWalkThrough::setNumberKeyframesPerLoop(int v) {
    walkThrough()->set_keyframe_samples(v, true);
    viewer_->update();
}


void DialogWalkThrough::setNumberLoops(int v) {
    walkThrough()->set_num_loops(v, true);
    viewer_->update();
}


void DialogWalkThrough::setInterpolationSpeed(double s) {
    interpolator()->set_interpolation_speed(s);
    viewer_->update();
}


void DialogWalkThrough::setFrameRate(int fps) {
    interpolator()->set_frame_rate(fps);
    viewer_->update();
}


void DialogWalkThrough::setMode(QAbstractButton *button) {
    if (!walkThrough())
        return;

    if (button == radioButtonFreeMode && walkThrough()->status() != WalkThrough::FREE_MODE) {
        widgetFreeMode->setVisible(true);
        widgetWalkingMode->setVisible(false);
        widgetRotateAroundAxis->setVisible(false);
        walkThrough()->set_status(WalkThrough::FREE_MODE);
        LOG(INFO) << "camera path creation set to 'free' mode";
    }
    else if (button == radioButtonWalkingMode && walkThrough()->status() != WalkThrough::WALKING_MODE) {
        widgetFreeMode->setVisible(false);
        widgetWalkingMode->setVisible(true);
        widgetRotateAroundAxis->setVisible(false);
        walkThrough()->set_status(WalkThrough::WALKING_MODE);
        LOG(INFO) << "camera path creation set to 'walking' mode";
    }
    else if (button == radioButtonRotateAroundAxis && walkThrough()->status() != WalkThrough::ROTATE_AROUND_AXIS) {
        widgetFreeMode->setVisible(false);
        widgetWalkingMode->setVisible(false);
        widgetRotateAroundAxis->setVisible(true);
        walkThrough()->set_status(WalkThrough::ROTATE_AROUND_AXIS);
        LOG(INFO) << "camera path creation set to 'rotate around axis' mode";
    }
}


void DialogWalkThrough::goToPreviousKeyframe()
{
    const int pos = walkThrough()->current_keyframe_index();
    if (pos <= 0)  // if not started (or at the 1st keyframe), move to the start view point
        walkThrough()->move_to(0);
    else
        walkThrough()->move_to(pos - 1);
    viewer_->update();
    LOG(INFO) << "moved to keyframe " << walkThrough()->current_keyframe_index();
}


void DialogWalkThrough::goToNextKeyframe()
{
    int pos = walkThrough()->current_keyframe_index();
    if (pos >= 0 && pos >= interpolator()->number_of_keyframes() - 1)  // if already at the end, move to the last view point
        walkThrough()->move_to(interpolator()->number_of_keyframes() - 1);
    else
        walkThrough()->move_to(pos + 1);
    viewer_->update();
    LOG(INFO) << "moved to keyframe " << walkThrough()->current_keyframe_index();
}


void DialogWalkThrough::removeLastKeyframe() {
    if (interpolator()->number_of_keyframes() == 0)
        LOG(WARNING) << "no keyframe can be removed (empty path)";
    else {
        walkThrough()->delete_last_keyframe();
        int index = walkThrough()->current_keyframe_index();
        viewer_->update();

        if (index == -1)
            LOG(WARNING) << "no keyframe can be removed (empty path)";
        else
            LOG(INFO) << "last keyframe removed (current keyframe: " << index << ")";
    }
}


void DialogWalkThrough::goToKeyframe(int p) {
    walkThrough()->move_to(p, false);
    viewer_->update();
}


void DialogWalkThrough::setInterpolationMethod(int idx) {
    if (idx == 0)
        interpolator()->set_interpolation_method(easy3d::KeyFrameInterpolator::INTERPOLATION);
    else
        interpolator()->set_interpolation_method(easy3d::KeyFrameInterpolator::FITTING);
    viewer_->update();
}


void DialogWalkThrough::clearPath() {
    if (interpolator()->number_of_keyframes() == 0 && interpolator()->number_of_keyframes() == 0) {
        LOG(WARNING) << "nothing to clear (empty path)";
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
#ifdef HAS_FFMPEG
        suggested_name = file_system::replace_extension(viewer_->currentModel()->name(), "mp4");
    const QString fileName = QFileDialog::getSaveFileName(this,
                                                          tr("Choose a file name"), QString::fromStdString(suggested_name),
                                                          tr("Supported formats (*.mp4)")
#else
    suggested_name = file_system::replace_extension(viewer_->currentModel()->name(), "png");
    const QString fileName = QFileDialog::getSaveFileName(this,
                                                          tr("Choose a file name"), QString::fromStdString(suggested_name),
                                                          tr("Supported formats (*.png)")
#endif
    );
    if (!fileName.isEmpty())
        lineEditOutputFile->setText(fileName);
}


void DialogWalkThrough::preview(bool b) {
#if 0
    // this single line works: very efficient (in another thread without overhead), but I want a better UI.
    walkThrough()->preview();
    viewer_->update();

#elif 0 // this also works, but low framerate (overhead from std::this_thread::sleep_for() and ProgressLogger)
    if (b) {
        if (!interpolator() || interpolator()->number_of_keyframes() == 0) {
            LOG_IF(WARNING, interpolator()->number_of_keyframes() == 0)
                            << "nothing to preview (camera path is empty). You may import a camera path from a file or"
                               " creat it by adding keyframes";
            previewButton->setChecked(false);
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
        previewButton->setChecked(false);

#else

    // this also handles the UI, but a bit complicated (because the Qt GUI actions cannot be triggered in another thread)
    // idea: animation signal -> Qt signals -> Qt action
    auto interpolationStopped = [this]() -> void { emit previewStopped(); };
    static int id_interpolationStopped = 0;
    static StopWatch w;
    if (b) {
        if (!interpolator() || interpolator()->number_of_keyframes() == 0) {
            LOG_IF(interpolator()->number_of_keyframes() == 0, WARNING)
                    << "nothing to preview (camera path is empty). "
                       "You may import a camera path from a file or create it by adding keyframes";
            disconnect(previewButton, SIGNAL(toggled(bool)), this, SLOT(preview(bool)));
            previewButton->setChecked(false);
            connect(previewButton, SIGNAL(toggled(bool)), this, SLOT(preview(bool)));
            return;
        }
        previewButton->setText("Stop");

        id_interpolationStopped = easy3d::connect(&interpolator()->interpolation_stopped, interpolationStopped);
        QObject::connect(this, &DialogWalkThrough::previewStopped, this, &DialogWalkThrough::onPreviewStopped);

        for (auto c : findChildren<QComboBox*>()) c->setEnabled(false);
        for (auto c : findChildren<QLabel*>()) c->setEnabled(false);
        for (auto c : findChildren<QPushButton*>()) c->setEnabled(c == previewButton);
        for (auto c : findChildren<QCheckBox*>()) c->setEnabled(false);
        for (auto c : findChildren<QRadioButton*>()) c->setEnabled(false);
        for (auto c : findChildren<QSpinBox*>()) c->setEnabled(false);
        for (auto c : findChildren<QDoubleSpinBox*>()) c->setEnabled(false);
        for (auto c : findChildren<QLineEdit*>()) c->setEnabled(false);
        for (auto c : findChildren<QToolButton*>()) c->setEnabled(false);
        for (auto c : findChildren<QSlider*>()) c->setEnabled(false);

        LOG(INFO) << "preview started...";
        w.start();
        interpolator()->start_interpolation();
    }
    else {
        easy3d::disconnect(&interpolator()->interpolation_stopped, id_interpolationStopped);
        QObject::disconnect(this, &DialogWalkThrough::previewStopped, this, &DialogWalkThrough::onPreviewStopped);

        interpolator()->stop_interpolation();
        LOG(INFO) << "preview finished. " << w.time_string();
        previewButton->setText("Preview");

        for (auto c : findChildren<QComboBox*>()) c->setEnabled(true);
        for (auto c : findChildren<QLabel*>()) c->setEnabled(true);
        for (auto c : findChildren<QPushButton*>()) c->setEnabled(true);
        for (auto c : findChildren<QCheckBox*>()) c->setEnabled(true);
        for (auto c : findChildren<QRadioButton*>()) c->setEnabled(true);
        for (auto c : findChildren<QSpinBox*>()) c->setEnabled(true);
        for (auto c : findChildren<QDoubleSpinBox*>()) c->setEnabled(true);
        for (auto c : findChildren<QLineEdit*>()) c->setEnabled(true);
        for (auto c : findChildren<QToolButton*>()) c->setEnabled(true);
        for (auto c : findChildren<QSlider*>()) c->setEnabled(true);

        viewer_->update();
    }
#endif
}


void DialogWalkThrough::record() {
    if (!interpolator() || interpolator()->number_of_keyframes() == 0) {
        LOG_IF(interpolator()->number_of_keyframes() == 0, WARNING)
                        << "nothing to record (camera path is empty). You may import a camera path from a file or"
                           " creat it by adding keyframes";
        return;
    }

    if (previewButton->isChecked())
        previewButton->setChecked(false);

    // make sure the cameras and path are not visible in recording
    const bool cameras_visible = walkThrough()->cameras_visible();
    if (cameras_visible)
        walkThrough()->set_cameras_visible(false);
    const bool path_visible = walkThrough()->path_visible();
    if (path_visible)
        walkThrough()->set_path_visible(false);

    const QString file = lineEditOutputFile->text();
    const int fps = spinBoxFPS->value();
    const int bitrate = spinBoxBitRate->value();

    hide();
    QApplication::processEvents();
    LOG(INFO) << "recording started...";
    StopWatch w;
    viewer_->recordAnimation(file, fps, bitrate, true);
    LOG(INFO) << "recording finished. " << w.time_string();

    // restore
    if (cameras_visible)
        walkThrough()->set_cameras_visible(true);
    if (path_visible)
        walkThrough()->set_path_visible(true);

    show();
}


void DialogWalkThrough::onPreviewStopped() {
    previewButton->setChecked(false);
}
