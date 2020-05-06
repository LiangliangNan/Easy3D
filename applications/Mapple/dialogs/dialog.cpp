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