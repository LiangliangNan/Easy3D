#include "widget_drawable.h"

#include "main_window.h"
#include "paint_canvas.h"

WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent) {
    auto window = dynamic_cast<MainWindow *>(parent);
    viewer_ = window->viewer();
}


WidgetDrawable::~WidgetDrawable() {
}
