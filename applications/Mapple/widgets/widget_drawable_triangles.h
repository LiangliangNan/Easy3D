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

    easy3d::TrianglesDrawable* activeDrawable();
    easy3d::Drawable* drawable() override;

public slots:
    void setActiveDrawable(const QString &) override;

    void setPhongShading(bool);
    void setDefaultColor() override;
    void setBackColor() override;
    void setTextureFile() override;

    void setHighlight(bool) override;

    void setTransparency(int);

private:
    Ui::WidgetTrianglesDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
