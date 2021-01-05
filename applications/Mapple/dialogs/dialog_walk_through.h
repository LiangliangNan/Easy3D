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

#ifndef DIALOG_WALK_THROUGH_H
#define DIALOG_WALK_THROUGH_H

#include "dialog.h"
#include "ui_dialog_walk_through.h"


namespace easy3d {
    class WalkThrough;
    class KeyFrameInterpolator;
}


class DialogWalkThrough : public Dialog, public Ui::DialogWalkThroughClass
{
	Q_OBJECT

public:
    explicit DialogWalkThrough(MainWindow *window);
	~DialogWalkThrough();

    easy3d::KeyFrameInterpolator* interpolator();
    easy3d::WalkThrough* walkThrough();

public Q_SLOTS :
    void importCameraPathFromFile();
    void exportCameraPathToFile();
    void showCameraPath(bool);
    void clearPath();

    void setWalkingMode(bool);

    void goToPosition(int);
    void goToPreviousPosition();
    void goToNextPosition();
    void removeLastPosition();

    void browse();
	void preview(bool);
    void record();

	void setCharacterHeightFactor(double);
	void setCharacterDistanceFactor(double);
	void setInterpolationSpeed(double);
	void setFrameRate(int);

    void onPreviewStopped();

signals:
    void previewStopped();

protected:
	virtual	void showEvent(QShowEvent* e);
	virtual void closeEvent(QCloseEvent* e);

    void numKeyramesChanged();
};

#endif // DIALOG_WALK_THROUGH_H
