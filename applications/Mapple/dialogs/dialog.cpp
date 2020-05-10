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


#include "dialogs/dialog.h"

#include "main_window.h"

#include <QDockWidget>
#include <QStyle>
#include <QLayout>


Dialog::Dialog(MainWindow *window, QDockWidget *dockWidgetCommand)
        : QDialog(window) {
    window_ = window;
    viewer_ = window_->viewer();
    dockWidgetCommand_ = dockWidgetCommand;
}


Dialog::~Dialog() {

}


void Dialog::bestSize() {
    // Liangliang: all these can be done by setting the dialog's layout constraints to "fixed".
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Query the title bar height of a QDockWidget:
    // Find the title bar's height using the pixelMetric member function of the dock's QStyle element.
    // You'll probably also want to query the margin as well since it adds space around the title bar and the layout
    // will need to be aware of it.
//    int titleBarHeight = dockWidgetCommand_->style()->pixelMetric(QStyle::PM_TitleBarHeight);
//    int titleBarMargin = dockWidgetCommand_->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);
//    const QSize &size = sizeHint();
//    dockWidgetCommand_->setFixedSize(size.width(), size.height() + titleBarHeight + titleBarMargin);
}