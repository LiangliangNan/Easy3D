#ifndef WIDGET_DRAWABLE_POINTS_H
#define WIDGET_DRAWABLE_POINTS_H

#include <unordered_map>
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
    void setDrawableVisible(bool);
    void setActiveDrawable(const QString &);
    void setPointSize(double);
    void setImposterStyle(const QString &);
    void setLighting(const QString &);
    void setColorScheme(const QString &);
    void setDefaultColor();
    void setDistinctBackColor(bool);
    void setBackColor();
    void setHighlight(bool);
    void setHighlightMin(int);
    void setHighlightMax(int);

    void setScalarFieldStyle(const QString &);

private:
    void connectAll();
    void disconnectAll();
    void disableUnavailableOptions();

private:
    Ui::WidgetPointsDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_POINTS_H
