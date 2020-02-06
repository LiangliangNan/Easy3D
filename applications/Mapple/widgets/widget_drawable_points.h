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

    easy3d::PointsDrawable* currentDrawable();
    easy3d::Drawable* drawable() override;

public slots:
    void setActiveDrawable(const QString &) override;
    void setDefaultColor() override;
    void setHighlight(bool) override;

private:
    Ui::WidgetPointsDrawable*   ui;
};

#endif // WIDGET_DRAWABLE_POINTS_H
