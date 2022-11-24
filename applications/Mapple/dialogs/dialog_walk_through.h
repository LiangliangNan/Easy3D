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

#ifndef DIALOG_WALK_THROUGH_H
#define DIALOG_WALK_THROUGH_H

#include "dialog.h"
#include "ui_dialog_walk_through.h"


namespace easy3d {
    class KeyFrameInterpolator;
}

class WalkThrough;

class DialogWalkThrough : public Dialog, public Ui::DialogWalkThroughClass
{
	Q_OBJECT

public:
    explicit DialogWalkThrough(MainWindow *window);
	~DialogWalkThrough() override = default;

    easy3d::KeyFrameInterpolator* interpolator();
    WalkThrough* walkThrough();

public Q_SLOTS :

    void setInterpolationMethod(int);
    void clearPath();

    void setMode(QAbstractButton *);

    void goToKeyframe(int);
    void goToPreviousKeyframe();
    void goToNextKeyframe();
    void removeLastKeyframe();

    void browse();
	void preview(bool);
    void record();

    void setFollowUp(bool);
	void setCharacterHeightFactor(double);
	void setCharacterDistanceFactor(double);

    void setZoomOutFactor(double);
    void setVerticalOffsetFactor(double);
    void setPitchAngle(double);
    void setNumberKeyframesPerLoop(int);
    void setNumberLoops(int);

	void setInterpolationSpeed(double);
	void setFrameRate(int);

    void onPreviewStopped();

signals:
    void previewStopped();

protected:
    void showEvent(QShowEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

    void numKeyramesChanged();
};

#endif // DIALOG_WALK_THROUGH_H
