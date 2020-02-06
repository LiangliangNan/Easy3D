#ifndef WIDGET_DRAWABLE_LINES_H
#define WIDGET_DRAWABLE_LINES_H

#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetLinesDrawable;
}

namespace easy3d {
    class LinesDrawable;
}


class WidgetLinesDrawable : public WidgetDrawable
{
    Q_OBJECT

public:
    explicit WidgetLinesDrawable(QWidget *parent);
    ~WidgetLinesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::LinesDrawable* drawable();

public slots:
    void setActiveDrawable(const QString &);
    void setDistinctBackColor(bool) {}
    void setLighting(int);
    void setBackColor() {}
    void setDefaultColor();
    void setHighlight(bool);

private:
    Ui::WidgetLinesDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_LINES_H
