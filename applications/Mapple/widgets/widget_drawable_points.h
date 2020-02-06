#ifndef WIDGET_DRAWABLE_POINTS_H
#define WIDGET_DRAWABLE_POINTS_H

#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetPointsDrawable;
}

namespace easy3d {
    class PointsDrawable;
}


class WidgetPointsDrawable : public WidgetDrawable
{
    Q_OBJECT

public:
    explicit WidgetPointsDrawable(QWidget *parent);
    ~WidgetPointsDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::PointsDrawable* drawable();

public slots:
    void setActiveDrawable(const QString &);
    void setLighting(int);
    void setDefaultColor();
    void setHighlight(bool);
    void setDistinctBackColor(bool) {}
    void setBackColor() {}

private:
    Ui::WidgetPointsDrawable*   ui;
};

#endif // WIDGET_DRAWABLE_POINTS_H
