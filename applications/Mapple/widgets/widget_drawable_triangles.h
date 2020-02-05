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

    easy3d::TrianglesDrawable* currentDrawable();
    easy3d::Drawable* drawable() override;

public slots:
    virtual void setDefaultColor() override;
    virtual void setUseColorProperty(bool) override;
    virtual void setHighlight(bool) override;

    virtual void setLighting(bool) override;
    virtual void setLightingTwoSides(int) override;
    virtual void setBackColor() override;

    virtual void setUseTexture(bool) override;
    virtual void setTextureFile() override;

    void setPhongShading(bool);
    void setTransparency(int);

private:
    Ui::WidgetTrianglesDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
