#ifndef WIDGET_DRAWABLE_TRIANGLES_H
#define WIDGET_DRAWABLE_TRIANGLES_H

#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetTrianglesDrawable;
}

namespace easy3d {
    class TrianglesDrawable;
}


class WidgetTrianglesDrawable : public WidgetDrawable
{
    Q_OBJECT

public:
    explicit WidgetTrianglesDrawable(QWidget *parent);
    ~WidgetTrianglesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::TrianglesDrawable* drawable();

public slots:
    void setDrawableVisible(bool);
    void setActiveDrawable(const QString &);
    void setPhongShading(bool);
    void setLighting(const QString &);
    void setColorScheme(const QString &);
    void setDefaultColor();
    void setDistinctBackColor(bool);
    void setBackColor();
    void setTextureFile();
    void setTextureRepeat(int);
    void setTextureFractionalRepeat(int);
    void setHighlight(bool);
    void setHighlightMin(int);
    void setHighlightMax(int);
    void setOpacity(int);

    void setScalarFieldStyle(const QString &);

private:
    void connectAll();
    void disconnectAll();
    void disableUnavailableOptions();

private:
    Ui::WidgetTrianglesDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
